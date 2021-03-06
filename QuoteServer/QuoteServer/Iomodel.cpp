/*++

Copyright (c) 2004 

模块名: 

iomodel.cpp

模块描述:

Winsock 完成端口类实现文件

作者:

PPP elssann@hotmail.com

开发环境:

Visual C++ 6.0, Windows 2000.

修订记录:

创建于: 2004.1.16

最后修改日期:
2004.1.23\


--*/
#include "stdafx.h"
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "iomodel.h"
#include <time.h>

#include "realdata.h"
#include "config.h"
#include "qs_data_struct.h"
#include "PlugManager.h"
#include "..\\..\\public\\commx\\synch.h"
#include "..\\..\\public\\commx\\report.h"
#include "..\\..\\public\\devmonitor\\ReportServer.h"
#include "DataPool.h"
#include "..\\protocol.h"

using namespace std;

#define MODULE_NAME	"QS"

extern Quote * GetSymbolQuote(char * c_pSymbol);
extern int GetSymbolData(char * c_pReq,char * c_pBuf,int c_iMaxLen);

void HexDump(const void *lpData, unsigned int c_nLength);

CompletionPortModel::CompletionPortModel()
/*++

函数描述:
构造函数，初始化线程句柄数组，初始化AcceptEx()调用的计数。初始化临界段代码变量。

Arguments:
无。

Return Value:
无。

--*/

{
	int i;
	m_bExit = false;
	m_bThreadLoopExit = false;//MyThreadLoop是否已经退出
	m_timeFarmUpdate = time(NULL);
	m_timeRealPush = time(NULL);
	for (i=0; i< MAXTHREAD_COUNT; i++)
	{
		m_hThreadArray[i] = INVALID_HANDLE_VALUE;
	}
	for (i=0; i< 20; i++)
	{
		m_hThreadReq[i] = INVALID_HANDLE_VALUE;
	}

	m_hThreadTimeout = INVALID_HANDLE_VALUE;
	m_hThreadLoop = INVALID_HANDLE_VALUE;
	m_lAcceptExCounter = 0;
	m_unIo = 0;
	m_unHandle = 0;

	m_nRecvTimeout = 300;
	m_nAcceptTimeout = 3;
	m_nError = 0;

	m_nRecvCount=0;
	m_nReleaseFail = 0;

	m_dwPreTick = GetTickCount();//for monitor

	m_lpListenHandleContext = NULL;
}//end of CompletionPortModel()



CompletionPortModel::~CompletionPortModel()
/*++

函数描述:
析构函数，释放链表所有结点。

Arguments:
无。

Return Value:

--*/
{
	int nCount = 0;
	PPER_IO_CONTEXT lpIoNode;
	map<unsigned int,PPER_IO_CONTEXT>::iterator iter;
	while (m_mapConnect.size() > 0)
	{
		iter = m_mapConnect.begin();
		lpIoNode = iter->second;
		m_mapConnect.erase(lpIoNode->unId);
		if (lpIoNode->m_nBufLen > 0)
			free(lpIoNode->m_pBuf);
		if (lpIoNode->m_nReqBufLen > 0)
			free(lpIoNode->m_pReqBuf);
		delete lpIoNode;
		nCount++;
	}

	nCount = 0;
	while(m_vIoLookaside.size() > 0)
	{
		if (m_vIoLookaside.back()->m_nBufLen > 0)
			free(m_vIoLookaside.back()->m_pBuf );
		if (m_vIoLookaside.back()->m_nReqBufLen > 0)
			free(m_vIoLookaside.back()->m_pReqBuf);
		delete m_vIoLookaside.back();
		m_vIoLookaside.pop_back();
		nCount++;
	}

	while(m_vHandleLookaside.size() > 0)
	{
		delete m_vHandleLookaside.back();
		m_vHandleLookaside.pop_back();		
	}

	if (m_lpListenHandleContext)
	{
		delete m_lpListenHandleContext;
		m_lpListenHandleContext = NULL;
	}
}//end of ~CompletionPortModel()

BOOL CompletionPortModel::CreateMyThreadLoop()
{
	m_hThreadLoop = CreateThread(NULL,0,MyThreadLoop,(LPVOID)this,0,NULL);
	return true;
}
BOOL CompletionPortModel::Init()
/*++

函数描述:
初始化，创建完成端口、创建完成端口线程，并调用类成员函数InitWinsock初始化Winsock、
建立一个监听套接字m_ListenSocket，并将此套接字同完成端口关联起来，获取AcceptEx指针。

Arguments:
无。

Return Value:
函数调用成功返回TRUE，失败返回FALSE。

--*/
{
	m_hCOP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == m_hCOP)
	{
		REPORT(MODULE_NAME,T("CreateIoCompletionPort() failed %d\n",GetLastError()),RPT_ERROR);
		return FALSE;
	}

	//
	//取得系统中CPU的数目，创建和CPU数目相等的线程，如果事先估计到完成端口处理线程会堵塞，
	//可以考虑创建 SysInfo.dwNumberOfProcessors*2个线程。一般在单处理器上创建和CPU数目相等
	//的线程就可以了
	//
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	REPORT(MODULE_NAME,T("SysInfo.dwNumberOfProcessors=%d,MaxCount=%d\n",SysInfo.dwNumberOfProcessors,MAXTHREAD_COUNT),RPT_INFO);
	SysInfo.dwNumberOfProcessors *= 2;
	if (MAXTHREAD_COUNT < SysInfo.dwNumberOfProcessors)
	{
		SysInfo.dwNumberOfProcessors = MAXTHREAD_COUNT;
	}

	for (int i=0; i<(int)SysInfo.dwNumberOfProcessors; i++)
	{
		m_hThreadArray[i] = CreateThread(NULL, 0, CompletionRoutine, (LPVOID)this, 0, NULL);
		if (NULL == m_hThreadArray[i])
		{
			while (i>0)
			{
				CloseHandle(m_hThreadArray[i-1]);
				m_hThreadArray[i-1] = INVALID_HANDLE_VALUE;
				i--;
			}//end of while
			REPORT(MODULE_NAME,T("CreateThead failed %d\n",GetLastError()),RPT_WARNING);
			CloseHandle(m_hCOP);
			m_hCOP = INVALID_HANDLE_VALUE;
			return FALSE;
		}
	}//end of for

	m_hThreadTimeout = CreateThread(NULL, 0, TimeoutRoutine, (LPVOID)this, 0, NULL);

	for(int j=0; j<4; j++)
	{
		//_beginthreadex		
		m_hThreadReq[j] = CreateThread(NULL,0,ReqRoutine,(LPVOID)this,0,NULL);
	}

	//
	//调用InitWinsock函数初始化Winsock、建立一个监听套接字m_ListenSocket，
	//并将此套接字同完成端口关联起来，获取AcceptEx指针。
	//
	BOOL bSuccess = InitWinsock();
	if (!bSuccess)
	{
		//
		//给完成端口线程发送消息，指示线程退出。
		//
		PostQueuedCompletionStatus(m_hCOP, 0, NULL, NULL);
		CloseThreadHandle();
		CloseHandle(m_hCOP);
		m_hCOP = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	//
	//调用BindAndListenSocket()绑定套接字并将套接字置于监听状态
	//
	bSuccess = BindAndListenSocket();	
	if (!bSuccess)
	{
		PostQueuedCompletionStatus(m_hCOP, 0, NULL, NULL);
		CloseThreadHandle();
		CloseHandle(m_hCOP);
		m_hCOP = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	return TRUE;	
}//end of Init()

void CompletionPortModel::UnInit()
{
	//PostQueuedCompletionStatus(m_hCOP, 0, NULL, NULL);

	if (INVALID_HANDLE_VALUE != m_hEvent)
	{
		CloseHandle(m_hEvent);
		m_hEvent = INVALID_HANDLE_VALUE;
	}
	CloseThreadHandle();
	if (INVALID_HANDLE_VALUE != m_hCOP)
	{
		CloseHandle(m_hCOP);
		m_hCOP = INVALID_HANDLE_VALUE;
	}
}
void CompletionPortModel::CloseThreadHandle()
/*++

函数描述:
对每一个创建的线程调用CloseHandle()。

Arguments:
无。

Return Value:
无。

--*/
{
	for (int i=0; i< MAXTHREAD_COUNT; i++)
	{
		if (INVALID_HANDLE_VALUE != m_hThreadArray[i])
		{
			CloseHandle(m_hThreadArray[i]);
			m_hThreadArray[i] = INVALID_HANDLE_VALUE;
		}
	}//end of for
	if(INVALID_HANDLE_VALUE != m_hThreadTimeout)
	{
		CloseHandle(m_hThreadTimeout);
		m_hThreadTimeout = INVALID_HANDLE_VALUE;
	}

	for(int j=0; j < 4; j++)
		if (INVALID_HANDLE_VALUE != m_hThreadReq[j])
		{
			CloseHandle(m_hThreadReq[j]);
			m_hThreadReq[j] = INVALID_HANDLE_VALUE;
		}

		if (INVALID_HANDLE_VALUE != m_hThreadLoop)
		{
			CloseHandle(m_hThreadLoop);
			m_hThreadLoop = INVALID_HANDLE_VALUE;
		}
		return;
}//end of CloseThreadHandle()




BOOL CompletionPortModel::InitWinsock()
/*++

函数描述:
初始化Winsock,创建一个监听套接字，获取AcceptEx函数指针，为监听套接字分配一个单句柄
数据，并将监听套接字与完成端口hCOP关联。

Arguments:
无。

Return Value:
函数调用成功返回TRUE，失败返回FALSE。

--*/
{
	WSADATA wsd;
	int nResult = WSAStartup(MAKEWORD(2,2), &wsd);
	if (0 != nResult)
	{
		REPORT(MODULE_NAME,T("WSAStartup() failed\n"),RPT_WARNING);
		return FALSE;
	}

	//	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 
	//		NULL, 0, WSA_FLAG_OVERLAPPED); 
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, 
		NULL, 0, WSA_FLAG_OVERLAPPED); 

	if (INVALID_SOCKET == m_ListenSocket)
	{
		REPORT(MODULE_NAME,T("WSASocket() failed\n"),RPT_WARNING);
		WSACleanup();
		return FALSE;
	}


	DWORD dwResult;

	//
	//获取微软SOCKET扩展函数指针
	//

	nResult = WSAIoctl( 
		m_ListenSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&g_GUIDAcceptEx,
		sizeof(g_GUIDAcceptEx),
		&lpAcceptEx,
		sizeof(lpAcceptEx),
		&dwResult,
		NULL,
		NULL
		);

	if (SOCKET_ERROR == nResult)
	{
		REPORT(MODULE_NAME,T("Get AcceptEx failed %d\n",WSAGetLastError()),RPT_WARNING);
		closesocket(m_ListenSocket);
		WSACleanup();

		return FALSE;
	}

	nResult = WSAIoctl(	
		m_ListenSocket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&g_GUIDTransmitFile,
		sizeof(g_GUIDTransmitFile),
		&lpTransmitFile,
		sizeof(lpTransmitFile),
		&dwResult,
		NULL,
		NULL
		);

	if (SOCKET_ERROR == nResult)
	{
		REPORT(MODULE_NAME,T("Get TransmitFile failed %d\n",WSAGetLastError()),RPT_WARNING);
		closesocket(m_ListenSocket);
		WSACleanup();

		return FALSE;
	}

	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	nResult = WSAIoctl(m_ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidGetAcceptExSockaddrs, sizeof(GUID), &m_lpGetAcceptExSockaddrsFun, sizeof(LPFN_GETACCEPTEXSOCKADDRS), &dwResult, NULL, NULL);
	REPORT(MODULE_NAME,T("WSAIoctl m_lpGetAcceptExSockaddrsFun ret=%d : %d\n",nResult,dwResult),RPT_INFO);
	//
	//为监听套接字分配一个单句柄数据
	//
	PPER_HANDLE_CONTEXT lpListenHandleContext = GetHandleFromLookaside();
	if (NULL == lpListenHandleContext)
	{
		closesocket(m_ListenSocket);
		WSACleanup();

		REPORT(MODULE_NAME,T("HeapAlloc() failed\n"),RPT_WARNING);

		return FALSE;
	}

	lpListenHandleContext->IoSocket = m_ListenSocket;

	//
	//将监听套接字m_ListenSocket和已经建立的完成端口关联起来
	//
	HANDLE hrc = CreateIoCompletionPort(
		(HANDLE)m_ListenSocket,
		m_hCOP,
		(ULONG)lpListenHandleContext,
		0
		);
	if (NULL == hrc)
	{
		closesocket(m_ListenSocket);
		delete lpListenHandleContext;
		WSACleanup();

		REPORT(MODULE_NAME,T("CreateIoCompletionPort failed %d\n",GetLastError()),RPT_WARNING);
		return FALSE;
	}

	m_lpListenHandleContext = lpListenHandleContext;
	return TRUE;
}//end of InitWinsock()




