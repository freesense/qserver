
#include "stdafx.h"
#include "../public/report.h"
#include "../public/commxbase.h"
#include "../public/synch.h"
#include "async_result.h"
#include <MSWSock.h>
#include <ws2tcpip.h>
#include <assert.h>

#pragma warning(disable:4312)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALLBACK TimerRoutine(void *lpParam, unsigned char TimerOrWaitFired)
{
	ITimerBase::ITimerCallbackBase *pCallback = (ITimerBase::ITimerCallbackBase*)lpParam;
	if (!pCallback)
		return;

	if (!pCallback->m_pCallback && pCallback->m_nTimerID)
	{
		DeleteTimerQueueTimer(g_hTimerQueue, (HANDLE)pCallback->m_nTimerID, NULL);
		::InterlockedExchange((LONG*)&pCallback->m_nTimerID, 0);
	}
	else if (-1 == pCallback->m_pCallback->handle_timeout(pCallback) && pCallback->m_nTimerID)
	{
		DeleteTimerQueueTimer(g_hTimerQueue, (HANDLE)pCallback->m_nTimerID, NULL);
		::InterlockedExchange((LONG*)&pCallback->m_nTimerID, 0);
	}
}

long ITimerBase::RegisterTimer(ITimerCallbackBase *pTimerParam, float tm_first, float tm_interval)
{
	BOOL bRet;
	long lRet;
	unsigned long nTmFirst = (unsigned long)(tm_first * 1000);
	unsigned long nTmInterval = (unsigned long)(tm_interval * 1000);

	if (!pTimerParam)
		bRet = CreateTimerQueueTimer((HANDLE*)(&lRet), g_hTimerQueue, TimerRoutine, this, nTmFirst, nTmInterval, 0);
	else
	{
		pTimerParam->m_pCallback = this;
		bRet = CreateTimerQueueTimer((HANDLE*)(&pTimerParam->m_nTimerID), g_hTimerQueue, TimerRoutine, pTimerParam, nTmFirst, nTmInterval, 0);
		lRet = pTimerParam->m_nTimerID;
	}

	if (!bRet)
		return 0;
	return lRet;
}

long ITimerBase::RegisterTimer(ITimerCallbackBase *pTimerParam, const char *tm_first, float tm_interval)
{
	int itm = atoi(tm_first);
	float iFirst = (float)(itm/10000*3600 + (itm%10000)/100*60 + itm%100);
	time_t now;
	time(&now);
	struct tm *lt = localtime(&now);
	int iNow = lt->tm_hour*3600+lt->tm_min*60+lt->tm_sec;

	float fTmFirst = 0.0f;
	if (iFirst > iNow)
		fTmFirst = iFirst - iNow;
	else
		fTmFirst = 24*3600 - (iNow - iFirst);

	return RegisterTimer(pTimerParam, fTmFirst, tm_interval);
}

void ITimerBase::CancelTimer(long TimerID)
{
	if (TimerID)
		DeleteTimerQueueTimer(g_hTimerQueue, (HANDLE)TimerID, NULL);
}

