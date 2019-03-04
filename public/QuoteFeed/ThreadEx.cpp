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
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}