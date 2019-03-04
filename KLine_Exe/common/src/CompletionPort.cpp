//CompletionPort.cpp
#include "stdafx.h"
#include "CompletionPort.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////////////
CCompletionPort::CCompletionPort()
{
	m_nPort          = 0;
	m_hIOComPort     = NULL;
	m_hListenSocket  = NULL;
	m_pAcceptThread  = NULL;
	m_hEventListen   = NULL;
	m_bRun           = FALSE;
	m_nAcceptTimeOut = 10;
	m_nRecvTimeout   = 60;
	m_nThreadCount   = 1;

	m_pAcceptEx             = NULL;
	m_pGetAcceptExSockaddrs = NULL;
	m_nAcceptRecvSize = -1;
}

CCompletionPort::~CCompletionPort()
{
}

BOOL CCompletionPort::StartServer(UINT nPort, int nAcceptSize, LPCSTR pLocalIP)
{
	m_nAcceptRecvSize = nAcceptSize;
	m_nPort      = nPort;
	if (pLocalIP != NULL)
		m_strLocalIP = pLocalIP;

	WSADATA  wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return FALSE;

	if (!InitServer())
		return FALSE;

	if (m_pAcceptThread != NULL)
		return FALSE;
	m_pAcceptThread = new CWorkThread(_AcceptThread, this);
	if (!m_pAcceptThread->Start())
		return FALSE;										 

	REPORT_RUN(MN, T("服务器已启动，监听端口[%d]\n", nPort), RPT_INFO, return TRUE);
}

void CCompletionPort::StopServer()
{
	m_bRun  = FALSE;
	if (m_hIOComPort != NULL)
	{
		for (int i=0; i<(int)m_nThreadCount; i++)
			PostQueuedCompletionStatus(m_hIOComPort, 0, 0, NULL);
	}

	if (m_pAcceptThread != NULL)
	{
		m_pAcceptThread->Stop();
		delete m_pAcceptThread;
		m_pAcceptThread = NULL;
	}

	//释放CWorkThread*指针
	while(m_dequeThread.Size())
	{
		CWorkThread* pThread = m_dequeThread.PopFront();
		if (pThread != NULL)
		{
			pThread->Stop();
			delete pThread;
			pThread = NULL;
		}
	}

	if (m_hListenSocket != NULL)
	{
		closesocket(m_hListenSocket);
		m_hListenSocket = NULL;
	}

	//释放监听事件句柄
	if(m_hEventListen != NULL)
	{
		CloseHandle(m_hEventListen);
		m_hEventListen = NULL;
	}

	//释放资源
	while(m_dequeIo.Size())
	{
		LPIO_CONTEXT pData = m_dequeIo.PopFront();
		delete pData;
	}

	while(m_dequeHandle.Size())
	{
		LPHANDLE_CONTEXT pData = m_dequeHandle.PopFront();
		delete pData;
	}

	{
		map<LPIO_CONTEXT, LPIO_CONTEXT>* pMap = m_mapIo.GetMap();
		map<LPIO_CONTEXT, LPIO_CONTEXT>::iterator pos = pMap->begin();

		while(pos != pMap->end())
		{
			closesocket(pos->second->hCltSock);
			delete pos->second;
			pos++;
		}
		pMap->clear();
	}

	{
		map<LPHANDLE_CONTEXT, LPHANDLE_CONTEXT>* pMap = m_mapHandle.GetMap();
		map<LPHANDLE_CONTEXT, LPHANDLE_CONTEXT>::iterator pos = pMap->begin();

		while(pos != pMap->end())
		{
			closesocket(pos->second->hSocket);
			delete pos->second;
			pos++;
		}
		pMap->clear();
	}

	if (m_hIOComPort != NULL)
	{
		CloseHandle(m_hIOComPort);
		m_hIOComPort = NULL;
	}
	WSACleanup();
	REPORT(MN, T("服务器已关闭\n"), RPT_INFO);
}