BOOL CompletionPortModel::BindAndListenSocket()
/*++

函数描述:
private函数，供Init调用。
将监听套接字m_ListenSocket绑定到本地IP地址，并置于监听模式。


Arguments:
无。

Return Value:
函数调用成功返回TRUE，失败返回FALSE。

--*/
{
	SOCKADDR_IN InternetAddr;
	InternetAddr.sin_family = AF_INET;

	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(g_cfg.m_strQuoteServerAddr.c_str());
	InternetAddr.sin_port = htons(g_cfg.m_unQuoteServerPort);   

	int nResult = bind(m_ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr));
	if (SOCKET_ERROR == nResult)
	{
		WSACleanup();
		closesocket(m_ListenSocket);

		REPORT(MODULE_NAME,T("Bind() failed %d\n",WSAGetLastError()),RPT_WARNING);
		return FALSE;
	}	

	nResult = listen(m_ListenSocket, 20);
	if (SOCKET_ERROR == nResult)
	{
		WSACleanup();
		closesocket(m_ListenSocket);

		REPORT(MODULE_NAME,T("Listen() failed\n",WSAGetLastError()),RPT_WARNING);
		return FALSE;
	}		

	return TRUE;
}//end of BindAndListenSocket()

DWORD __stdcall ReqRoutine(LPVOID Param)
/*++

函数描述:
实时推送。

Arguments:

Return Value:
线程退出代码。

--*/
{
	CompletionPortModel* pThis = (CompletionPortModel*)Param;
	CRealData RealData;
	bool bRet;
	while(!pThis->m_bExit)
	{
		bool bUsed;
		PPER_IO_CONTEXT lpNode;
		lpNode = pThis->GetNewReq();
		if (NULL == lpNode)
			Sleep(5);
		else
		{
			bUsed = false;
			unsigned short usFuncNo = 0;
			unsigned int unReqLen = 0;
			RealData.PharseReq(lpNode->m_pBuf,usFuncNo,unReqLen);
			if (usFuncNo > 1000)//
			{
				SMsgID pID;
				pID.m_unID = lpNode->unId;
				if (lpNode->unIdMain > 0)
					pID.m_unID = lpNode->unIdMain;
				if (lpNode->m_usProtocolType == 2)
					pID.m_unID += 0x10000000;
				pID.m_unSocket = lpNode->sClient;
				if (g_cfg.m_bDebug)
					REPORT(MODULE_NAME,T("发送插件请求[功能号=%d][长度=%d][ID=%d][SOCKET=%d]\n",usFuncNo,unReqLen,pID.m_unID,pID.m_unSocket),RPT_DEBUG);
				int iRet = g_pPlugs->SendRequest(&pID,usFuncNo,lpNode->m_pBuf,unReqLen);

				if (iRet == -1)
				{//没有对应的处理模块，发送失败信息
					char acMsg[256];
					memset(acMsg,0,sizeof(acMsg));
					sprintf(acMsg,"没有处理功能号(%d)模块",usFuncNo);
					REPORT(MODULE_NAME,T("处理附加请求线程[%s]\n",acMsg),RPT_WARNING);
					RealData.MakeWrongReturn(lpNode->m_pBuf,acMsg,strlen(acMsg));
					lpNode->m_nReqBufLen = RealData.m_unRepBufLen;
					bRet = true;
					/*
					memcpy(lpNode->m_pBuf,RealData.GetRepBuf(),RealData.GetRepLen());
					lpNode->wsaBuffer.buf = lpNode->m_pBuf;
					lpNode->wsaBuffer.len = RealData.GetRepLen();
					lpNode->m_nRequireLen = RealData.GetRepLen();
					lpNode->m_nOverLen = 0;
					lpNode->IoOperation = IoExtraSend;
					pThis->ASYSendBuffer(lpNode);*/
				}
				else
					bRet = false;
			}//
			else
			{
				if (usFuncNo == LINETESTREP)
				{
					//REPORT(MODULE_NAME,T("LINETESTREP %u\n",usFuncNo),RPT_INFO);
					bRet = false;
				}
				else
				{
					bRet = RealData.GetData(lpNode->m_pBuf);
					//REPORT(MODULE_NAME,T("RealData.GetData %u result %d\n",usFuncNo,bRet),RPT_INFO);
				}
			}

			if (bRet)
			{
				if (RealData.GetRepLen() > 0)
				{
					if ((RealData.GetRepLen() + HTTP_HEADER_SIZE)  > lpNode->m_nBufLen)
					{
						free(lpNode->m_pBuf);
						lpNode->m_nBufLen = RealData.GetRepLen() + HTTP_HEADER_SIZE;
						lpNode->m_pBuf = (char *)malloc(lpNode->m_nBufLen);
					}
					int httplen = 0;
					if (lpNode->m_usProtocolType == 2)
					{
						char szHttp[8096];
						httplen=sprintf(szHttp, "HTTP/1.1 200 OK, Success\r\nContent-length: %d\r\ncontent-type: audio/mp3;charset=UTF-8\r\ncontent-source : licaiguanjia\r\n\r\n", RealData.GetRepLen());
						memcpy(lpNode->m_pBuf,szHttp,httplen);
					}

					memcpy(lpNode->m_pBuf + httplen,RealData.GetRepBuf(),RealData.GetRepLen());
					lpNode->wsaBuffer.buf = lpNode->m_pBuf;
					lpNode->wsaBuffer.len = RealData.GetRepLen() + httplen;
					lpNode->m_nRequireLen = RealData.GetRepLen() + httplen;
					lpNode->m_nOverLen = 0;
					lpNode->IoOperation = IoExtraSend;
					bUsed = true;
					pThis->ASYSendBuffer(lpNode);
				}
			}
			if (!bUsed)
				pThis->InsertToLookaside(lpNode,NULL);
		}
	}
	REPORT(MODULE_NAME,T("处理附加请求线程(ReqRouteLine)结束\n"),RPT_WARNING);
	return 0;
}

DWORD __stdcall TimeoutRoutine(LPVOID Param)
/*++

函数描述:
处理超时。

Arguments:

Return Value:
线程退出代码。

--*/
{
	int nCount = 0;
	unsigned int unOldTickCountSH = 0;
	unsigned int unOldTickCountSZ = 0;
	bool bRealPush = false;
	unsigned int unToday = 0;
	unsigned int unOpenTime = 929;
	CompletionPortModel* pThis = (CompletionPortModel*)Param;
	while(!pThis->m_bExit)
	{
		Quote * pQuote;
		CQuoteData QuoteData(NULL);
		if (g_pDataFarm->m_hashIdx.cast("000001.SH",QuoteData,true) > 0)
		{
			if (QuoteData.m_Idx.cntTick != unOldTickCountSH)
			{
				unOldTickCountSH = QuoteData.m_Idx.cntTick;
				bRealPush = true;
			}
			else
				bRealPush = false;
		}

		if (g_pDataFarm->m_hashIdx.cast("399001.SZ",QuoteData,true) > 0)
		{
			if (QuoteData.m_Idx.cntTick != unOldTickCountSZ)
			{
				unOldTickCountSZ = QuoteData.m_Idx.cntTick;
				bRealPush = true;
			}
			//else
			//	bRealPush = false;
		}

		int nAccept = pThis->CheckTimeout(bRealPush);
		Sleep(5 * 200);
		nCount++;
		if ((nCount % 9) == 0)
		{
			//REPORT(MODULE_NAME,T("g_IndexSummary.SendReq6001();\n"),RPT_INFO);
			g_IndexSummary.SendReq6001();
		}
		if (nCount == 10)
		{
			pThis->OutputState(nAccept);
			nCount = 0;
		}

		//判断是否超时
		DWORD dwCurTick = GetTickCount();
		if (dwCurTick - pThis->m_dwPreTick > (DWORD)g_cfg.m_nDevTimeout*1000)
		{
			REPORT(MODULE_NAME,T("QuoteServer 心跳\n"),RPT_HEARTBEAT);
			pThis->m_dwPreTick = dwCurTick;
		}
		//判断是否换日
		time_t now;
		time(&now);
		struct tm *lt = localtime(&now);

		unsigned int un = lt->tm_year * 10000 + lt->tm_mon * 100 + lt->tm_mday;
		unsigned int unTime = lt->tm_hour * 100 + lt->tm_min;
		if (un != unToday || unTime > unOpenTime)
		{
			if (un != unToday)
			{
				unToday = un;
				unOpenTime = 926;
				REPORT(MODULE_NAME,T("QuoteServer 新的一天[%d][%d][%d]\n",un,unOpenTime,unTime),RPT_INFO);
			}
			if(unTime > unOpenTime)
			{
				if (unTime >= 926)
					unOpenTime = 959;
				if (unTime >= 959)
					unOpenTime = 2400;
				REPORT(MODULE_NAME,T("QuoteServer 新的时间[%d]\n",unOpenTime),RPT_INFO);
			}

			//g_StockKindManager.SendReq6004();
			g_DataPool.m_rwlock.WaitWrite();
			g_DataPool.RemoveAll();
			g_DataPool.m_rwlock.EndWrite();
		}
	}
	REPORT(MODULE_NAME,T("超时线程(TimeoutRoutine)结束\n"),RPT_WARNING);
	return 0;
}

DWORD __stdcall MyThreadLoop(LPVOID Param)
{
	DWORD dwResult;
	int nCounter = 0;
	CompletionPortModel* pThis = (CompletionPortModel*)Param;
	int iAdd = 0;
	REPORT(MODULE_NAME,T("QuoteServer 开始运行!\n"),RPT_WARNING);
	while (!pThis->m_bExit)
	{		
		dwResult = WaitForSingleObject(pThis->m_hEvent, 10000);

		if (WAIT_FAILED == dwResult)
		{
			for(int n=0; n<MAXTHREAD_COUNT; n++)
				PostQueuedCompletionStatus(pThis->m_hCOP, 0, NULL, NULL);
			REPORT(MODULE_NAME,T("WSAWaitForMutipleEvents() failed %d\n",WSAGetLastError()),RPT_WARNING);
			return FALSE;
		}

		if (WAIT_TIMEOUT != dwResult)
		{
			//REPORT(MODULE_NAME,T("PostAcceptEx is running\n"),RPT_INFO);
			if (FALSE == pThis->PostAcceptEx())
			{
				REPORT(MODULE_NAME,T("PostAcceptEx() error\n"),RPT_WARNING);
				for(int n=0; n<MAXTHREAD_COUNT; n++)
					PostQueuedCompletionStatus(pThis->m_hCOP, 0, NULL, NULL);

				return FALSE;
			}
		}
		else
		{
			//m_rwLockConnect.WaitWrite();
			//REPORT(MODULE_NAME,T("runinfo:Node=%d Nodeaside=%d Handleaside=%d F=%d I=%d H=%d\n",CountNode(),CountLookaside(),CountHandleaside(),m_nReleaseFail,m_unIo,m_unHandle),RPT_INFO);
			//m_rwLockConnect.EndWrite();
		}
	}

	closesocket(pThis->m_ListenSocket);
	PPER_IO_CONTEXT lpIoNode;
	map<unsigned int,PPER_IO_CONTEXT>::iterator iter;
	for(iter=pThis->m_mapConnect.begin(); iter!=pThis->m_mapConnect.end(); iter++)
	{
		lpIoNode = iter->second;
		closesocket(lpIoNode->sClient);
	}

	if (pThis->m_hCOP != INVALID_HANDLE_VALUE)
	{
		for(int n=0; n<MAXTHREAD_COUNT; n++)
			PostQueuedCompletionStatus(pThis->m_hCOP, 0, NULL, NULL);
		pThis->CloseThreadHandle();
		CloseHandle(pThis->m_hCOP);
		pThis->m_hCOP = INVALID_HANDLE_VALUE;
	}

	REPORT(MODULE_NAME,T("主线程退出!\n"),RPT_WARNING);
	pThis->m_bThreadLoopExit = true;
	return 0;
}//end of CheckConnectTime

