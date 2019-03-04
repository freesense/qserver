// ThreadEx.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "ThreadEx.h"
#include <process.h>

//////////////////////////////////////////////////////////////////////////////////////////////

CThreadEx::CThreadEx()
{
	m_hThread = NULL;
	m_bRun    = FALSE;
}

CThreadEx::~CThreadEx()
{
   Stop();
}

BOOL CThreadEx::Start()
{
	if(m_hThread)
		return FALSE;

	UINT dwThreadID = 0;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProc, this, 0, &dwThreadID);
	if(m_hThread)
	{
		m_bRun = TRUE;
		return TRUE;
	}

	return FALSE;
}

UINT CThreadEx::_ThreadProc(LPVOID pParam)
{
	CThreadEx* pThread = (CThreadEx*)pParam;
	pThread->Run();
	return 0xDEAD0001;
}

void CThreadEx::Run()
{
  // int i=0;
}

void CThreadEx::Stop()
{
	m_bRun = FALSE;
	if(m_hThread)
	{
		WaitThreadExitEx(m_hThread);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void CThreadEx::WaitThread()
{
	WaitThreadExitEx(m_hThread);
}

//有些子线程需要向主线程SendMessage,
//如果在主线程中调用WaitForSingleObject等待这些线程结束,而子线程恰好调用了SendMessage,则会形成死锁.
//为此,可以调用WaitThreadExitEx,WaitThreadExitEx在等待子线程状态变化的同时能处理主线程中的消息.
//WaitThreadExitEx能过滤等待过程中的用户输入,如mouse,keyboard.
BOOL CThreadEx::WaitThreadExitEx(HANDLE handle)
{
	while (TRUE)
    {
        DWORD result ; 
        MSG msg ; 
		
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
        {
           	if( (msg.message>= WM_MOUSEFIRST && msg.message<=WM_MOUSELAST)
				|| (msg.message>= WM_KEYFIRST && msg.message<=WM_KEYLAST)
				)
			{
				//skip user input
			}
			else
			{
				DispatchMessage(&msg); 
			}
        }
		
        HANDLE *lphObjects =&handle;
		DWORD cObjects=1;
		// Wait for any message sent or posted to this queue or for one of the passed handles be set to signaled.
        result = MsgWaitForMultipleObjects(cObjects, lphObjects,FALSE, INFINITE, 
			//QS_ALLINPUT
			QS_SENDMESSAGE 
			| QS_TIMER 
			| QS_PAINT 
			| QS_ALLPOSTMESSAGE
			
			); 
		
        // The result tells us the type of event we have.
        if (result == (WAIT_OBJECT_0 + cObjects))
        {
            //DT(" New messages have arrived. Dispatch message");
            // Continue to the top of the always while loop to 
            // dispatch them and resume waiting.
            continue;
        }
        else 
        { 
            //DW("One of the handles became signaled.,exit MessageLoop");
			return TRUE;
            //DoStuff (result - WAIT_OBJECT_0) ; 
        } // End of else clause.
    }
}