BOOL CCompletionPort::InitServer()
{
	m_bRun = TRUE;
	m_hIOComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIOComPort == NULL)
		return FALSE;

	m_hListenSocket = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 
		0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_hListenSocket)
	{
		REPORT_RUN(MN, T("WSASocket() failed with error:%d\n", WSAGetLastError()), RPT_ERROR, return FALSE;);
	}

	if (CreateIoCompletionPort((void*)m_hListenSocket, m_hIOComPort, 0, 0) == NULL)
	{
		REPORT_RUN(MN, T("CreateIoCompletionPort() failed with error:%d\n", GetLastError()), RPT_ERROR, return FALSE;);
	}

	SOCKADDR_IN  addrIn = {0};
	addrIn.sin_family = AF_INET;
	addrIn.sin_port   = htons(m_nPort);
	if (m_strLocalIP.empty())
		addrIn.sin_addr.S_un.S_addr = INADDR_ANY;
	else
		addrIn.sin_addr.S_un.S_addr = inet_addr(m_strLocalIP.c_str());

	if (SOCKET_ERROR == bind(m_hListenSocket, (sockaddr*)&addrIn, sizeof(sockaddr)))
	{
		REPORT_RUN(MN, T("bind() failed with error:%d\n", WSAGetLastError()), RPT_ERROR, return FALSE;);
	}

	if (SOCKET_ERROR == listen(m_hListenSocket, 10))
	{
		REPORT_RUN(MN, T("listen() failed with error:%d\n", WSAGetLastError()), RPT_ERROR, return FALSE;);
	}

	DWORD dwTemp = 0;
	if(SOCKET_ERROR == WSAIoctl(m_hListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&g_GUIDAcceptEx, sizeof(g_GUIDAcceptEx), &m_pAcceptEx, sizeof(m_pAcceptEx),
		&dwTemp, NULL, NULL))
	{
		REPORT_RUN(MN, T("WSAIoctl() failed with error:%d\n", WSAGetLastError()), RPT_ERROR, {m_pAcceptEx = NULL; return FALSE;});
	}

	if(SOCKET_ERROR == WSAIoctl(m_hListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&g_GUIDGetAcceptExSockaddrs, sizeof(g_GUIDGetAcceptExSockaddrs), &m_pGetAcceptExSockaddrs,
		sizeof(m_pGetAcceptExSockaddrs), &dwTemp, NULL, NULL))
	{
		REPORT_RUN(MN, T("WSAIoctl() failed with error:%d\n", WSAGetLastError()), RPT_ERROR, {m_pGetAcceptExSockaddrs = NULL; return FALSE;});
	}

	SYSTEM_INFO sysInfo = {0};
	GetSystemInfo(&sysInfo);
	m_nThreadCount = sysInfo.dwNumberOfProcessors * 2;
	if (MAX_THREAD_COUNT < m_nThreadCount)
		m_nThreadCount = MAX_THREAD_COUNT;

	for(unsigned int i=0; i<m_nThreadCount; i++)
	{
		CWorkThread* pWorkThread = new CWorkThread(_WorkThread, this);
		if (pWorkThread != NULL)
		{
			m_dequeThread.PushBack(pWorkThread);
			if (!pWorkThread->Start())
			{
				REPORT_RUN(MN, T("pWorkThread->Start() failed\n"), RPT_ERROR, return FALSE;);
			}
		}
		else
		{
			REPORT_RUN(MN, T("new CWorkThread() failed\n"), RPT_ERROR, return FALSE;);
		}
	}

	m_hEventListen = CreateEvent(NULL,FALSE,FALSE,NULL);
	if(NULL == m_hEventListen)
		return FALSE;
	if(SOCKET_ERROR == ::WSAEventSelect(m_hListenSocket, m_hEventListen, FD_ACCEPT))
		return FALSE;

	return TRUE;
}

UINT _stdcall CCompletionPort::_WorkThread(void* pParam)
{
	CCompletionPort* pComPort = (CCompletionPort*)pParam;
	return pComPort->WorkThread();
}