DWORD __stdcall CompletionRoutine(LPVOID Param)
/*++

函数描述:
完成端口处理线程，循环调用GetQueuedCompletionStatus来获取IO操作结果。

Arguments:

Return Value:
线程退出代码。

--*/
{
	CompletionPortModel* pThis = (CompletionPortModel*)Param;
	DWORD dwNumberBytes;
	PPER_HANDLE_CONTEXT lpHandleContext = NULL;
	LPWSAOVERLAPPED lpOverlapped = NULL;
	int nResult;
	BOOL bSuccess;
	CRealData RealData;
	PRUN_INFO  pRunInfo;
	pRunInfo = new RUN_INFO();
	memset(pRunInfo,0,sizeof(RUN_INFO));

	pThis->AddRunInfo(pRunInfo);
	while (!pThis->m_bExit)
	{
		bSuccess = GetQueuedCompletionStatus(
			pThis->m_hCOP,
			&dwNumberBytes,
			(ULONG  *)&lpHandleContext,
			&lpOverlapped,
			INFINITE
			);

		if (FALSE == bSuccess)
		{
			PPER_IO_CONTEXT lpPerIoContext = (PPER_IO_CONTEXT)lpOverlapped;
			REPORT(MODULE_NAME,T("GetQ failed SOCKET=%d,error=%d\n",lpPerIoContext->sClient,GetLastError()),RPT_DEBUG);
			if( (WSAGetLastError() == ERROR_NETNAME_DELETED) || (WSAGetLastError()==ERROR_SEM_TIMEOUT)) 
			{
				if (lpPerIoContext->IoOperation == IoExtraSend)//附加发送失败
				{
					pThis->InsertToLookaside(lpPerIoContext, NULL);
				}
				else
				{
					closesocket(lpPerIoContext->sClient);
					pThis->OnSocketClose(lpPerIoContext->unId,lpPerIoContext->sClient);
					if (pThis->ReleaseNode(lpPerIoContext))//试图去除
					{
						if (lpPerIoContext->IoOperation != IoAccept)
							pThis->InsertToLookaside(lpPerIoContext, lpHandleContext);
						else
							pThis->InsertToLookaside(lpPerIoContext, NULL);
					}
				}
			}
			continue;
		}

		PPER_IO_CONTEXT lpPerIoContext = (PPER_IO_CONTEXT)lpOverlapped;
		if (NULL == lpHandleContext)
		{
			REPORT(MODULE_NAME,T("收到空句柄退出\n"),RPT_INFO);
			if (pThis->m_lpListenHandleContext)
			{
				delete pThis->m_lpListenHandleContext;
				pThis->m_lpListenHandleContext = NULL;
			}

			//
			//PostQueuedCompletionStatus发过来一个空的单句柄数据，表示线程要退出了。
			//
			return 0;
		}

		//REPORT(MODULE_NAME,T("GetQ success type=%d num=%d [%d][%d]\n",lpPerIoContext->IoOperation,dwNumberBytes,lpPerIoContext->sClient,lpPerIoContext->unId),RPT_INFO);
		time_t timeNow;
		timeNow = time(NULL);
		if (pRunInfo->m_tmRec != timeNow)
		{
			//REPORT("QS",T("A=%d,S=%d,R=%d,T=%d\n",pRunInfo->m_unAccept_S,pRunInfo->m_unSend_S,pRunInfo->m_unRecv_S,pRunInfo->m_tmRec),RPT_INFO);
			pRunInfo->m_unAccept_S = 0;
			pRunInfo->m_unRecv_S = 0;
			pRunInfo->m_unSend_S = 0;
			pRunInfo->m_tmRec = timeNow;
		}
		if(IoAccept != lpPerIoContext->IoOperation)
		{
			if((!bSuccess) || (bSuccess && (0 == dwNumberBytes ))) //处理的字节数字为0,认为出错了
			{
				//REPORT(MODULE_NAME,T("GetQ Close socket I=%d S=%d\n",lpPerIoContext->unId,lpPerIoContext->sClient),RPT_DEBUG);
				if (lpPerIoContext->IoOperation == IoExtraSend)//附加发送失败
				{
					pThis->InsertToLookaside(lpPerIoContext, NULL);
				}
				else
				{
					closesocket(lpPerIoContext->sClient);
					pThis->OnSocketClose(lpPerIoContext->unId,lpPerIoContext->sClient);	
					if (pThis->ReleaseNode(lpPerIoContext))
					{
						if (lpPerIoContext->IoOperation != IoExtraSend)
							pThis->InsertToLookaside(lpPerIoContext, lpHandleContext);
						else
							pThis->InsertToLookaside(lpPerIoContext, NULL);
					}
				}
				continue;
			}
		}
		HANDLE hResult;
		PPER_HANDLE_CONTEXT lpNewperHandleContext = NULL;

		DWORD i;
		struct sockaddr *p_local_addr;
		int local_addr_len = sizeof(struct sockaddr_in);
		struct sockaddr *p_remote_addr;
		int remote_addr_len = sizeof(struct sockaddr_in);
		struct sockaddr_in *p_v4_addr;
		std::string strIP;

		switch(lpPerIoContext->IoOperation)
		{
		case IoAccept : 


			i = sizeof(struct sockaddr_in) + 16;
			pThis->m_lpGetAcceptExSockaddrsFun(
				lpPerIoContext->m_pBuf,
				8,
				i, i,
				&p_local_addr,
				&local_addr_len,
				&p_remote_addr,
				&remote_addr_len
				);

			//p_v4_addr = (struct sockaddr_in *)p_local_addr;
			//REPORT(MODULE_NAME,T("本地地址%s:%d\n",inet_ntoa(p_v4_addr->sin_addr), ntohs(p_v4_addr->sin_port)),RPT_INFO);
			p_v4_addr = (struct sockaddr_in *)p_remote_addr;
			if (g_cfg.m_bDebug)
				REPORT(MODULE_NAME,T("远程地址%s:%d:%d\n",inet_ntoa(p_v4_addr->sin_addr), ntohs(p_v4_addr->sin_port),lpPerIoContext->sClient),RPT_INFO);
			strIP = inet_ntoa(p_v4_addr->sin_addr);
			strIP = "<" + strIP + ">";
			if (strstr(g_strNoLineTest.c_str(),strIP.c_str()))
			{
				lpPerIoContext->m_nNeedLineTest = -1;
				REPORT(MODULE_NAME,T("远程地址%s:%d,不需要超时设置\n",inet_ntoa(p_v4_addr->sin_addr), ntohs(p_v4_addr->sin_port)),RPT_INFO);
			}
			else
				lpPerIoContext->m_nNeedLineTest = 1;

			nResult = setsockopt(
				lpPerIoContext->sClient, 
				SOL_SOCKET,
				SO_UPDATE_ACCEPT_CONTEXT,
				(char *)&pThis->m_ListenSocket,
				sizeof(pThis->m_ListenSocket)
				);
			if(SOCKET_ERROR == nResult) 
			{
				closesocket(lpPerIoContext->sClient);	
				pThis->OnSocketClose(lpPerIoContext->unId,lpPerIoContext->sClient);	
				if (pThis->ReleaseNode(lpPerIoContext))
					pThis->InsertToLookaside(lpPerIoContext, NULL);
				continue;
			}
			//pThis->m_lpGetAcceptExSockaddrsFun(&(lpPerIoContext->m_pBuf), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &lpLocalAddr, &nLen, &lpRemoteAddr, &nLen);
			//ZeroMemory(&SockAddr,sizeof(SockAddr));
			//memcpy(&SockAddr, lpRemoteAddr, sizeof(struct sockaddr));
			//pSockAddr = (SOCKADDR_IN *)lpRemoteAddr;
			//REPORT(MODULE_NAME,T("CLIENT %s，不做线路测试和超时处理\n",inet_ntoa(SockAddr.sin_addr)),RPT_INFO);
			//REPORT(MODULE_NAME,T("CLIENT %s，不做线路测试和超时处理\n",inet_ntoa(pSockAddr->sin_addr)),RPT_INFO);

			//lpPerIoContext->m_bNeedLineTest = pThis->IsNeedLineTest(lpPerIoContext->sClient);
			/*
			ZeroMemory(&RemoteAddr,sizeof(RemoteAddr));   
			if   (SOCKET_ERROR   ==   getpeername(lpPerIoContext->sClient,(SOCKADDR*)&RemoteAddr,&addrlen))   
			{   
			strNatIP =   "0.0.0.0";   
			}   
			else   
			{   
			SOCKADDR_IN   *pRemoteAddr   =   (SOCKADDR_IN*)&RemoteAddr;   
			strNatIP   =   inet_ntoa(pRemoteAddr->sin_addr);
			strNatIP = "<" + strNatIP + ">";
			if (strstr(g_strNoLineTest.c_str(),strNatIP.c_str()))
			{
			lpPerIoContext->m_bNeedLineTest = false;
			REPORT(MODULE_NAME,T("CLIENT %s，不做线路测试和超时处理\n",strNatIP.c_str()),RPT_INFO);
			}
			}
			*/			

			lpNewperHandleContext = pThis->GetHandleFromLookaside();
			if (NULL == lpNewperHandleContext)
			{
				if (NULL == lpNewperHandleContext)
				{
					closesocket(lpPerIoContext->sClient);
					if (pThis->ReleaseNode(lpPerIoContext))
						pThis->InsertToLookaside(lpPerIoContext, NULL);
					continue;
				}				
			}

			lpNewperHandleContext->IoSocket = lpPerIoContext->sClient;

			//
			//将新建立的套接字关联到完成端口
			//
			hResult = CreateIoCompletionPort(
				(HANDLE)lpPerIoContext->sClient,\
				pThis->m_hCOP,
				(DWORD)lpNewperHandleContext,
				0
				);
			if (NULL == hResult)
			{
				closesocket(lpPerIoContext->sClient);
				if (pThis->ReleaseNode(lpPerIoContext))
				{
					pThis->InsertToLookaside(lpPerIoContext, lpNewperHandleContext);
				}
				continue;
			}
			pRunInfo->m_unAccept++;
			pRunInfo->m_unAccept_S++;
			pRunInfo->m_unRecv += dwNumberBytes;
			pRunInfo->m_unRecv_S += dwNumberBytes;

			if (dwNumberBytes)
			{
				//
				//分析处理数据。
				//
				lpPerIoContext->m_timeCheck  = timeNow;//更新心跳时间
				lpPerIoContext->m_timeLineTest = timeNow;//更新线路测试时间
				lpPerIoContext->m_nOverLen = dwNumberBytes;
				lpPerIoContext->IoOperation = IoRead;
				pThis->HandleData(lpPerIoContext, IO_READ_COMPLETION,&RealData);
				bSuccess = pThis->DataAction(lpPerIoContext, lpNewperHandleContext);
				if (FALSE == bSuccess)
				{
					continue;
				}
			}

			//
			//如果连接成功但是没有收到数据
			//
			else
			{
				pThis->HandleData(lpPerIoContext, IO_ACCEPT_COMPLETION,&RealData);
				bSuccess = pThis->DataAction(lpPerIoContext, lpNewperHandleContext);
				if (FALSE == bSuccess)
				{
					continue;
				}
			}
			break;//end of case IoAccept

		case IoRead:
			lpPerIoContext->m_timeCheck = timeNow;
			lpPerIoContext->m_timeLineTest = timeNow;//更新线路测试时间

			lpPerIoContext->m_nOverLen += dwNumberBytes;
			pRunInfo->m_unRecv += dwNumberBytes;
			pRunInfo->m_unRecv_S += dwNumberBytes;
			pThis->HandleData(lpPerIoContext, IO_READ_COMPLETION,&RealData);
			bSuccess = pThis->DataAction(lpPerIoContext, lpNewperHandleContext);
			if (FALSE == bSuccess)
			{
				continue;
			}

			break;//end of case IoRead

		case IoWrite:
			lpPerIoContext->m_timeCheck  = timeNow;//更新心跳时间
			lpPerIoContext->m_timeLineTest = timeNow;//更新线路测试时间
			lpPerIoContext->m_nOverLen += dwNumberBytes;

			pRunInfo->m_unSend += dwNumberBytes;
			pRunInfo->m_unSend_S += dwNumberBytes;

			pThis->HandleData(lpPerIoContext, IO_WRITE_COMPLETION,&RealData);
			bSuccess = pThis->DataAction(lpPerIoContext, lpNewperHandleContext);

			if (FALSE == bSuccess)
			{
				continue;
			}	

			break;
		case IoExtraSend:
			lpPerIoContext->m_timeCheck  = timeNow;//更新心跳时间
			lpPerIoContext->m_timeLineTest = timeNow;//更新线路测试时间
			lpPerIoContext->m_nOverLen += dwNumberBytes;

			pRunInfo->m_unSend += dwNumberBytes;
			pRunInfo->m_unSend_S += dwNumberBytes;

			pThis->HandleData(lpPerIoContext, IO_EXTRA_SEND_COMPLETION,&RealData);
			bSuccess = pThis->DataAction(lpPerIoContext, lpNewperHandleContext);
			if (FALSE == bSuccess)
			{
				continue;
			}	
			break;
		default:
			continue;
			break;
		}
	}

	pThis->RemoveRunInfo(pRunInfo);
	delete pRunInfo;
	REPORT(MODULE_NAME,T("完成端口线程退出\n"),RPT_INFO);

	return 0;

}//end of CompletionRoutine()



