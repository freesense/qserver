//SocketServer.cpp
//
#include "stdafx.h"
#include "SocketServer.h"
#include <process.h>
#include "KLine_Exe.h"
///////////////////////////////////////////////////////////////////////////
CClientConnect::CClientConnect()
{
	m_hCltSock = INVALID_SOCKET;
	memset(&m_addrClt, 0, sizeof(SOCKADDR_IN));
	m_hThread = NULL;
}

CClientConnect::CClientConnect(SOCKET hCltSock, SOCKADDR_IN* pAddrClt)
{
	m_hCltSock = hCltSock;
	memcpy(&m_addrClt, pAddrClt, sizeof(SOCKADDR_IN));
	m_hThread = NULL;
}

CClientConnect::~CClientConnect()
{

}

void CClientConnect::SetSocketInfo(SOCKET hCltSock, SOCKADDR_IN* pAddrClt)
{
	m_hCltSock = hCltSock;
	memcpy(&m_addrClt, pAddrClt, sizeof(SOCKADDR_IN));
}

void CClientConnect::CloseSocket()
{
	if (m_hCltSock != INVALID_SOCKET)
	{
		closesocket(m_hCltSock);
		m_hCltSock = INVALID_SOCKET;
	}
}

void CClientConnect::StartRecvData()
{
	ASSERT(m_hCltSock != INVALID_SOCKET);
	while(TRUE)
	{
		int nRet = recv(m_hCltSock, m_chRecvBuf, RECV_BUF_SIZE, 0);
		if (nRet == SOCKET_ERROR)
		{
			REPORT(MN, T("recv() failed with error %d\n", WSAGetLastError()), RPT_ERROR);
			CloseSocket();
			break;
		}
		else if(nRet == 0)
		{
			CString strIP = inet_ntoa(m_addrClt.sin_addr);
			REPORT(MN, T("client [%s] connection has been gracefully closed\n"), RPT_INFO);
			CloseSocket();
			break;
		}
		else
		{
			ParseData(m_chRecvBuf, nRet);
		}
	}
}

void CClientConnect::ParseData(const char* pData, int nLen)
{
	ASSERT(FALSE);
}

BOOL CClientConnect::SendData(const char* pData, int nLen)
{
	char* pDataPos = (char*)pData;
	int nSent = 0;
	while(nSent<nLen)
	{
		int nRet = send(m_hCltSock, pDataPos, (nLen-nSent), 0);
		if (nRet == SOCKET_ERROR)
		{
			REPORT(MN, T("send() failed with error %d\n", WSAGetLastError()), RPT_ERROR);
			return FALSE;
		}

		nSent += nRet;
	}
	
	return TRUE;
}

void CClientConnect::LogEvent(const char* pFormat, ...)
{
	char chMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(chMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);
}
////////////////////////////////////////////////////////////////////////////

CSocketServer::CSocketServer()
{
	m_hServSock     = INVALID_SOCKET;
	m_pAcceptThread = NULL;
	m_bSerRun       = FALSE;

	InitializeCriticalSection(&m_csVecCltCon);
}

CSocketServer::~CSocketServer()
{
	DeleteCriticalSection(&m_csVecCltCon);
}

BOOL CSocketServer::StartServer(UINT nPort)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		REPORT(MN, T("WSAStartup() failed!\n"), RPT_ERROR);
		return FALSE;
	}

	m_hServSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_hServSock == INVALID_SOCKET)
	{
		REPORT_RUN(MN, T("socket() failed with error %d\n", WSAGetLastError()), RPT_ERROR, return FALSE);
	}

	SOCKADDR_IN	 addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.S_un.S_addr = ADDR_ANY;

	if (bind(m_hServSock, (sockaddr*)&addr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		REPORT_RUN(MN, T("bind() failed with error %d\n", WSAGetLastError()), RPT_ERROR, return FALSE);
	}

	if (listen(m_hServSock, 5) == SOCKET_ERROR)
	{
		REPORT_RUN(MN, T("listen() failed with error %d\n", WSAGetLastError()), RPT_ERROR, return FALSE);
	}

	m_pAcceptThread = new CWorkThread(_AcceptThread, this);
	if (m_pAcceptThread == NULL)
		return FALSE;
	m_pAcceptThread->Start();

	return TRUE;
}