UINT CCompletionPort::WorkThread()
{
	LPIO_CONTEXT      pIoContext;
	LPHANDLE_CONTEXT  pHandleContext;

	DWORD dwTransCount = 0;
	DWORD dwRet        = 0;
	DWORD dwRecv       = 0;
	DWORD dwFlags      = 0;

	while(TRUE)
	{
		BOOL bRet = GetQueuedCompletionStatus(m_hIOComPort, &dwTransCount,
			(LPDWORD)(&pHandleContext), (LPOVERLAPPED*)(&pIoContext), INFINITE);
		if (!bRet)
		{	
			DWORD dwErrCode = GetLastError();
			if (ERROR_NETNAME_DELETED == dwErrCode)
			{//客户端强制退出
				OnDisconnect(pHandleContext);
				continue;
			}

			REPORT(MN, T("GetQueuedCompletionStatus failed with error %d\n", dwErrCode), RPT_WARNING);
			continue;
		}

		if (pHandleContext == NULL && pIoContext == NULL && dwTransCount == 0)
		{
			//关闭线程				   
			REPORT(MN, T("work 线程[0x%08x]结束\n", GetCurrentThreadId()), RPT_INFO);
			return 0xDEADDEAD;
		}
		else if(dwTransCount == 0 && pIoContext != NULL && pHandleContext != NULL)
		{
			//客户端关闭, 释放资源
			DisConnected(pHandleContext, pIoContext);	   
			continue;
		}

		//收发数据
		switch(pIoContext->byIoType)
		{
		case IO_ACCEPT:
			{
				//将侦听SOCKET的属性复制给客户端SOCKET， 因为accept 创建的 socket 
				//会自动继承侦听 socket 的属性，而AcceptEx不会
				if(SOCKET_ERROR == setsockopt(pIoContext->hCltSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
					(char*)&m_hListenSocket, sizeof(m_hListenSocket)))
				{
					REPORT(MN, T("setsockopt() failed with error %d\n", WSAGetLastError()), RPT_ERROR);

					closesocket(pIoContext->hCltSock);
					ReleaseIoContext(pIoContext);
					continue;
				}
				else
				{
					//分配单句柄数据
					LPHANDLE_CONTEXT pNewHandleContext = GetHandleContext();
					m_mapHandle.Insert(pNewHandleContext);

					pNewHandleContext->hSocket = pIoContext->hCltSock;
					if(!CreateIoCompletionPort((HANDLE)pIoContext->hCltSock, m_hIOComPort,
						(ULONG_PTR)pNewHandleContext, 0))
					{
						::closesocket(pIoContext->hCltSock);
						ReleaseHandleContext(pNewHandleContext);
						ReleaseIoContext(pIoContext);
						continue;
					}
					OnConnect(pIoContext->hCltSock);
					OnAccept(pNewHandleContext, pIoContext, dwTransCount);	
				}

				break;
			}
		case IO_SEND:
			{
				if (dwTransCount < 0)
					DisConnected(pHandleContext, pIoContext);

				pIoContext->tmPreCheck = time(NULL);
				OnSend(pIoContext);
				ReleaseIoContext(pIoContext);
				break;
			}
		case IO_RECV:
			{
				//如果是无效的数据则关闭连接
				pIoContext->tmPreCheck = time(NULL);
				if (!OnRecv(pHandleContext, pIoContext, dwTransCount))
				{
					REPORT(MN, T("收到无效的数据，关闭连接\n", pIoContext->byIoType), RPT_WARNING);
					DisConnected(pHandleContext, pIoContext);
					continue;
				}
				break;
			}
		default:
			{
				REPORT(MN, T("can't process byIoType=%d\n", pIoContext->byIoType), RPT_INFO);
				DisConnected(pHandleContext, pIoContext);
			}
		}
	}

	return 0;
}


void CCompletionPort::DisConnected(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext)
{
	if (pIoContext != NULL)
		closesocket(pIoContext->hCltSock);

	ReleaseHandleContext(pHandleContext);
	ReleaseIoContext(pIoContext);

	OnDisconnect(pHandleContext);
}

BOOL CCompletionPort::SendPack(SOCKET hSock, void* pData, DWORD dwDataSize)
{
	DWORD dwRet   = 0;
	DWORD dwSend  = 0;
	DWORD dwFlags = 0;

	LPIO_CONTEXT pNewIoContext = GetIoContext();
	m_mapIo.Insert(pNewIoContext);
	memset(pNewIoContext, 0, sizeof(IO_CONTEXT));
	pNewIoContext->byIoType    = IO_SEND;
	memcpy(pNewIoContext->Buffer, pData, dwDataSize);
	pNewIoContext->DataBuf.buf = pNewIoContext->Buffer;
	pNewIoContext->DataBuf.len = dwDataSize;
	pNewIoContext->hCltSock    = hSock;

	dwRet = WSASend(hSock, &pNewIoContext->DataBuf, 1, &dwSend,
		dwFlags, &pNewIoContext->overlapped, NULL);
	if(SOCKET_ERROR == dwRet)
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			REPORT(MN, T("WSASend() failed with error %d\n", WSAGetLastError()), RPT_ERROR);

			ReleaseIoContext(pNewIoContext);
			return FALSE;
		}
	}

	return TRUE;
}