BOOL CompletionPortModel::PostAcceptEx()
/*++

Fucntion Description:

连续发出10个AcceptEx调用。

Arguments:

Return Value:

函数调用成功返回TRUE，失败返回FALSE。

--*/
{
	int nZero = 0;
	//return TRUE;
	while (m_lAcceptExCounter < 30)
	{
		SOCKET AcceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, \
			NULL, 0, WSA_FLAG_OVERLAPPED); 	
		if (INVALID_SOCKET == AcceptSocket)
		{
			REPORT(MODULE_NAME,T("WSASocket failed\n"),RPT_WARNING);
			return FALSE;
		}
		/*
		nZero = 1024 * 1024;
		int nResult = setsockopt(AcceptSocket, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));
		if( nResult == SOCKET_ERROR)
		{
		cout << "setsockopt(SNDBUF) failed: " << WSAGetLastError();
		REPORT(MODULE_NAME,T("setsockopt(SNDBUF) failed:,error=%d\n",WSAGetLastError()),RPT_INFO);
		return FALSE;
		}
		*/
		/*
		nResult = setsockopt(AcceptSocket, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));
		if( nResult == SOCKET_ERROR)
		{
		cout << "setsockopt(SNDBUF) failed: " << WSAGetLastError();

		return FALSE;
		}
		--*/		
		PPER_IO_CONTEXT lpAcceptExIoContext = GetIoFromLookaside();
		if (NULL == lpAcceptExIoContext)
		{
			closesocket(AcceptSocket);
			return FALSE;
		}
		lpAcceptExIoContext->sClient = AcceptSocket;
		lpAcceptExIoContext->IoOperation = IoAccept;
		lpAcceptExIoContext->SocketType = SocketServer;

		lpAcceptExIoContext->wsaBuffer.buf = lpAcceptExIoContext->m_pBuf;
		lpAcceptExIoContext->wsaBuffer.len = lpAcceptExIoContext->m_nBufLen;
		lpAcceptExIoContext->m_nOverLen = 0;
		lpAcceptExIoContext->m_usProtocolType = 0;//开始不知道是什么协议
		InsertNode(lpAcceptExIoContext);
		DWORD dwBytes;
		BOOL bSuccess = lpAcceptEx(
			m_ListenSocket,
			lpAcceptExIoContext->sClient,
			lpAcceptExIoContext->m_pBuf,
			8,//lpAcceptExIoContext->wsaBuffer.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
			sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16,
			&dwBytes,
			&(lpAcceptExIoContext->ol));

		if (FALSE == bSuccess)
		{
			int nResult = WSAGetLastError();
			if (nResult != ERROR_IO_PENDING)
			{
				REPORT(MODULE_NAME,T("AcceptEx(%d) failed %d\n",m_lAcceptExCounter,nResult),RPT_WARNING);
				REPORT(MODULE_NAME,T("STATE:Node=%d Nodeaside=%d Handleaside=%d F=%d I=%d H=%d\n",CountNode(),CountLookaside(),CountHandleaside(),m_nReleaseFail,m_unIo,m_unHandle),RPT_WARNING);
				closesocket(AcceptSocket);
				ReleaseNode(lpAcceptExIoContext);
				InsertToLookaside(lpAcceptExIoContext, NULL);
				continue;
				//return FALSE;
			}
		} 
		InterlockedExchangeAdd((LONG *)&m_lAcceptExCounter, 1);
	}
	InterlockedExchangeAdd((LONG *)&m_lAcceptExCounter, -30);
	return TRUE;
}//end of PostAccetExRoutine()

int CompletionPortModel::CountNode()
{
	return m_mapConnect.size();
}

void CompletionPortModel::InsertNode(PPER_IO_CONTEXT pNode)
/*++

Fucntion Description:

根据参数类型将传递进来结点插入到相应的链表头。

Arguments:

pNode       -  要插入链表中的结点
pHandleNode -  要插入链表中的结点

Return Value:

无.

--*/
{
	if (NULL != pNode)
	{		
		m_rwLockConnect.WaitWrite();
		m_mapConnect[pNode->unId] = pNode;
		//REPORT(MODULE_NAME,T("INSERTNODE ID=%d SOCKET=%d \n",pNode->unId,pNode->sClient),RPT_INFO);
		m_rwLockConnect.EndWrite();
	}

	return;
}//end of InsertNode


bool CompletionPortModel::ReleaseNode(PPER_IO_CONTEXT pNode)
/*++

Fucntion Description:

将参数中传递的结点从链表中解除,但不释放结点。以便不让ThreadLoop函数对其进行超时检测。
此函数在完成端口线程里收发数据成功后调用。

Arguments:

要从链表中释放的结点。

Return Value:

无。

--*/
{
	bool bRet;
	m_rwLockConnect.WaitWrite();
	if (m_mapConnect.find(pNode->unId) != m_mapConnect.end())
	{
		bRet = true;
		//REPORT(MODULE_NAME,T("RELEASENODE ID=%d S=%d OK\n",pNode->unId,pNode->sClient),RPT_WARNING);
	}
	else
	{
		bRet = false;
		//REPORT(MODULE_NAME,T("RELEASENODE FAIL ID=%d S=%d IoOperation=%d\n",pNode->unId,pNode->sClient,pNode->IoOperation),RPT_DEBUG);
		//REPORT(MODULE_NAME,T("RELEASENODE FAIL ID=%d S=%d B\n",pNode->unId,pNode->sClient),RPT_DEBUG);
		//std::map<unsigned int,PPER_IO_CONTEXT>::iterator iter;
		//for(iter=m_mapConnect.begin(); iter!=m_mapConnect.end(); iter++)
		//{
		//	REPORT(MODULE_NAME,T("IN MAP FIRST=%d ID=%d S=%d \n",iter->first,iter->second->unId,iter->second->sClient),RPT_DEBUG);
		//}
		//REPORT(MODULE_NAME,T("RELEASENODE FAIL ID=%d S=%d E\n",pNode->unId,pNode->sClient),RPT_DEBUG);
	}
	m_mapConnect.erase(pNode->unId);
	if (!bRet)
		m_nReleaseFail++;

	m_rwLockConnect.EndWrite();
	return bRet;
}//end of RealeseNode