void ITimerBase::ChangeTimer(long TimerID, float tm_first, float tm_interval)
{
	if (TimerID)
	{
		HANDLE hTimer = (HANDLE)TimerID;
		unsigned long nTmFirst = (unsigned long)(tm_first * 1000);
		unsigned long nTmInterval = (unsigned long)(tm_interval * 1000);
		ChangeTimerQueueTimer(g_hTimerQueue, hTimer, nTmFirst, nTmInterval);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* ICommX::GetHostname(char *lpBuffer, unsigned int nBufSize)
{
	if (0 == gethostname(lpBuffer, nBufSize))
		return lpBuffer;
	return NULL;
}

char* ICommX::GetAddrFromSockaddr(char *lpBuffer, unsigned int nBufSize, void *lp_sockaddr_in)
{
	sockaddr_in *psai = (sockaddr_in*)lp_sockaddr_in;
	int nLen = sprintf(lpBuffer, "%d.%d.%d.%d", psai->sin_addr.S_un.S_un_b.s_b1,
		psai->sin_addr.S_un.S_un_b.s_b2, psai->sin_addr.S_un.S_un_b.s_b3, psai->sin_addr.S_un.S_un_b.s_b4);
	if (psai->sin_port)
		sprintf(&lpBuffer[nLen], ":%d", ntohs(psai->sin_port));
	return lpBuffer;
}

void* ICommX::GetSockaddrFromIp(void *lp_sockaddr_in, char *lpNetaddr)
{
	sockaddr_in *psai = (sockaddr_in*)lp_sockaddr_in;
	memset(psai, 0x00, sizeof(sockaddr_in));
	char *lpColon = strchr(lpNetaddr, ':');
	if (!lpColon)
	{
		psai->sin_addr.s_addr = INADDR_ANY;
		psai->sin_port = htons(atoi(lpNetaddr));
	}
	else
	{
		char szTmp[22];
		memcpy(szTmp, lpNetaddr, lpColon - lpNetaddr);
		szTmp[lpColon - lpNetaddr] = 0x00;
		psai->sin_addr.s_addr = inet_addr(szTmp);
		psai->sin_port = htons(atoi(lpColon+1));
	}
	return psai;
}

char* ICommX::GetAddrFromHostname(char *lpBuffer, unsigned int nBufSize, const char *lpHostname, void **lpp_addr_info)
{
	if (*lpp_addr_info)
	{
		addrinfo *lpAddrInfo = ((addrinfo*)*lpp_addr_info)->ai_next;
		*lpp_addr_info = lpAddrInfo;
		if (!lpAddrInfo)
			return NULL;
		sockaddr_in *psai = (sockaddr_in*)(lpAddrInfo->ai_addr);
		int nLen = sprintf(lpBuffer, "%d.%d.%d.%d", psai->sin_addr.S_un.S_un_b.s_b1,
			psai->sin_addr.S_un.S_un_b.s_b2, psai->sin_addr.S_un.S_un_b.s_b3, psai->sin_addr.S_un.S_un_b.s_b4);
		if (psai->sin_port)
			sprintf(&lpBuffer[nLen], ":%d", ntohs(psai->sin_port));
		return lpBuffer;
	}

	char *lpServname = NULL;
	char *lpTmp = new char[strlen(lpHostname) + 1];
	memcpy(lpTmp, lpHostname, strlen(lpHostname));
	lpTmp[strlen(lpHostname)] = 0;
	char *lpNodename = strtok(lpTmp, ":");

	struct addrinfo aiHints;
	memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_INET;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = IPPROTO_TCP;
	addrinfo *aiList = NULL;

	int iErrCode = getaddrinfo(lpNodename, lpServname, &aiHints, &aiList);
	delete []lpTmp;
	if (iErrCode)
		return 0;

	*lpp_addr_info = aiList;
	sockaddr_in *psai = (sockaddr_in*)(aiList->ai_addr);
	int nLen = sprintf(lpBuffer, "%d.%d.%d.%d", psai->sin_addr.S_un.S_un_b.s_b1,
		psai->sin_addr.S_un.S_un_b.s_b2, psai->sin_addr.S_un.S_un_b.s_b3, psai->sin_addr.S_un.S_un_b.s_b4);
	if (psai->sin_port)
		sprintf(&lpBuffer[nLen], ":%d", ntohs(psai->sin_port));
	return lpBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
ICommX::ICommX()
{
	m_socket = (UINT)INVALID_SOCKET;
	ZeroMemory(&m_szLocalAddress, sizeof(m_szLocalAddress));
	ZeroMemory(&m_szPeerAddress, sizeof(m_szPeerAddress));
	m_pResult = NULL;
	m_lock_result = new LockRW;
	m_nAsyncCount = 0;
	m_bRelease = false;
	m_bDebug = false;
}

ICommX::~ICommX()
{
	if (m_pResult)
	{
		delete m_pResult;
		m_pResult = NULL;
	}
	delete m_lock_result;
	m_lock_result = NULL;
}

IAsyncOverlappedResult* ICommX::get_connect_result()
{
	m_lock_result->WaitRead();
	IAsyncOverlappedResult *pResult = m_pResult;
	while (pResult)
	{
		CAsyncConnectResult* pt = dynamic_cast<CAsyncConnectResult*>(pResult);
		if (pt)
		{
			m_lock_result->EndRead();
			return pt;
		}
		pResult = pResult->m_pNext;
	}
	m_lock_result->EndRead();

	CAsyncConnectResult* pt = new CAsyncConnectResult;
	if (!pt)
		return NULL;

	m_lock_result->WaitWrite();
	pt->m_pNext = m_pResult;
	m_pResult = pt;
	m_lock_result->EndWrite();
	return pt;
}

IAsyncOverlappedResult* ICommX::get_read_result()
{
	m_lock_result->WaitRead();
 	IAsyncOverlappedResult *pResult = m_pResult;
 	while (pResult)
 	{
 		CAsyncReadResult* pt = dynamic_cast<CAsyncReadResult*>(pResult);
 		if (pt)
 		{
 			m_lock_result->EndRead();
 			return pt;
 		}
 		pResult = pResult->m_pNext;
 	}
 	m_lock_result->EndRead();

	CAsyncReadResult* pt;

	if (!m_bDebug)
		pt = new CAsyncReadResult(NULL);
	else
	{
		char szTmp[256];
		sprintf(szTmp, "%s.read", GetPeerAddress());
		char *lpTmp = strchr(szTmp, ':');
		lpTmp[0] = '.';
		pt = new CAsyncReadResult(szTmp);
	}

 	if (!pt)
 		return NULL;

 	m_lock_result->WaitWrite();
 	pt->m_pNext = m_pResult;
 	m_pResult = pt;
 	m_lock_result->EndWrite();
	return pt;
}

IAsyncOverlappedResult* ICommX::get_write_result()
{
	m_lock_result->WaitRead();
	IAsyncOverlappedResult *pResult = m_pResult;
	while (pResult)
	{
		CAsyncWriteResult* pt = dynamic_cast<CAsyncWriteResult*>(pResult);
		if (pt)
		{
			m_lock_result->EndRead();
			return pt;
		}
		pResult = pResult->m_pNext;
	}
	m_lock_result->EndRead();

	CAsyncWriteResult* pt;
	if (!m_bDebug)
		pt = new CAsyncWriteResult(NULL);
	else
	{
		char szTmp[256];
		sprintf(szTmp, "%s.write", GetPeerAddress());
		char *lpTmp = strchr(szTmp, ':');
		lpTmp[0] = '.';
		pt = new CAsyncWriteResult(szTmp);
	}

	if (!pt)
		return NULL;
	
	m_lock_result->WaitWrite();
	pt->m_pNext = m_pResult;
	m_pResult = pt;
 	m_lock_result->EndWrite();
	return pt;
}

ICommX* ICommX::handle_accept(const char *lpLocalAddr, const char *lpPeerAddr)
{
	return NULL;
}

int ICommX::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	return this->Write();
}

int ICommX::handle_read(char *lpBuffer, unsigned int nLength)
{
	return 0;
}

int ICommX::handle_write(char *lpData, unsigned int nLength, bool bSendOK)
{
	return 0;
}

int ICommX::handle_close(int iError)
{
	CAsyncWriteResult *pResult = (CAsyncWriteResult*)get_write_result();
	pResult->OnClose(this);
	return -1;
}

void ICommX::handle_release()
{
}

char* ICommX::GetRecvBuffer(int readID)
{
	return NULL;
}

unsigned int ICommX::GetRecvBufSize(int readID)
{
	return 0;
}

int ICommX::handle_timeout(ITimerCallbackBase *pTimerParam)
{
	return 0;
}

void ICommX::Close()
{
	long nCount = m_nAsyncCount;

	m_lock_result->lock();
	m_bRelease = true;
	::shutdown(m_socket, SD_BOTH);
	::closesocket(m_socket);
	m_socket = INVALID_SOCKET;
	m_lock_result->unlock();

	if (0 == nCount)
	{
		if (-1 == handle_close(WSAGetLastError()))
			handle_release();
	}
}

bool ICommX::checkdump_read_buffer(unsigned short nLength, const char *lpCompare, unsigned short nCmpLen)
{
	CAsyncReadResult *p_read_result = (CAsyncReadResult*)get_read_result();
	if (lpCompare && p_read_result && p_read_result->m_nReceivedSize >= nCmpLen)
	{
		int iCmp = memcmp(&p_read_result->m_lpReadBuffer[p_read_result->m_nReceiveSize], lpCompare, nCmpLen);
		CHECKDUMP_RUN(iCmp != 0, MN, p_read_result->m_lpReadBuffer + p_read_result->m_nReceiveSize,
			p_read_result->m_nReceivedSize > nLength ? nLength : p_read_result->m_nReceivedSize,
			T("dump_read_buffer(%d) from %s, received: %d\n", nLength, GetPeerAddress(), p_read_result->m_nReceivedSize),
			RPT_DEBUG, return false);
	}
	return true;
}

int ICommX::Read(int readID, int iLength, unsigned int nCount, const char *lpszString)
{
	CAsyncReadResult *p_read_result = (CAsyncReadResult*)get_read_result();
	if (p_read_result == NULL)
		return -1;

	if (-1 != iLength)
	{
		if (p_read_result->m_nReceivedSize && p_read_result->m_nReceivedSize >= (unsigned int)iLength)
		{
			char *lpBuffer = GetRecvBuffer(readID);
			memcpy(lpBuffer, p_read_result->m_lpReadBuffer + p_read_result->m_nReceiveSize, iLength);
			p_read_result->m_nReceiveSize += iLength;
			p_read_result->m_nReceivedSize -= iLength;
			int read_result = handle_read(lpBuffer, iLength);
			if (-1 == read_result)
			{
				Close();
				return -2;
			}
			return 0;
		}

		p_read_result->m_lpReadBuffer = GetRecvBuffer(readID);
		memcpy(p_read_result->m_lpReadBuffer, p_read_result->m_wb.buf + p_read_result->m_nReceiveSize, p_read_result->m_nReceivedSize);
		p_read_result->m_wb.buf = p_read_result->m_lpReadBuffer + p_read_result->m_nReceivedSize;
		p_read_result->m_wb.len = GetRecvBufSize(readID) - p_read_result->m_nReceivedSize;
		p_read_result->m_nReceiveSize = iLength;
	}
	else
	{
		p_read_result->m_wb.buf += readID;
		p_read_result->m_wb.len -= readID;
	}

	p_read_result->m_lpReceiveFlag = (char*)lpszString;
	p_read_result->m_nFlagRepeatTimes = nCount;

	DWORD dwRecvBytes = 0, dwFlags = 0;
	int iRecv = ::WSARecv(m_socket, &p_read_result->m_wb, 1, &dwRecvBytes, &dwFlags, p_read_result, NULL);
	if (iRecv == 0)
	{
		::InterlockedIncrement(&m_nAsyncCount);
		return 0;
	}

	iRecv = ::WSAGetLastError();
	if (ERROR_IO_PENDING == iRecv)
	{
		::InterlockedIncrement(&m_nAsyncCount);
		return 0;
	}

	return iRecv;
}

int ICommX::Write(void *lpData, unsigned int nLength)
{
	CAsyncWriteResult *p_write_result = (CAsyncWriteResult*)get_write_result();
	if (p_write_result == NULL)
		return -1;

	if (lpData)
	{
		if (1 != p_write_result->AddPreSend(lpData, nLength))
			return 0;
	}
	else
	{
		nLength = p_write_result->GetSendData(&lpData);
		if (!nLength)		/** 队列中没有待发数据 [6/24/2009 xinl] */
			return 0;
	}

	p_write_result->m_wb.buf = (char*)lpData;
	p_write_result->m_wb.len = nLength;

	DWORD dwSendBytes = 0, dwFlags = 0;
	int iWrite = ::WSASend(m_socket, &p_write_result->m_wb, 1, &dwSendBytes, dwFlags, p_write_result, NULL);

	if (iWrite == 0)
	{
		::InterlockedIncrement(&m_nAsyncCount);
		return 0;
	}

	iWrite = ::WSAGetLastError();
	if (ERROR_IO_PENDING == iWrite)
	{
		::InterlockedIncrement(&m_nAsyncCount);
		return 0;
	}

	return iWrite;
}

int ICommX::accept(IAsyncOverlappedResult *pResult)
{
	CAsyncAcceptResult *pa_result = (CAsyncAcceptResult*)pResult;
	pa_result->m_socket = (UINT)::WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == pa_result->m_socket)
	{
		delete pResult;
		return -1;
	}

	DWORD dwBytesReceived = 0;
	if (lpfnAcceptEx(m_socket, pa_result->m_socket, pa_result->m_lpBuffer, pa_result->m_nBufferSize - 2 * (sizeof(sockaddr_in) + 16),
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (LPDWORD)&dwBytesReceived, (LPOVERLAPPED)pResult))
	{
		::InterlockedIncrement(&m_nAsyncCount);
		return 0;	// Immediate success: the OVERLAPPED will still get queued.
	}

	int de = ::WSAGetLastError();
	if (ERROR_IO_PENDING == de)
	{
		::InterlockedIncrement(&m_nAsyncCount);
		return 0;	// The IO will complete proactively: the OVERLAPPED will still get queued.
	}

	return -1;
}

int ICommX::accept(unsigned short nAcceptCount, unsigned int nInitialRead)
{
	DWORD dwBytesReceived = 0;
	for (int i = 0; i < nAcceptCount; i++)
	{
		CAsyncAcceptResult *pResult = new CAsyncAcceptResult(nInitialRead);
		if (!pResult)
			return -1;
		m_lock_result->lock();
		pResult->m_pNext = m_pResult;
		m_pResult = pResult;
		m_lock_result->unlock();

		pResult->m_socket = (UINT)::WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == pResult->m_socket)
		{
			delete pResult;
			m_pResult = NULL;
			continue;
		}

		if (lpfnAcceptEx(m_socket, pResult->m_socket, pResult->m_lpBuffer, nInitialRead,
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			(LPDWORD)&dwBytesReceived, (LPOVERLAPPED)pResult))
		{
			::InterlockedIncrement(&m_nAsyncCount);
			continue;	// Immediate success: the OVERLAPPED will still get queued.
		}

		int de = ::WSAGetLastError();
		if (ERROR_IO_PENDING == de)
		{
			::InterlockedIncrement(&m_nAsyncCount);
			continue;	// The IO will complete proactively: the OVERLAPPED will still get queued.
		}
	}

	return 0;
}