void CCompletionPort::OnAccept(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext,
							   DWORD dwTransCount)
{
	assert(pHandleContext != NULL);
	if (dwTransCount > 0)
	{
		SOCKADDR* lpLocalSockaddr = NULL;
		SOCKADDR* lpRemoteSockaddr = NULL;
		int LocalSockAddrLen  =0;
		int RemoteSockaddrLen = 0;
		m_pGetAcceptExSockaddrs(pIoContext->Buffer, m_nAcceptRecvSize, sizeof(SOCKADDR_IN) + 16, 
			sizeof(SOCKADDR_IN) + 16, &lpLocalSockaddr, &LocalSockAddrLen, &lpRemoteSockaddr, 
			&RemoteSockaddrLen);
		if(lpRemoteSockaddr != NULL)
			memcpy(&pHandleContext->addr, lpRemoteSockaddr, sizeof(SOCKADDR_IN));

		//如果是无效的数据则关闭连接
		pIoContext->tmPreCheck = time(NULL);
		if (!OnRecv(pHandleContext, pIoContext, dwTransCount))
		{
			REPORT(MN, T("收到无效的数据，关闭连接\n", WSAGetLastError()), RPT_WARNING);
			DisConnected(pHandleContext, pIoContext);
			return;
		}
	}
}

BOOL CCompletionPort::RecvPack(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext, int nLen, int nReaded)
{
	DWORD dwRet   = 0;
	DWORD dwRecv  = 0;
	DWORD dwFlags = 0;

	if (nLen > BUF_SIZE)
	{
		REPORT(MN, T("请求数据大小:%d 超过缓冲区大小:%d 关闭连接\n", nLen, BUF_SIZE), RPT_WARNING);
		DisConnected(pHandleContext, pIoContext);
		return FALSE;
	}
	if (nReaded == 0)
		memset(pIoContext->Buffer, 0, BUF_SIZE);
	
	pIoContext->byIoType    = IO_RECV;
	pIoContext->DataBuf.buf = pIoContext->Buffer+nReaded;
	pIoContext->DataBuf.len = nLen;

	dwRet = WSARecv(pIoContext->hCltSock, &pIoContext->DataBuf, 1, &dwRecv, &dwFlags,
		&pIoContext->overlapped, NULL);
	if(SOCKET_ERROR == dwRet)
	{
		if(WSA_IO_PENDING != WSAGetLastError())
		{
			REPORT(MN, T("WSARecv() failed with error %d\n", WSAGetLastError()), RPT_ERROR);

			DisConnected(pHandleContext, pIoContext);
			return FALSE;
		}
	}	
	return TRUE;
}


unsigned int CCompletionPort::_AcceptThread(void* pParam)
{
	CCompletionPort* pComPort = (CCompletionPort*)pParam;
	return pComPort->AcceptThread();
}

unsigned int CCompletionPort::AcceptThread()
{
	REPORT(MN, T("服务器正在监听...\n"), RPT_INFO);
	DWORD dwRet = 0;
	while(m_bRun)
	{
		dwRet = ::WaitForSingleObject(m_hEventListen, 1000);
		if (WAIT_FAILED == dwRet)	      //失败
		{

			REPORT(MN, T("WaitForSingleObject() failed  with error:%d\n", GetLastError()), RPT_ERROR);
			return 0xDEADACCE;
		}
		else if (WAIT_TIMEOUT == dwRet)	 //超时	 
		{
			m_mapIo.Lock();
			map<LPIO_CONTEXT, LPIO_CONTEXT>* pMap = m_mapIo.GetMap();
			map<LPIO_CONTEXT, LPIO_CONTEXT>::iterator pos = pMap->begin();
			int nValue = 0;
			int nLen = sizeof(nValue);
			while(pos != pMap->end())
			{
				if (pos->second->byIoType == IO_ACCEPT)
				{
					if (SOCKET_ERROR == getsockopt(pos->second->hCltSock, SOL_SOCKET, 
						SO_CONNECT_TIME, (char*)&nValue, &nLen))
					{
						++pos;
						continue;
					}
					else if (nValue != -1 && m_nAcceptTimeOut != 0 && nValue >= m_nAcceptTimeOut)
					{
						closesocket(pos->second->hCltSock);	//如果在设置的时间范围内没有发送数据就关闭连接
						m_dequeIo.PushBack(pos->second);
						pMap->erase(pos++);
						continue;
					}
				}
				else
				{
					time_t tmCur = time(NULL);
					if (pos->second->tmPreCheck != 0 && 
						tmCur - pos->second->tmPreCheck > m_nRecvTimeout*2)  //没有收到数据
					{
						OnTimeOut(pos->second);
						closesocket(pos->second->hCltSock);	//如果在设置的时间范围内没有收到数据就关闭连接
						m_dequeIo.PushBack(pos->second);
						pMap->erase(pos++);
						continue;
					}
				}
				pos++;
			}

			m_mapIo.Unlock();
		}
		else if (WAIT_OBJECT_0 == dwRet) //有信号
		{	
			if (!PostAcceptEx())
				return 0xDEADACCE;
		}
	}

	REPORT(MN, T("Accept 线程[0x%08x]结束\n", GetCurrentThreadId()), RPT_INFO);
	return 0xDEADACCE;
}