BOOL CompletionPortModel::HandleData(PPER_IO_CONTEXT lpPerIoContext, int nFlags,CRealData * c_pRealData)
{
	if (IO_READ_COMPLETION == nFlags)
	{		
		//REPORT(MODULE_NAME,T("already recved=%d\n",lpPerIoContext->m_nOverLen),RPT_INFO);
		c_pRealData->Reset();
		int nGetReq = DealReq(lpPerIoContext,c_pRealData);
		//如果发现数据有问题则停止此连接
		if (nGetReq < 0)
		{
			//REPORT(MODULE_NAME,T("nGetReq = %d IoEnd I=%d S=%d \n",nGetReq,lpPerIoContext->unId,lpPerIoContext->sClient),RPT_INFO);
			ZeroMemory(&(lpPerIoContext->ol), sizeof(WSAOVERLAPPED));
			lpPerIoContext->IoOperation = IoEnd;
			return TRUE;
		}
		if (nGetReq == 1 && c_pRealData->GetRepLen() > 0)//发送数据
		{
			ZeroMemory(&(lpPerIoContext->ol), sizeof(WSAOVERLAPPED));
			lpPerIoContext->IoOperation = IoWrite;
			PPACKETHEAD pHead = (PPACKETHEAD)c_pRealData->GetRepBuf();
			if (g_cfg.m_bDebug)
				REPORT(MODULE_NAME,T("数据生成,长度=%d SOCKET=%d UID=%d [COMMXHEAD:LEN=%d SEQ=%d FUNCNO=%d]\n",c_pRealData->GetRepLen(),lpPerIoContext->sClient,lpPerIoContext->unId,pHead->m_commxHead.GetLength(),pHead->m_commxHead.GetUS(pHead->m_commxHead.SerialNo),pHead->m_commxHead.GetUS(pHead->m_usFuncNo)),RPT_INFO);
			if (lpPerIoContext->m_nBufLen < (c_pRealData->GetRepLen() + HTTP_HEADER_SIZE))
			{
				free(lpPerIoContext->m_pBuf);
				lpPerIoContext->m_nBufLen = c_pRealData->GetRepLen() + HTTP_HEADER_SIZE;
				lpPerIoContext->m_pBuf = (char *)malloc(c_pRealData->GetRepLen() + HTTP_HEADER_SIZE);
			}
			int httplen = 0;
			if (lpPerIoContext->m_usProtocolType == 2)
			{
				char szHttp[8096];
				httplen=sprintf(szHttp, "HTTP/1.1 200 OK, Success\r\nContent-length: %d\r\ncontent-type: audio/mp3;charset=UTF-8\r\ncontent-source : licaiguanjia\r\n\r\n", c_pRealData->GetRepLen());
				memcpy(lpPerIoContext->m_pBuf,szHttp,httplen);
			}

			memcpy(lpPerIoContext->m_pBuf + httplen,c_pRealData->GetRepBuf(),c_pRealData->GetRepLen());
			lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf;
			lpPerIoContext->wsaBuffer.len = c_pRealData->GetRepLen() + httplen;
			lpPerIoContext->m_nOverLen = 0;
			lpPerIoContext->m_nRequireLen = c_pRealData->GetRepLen() + httplen;
			lpPerIoContext->m_usProtocolType = 0;
		}
		else
		{
			ZeroMemory(&(lpPerIoContext->ol), sizeof(WSAOVERLAPPED));
			lpPerIoContext->IoOperation = IoRead;
			//REPORT(MODULE_NAME,T("DealReq=%d,read new,already recved=%d\n",nGetReq,lpPerIoContext->m_nOverLen),RPT_INFO);
			if (nGetReq == 1)
			{
				lpPerIoContext->m_nOverLen = 0;
				lpPerIoContext->m_nRequireLen = 8;
				lpPerIoContext->m_usProtocolType = 0;
				lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf;
				lpPerIoContext->wsaBuffer.len = 8;
			}
			else
			{
				lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf + lpPerIoContext->m_nOverLen;
				lpPerIoContext->wsaBuffer.len = lpPerIoContext->m_nRequireLen - lpPerIoContext->m_nOverLen;
			}
		}
		return TRUE;
	}

	if ((IO_WRITE_COMPLETION == nFlags) || (IO_EXTRA_SEND_COMPLETION == nFlags))
	{
		if (lpPerIoContext->m_nOverLen > lpPerIoContext->m_nRequireLen)
		{
			//异常，终止此连接
			if (g_cfg.m_bDebug)
				REPORT(MODULE_NAME,T("异常终止连接 m_nOverLen(%d)>m_nRequireLen(%d) IoOperation=%d I=%d S=%d \n",lpPerIoContext->m_nOverLen,lpPerIoContext->m_nRequireLen,lpPerIoContext->IoOperation,lpPerIoContext->unId,lpPerIoContext->sClient),RPT_INFO);
			if (IO_WRITE_COMPLETION == nFlags)
				lpPerIoContext->IoOperation = IoEnd;
			else
				lpPerIoContext->IoOperation = IoExtraSendEnd;
			return  true;
		}
		if (lpPerIoContext->m_nOverLen == lpPerIoContext->m_nRequireLen)
		{
			if (IO_WRITE_COMPLETION == nFlags)
			{
				PPACKETHEAD pHead = (PPACKETHEAD)lpPerIoContext->m_pBuf;
				if (g_cfg.m_bDebug)
				{
					if (strstr(lpPerIoContext->m_pBuf,"HTTP") && strstr(lpPerIoContext->m_pBuf,"\r\n\r\n"))
						pHead = (PPACKETHEAD)(strstr(lpPerIoContext->m_pBuf,"\r\n\r\n") + 4);
					REPORT(MODULE_NAME,T("数据发送完成 SOCKET=%d UID=%d len=%d [COMMXHEAD:LEN=%d SEQ=%d FUNCNO=%d]\n",lpPerIoContext->sClient,lpPerIoContext->unId,lpPerIoContext->m_nOverLen,pHead->m_commxHead.GetLength(),pHead->m_commxHead.GetUS(pHead->m_commxHead.SerialNo),pHead->m_commxHead.GetUS(pHead->m_usFuncNo)),RPT_INFO);
					if (g_cfg.m_bDumpData)
					{
						HexDump(lpPerIoContext->m_pBuf,lpPerIoContext->m_nOverLen);
						//REPORT(MODULE_NAME,T("%s\n",str.c_str()),RPT_INFO);
					}
				}
				lpPerIoContext->IoOperation = IoRead;
				ZeroMemory(&(lpPerIoContext->ol), sizeof(WSAOVERLAPPED));
				ZeroMemory(lpPerIoContext->m_pBuf, lpPerIoContext->m_nBufLen);
				lpPerIoContext->wsaBuffer.len = 8;
				lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf;
				lpPerIoContext->m_nOverLen = 0;
				lpPerIoContext->m_nRequireLen = 8;
				lpPerIoContext->m_usProtocolType = 0;
			}
			else
			{
				PPACKETHEAD pHead = (PPACKETHEAD)lpPerIoContext->m_pBuf;
				if (g_cfg.m_bDebug)
				{
					if (strstr(lpPerIoContext->m_pBuf,"HTTP") && strstr(lpPerIoContext->m_pBuf,"\r\n\r\n"))
						pHead = (PPACKETHEAD)(strstr(lpPerIoContext->m_pBuf,"\r\n\r\n") + 4);
					REPORT(MODULE_NAME,T("数据发送完成 SOCKET=%d UID=%d len=%d [COMMXHEAD:LEN=%d SEQ=%d FUNCNO=%d]\n",lpPerIoContext->sClient,lpPerIoContext->unId,lpPerIoContext->m_nOverLen,pHead->m_commxHead.GetLength(),pHead->m_commxHead.GetUS(pHead->m_commxHead.SerialNo),pHead->m_commxHead.GetUS(pHead->m_usFuncNo)),RPT_INFO);
					if (g_cfg.m_bDumpData)
					{
						HexDump(lpPerIoContext->m_pBuf,lpPerIoContext->m_nOverLen);
						//REPORT(MODULE_NAME,T("%s\n",str.c_str()),RPT_INFO);
					}
				}
				lpPerIoContext->IoOperation = IoExtraSendEnd;
				ZeroMemory(&(lpPerIoContext->ol), sizeof(WSAOVERLAPPED));
				ZeroMemory(lpPerIoContext->m_pBuf, lpPerIoContext->m_nBufLen);
				lpPerIoContext->wsaBuffer.len = lpPerIoContext->m_nBufLen;
				lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf;
				lpPerIoContext->m_nOverLen = 0;
				lpPerIoContext->m_nRequireLen = 0;
				lpPerIoContext->m_usProtocolType = 0;
			}
			return TRUE;
		}
		else
		{	//继续发送当前片
			ZeroMemory(&(lpPerIoContext->ol), sizeof(WSAOVERLAPPED));
			lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf + lpPerIoContext->m_nOverLen;
			lpPerIoContext->wsaBuffer.len = lpPerIoContext->m_nRequireLen - lpPerIoContext->m_nOverLen;
			//if (lpPerIoContext->wsaBuffer.len > 1024)
			//	lpPerIoContext->wsaBuffer.len = 1024;
		}
		return TRUE;
	}

	if (IO_ACCEPT_COMPLETION == nFlags)
	{
		//
		//刚建立了一个连接，并且没有收发数据，，，，
		//
		lpPerIoContext->m_nOverLen = 0;
		lpPerIoContext->IoOperation = IoRead;
		ZeroMemory(&(lpPerIoContext->ol), sizeof(WSAOVERLAPPED));
		ZeroMemory(lpPerIoContext->m_pBuf, lpPerIoContext->m_nBufLen);
		lpPerIoContext->wsaBuffer.len = lpPerIoContext->m_nBufLen;
		lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf;

		return TRUE;
	}

	return FALSE;
}// end of HandleData()


BOOL CompletionPortModel::AllocEventMessage()
/*++

Fucntion Description:

将FD_ACCEPT事件注册到m_hEvent，这样当可用AcceptEx调用被耗尽的时候，就会触发FD_ACCEPT
事件，然后ThreadLoop里的WaitForSingleObject就会成功返回，导致PostAcceptEx被调用。

Arguments:
无。

Return Value:
函数调用成功返回TRUE，失败返回FALSE。

--*/
{
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == m_hEvent)
	{
		PostQueuedCompletionStatus(m_hCOP, 0, NULL, NULL);
		REPORT(MODULE_NAME,T("CreateEvent() failed %d\n",GetLastError()),RPT_WARNING);
		return FALSE;
	}

	int nResult = WSAEventSelect(m_ListenSocket, m_hEvent, FD_ACCEPT);
	if (SOCKET_ERROR == nResult)
	{
		PostQueuedCompletionStatus(m_hCOP, 0, NULL, NULL);
		CloseHandle(m_hEvent);
		m_hEvent = INVALID_HANDLE_VALUE;
		REPORT(MODULE_NAME,T("WSAEventSelect() failed\n",WSAGetLastError()),RPT_WARNING);
		return FALSE;
	}

	return TRUE;
}//end of AllocEventMessage()
void CompletionPortModel::EndEventMessage()
{
	if (m_hEvent == INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hEvent);
		m_hEvent = INVALID_HANDLE_VALUE;
	}
}

BOOL CompletionPortModel::DataAction(PPER_IO_CONTEXT lpPerIoContext, PPER_HANDLE_CONTEXT lpNewperHandleContext)
/*++

Fucntion Description:

根据参数lpPerIoContext的成员IoOperation的值来进行下一步IO操作。

Arguments:

lpPerIoContext        - 将随WSASend或者WSARecv一起投递的扩展WSAOVERLAPPED结构。
lpNewperHandleContext - AcceptEx调用成功后给新套接字分配的“单句柄数据”。

Return Value:
函数调用成功返回TRUE，失败返回FALSE。

--*/
{
	int nResult;
	int iCount=0;
	DWORD dwIosize = 0;
	DWORD dwFlags =0;
	//this->m_clientContainer.UpdateClientIOState(lpPerIoContext->unId,lpPerIoContext->sClient,lpPerIoContext->IoOperation);
	if (IoWrite == lpPerIoContext->IoOperation || IoExtraSend == lpPerIoContext->IoOperation)
	{
		nResult = WSASend(lpPerIoContext->sClient,
			&(lpPerIoContext->wsaBuffer),
			1,
			&dwIosize,
			0,
			&(lpPerIoContext->ol),
			NULL
			);
		//REPORT(MODULE_NAME,T("WSASend result=%d error=%d len=%d id=%d\n",nResult,WSAGetLastError(),lpPerIoContext->wsaBuffer.len,lpPerIoContext->unId),RPT_INFO);

		if((SOCKET_ERROR==nResult) && (ERROR_IO_PENDING != WSAGetLastError()))
		{
			REPORT(MODULE_NAME,T("WSASend failed error=%d\n",WSAGetLastError()),RPT_WARNING);
			closesocket(lpPerIoContext->sClient);
			OnSocketClose(lpPerIoContext->unId,lpPerIoContext->sClient);	
			if (ReleaseNode(lpPerIoContext))
			{
				InsertToLookaside(lpPerIoContext, NULL);
				InsertToLookaside(NULL, lpNewperHandleContext);
			}
			return FALSE;
		}
		return true;
	}

	if (IoRead == lpPerIoContext->IoOperation)
	{
		//REPORT(MODULE_NAME,T("To WSARecv %d [%d][%d][%d]\n",lpPerIoContext->sClient,dwIosize,lpPerIoContext->wsaBuffer.len,lpPerIoContext->unId),RPT_INFO);
		nResult = WSARecv(lpPerIoContext->sClient,
			&(lpPerIoContext->wsaBuffer),
			1,
			&dwIosize,
			&dwFlags,
			&(lpPerIoContext->ol),
			NULL
			);

		if((SOCKET_ERROR==nResult) && (ERROR_IO_PENDING != WSAGetLastError()))
		{
			REPORT(MODULE_NAME,T("WSARecv failed error=%d [%d][%d]\n",WSAGetLastError(),lpPerIoContext->sClient,lpPerIoContext->unId),RPT_WARNING);
			closesocket(lpPerIoContext->sClient);
			OnSocketClose(lpPerIoContext->unId,lpPerIoContext->sClient);	
			if (ReleaseNode(lpPerIoContext))
			{
				InsertToLookaside(lpPerIoContext, NULL);
				InsertToLookaside(NULL, lpNewperHandleContext);
			}
			return FALSE;
		}

		return true;
	}

	if (IoEnd == lpPerIoContext->IoOperation)
	{
		//REPORT(MODULE_NAME,T("IoEnd to ReleaseNode %d %d\n",lpPerIoContext->sClient,lpPerIoContext->unId),RPT_INFO);
		closesocket(lpPerIoContext->sClient);
		OnSocketClose(lpPerIoContext->unId,lpPerIoContext->sClient);	
		if (lpNewperHandleContext)
		{
			InsertToLookaside(NULL, lpNewperHandleContext);
		}		
		if (ReleaseNode(lpPerIoContext))
		{
			InsertToLookaside(lpPerIoContext, NULL);
		}
		return true;
	}
	if (IoExtraSendEnd == lpPerIoContext->IoOperation)
	{
		if (lpNewperHandleContext)
		{
			InsertToLookaside(NULL, lpNewperHandleContext);
		}
		InsertToLookaside(lpPerIoContext, NULL);
		return true;
	}

	return TRUE;
}// end of DataAction()

void CompletionPortModel::SetTimeout(int c_nAcceptTimeout,int c_nRecvTimeout)
/*
设置超时时间，-1表示不使用超时时间
*/
{
	m_nAcceptTimeout = c_nAcceptTimeout;
	m_nRecvTimeout = c_nRecvTimeout;
}