void CSocketServer::StopServer()
{
	m_bSerRun = FALSE;   //Accept线程退出循环
	if (m_hServSock != INVALID_SOCKET)
	{
		closesocket(m_hServSock);
		m_hServSock = INVALID_SOCKET;
	}

	if (m_pAcceptThread != NULL)
	{
		m_pAcceptThread->Stop();
		delete m_pAcceptThread;
		m_pAcceptThread = NULL;
	}

	CVecCltCon::iterator pos = m_vecCltCon.begin();
	while(pos != m_vecCltCon.end())
	{
		if (!(*pos)->IsIdle())
		{
			(*pos)->CloseSocket();
			WaitForSingleObject((*pos)->m_hThread);
			CloseHandle((*pos)->m_hThread);
		}
		
		delete (*pos);
		pos++;
	}

	WSACleanup();
}

/*void CSocketServer::LogEvent(const char* pFormat, ...)
{
	char chMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(chMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);

	//处理
	theApp.LogEvent(RPT_DEBUG, chMsg);
}*/

UINT CSocketServer::_AcceptThread(LPVOID pParam)
{
	CSocketServer* pSockServ = (CSocketServer*)pParam;

	pSockServ->m_bSerRun = TRUE;

	REPORT(MN, T("服务器已启动，正在监听客户端连接...\n"), RPT_INFO);
	SOCKADDR_IN addrClt = {0};
	int nLen = sizeof(SOCKADDR_IN);
	while(pSockServ->m_bSerRun)
	{
		SOCKET hCltSock = accept(pSockServ->m_hServSock, (sockaddr*)&addrClt, &nLen);
		if (hCltSock == INVALID_SOCKET)
		{
			REPORT(MN, T("accept() failed with error %d\n", WSAGetLastError()), RPT_ERROR);
			break;
		}

		CString strIP = inet_ntoa(addrClt.sin_addr);
		REPORT(MN, T("client IP[%s] connect %d\n", strIP), RPT_ERROR);

		CClientConnect* pCltCon  = NULL;
		EnterCriticalSection(&pSockServ->m_csVecCltCon);
		CVecCltCon::iterator pos = pSockServ->m_vecCltCon.begin();
		while(pos != pSockServ->m_vecCltCon.end())
		{
			if ((*pos)->IsIdle())
			{
				pCltCon = *pos;
				pCltCon->SetSocketInfo(hCltSock, &addrClt);
				break;
			}
			pos++;
		}
		LeaveCriticalSection(&pSockServ->m_csVecCltCon);
		if (pCltCon == NULL)
		{
			//DW("创建新的客户端连接");
			pCltCon = new CCltConEx(hCltSock, &addrClt);
			pSockServ->m_vecCltCon.push_back(pCltCon);
		}

		HANDLE hCltThread = (HANDLE)_beginthreadex(NULL, 0, _ClientThread, (void*)pCltCon, 0, NULL);
		pCltCon->m_hThread = hCltThread;
	}
	return 0xDEADACCE;
}

UINT CSocketServer::_ClientThread(LPVOID pParam)
{
	CClientConnect* pCltCon = (CClientConnect*)pParam;
	pCltCon->StartRecvData();
	return 0xDEAD000C;
}

void CSocketServer::WaitForSingleObject(HANDLE hHandle)
{
#ifdef _MFC_VER
	BOOL bQuit = FALSE;
	while(!bQuit)
	{
		int rc;
		rc = ::MsgWaitForMultipleObjects(
			1,// 需要等待的对象数量
			&hHandle,	// 对象树组
			FALSE,		//等待所有的对象
			INFINITE,  // 等待的时间
			(DWORD)(QS_ALLINPUT | QS_ALLPOSTMESSAGE)// 事件类型　　  
			);

		//等待的事件激发
		if( rc ==  WAIT_OBJECT_0 )
		{			
			bQuit = TRUE;
		}//其他windows消息
		else if( rc == WAIT_OBJECT_0 + 1 )			
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage (&msg);
				DispatchMessage(&msg);
			}			
		} 
	}
#else
	WaitForSingleObject(hHandle, INFINITE);
#endif
}