BOOL CCompletionPort::PostAcceptEx()
{
	for(int i=0; i<10; i++)
	{
		SOCKET hAcceptSock = WSASocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, NULL,
			0, WSA_FLAG_OVERLAPPED);
		if(INVALID_SOCKET == hAcceptSock)
		{
			REPORT(MN, T("WSASocket() failed with error %d\n", WSAGetLastError()), RPT_ERROR);
			return FALSE;
		}

		LPIO_CONTEXT pIo_Context = GetIoContext();
		m_mapIo.Insert(pIo_Context);
		memset(pIo_Context, 0, sizeof(IO_CONTEXT));

		pIo_Context->byIoType    = IO_ACCEPT;
		pIo_Context->DataBuf.buf = pIo_Context->Buffer;
		pIo_Context->DataBuf.len = BUF_SIZE;
		pIo_Context->hCltSock    = hAcceptSock;

		DWORD dwReceived;
		if (m_nAcceptRecvSize == -1)
			m_nAcceptRecvSize = BUF_SIZE - ((sizeof(SOCKADDR_IN)+16) * 2);

		BOOL bRet = m_pAcceptEx(m_hListenSocket, hAcceptSock, pIo_Context->Buffer,
			m_nAcceptRecvSize, sizeof(SOCKADDR_IN)+16,
			sizeof(SOCKADDR_IN)+16, &dwReceived, &pIo_Context->overlapped);

		if (!bRet)
		{
			if(ERROR_IO_PENDING != WSAGetLastError())
			{
				REPORT(MN, T("m_pAcceptEx() failed \n"), RPT_ERROR);
				
				::closesocket(pIo_Context->hCltSock);
				ReleaseIoContext(pIo_Context);
				return FALSE;
			}
		}
	}

	return TRUE;
}

LPIO_CONTEXT CCompletionPort::GetIoContext()
{
	LPIO_CONTEXT pIo_Context = m_dequeIo.PopFront();
	if (pIo_Context == NULL)
	{
		pIo_Context = new IO_CONTEXT;
		if (pIo_Context == NULL)
		{
			REPORT(MN, T("new IO_CONTEXT failed\n", GetCurrentThreadId()), RPT_ERROR);
			return NULL;
		}
	}
	assert(pIo_Context != NULL);
	return pIo_Context;
}

void CCompletionPort::ReleaseIoContext(LPIO_CONTEXT pData)
{
	if (pData != NULL)
	{
		if (m_mapIo.Remove(pData))
			m_dequeIo.PushBack(pData);
	}
}

LPHANDLE_CONTEXT CCompletionPort::GetHandleContext()
{
	LPHANDLE_CONTEXT pData = m_dequeHandle.PopFront();
	if (pData == NULL)
	{
		pData = new HANDLE_CONTEXT;
		if (pData == NULL)
		{
			REPORT(MN, T("new HANDLE_CONTEXT failed \n"), RPT_ERROR);
			return NULL;
		}
	}

	assert(pData != NULL);
	return pData;
}

void CCompletionPort::ReleaseHandleContext(LPHANDLE_CONTEXT pData)
{
	if (pData != NULL)
	{
		if (m_mapHandle.Remove(pData))
			m_dequeHandle.PushBack(pData);
	}
}

/*void CCompletionPort::LogEvent(WORD nLogType, LPCSTR pFormat, ...)
{
	char szMsg[DB_MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(szMsg, DB_MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);
	////日志
}*/

std::string CCompletionPort::GetPeerIP(SOCKET hSock, UINT& nPort)
{
	std::string strIP;
	SOCKADDR_IN addr = {0};
	int nLen = sizeof(sockaddr);
	if (SOCKET_ERROR != getpeername(hSock, (sockaddr*)&addr, &nLen))
	{
		strIP = inet_ntoa(addr.sin_addr);
		nPort = ntohs(addr.sin_port);
	}
	
	return strIP;
}

void CCompletionPort::OnConnect(SOCKET hSock)
{
}
void CCompletionPort::OnDisconnect(LPHANDLE_CONTEXT pHandleContext)
{
}

BOOL CCompletionPort::OnRecv(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext, DWORD dwDataSize)
{
	return TRUE;
}
void CCompletionPort::OnSend(LPIO_CONTEXT pIoContext)
{
}

void CCompletionPort::OnTimeOut(LPIO_CONTEXT pIoContext)
{
}