void CompletionPortModel::SetLineTestTime(int c_nTime)
{
	m_nLineTest = c_nTime;
}
void CompletionPortModel::GetAddressAndPort()
/*++

Fucntion Description:

由类构造函数调用的函数，用来输入服务器要绑定的本地IP地址和端口。

Arguments:

无。

Return Value:

无。   

--*/
{
	cout << "\nPlease input a port: ";
	cin >> uPort;
	cout << "\nPlease input localaddress:";
	cin >> szAddress;

	system("cls");

	return;
}// end of GetAddressAdnPort


int CompletionPortModel::CountHandleaside()
{
	return m_vHandleLookaside.size();
}
int CompletionPortModel::CountLookaside()
{
	return m_vIoLookaside.size();
}

void CompletionPortModel::InsertToLookaside(PPER_IO_CONTEXT lpIoNode, PPER_HANDLE_CONTEXT lpHandleNode)
/*++

Fucntion Description:

给旁视列表的链表中插入一个空闲的结点。

Arguments:

lpIoNode     - 要插入的结点，类型为PPER_IO_CONTEXT。
lpHandleNode - 要插入的结点，类型为PPER_HANDLE_CONTEXT。

Return Value:

无。   

--*/
{
	if (NULL != lpIoNode)
	{
		m_rwLockIo.WaitWrite();
		m_vIoLookaside.push_back(lpIoNode);
		m_rwLockIo.EndWrite();
	}

	if (NULL != lpHandleNode)
	{
		m_rwLockHandle.WaitWrite();
		m_vHandleLookaside.push_back(lpHandleNode);
		m_rwLockHandle.EndWrite();
	}

	return;
}


PPER_IO_CONTEXT CompletionPortModel::GetIoFromLookaside()
/*++

Fucntion Description:

从旁视列表中解除一个结点并将其返回。

Arguments:

无。

Return Value:

返回一个PPER_IO_CONTEXT类型的结点。   

--*/
{
	PPER_IO_CONTEXT lpReturnNode;
	m_rwLockIo.WaitWrite();
	if (m_vIoLookaside.size() > 0)
	{
		lpReturnNode = m_vIoLookaside.back(); 
		lpReturnNode->m_timeCheck = 0;
		lpReturnNode->m_nOverLen = 0;
		lpReturnNode->m_timeLineTest = 0;
		lpReturnNode->sClient = 0;
		lpReturnNode->m_nSocketError = 0;
		lpReturnNode->unIdMain = 0;
		//REPORT(MODULE_NAME,T("GetIoFromLookaside ID=%d SOCKET=%d\n",lpReturnNode->unId,lpReturnNode->sClient),RPT_INFO);
		m_vIoLookaside.pop_back();
	}
	else
	{
		m_unIo++;
		lpReturnNode = new PER_IO_CONTEXT();
		if (NULL == lpReturnNode)
		{
			m_rwLockIo.EndWrite();
			REPORT(MODULE_NAME,T("HeadpAlloc() failed\n"),RPT_WARNING);
			return NULL;
		}
		memset(lpReturnNode,0,sizeof(PER_IO_CONTEXT));
		lpReturnNode->m_timeCheck = 0;
		lpReturnNode->m_nOverLen = 0;
		lpReturnNode->m_timeLineTest = 0;
		lpReturnNode->unId = m_unIo;
		lpReturnNode->unIdMain = 0;
		lpReturnNode->m_pBuf = (char *)malloc(BUFFER_SIZE);
		lpReturnNode->m_nBufLen = BUFFER_SIZE;
		lpReturnNode->m_pReqBuf = NULL;
		lpReturnNode->m_nReqBufLen = 0;
		lpReturnNode->m_nSocketError = 0;
	}
	ZeroMemory(&(lpReturnNode->ol), sizeof(lpReturnNode->ol));
	ZeroMemory(lpReturnNode->m_pBuf, lpReturnNode->m_nBufLen);
	m_rwLockIo.EndWrite();

	return lpReturnNode;
}



PPER_HANDLE_CONTEXT CompletionPortModel::GetHandleFromLookaside()
/*++

Fucntion Description:

从旁视列表中解除一个结点并将其返回。

Arguments:

无。

Return Value:

返回一个PPER_HANDLE_CONTEXT类型的结点。   

--*/
{
	m_rwLockHandle.WaitWrite();
	PPER_HANDLE_CONTEXT lpReturnNode = NULL;
	if (m_vHandleLookaside.size() > 0)
	{
		lpReturnNode = m_vHandleLookaside.back();
		m_vHandleLookaside.pop_back();
	}
	else
	{
		m_unHandle++;
		lpReturnNode = new PER_HANDLE_CONTEXT();
		if (NULL == lpReturnNode)
		{
			REPORT(MODULE_NAME,T("HeapAlloc() failed\n"),RPT_WARNING);
		}				
		memset(lpReturnNode,0,sizeof(PER_HANDLE_CONTEXT));
	}
	m_rwLockHandle.EndWrite();

	return lpReturnNode;
}

// 异步发送数据
bool CompletionPortModel::ASYSendBuffer(PPER_IO_CONTEXT c_lpIo)
{
	int nResult;
	DWORD dwIosize = 0;
	DWORD dwFlags =0;
	memset(&c_lpIo->ol,0,sizeof(WSAOVERLAPPED));
	if (c_lpIo->IoOperation == IoExtraSend)
	{
		nResult = WSASend(c_lpIo->sClient,
			&(c_lpIo->wsaBuffer),
			1,
			&dwIosize,
			dwFlags,
			&(c_lpIo->ol),
			NULL
			);
		//REPORT(MODULE_NAME,T("WSASend() len=%d result=%d error=%d\n",c_lpIo->wsaBuffer.len,nResult,WSAGetLastError()),RPT_INFO);
		if((SOCKET_ERROR==nResult) && (ERROR_IO_PENDING != WSAGetLastError()))
		{			
			InsertToLookaside(c_lpIo, NULL);
			return FALSE;
		}
	}

	return true;
}
int CompletionPortModel::CloseAllSocket()
{
	//closesocket(m_ListenSocket);
	PPER_IO_CONTEXT lpNode;
	map<unsigned int,PPER_IO_CONTEXT>::iterator iter,iterNext;

	m_rwLockConnect.WaitWrite();
	iter = m_mapConnect.begin();
	while(iter != m_mapConnect.end())
	{					
		lpNode = iter->second;
		closesocket(lpNode->sClient);
		iter++;
	}
	m_rwLockConnect.EndWrite();

	//WSACleanup();
	return 0;
}
int CompletionPortModel::CheckTimeout(bool c_bRealPush)
/*++

Fucntion Description:

对系统中已经建立成功了的并且还没有收发过数据的SOCKET连接进行检测，如果某个连接已经建立了超过指定时间，并且还没收发过数据，则强制关闭。
对系统中不处于活动的连接，超过指定时间后，强制关闭

Arguments:

无。

Return Value:

函数调用成功返回TRUE，调用失败返回FALSE；

--*/
{
	int nOptval;
	int nOptlen;
	int nResult;
	int nCounter = 0;
	time_t timeRun;

	PPER_IO_CONTEXT lpNode;
	map<unsigned int,PPER_IO_CONTEXT>::iterator iter,iterNext;
	timeRun = time(NULL);//当前时间
	bool bRealPush = false;
	if (m_timeRealPush < m_timeFarmUpdate)
	{
		m_timeRealPush = timeRun;
		bRealPush = true;
	}
	bRealPush = c_bRealPush;
	m_rwLockConnect.WaitWrite();
	int iAccept = 0;
	unsigned int unID = 0;
	//for(iter = m_mapConnect.begin();iter != m_mapConnect.end();iter++)
	iter = m_mapConnect.begin();
	while(iter != m_mapConnect.end())
	{					
		lpNode = iter->second;
		if (lpNode->m_nSocketError < 0)
		{
			lpNode->m_nSocketError--;
			if (lpNode->m_nSocketError < -30)//超过3秒，释放此节点
				OnSocketClose(lpNode->unId,lpNode->sClient);
			if (g_cfg.m_bDebug)
				REPORT(MODULE_NAME,T("错误连接，移除 ID=%d socket =%d operation=%d\n",lpNode->unId,lpNode->sClient,lpNode->IoOperation),RPT_DEBUG);

			iter = m_mapConnect.erase(iter);
			if (lpNode->IoOperation != IoExtraSend && lpNode->IoOperation != IoAccept)
				InsertToLookaside(lpNode, NULL);
			else
				InsertToLookaside(lpNode, NULL);
			//REPORT(MODULE_NAME,T("错误连接，移除结束 ID=%d socket =%d operation=%d\n",lpNode->unId,lpNode->sClient,lpNode->IoOperation),RPT_DEBUG);

			continue;
		}
		else
		{
			nOptval = 0;
			nOptlen = sizeof(nOptval);
			nResult = getsockopt(
				lpNode->sClient,
				SOL_SOCKET,
				SO_CONNECT_TIME,
				(char*)&nOptval,
				&nOptlen
				);
			if (nResult == SOCKET_ERROR)
			{
				closesocket(lpNode->sClient);//关闭
				if (g_cfg.m_bDebug)
					REPORT(MODULE_NAME,T("getsockopt SOCKET_ERROR ID=%d socket =%d nOptval=%d\n",lpNode->unId,lpNode->sClient,nOptval),RPT_DEBUG);
				lpNode->m_nSocketError = -1;//开始记录
				iter++;
				continue;
			}
		}
		if(lpNode->IoOperation == IoAccept)
		{
			if (m_nAcceptTimeout > 0)
			{
				iAccept++;
				if ((lpNode->m_nNeedLineTest != -1) && (nOptval!=0xFFFFFFFF) && (nOptval>m_nAcceptTimeout))//超过指定时间没有接收到数据
				{
					DWORD i;
					struct sockaddr *p_local_addr;
					int local_addr_len = sizeof(struct sockaddr_in);
					struct sockaddr *p_remote_addr;
					int remote_addr_len = sizeof(struct sockaddr_in);
					struct sockaddr_in *p_v4_addr;				

					i = sizeof(struct sockaddr_in) + 16;
					m_lpGetAcceptExSockaddrsFun(
						lpNode->m_pBuf,
						8,
						i, i,
						&p_local_addr,
						&local_addr_len,
						&p_remote_addr,
						&remote_addr_len
						);

					//p_v4_addr = (struct sockaddr_in *)p_local_addr;
					//REPORT(MODULE_NAME,T("本地地址%s:%d\n",inet_ntoa(p_v4_addr->sin_addr), ntohs(p_v4_addr->sin_port)),RPT_INFO);
					p_v4_addr = (struct sockaddr_in *)p_remote_addr;
					//REPORT(MODULE_NAME,T("远程地址%s:%d\n",inet_ntoa(p_v4_addr->sin_addr), ntohs(p_v4_addr->sin_port),),RPT_INFO);


					std::string strIP = inet_ntoa(p_v4_addr->sin_addr);
					strIP = "<" + strIP + ">";
					if (strstr(g_strNoLineTest.c_str(),strIP.c_str()))
					{
						lpNode->m_nNeedLineTest = -1;
						if (g_cfg.m_bDebug)
							REPORT(MODULE_NAME,T("超时(%d) 不关闭 id=%d socket =%d IP=%s\n",nOptval,lpNode->unId,lpNode->sClient,strIP.c_str()),RPT_DEBUG);
					}
					else
					{
						lpNode->m_nNeedLineTest = 1;
						if (g_cfg.m_bDebug)
							REPORT(MODULE_NAME,T("超时(%d) 无数据关闭 id=%d socket =%d\n",nOptval,lpNode->unId,lpNode->sClient),RPT_DEBUG);
						closesocket(lpNode->sClient);//关闭
					}
				}
			}
			iter++;
			continue;
		}

		if (lpNode->m_timeCheck == 0)
		{
			lpNode->m_timeCheck = timeRun;
			lpNode->m_timeLineTest = timeRun;
		}

		if (m_nLineTest > 0)//线路检测
		{
			if (lpNode->SocketType != SocketClient)	//不是连接QuoteFarm的客户端
				if ((timeRun - lpNode->m_timeLineTest) > m_nLineTest)//30秒一次心跳
				{
					lpNode->m_timeLineTest = timeRun;
					//CRealData RealData;
					//char buf[256];
					//int iLen;
					//iLen = RealData.MakeLINETESTREQ(buf);
					//ASYSendBuffer(lpNode->sClient,buf,iLen);
				}
		}

		if (m_nRecvTimeout > 0)
		{
			if (lpNode->m_nNeedLineTest == 1 && (timeRun - lpNode->m_timeCheck) > m_nRecvTimeout)//没有收到数据
			{
				if (g_cfg.m_bDebug)
					REPORT(MODULE_NAME,T("没有活动(%d) 关闭close id=%d socket=%d\n",m_nRecvTimeout,lpNode->unId,lpNode->sClient),RPT_DEBUG);
				closesocket(lpNode->sClient);//关闭
				iter++;
				continue;
			}
		}
		if (lpNode->m_bRealPush && bRealPush)
			//if (lpNode->m_bRealPush)
		{
			if (lpNode->m_nReqBufLen > 0)
				PushNewReq(lpNode->sClient,lpNode->m_pReqBuf);
		}
		iter++;
	}
	m_rwLockConnect.EndWrite();
	return iAccept;
}//end of CheckConnectTime

