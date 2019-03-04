
#include "stdafx.h"
#include "async_result.h"
#include "../public/report.h"
#include "../public/xexception.h"

HANDLE g_hIocp = NULL;
HANDLE g_hTimerQueue = NULL;
atomint g_nThreadCount = 0;
LPFN_ACCEPTEX lpfnAcceptEx = NULL;
LPFN_CONNECTEX lpfnConnectEx = NULL;
HANDLE g_hCommThread = NULL;
atomint g_nCommxRef = 0;

DWORD WINAPI ThreadProc(LPVOID IocpHandle)
{
	PSEH->DoCatchCpp();

	DWORD dwTransCount = 0;
	ICommX *pIcommX = NULL;
	IAsyncOverlappedResult *pResult = NULL;
	OVERLAPPED *pol = NULL;
	HANDLE hIocp = reinterpret_cast<HANDLE>(IocpHandle);
	++g_nThreadCount;

	while (1)
	{
		BOOL bQcs = ::GetQueuedCompletionStatus(hIocp, &dwTransCount,
			(PULONG)&pIcommX, (LPOVERLAPPED*)(&pol), INFINITE);
		pResult = (IAsyncOverlappedResult*)pol;

		if (bQcs == ERROR_SUCCESS && pResult && dwTransCount == 0)
		{/// socket closed or error
			if (0 == pIcommX->handle_result(pResult))
				pIcommX->Close();
		}
		else if (bQcs && pResult)
			pResult->complete(pIcommX, dwTransCount);
		else if (bQcs && !pResult)
			break;	// 收到线程退出包
		else
		{
#ifdef _DEBUG
			REPORT(MN, T("未知的完成端口状态: 0x%08x\n", pIcommX), RPT_WARNING);
#endif
		}
	}

	--g_nThreadCount;
	return 0;
}

int COMMXAPI start_comm_loop(unsigned short nThreadNum)
{
	++g_nCommxRef;
	if (WaitForSingleObject(g_hCommThread, 0) == WAIT_OBJECT_0)
		return 0;

	if (!g_hCommThread)
		g_hCommThread = CreateEvent(NULL, FALSE, TRUE, NULL);

	if (nThreadNum == 0)
	{
		SYSTEM_INFO si={0};
		GetSystemInfo(&si);
		si.dwNumberOfProcessors <<= 1;
		nThreadNum = (unsigned short)si.dwNumberOfProcessors;
	}

	WSAData wd;
	WSAStartup(MAKEWORD(2, 2), &wd);

	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	for (UINT i = 0; i < nThreadNum; i++)
		QueueUserWorkItem(ThreadProc, g_hIocp, WT_EXECUTELONGFUNCTION);

	return 0;
}

void COMMXAPI stop_comm_loop()
{
	if ((--g_nCommxRef) != (long)0)
		return;

	for (int i = 0; i < g_nThreadCount; i++)
		::PostQueuedCompletionStatus(g_hIocp, 0, 0, 0);
	while (g_nThreadCount)
	{
		Sleep(20);
		::PostQueuedCompletionStatus(g_hIocp, 0, 0, 0);
	}

	CloseHandle(g_hIocp);
	WSACleanup();
	CloseHandle(g_hCommThread);
	g_hCommThread = NULL;
}

bool COMMXAPI InitializeTimerQueue()
{
	g_hTimerQueue = CreateTimerQueue();
    if (!g_hTimerQueue)
        return false;
	return true;
}

void COMMXAPI UninitializeTimerQueue()
{
	DeleteTimerQueueEx(g_hTimerQueue, INVALID_HANDLE_VALUE);
}

BOOL COMMXAPI post_completion(unsigned int nTransBytes, unsigned long key, IAsyncOverlappedResult *lpResult)
{
	return ::PostQueuedCompletionStatus(g_hIocp, nTransBytes, key, lpResult);
}
