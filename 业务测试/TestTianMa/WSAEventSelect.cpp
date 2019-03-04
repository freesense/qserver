//WSAEventSelect.cpp
#include "stdafx.h"
#include "WSAEventSelect.h"
#include <process.h>

/////////////////////////////////////////////////////////////////////////////
CWSAEventSelect::CWSAEventSelect()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_hSock = INVALID_SOCKET;
	memset(&m_addr, 0, sizeof(m_addr));
	memset(&m_NetworkEvents, 0, sizeof(m_NetworkEvents));
	memset(m_EventArray, 0, sizeof(m_EventArray));

	m_hThread    = NULL;
	m_nTimeOut   = 60;
	m_nSockCount = 0;
	m_dwPreTick  = ::GetTickCount();
	::InitializeCriticalSection(&m_csLock);
}

CWSAEventSelect::~CWSAEventSelect()
{
	WSACleanup();
	StopThread();

	::LeaveCriticalSection(&m_csLock);
}

BOOL CWSAEventSelect::StartThread()
{
	StopThread();

	if (m_hThread != NULL)
		return FALSE;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProc, this, 0, NULL);
	if (m_hThread == NULL)
		return FALSE;

	return TRUE;
}

void CWSAEventSelect::StopThread()
{
	m_bRun = FALSE;
	if (m_hThread != NULL)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

BOOL CWSAEventSelect::Connect(string strIP, UINT nPort)
{
	::EnterCriticalSection(&m_csLock);
	m_strIP = strIP;
	m_nPort = nPort;

	if (m_hSock != INVALID_SOCKET)
		Disconnect();

	m_hSock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_hSock == INVALID_SOCKET)
	{
		::LeaveCriticalSection(&m_csLock);
		return FALSE;
	}

	m_nSockCount = 1;
	m_EventArray[0] = WSACreateEvent();
	if (m_EventArray[0] == WSA_INVALID_EVENT)
	{
		REPORT(MN, T("WSACreateEvent() failed with error code:%d\n", WSAGetLastError()), RPT_ERROR);
		::LeaveCriticalSection(&m_csLock);
		return FALSE;
	}

	if (SOCKET_ERROR == WSAEventSelect(m_hSock, m_EventArray[0], 
		FD_READ | FD_CLOSE | FD_CONNECT | FD_WRITE))
	{
		REPORT(MN, T("WSAEventSelect() failed with error code:%d\n", WSAGetLastError()), RPT_ERROR);
		::LeaveCriticalSection(&m_csLock);
		return FALSE;
	}

	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(nPort);
	m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	
	if (SOCKET_ERROR == connect(m_hSock, (sockaddr*)&m_addr, sizeof(sockaddr_in)))
	{
		DWORD dwRet = WSAGetLastError();
		if (dwRet != WSAEWOULDBLOCK)
		{
			REPORT(MN, T("connect() failed with error code:%d\n", dwRet), RPT_ERROR);
			::LeaveCriticalSection(&m_csLock);
			return FALSE;
		}
	}

	::LeaveCriticalSection(&m_csLock);
	return TRUE;
}

void CWSAEventSelect::Disconnect()
{
	::EnterCriticalSection(&m_csLock);
	m_nSockCount = 0;
	if (m_hSock != INVALID_SOCKET)
	{
		shutdown(m_hSock, SD_SEND);

		int nRet = 0;
		char szRecvBuf[1024];

		do
		{
			nRet = recv(m_hSock, szRecvBuf, sizeof(szRecvBuf), 0);
		} while (nRet != 0 && nRet != SOCKET_ERROR);

		shutdown(m_hSock, SD_RECEIVE);
		closesocket(m_hSock);
		m_hSock = INVALID_SOCKET;
	}

	if (m_EventArray[0] != NULL)
	{
		WSACloseEvent(m_EventArray[0]);
		m_EventArray[0] = NULL;
	}
	::LeaveCriticalSection(&m_csLock);
}

void CWSAEventSelect::Reconnect()
{
	Disconnect();
	if (!m_bRun)
		return;

	Sleep(1000);
	Connect(m_strIP, m_nPort);
}

UINT CWSAEventSelect::_ThreadProc(LPVOID lpParam)
{
	CWSAEventSelect* pObject = (CWSAEventSelect*)lpParam;
	pObject->ThreadProc();

	return 0xDEAD;
}

