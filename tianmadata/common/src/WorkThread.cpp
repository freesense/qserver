//WorkThread.cpp
#include "stdafx.h"
#include "WorkThread.h"
#include <process.h>

///////////////////////////////////////////////////////////////////
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
}

bool CWorkThread::Start()
{
	if (m_hThread != NULL || m_pThreadFun == NULL)
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
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}