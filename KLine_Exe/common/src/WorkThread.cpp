//WorkThread.cpp
//
#include "stdafx.h"
#include "WorkThread.h"
#include <process.h>

////////////////////////////////////////////////////////////////////////
CWorkThread::CWorkThread()
{
	m_hThread  = NULL;
	m_pThreadFun = NULL;
	m_pParam   = NULL;
}

CWorkThread::CWorkThread(PTHREADFUN pThreadFun, void* pParam)
{
	m_hThread  = NULL;
	m_pThreadFun = pThreadFun;
	m_pParam   = pParam;
}

CWorkThread::~CWorkThread()
{
	Stop();
}

bool CWorkThread::Start()
{
	if (m_hThread != NULL)
		return false;

	m_hThread = (void*)_beginthreadex(NULL, 0, m_pThreadFun, m_pParam, 0, NULL);
	if (m_hThread == NULL)
		return false;

	return true;
}

void CWorkThread::Stop()
{
	if (m_hThread != NULL)
	{
		WaitForSingleObject(m_hThread);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void CWorkThread::WaitStop()
{
	if (m_hThread != NULL)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

}
void CWorkThread::WaitForSingleObject(HANDLE hHandle)
{
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
}