void CWSAEventSelect::ThreadProc()
{
	m_bRun = TRUE;
	while(m_bRun)
	{
		::EnterCriticalSection(&m_csLock);
		if (m_nSockCount <= 0)
		{
			Sleep(1000);
			::LeaveCriticalSection(&m_csLock);
			continue;
		}

		DWORD dwRet = WSAWaitForMultipleEvents(m_nSockCount, m_EventArray, 
			FALSE, 1000, FALSE);

		if (WSA_WAIT_FAILED == dwRet)
		{
			REPORT(MN, T("WSAWaitForMultipleEvents() failed with error code:%d\n", GetLastError()), RPT_ERROR);
			::LeaveCriticalSection(&m_csLock);
			return;
		}
		
		int nEventIndex = dwRet - WSA_WAIT_EVENT_0;
		if (m_hSock == INVALID_SOCKET)
		{
			::LeaveCriticalSection(&m_csLock);
			continue;
		}
		if (SOCKET_ERROR == WSAEnumNetworkEvents(m_hSock, m_EventArray[nEventIndex], &m_NetworkEvents))
		{
			REPORT(MN, T("WSAEnumNetworkEvents() failed with error code:%d\n", GetLastError()), RPT_ERROR);
			::LeaveCriticalSection(&m_csLock);
			return;
		}

		::LeaveCriticalSection(&m_csLock);
		if (m_NetworkEvents.lNetworkEvents & FD_CONNECT)
		{
			if (m_NetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0)
			{
				REPORT(MN, T("FD_CONNECT failed with error code:%d\n", m_NetworkEvents.iErrorCode[FD_CONNECT_BIT]), RPT_ERROR);
				//重连
				Reconnect();
			}
			else
				OnConnect();
		}
		else if (m_NetworkEvents.lNetworkEvents & FD_READ)
		{
			if (m_NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
			{
				REPORT(MN, T("FD_READ failed with error code:%d\n", m_NetworkEvents.iErrorCode[FD_READ_BIT]), RPT_ERROR);
				//重连
				Reconnect();
			}
			else
				OnRead();
		}
		else if (m_NetworkEvents.lNetworkEvents & FD_WRITE)
		{
			if (m_NetworkEvents.iErrorCode[FD_WRITE_BIT] != 0)
			{
				REPORT(MN, T("FD_WRITE failed with error code:%d\n", m_NetworkEvents.iErrorCode[FD_WRITE_BIT]), RPT_ERROR);
				//重连
				Reconnect();
			}
			else
				OnWrite();
		}
		else if (m_NetworkEvents.lNetworkEvents & FD_CLOSE)
		{
			m_nSockCount = 0;
			if (m_NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
			{
				REPORT(MN, T("FD_CLOSE failed with error code:%d\n", m_NetworkEvents.iErrorCode[FD_CLOSE_BIT]), RPT_ERROR);
				//Disconnect();
				Reconnect();
			}
			else
				OnDisconnect();
		}

		DWORD dwCurTick = ::GetTickCount();
		if (dwCurTick - m_dwPreTick >= m_nTimeOut)
		{
			m_dwPreTick = dwCurTick;
			OnTimer();
		}
	}
}

void CWSAEventSelect::OnConnect()
{
}

void CWSAEventSelect::OnDisconnect()
{
}

void CWSAEventSelect::OnWrite()
{
}

void CWSAEventSelect::OnRead()
{
}

void CWSAEventSelect::OnTimer()
{
}

int CWSAEventSelect::Read(const char* pBuf, int nReadLen)
{
	DWORD dwFlags = 0;
	DWORD dwRead  = 0;
	WSABUF wsaBuf;
	wsaBuf.buf = (char*)pBuf;
	wsaBuf.len = nReadLen;

	if (SOCKET_ERROR == WSARecv(m_hSock, &wsaBuf, 1, &dwRead,
		&dwFlags, NULL, NULL))
	{
		DWORD dwRet = WSAGetLastError();
		if (dwRet != WSAEWOULDBLOCK)
		{
			REPORT(MN, T("WSARecv() failed with error code:%d\n", dwRet), RPT_ERROR);
			return -1;
		}
	}

	return dwRead;
}

BOOL CWSAEventSelect::Write(const char* pBuf, int nLen)
{
	WSABUF wsaBuf;
	memcpy(m_szWriteBuf, pBuf, nLen);
	wsaBuf.buf = m_szWriteBuf;
	wsaBuf.len = nLen;

	DWORD dwSend = 0;
	if (SOCKET_ERROR == WSASend(m_hSock, &wsaBuf, 1, &dwSend, 0, NULL, NULL))
	{
		DWORD dwRet = WSAGetLastError();
		if (dwRet != WSAEWOULDBLOCK)
		{
			REPORT(MN, T("WSAGetLastError() failed with error code:%d\n", dwRet), RPT_ERROR);
			return FALSE;
		}
	}

	OnWrite();
	return TRUE;
}