int ICommX::Listen(const char *lpszAddress, unsigned short nAcceptCount, unsigned int nInitialRead)
{
	assert(m_socket == INVALID_SOCKET);
	m_socket = (UINT)::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_socket)
		return -1;
	strcpy(m_szLocalAddress, (char*)lpszAddress);

	if (!lpfnAcceptEx)
	{
		GUID guidAcceptEx = WSAID_ACCEPTEX;
		DWORD dwBytes = 0;
		if (::WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidAcceptEx, sizeof(guidAcceptEx), &lpfnAcceptEx, sizeof(lpfnAcceptEx),
			&dwBytes, NULL, NULL))
			return -1;
	}

	HANDLE h = CreateIoCompletionPort((HANDLE)(SOCKET)m_socket, g_hIocp, (ULONG_PTR)this, 0);

	SOCKADDR_IN addr;
	ICommX::GetSockaddrFromIp(&addr, (char*)lpszAddress);
	addr.sin_family = AF_INET;

	if (SOCKET_ERROR == ::bind((SOCKET)m_socket, (sockaddr*)(&addr), sizeof(SOCKADDR_IN)))
	{
		closesocket(m_socket);
		return -1;
	}

	if (SOCKET_ERROR == ::listen(m_socket, SOMAXCONN))
	{
		closesocket(m_socket);
		return -1;
	}

	return accept(nAcceptCount, nInitialRead);
}