int CompletionPortModel::ConnectTo(char * c_pHost, int c_nPort)
{
	int iRtnCode;
	SOCKET   iConSock;
	WSADATA wsaData;
	struct sockaddr_in stServAddr;

	memset(&stServAddr,0, sizeof(stServAddr));
	stServAddr.sin_family = AF_INET;
	stServAddr.sin_addr.s_addr =inet_addr(c_pHost);
	stServAddr.sin_port = htons(c_nPort);

	if(WSAStartup( MAKEWORD( 2, 2 ), &wsaData )!=0)
		return INVALID_SOCKET;

	iConSock=socket(AF_INET, SOCK_STREAM, 0);
	if(iConSock == INVALID_SOCKET )
	{
		return iConSock;
	}
	iRtnCode=connect(iConSock, (struct sockaddr*) &stServAddr, sizeof(stServAddr));

	if(iRtnCode <0)
	{
		closesocket(iConSock);
		return INVALID_SOCKET;
	}

	PPER_IO_CONTEXT lpPerIoContext = GetIoFromLookaside();
	if (NULL == lpPerIoContext)
	{
		closesocket(iConSock);
		return INVALID_SOCKET;
	}
	lpPerIoContext->sClient = iConSock;
	lpPerIoContext->IoOperation = IoRead;
	lpPerIoContext->SocketType = SocketClient;

	ZeroMemory(lpPerIoContext->m_pBuf, lpPerIoContext->m_nBufLen);
	lpPerIoContext->wsaBuffer.buf = lpPerIoContext->m_pBuf;
	lpPerIoContext->wsaBuffer.len = lpPerIoContext->m_nBufLen;
	lpPerIoContext->m_nOverLen = 0;

	HANDLE hResult;
	PPER_HANDLE_CONTEXT lpNewperHandleContext = NULL;

	lpNewperHandleContext = GetHandleFromLookaside();
	if (NULL == lpNewperHandleContext)
	{
		closesocket(iConSock);
		REPORT(MODULE_NAME,T("HeapAlloc() failed\n"),RPT_WARNING);
		InsertToLookaside(lpPerIoContext, NULL);			
		return INVALID_SOCKET;
	}

	lpNewperHandleContext->IoSocket = lpPerIoContext->sClient;

	hResult = CreateIoCompletionPort(
		(HANDLE)lpPerIoContext->sClient,\
		m_hCOP,
		(DWORD)lpNewperHandleContext,
		0
		);

	if (NULL == hResult)
	{
		REPORT(MODULE_NAME,T("CreateCompletionPort() failed\n"),RPT_WARNING);
		closesocket(iConSock);
		InsertToLookaside(lpPerIoContext, NULL);
		InsertToLookaside(NULL, lpNewperHandleContext);
		return INVALID_SOCKET;
	}

	InsertNode(lpPerIoContext);
	DataAction(lpPerIoContext,lpNewperHandleContext);
	return iConSock;

}

int CompletionPortModel::ConnectToFarmer(char *c_pHost, int c_nPort)
{
	m_clientToFarmer.SetServer(c_pHost, c_nPort);
	m_clientToFarmer.SetIoModel(this);
	m_clientToFarmer.CreateMonThread();
	return 0;
}

void CompletionPortModel::OnSocketClose(unsigned int c_unID,SOCKET c_s)
{
	m_clientToFarmer.OnSocketClose(c_s);
}

PPER_IO_CONTEXT CompletionPortModel::FindIO(unsigned int c_ID,SOCKET c_socket)
{
	map<unsigned int,PPER_IO_CONTEXT>::iterator iter,iterNext;
	//m_rwLockConnect.WaitWrite();
	if (m_mapConnect.count(c_ID) > 0)
	{
		if (m_mapConnect[c_ID]->sClient == c_socket)
			return m_mapConnect[c_ID];
	}
	//m_rwLockConnect.EndWrite();
	return NULL;
}

PPER_IO_CONTEXT CompletionPortModel::FindIO(unsigned int c_ID)
{
	map<unsigned int,PPER_IO_CONTEXT>::iterator iter;

	if (m_mapConnect.count(c_ID) > 0)
	{
		return m_mapConnect[c_ID];
	}
	return NULL;
}


PPER_IO_CONTEXT CompletionPortModel::GetNewReq()
{
	PPER_IO_CONTEXT lpRet;
	m_lockVReq.WaitWrite();
	if (m_listReq.size() == 0)
	{
		m_lockVReq.EndWrite();
		return NULL;
	}
	//REPORT(MODULE_NAME,T("GetNewReq<%d> one Enter\n",m_listReq.size()),RPT_INFO);

	lpRet = m_listReq.front();
	m_listReq.pop_front();
	//REPORT(MODULE_NAME,T("GetNewReq<%d> one Out\n",m_listReq.size()),RPT_INFO);
	m_lockVReq.EndWrite();
	return lpRet;
}
void CompletionPortModel::PushNewReq(PPER_IO_CONTEXT c_lpIo)
{
	PPER_IO_CONTEXT lpNode = GetIoFromLookaside();
	if (lpNode == NULL)
		return;
	lpNode->IoOperation = IoExtraSend;
	lpNode->m_nOverLen = 0;
	lpNode->m_nRequireLen = 0;
	lpNode->m_timeCheck = 0;
	lpNode->m_timeLineTest = 0;
	lpNode->unIdMain = c_lpIo->unId;
	lpNode->sClient = c_lpIo->sClient;
	lpNode->SocketType = c_lpIo->SocketType;
	lpNode->m_usProtocolType = c_lpIo->m_usProtocolType;//协议类型
	lpNode->m_nHeadLen = c_lpIo->m_nHeadLen;
	memcpy(lpNode->m_acHttpHead,c_lpIo->m_acHttpHead,c_lpIo->m_nHeadLen);
	if (lpNode->m_nBufLen < (unsigned int)c_lpIo->m_nOverLen)
	{
		free(lpNode->m_pBuf);
		lpNode->m_pBuf = (char *)malloc(c_lpIo->m_nOverLen + 1);
	}
	memcpy(lpNode->m_pBuf,c_lpIo->m_pBuf,c_lpIo->m_nOverLen);
	lpNode->m_nOverLen = c_lpIo->m_nOverLen;
	if (g_cfg.m_bDebug)
		REPORT(MODULE_NAME,T("插件服务 协议类型=%d,长度=%d,SOCKET=%d,UID=%d %d\n",c_lpIo->m_usProtocolType,c_lpIo->m_nOverLen,lpNode->sClient,c_lpIo->unId),RPT_INFO);
	m_lockVReq.WaitWrite();
	m_listReq.push_back(lpNode);
	m_lockVReq.EndWrite();
}
void CompletionPortModel::PushNewReq(SOCKET c_sClient,char * c_pReq)
{
	PPER_IO_CONTEXT lpNode = GetIoFromLookaside();
	if (lpNode == NULL)
		return;
	lpNode->IoOperation = IoExtraSend;
	lpNode->m_nOverLen = 0;
	lpNode->m_nRequireLen = 0;
	lpNode->m_timeCheck = 0;
	lpNode->m_timeLineTest = 0;
	lpNode->sClient = c_sClient;
	lpNode->SocketType = SocketServer;
	unsigned int unLen = *(unsigned int *)(c_pReq + 4) + 8;
	if (lpNode->m_nBufLen < unLen)
	{
		free(lpNode->m_pBuf);
		lpNode->m_pBuf = (char *)malloc(unLen + 1);
	}
	memcpy(lpNode->m_pBuf,c_pReq,unLen);
	lpNode->m_nOverLen = unLen;
	m_lockVReq.WaitWrite();
	m_listReq.push_back(lpNode);
	//REPORT(MODULE_NAME,T("PushNewReq<%d> %d %d %d\n",m_listReq.size(),lpNode->unId,lpNode->sClient,lpNode->m_nOverLen),RPT_INFO);
	m_lockVReq.EndWrite();
}

// 检查处理接收到的数据，返回是否成功
int CompletionPortModel::DealReq(PPER_IO_CONTEXT c_lpIO,CRealData * c_pRealData)
{
	unsigned int unPackLen;
	char * pBuf;
	unsigned short usFuncNo;
	pBuf = c_lpIO->m_pBuf;
	if (c_lpIO->m_nOverLen < 8)
	{
		c_lpIO->m_nRequireLen = 8;
		return 0;
	}
	//REPORT(MODULE_NAME,T("RECV [%d][%d]\n",c_lpIO->m_nRequireLen,c_lpIO->m_nOverLen),RPT_INFO);
	//检查是否是http协议
	if (c_lpIO->m_usProtocolType == 0)
	{
		//REPORT(MODULE_NAME,T("RECV [%d][%d][%s]\n",c_lpIO->m_nRequireLen,c_lpIO->m_nOverLen,c_lpIO->m_pBuf),RPT_INFO);
		if (strnicmp(c_lpIO->m_pBuf,"post",4) == 0)
		{
			//是http协议
			c_lpIO->m_usProtocolType = 2;
			c_lpIO->m_nHeadLen = 0;
		}
		else
		{
			c_lpIO->m_usProtocolType = 1;
		}
	}
	if (c_lpIO->m_usProtocolType == 2 && c_lpIO->m_nHeadLen == 0)//是http协议并且没有读到结束符号
	{
		char * pBody;
		pBody = memstr(c_lpIO->m_pBuf,c_lpIO->m_nOverLen,"\r\n\r\n",4);
		if (!pBody)//还没有读到结束标记
		{
			//继续读
			c_lpIO->m_nRequireLen = HTTP_HEADER_SIZE - c_lpIO->m_nOverLen;
			return 0;
		}
		else
		{//已经读到http结束标记
			int nHeadLen;
			nHeadLen = pBody - c_lpIO->m_pBuf + 4;
			memcpy(c_lpIO->m_acHttpHead,c_lpIO->m_pBuf,c_lpIO->m_nOverLen);//先复制到头缓冲区
			c_lpIO->m_nHeadLen = nHeadLen;//头的实际长度
			//将真实头复制到接收缓冲区
			memcpy(c_lpIO->m_pBuf,c_lpIO->m_acHttpHead + nHeadLen,c_lpIO->m_nOverLen - nHeadLen);
			//REPORT(MODULE_NAME,T("post HEAD[%d][%s]\n",c_lpIO->m_nHeadLen,c_lpIO->m_acHttpHead),RPT_INFO);
			c_lpIO->m_nOverLen -= nHeadLen;
			if (c_lpIO->m_nOverLen < 8)//还没有读完真正的头
			{
				c_lpIO->m_nRequireLen = 8;
				return 0;
			}
		}
	}
	CommxHead * pCommxHead = (CommxHead *)pBuf;
	//REPORT(MODULE_NAME,T("收到数据包 PACKET [%d][%d][%d][%d]\n",pCommxHead->GetVersion(),pCommxHead->GetByteorder(),pCommxHead->GetEncoding(),pCommxHead->GetLength()),RPT_DEBUG);	
	if (!pCommxHead->IsValid())
	{
		REPORT(MODULE_NAME,T("错误包 PACKET [%d][%d][%d][%d]\n",pCommxHead->GetVersion(),pCommxHead->GetByteorder(),pCommxHead->GetEncoding(),pCommxHead->GetLength()),RPT_DEBUG);
		return -999;
	}
	//unPackLen = *((unsigned int *)(pBuf + 4)) + 8;//正确的包长
	unPackLen = pCommxHead->GetLength() + 8;
	//REPORT(MODULE_NAME,T("unPackLen=%d \n",unPackLen),RPT_INFO);
	if (unPackLen > 1024 * 1024)
	{
		REPORT(MODULE_NAME,T("错误包 PACKET [%d][%d][%d][%d]\n",pCommxHead->GetVersion(),pCommxHead->GetByteorder(),pCommxHead->GetEncoding(),pCommxHead->GetLength()),RPT_DEBUG);
		return -999;
	}
	if (unPackLen > c_lpIO->m_nBufLen)
	{
		if (c_lpIO->m_nOverLen > 8)
			return -1;
	}
	if (unPackLen == c_lpIO->m_nOverLen)
	{
		//将请求放入队列
		usFuncNo = pCommxHead->GetUS(*((unsigned short *)(pBuf + 8)));
		if (c_lpIO->SocketType == SocketClient)
		{		
			if (usFuncNo == FARMNOTIFY)//只处理行情变化通知
			{
				//REPORT(MODULE_NAME,T("QuoteFarm Update data,len=%d\n",unPackLen),RPT_DEBUG);
				//启动RealPush
				OnFarmUpdateData(pBuf);
				//通知插件
				g_pPlugs->UpdateData(pBuf,unPackLen);
			}
			else
			{
				//REPORT(MODULE_NAME,T("QuoteFarm Heart Beat\n"),RPT_DEBUG);
			}
		}
		else
		{
			if (g_cfg.m_bDebug)
				REPORT(MODULE_NAME,T("收到数据包 PACKET [版本:%d][字节序:%d][编码:%d][SEQ:%d][包长:%d][功能号：%d][SOCKET=%d][UID=%d]\n",pCommxHead->GetVersion(),pCommxHead->GetByteorder(),pCommxHead->GetEncoding(),pCommxHead->GetUS(pCommxHead->SerialNo),pCommxHead->GetLength(),usFuncNo,c_lpIO->sClient,c_lpIO->unId),RPT_DEBUG);	
			if (usFuncNo < 1000)
			{
				if (g_cfg.m_bRunWithQuote || usFuncNo == 0)
				{
					if (c_pRealData->GetData(pBuf) == false)
						return -2;
					//PushNewReq(c_lpIO);
					unsigned short usFirstReqType;
					usFirstReqType = pCommxHead->GetUS(*((unsigned short *)(pBuf + 12)));//正确的包长
					if (usFirstReqType & REALPUSHREQ)
					{
						c_lpIO->m_bRealPush = true;
						if (c_lpIO->m_nReqBufLen < unPackLen)
						{
							if (c_lpIO->m_nReqBufLen > 0)
								free(c_lpIO->m_pReqBuf);
							c_lpIO->m_pReqBuf = (char *)malloc(unPackLen + 1);
							c_lpIO->m_nReqBufLen = unPackLen + 1;
						}
						memcpy(c_lpIO->m_pReqBuf,c_lpIO->m_pBuf,unPackLen);
						if (g_cfg.m_bDebug)
							REPORT(MODULE_NAME,T("RealPush usFuncNo=%d,FirstReq=%d\n",usFuncNo,usFirstReqType),RPT_DEBUG);
					}
				}
			}
			else
			{
				//REPORT(MODULE_NAME,T("PushNewReq usFuncNo=%d\n",usFuncNo),RPT_DEBUG);
				c_pRealData->m_unRepLen = 0;
				PushNewReq(c_lpIO);
			}
		}
		return 1;
	}
	c_lpIO->m_nRequireLen = unPackLen;
	if(unPackLen > c_lpIO->m_nBufLen)
	{
		char * pTmp = (char *)malloc(unPackLen);
		memcpy(pTmp,c_lpIO->m_pBuf,c_lpIO->m_nOverLen);
		free(c_lpIO->m_pBuf);
		c_lpIO->m_pBuf = pTmp;
		c_lpIO->m_nBufLen = unPackLen;
	}
	return 0;
}

void CompletionPortModel::OnFarmUpdateData(char * c_pBuf)
{
	m_timeFarmUpdate = time(NULL);//将更新时间设置为当前时间
}

void CompletionPortModel::OutputState(int &c_nAccept)
{
	RUN_INFO f;
	memset(&f,0,sizeof(RUN_INFO));
	TotalRunInfo(&f);
	REPORT(MODULE_NAME,T("连接=%d 等待=%d 内存<<空闲(%d,%d) 失败=%d I=%d H=%d>>\n",CountNode(),c_nAccept,CountLookaside(),CountHandleaside(),m_nReleaseFail,m_unIo,m_unHandle),RPT_ADDI_INFO);
	REPORT(MODULE_NAME,T("连接数量(%d,%d) 接收(%d,%d) 发送(%d,%d)\n",f.m_unAccept,f.m_unAccept_S,f.m_unRecv,f.m_unRecv_S,f.m_unSend,f.m_unSend_S),RPT_INFO);

	//REPORT(MODULE_NAME,T("INFO: MAX=%d PROCESS=%d PRE=%d CUR=%d DUR=%d \n",m_nMaxSecondDone,m_unDone,m_nLastSecondDone,m_nSecondDone,n),RPT_INFO);
	/*
	Quote * pq;
	pq = GetSymbolQuote("000006.sz");
	if (pq == NULL)
	cout<<"GetSymbolQuote FAIL"<<endl;
	else
	cout<<"Get 000006.sz data::"<<pq->zjjg <<"  "<<pq->jrkp <<endl;

	SPlugReq req;
	req.m_unType = REQ_QUOTE;
	char buf[4096];
	strcpy(req.m_acSymbol,"000001.SH");
	int nr = GetSymbolData((char *)&req,buf,4096);
	cout<<"GetSymbolData return="<<nr<<endl;
	*/
	//char pBuf[1024];
	//g_Plugs.UpdateData(pBuf,1024);
	/*
	m_rwLockConnect.WaitWrite();
	REPORT(MODULE_NAME,T("OutputState BEGIN\n"),RPT_DEBUG);
	std::map<unsigned int,PPER_IO_CONTEXT>::iterator iter;
	for(iter=m_mapConnect.begin(); iter!=m_mapConnect.end(); iter++)
	{
	REPORT(MODULE_NAME,T("IN MAP FIRST=%d ID=%d S=%d \n",iter->first,iter->second->unId,iter->second->sClient),RPT_DEBUG);
	}
	REPORT(MODULE_NAME,T("OutputState END\n"),RPT_DEBUG);
	m_rwLockConnect.EndWrite();
	*/	
}

void CompletionPortModel::AddRunInfo(PRUN_INFO c_pRun_Info)
{
	std::vector<PRUN_INFO>::iterator iter;
	m_lockRun_Info.WaitWrite();
	m_vpRun_Info.push_back(c_pRun_Info);
	m_lockRun_Info.EndWrite();
}

void CompletionPortModel::RemoveRunInfo(PRUN_INFO c_pRun_Info)
{
	std::vector<PRUN_INFO>::iterator iter;
	m_lockRun_Info.WaitWrite();
	for(iter=m_vpRun_Info.begin(); iter!=m_vpRun_Info.end(); iter++)
	{
		if (c_pRun_Info == *iter)
		{
			m_vpRun_Info.erase(iter);
			break;
		}
	}
	m_lockRun_Info.EndWrite();
}

void CompletionPortModel::TotalRunInfo(PRUN_INFO c_pRun_Info)
{
	std::vector<PRUN_INFO>::iterator iter;
	m_lockRun_Info.WaitWrite();
	for(iter=m_vpRun_Info.begin(); iter!=m_vpRun_Info.end(); iter++)
	{
		c_pRun_Info->m_unAccept += (*iter)->m_unAccept;
		c_pRun_Info->m_unAccept_S += (*iter)->m_unAccept_S;
		c_pRun_Info->m_unSend += (*iter)->m_unSend;
		c_pRun_Info->m_unSend_S += (*iter)->m_unSend_S;
		c_pRun_Info->m_unRecv += (*iter)->m_unRecv;
		c_pRun_Info->m_unRecv_S += (*iter)->m_unRecv_S;
	}
	m_lockRun_Info.EndWrite();
}

bool CompletionPortModel::IsNeedLineTest(SOCKET c_id)
{
	SOCKADDR_IN RemoteAddr;   
	int   addrlen   =   sizeof(SOCKADDR_IN);
	std::string strNatIP;

	ZeroMemory(&RemoteAddr,sizeof(RemoteAddr));   
	if   (SOCKET_ERROR   !=   getpeername(c_id,(SOCKADDR*)&RemoteAddr,&addrlen))   
	{   
		SOCKADDR_IN   *pRemoteAddr   =   (SOCKADDR_IN*)&RemoteAddr;   
		strNatIP   =   inet_ntoa(pRemoteAddr->sin_addr);
		strNatIP = "<" + strNatIP + ">";
		//REPORT(MODULE_NAME,T("CLIENT %s Connected %s\n",strNatIP.c_str(),g_strNoLineTest.c_str()),RPT_INFO);
		if (strstr(g_strNoLineTest.c_str(),strNatIP.c_str()))
		{
			if (g_cfg.m_bDebug)
				REPORT(MODULE_NAME,T("CLIENT %s，不做线路测试和超时处理\n",strNatIP.c_str()),RPT_INFO);
			return false;
		}
	}
	return true;
}

char* CompletionPortModel::memstr(const void* buf, unsigned int buflen, const void* lpfind, unsigned int findlen)
{
	unsigned int nPos = 0;
	while (1)
	{
		char* lpTmp = (char*)memchr(&((char*)buf)[nPos], ((char*)lpfind)[0], buflen - nPos);
		if (!lpTmp)
			return NULL;
		if (0 == memcmp(lpTmp, lpfind, findlen))
			return lpTmp;
		nPos = lpTmp - (char*)buf + 1;
		if (nPos >= buflen)
			return NULL;
	}
}


//输出16进制数据
void HexDump(const void *lpData, unsigned int c_nLength)
{
	char szAlpha[4], szSeeable[16], szTitle[128];
	int nLength = c_nLength;
	if (nLength > 1024)
		nLength = 1024;
	sprintf(szTitle, "Dump first %d of %d bytes:\n", nLength,c_nLength);
	string sr = szTitle;
	int nLine=0;
	int nBlockCount=0;
	sprintf(szTitle, "Dump 128 block begin at %d of %d bytes:\n", nBlockCount * 128,nLength);
	sr += szTitle;
	for (int m = 0; m < nLength; m++)
	{
		short idx = m%16;
		unsigned char c = ((char*)lpData)[m];
		sprintf(szAlpha, "%02X ", c);
		sr.append(szAlpha, 3);

		if (isprint(c))
			szSeeable[idx] = c;
		else
			szSeeable[idx] = '.';
		if (idx == 15)
		{
			sr += "-> ";
			sr.append(szSeeable , idx+1);
			sr += "\n";
			nLine++;
		}
		else if (m == nLength - 1)
		{
			sr.append((16 - idx - 1) * 3, ' ');
			sr += "-> ";
			sr.append(szSeeable , idx+1);
			sr += "\n";
			nLine++;
		}
		if (nLine == 8)
		{
			REPORT(MODULE_NAME,T("%s",sr.c_str()),RPT_DEBUG);
			nLine = 0;
			nBlockCount++;
			sprintf(szTitle, "Dump 128 block begin at %d of %d bytes:\n", nBlockCount * 128,nLength);
			sr = szTitle;
		}
	}

	if (sr.length() > 0 && nLine > 0)
		REPORT(MODULE_NAME,T("%s",sr.c_str()),RPT_DEBUG);
}