int ICommX::Connect(const char *lpszAddress, const char *lpSendBuffer, unsigned int nSendLen)
{
//	assert(m_socket == INVALID_SOCKET);
	bool bNeedBind = false;
	if (m_socket != INVALID_SOCKET)
		closesocket(m_socket);

	bNeedBind = true;
	m_socket = (UINT)::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_socket)
		return -1;

	strcpy(m_szPeerAddress, (char*)lpszAddress);

	if (!lpfnConnectEx)
	{
		GUID guidConnectEx = WSAID_CONNECTEX;
		DWORD dwBytes = 0;
		if (::WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&guidConnectEx, sizeof(guidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx),
			&dwBytes, NULL, NULL))
			return -1;
	}

	HANDLE h = CreateIoCompletionPort((HANDLE)(SOCKET)m_socket, g_hIocp, (ULONG_PTR)this, 0);

	SOCKADDR_IN addr, addr_local;
	ICommX::GetSockaddrFromIp(&addr, (char*)lpszAddress);
	addr.sin_family = AF_INET;
	ICommX::GetSockaddrFromIp(&addr_local, "0");
	addr_local.sin_family = AF_INET;

	if (bNeedBind && SOCKET_ERROR == ::bind((SOCKET)m_socket, (sockaddr*)(&addr_local), sizeof(SOCKADDR_IN)) &&
		-1 == handle_close(WSAGetLastError()))
	{
		closesocket(m_socket);
		return -1;
	}

	DWORD dwRecvSize = 0;
	CAsyncConnectResult *p_connect_result = (CAsyncConnectResult*)get_connect_result();
	if (p_connect_result == NULL)
	{
		closesocket(m_socket);
		return -1;
	}
	p_connect_result->m_socket = m_socket;

	if (lpfnConnectEx((SOCKET)m_socket, (sockaddr*)(&addr), sizeof(SOCKADDR_IN),
		(LPVOID)lpSendBuffer, nSendLen,
		&dwRecvSize, (LPOVERLAPPED)p_connect_result))
		::InterlockedIncrement(&m_nAsyncCount);

	int de = ::WSAGetLastError();
	if (ERROR_IO_PENDING == de)
	{
		::InterlockedIncrement(&m_nAsyncCount);
		return 0;
	}
	return -1;
}

long ICommX::handle_result(IAsyncOverlappedResult *pResult)
{
	m_lock_result->lock();
	IAsyncOverlappedResult *p_aol_result = m_pResult;
	IAsyncOverlappedResult *p_aol_prev = NULL;
	while (p_aol_result)
	{
		if (p_aol_result == pResult)
		{
			if (p_aol_prev == NULL)
				m_pResult = p_aol_result->m_pNext;
			else
				p_aol_prev->m_pNext = p_aol_result->m_pNext;
			pResult->m_pNext = NULL;
			delete pResult;
			pResult = NULL;
			break;
		}

		p_aol_prev = p_aol_result;
		p_aol_result = p_aol_result->m_pNext;
	}
	m_bRelease = true;
	m_lock_result->unlock();
	return ::InterlockedDecrement(&m_nAsyncCount);
}

UINT ICommX::GetSocket()
{
	return m_socket;
}

const char* ICommX::GetPeerAddress()
{
	return m_szPeerAddress;
}

const char* ICommX::GetLocalAddress()
{
	return m_szLocalAddress;
}

long ICommX::GetAsyncCount()
{
	return m_nAsyncCount;
}
