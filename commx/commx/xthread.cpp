
#include "stdafx.h"
#include "../public/xthread.h"
#include <string.h>
#include <assert.h>
#ifdef _WIN32
	#include <windows.h>
	#include <process.h>
#endif

#pragma warning(disable:4312 4244)

XThread::XThread(unsigned short check_time)
: m_pThreadStatus(NULL), m_nCheckTime(check_time), m_lTimerId(0)
{
#ifdef _WIN32
	m_pThreadHandler = NULL;
#endif
}

unsigned int XThread::open(unsigned int initFlag, unsigned int trdnum)
{
	m_nThreadNum = 0;
	Thread temp = &XThread::svc;
	unsigned long TempAddr;
	memcpy(&TempAddr, &temp, sizeof(unsigned long));

	int nThread = 0;
	m_pThreadStatus = new _xthread_status[trdnum];
#ifdef _WIN32
	m_pThreadHandler = new unsigned long[trdnum];
#endif

	for (unsigned int i = 0; i < trdnum; i++)
	{
#ifdef _WIN32
		m_pThreadHandler[nThread] = _beginthreadex(0, 0,
			(unsigned(__stdcall*)(void*))TempAddr, (void*)this,
			initFlag, &m_pThreadStatus[i].m_thread_id);
		if (m_pThreadHandler[nThread])
		{
			nThread++;
			m_nThreadNum++;
		}
#elif defined _POSIX_C_SOURCE
		pthread_attr_t sPthreadAttr;
		pthread_attr_init(&sPthreadAttr);
		if (0 == pthread_create(&m_pThreadStatus[nThread].m_thread_id,&sPthreadAttr,TempAddr,(void*)this))
		{
			nThread++;
			m_nThreadNum++;
		}
		pthread_attr_destroy(&sPthreadAttr);
#endif
	}

	if (m_nCheckTime != 0 && initFlag == 0)
		m_lTimerId = RegisterTimer(NULL, m_nCheckTime, m_nCheckTime);
	return m_nThreadNum;
}

void XThread::wait()
{
	if (m_lTimerId)
	{
		CancelTimer(m_lTimerId);
		m_lTimerId = 0;
	}

#ifdef _WIN32
	unsigned int i = 0;
	while (1)
	{
		int count = m_nThreadNum - i >= 64 ? 64 : m_nThreadNum - i;
		WaitForMultipleObjects(count, (HANDLE*)(&m_pThreadHandler[i]), TRUE, INFINITE);
		for (int m = 0; m < count; m++)
			::CloseHandle((HANDLE)(m_pThreadHandler[i+m]));
		i += count;
		if (i >= m_nThreadNum)
			break;
	}

	delete []m_pThreadHandler;
#elif defined _POSIX_C_SOURCE
	for (unsigned int i = 0; i < m_nThreadNum; i++)
		pthread_join(m_pThreadStatus[i].m_thread_id, NULL);
#endif

	delete []m_pThreadStatus;
}

void XThread::run()
{
	for (unsigned int i = 0; i < m_nThreadNum; i++)
	{
#ifdef _WIN32
		ResumeThread((HANDLE)m_pThreadHandler[i]);
#elif defined _POSIX_C_SOURCE
#endif
	}

	if (m_nCheckTime != 0)
		m_lTimerId = RegisterTimer(NULL, m_nCheckTime, m_nCheckTime);
}

void XThread::pause()
{
	if (m_lTimerId)
	{
		CancelTimer(m_lTimerId);
		m_lTimerId = 0;
	}

	for (unsigned int i = 0; i < m_nThreadNum; i++)
	{
#ifdef _WIN32
		SuspendThread((HANDLE)m_pThreadHandler[i]);
#elif defined _POSIX_C_SOURCE
#endif
	}
}

pthread_t XThread::GetThreadID()
{
#ifdef _WIN32
	return GetCurrentThreadId();
#elif defined _POSIX_C_SOURCE
	return pthread_self();
#endif
}

pthread_t XThread::GetThreadID(unsigned int idx)
{
	assert(idx < m_nThreadNum);
	return m_pThreadStatus[idx].m_thread_id;
}

#ifdef _WIN32
void XThread::SetThreadPriority(pthread_t tid, int nPriority)
{
	for (unsigned int i = 0; i < m_nThreadNum; i++)
	{
		if (tid == m_pThreadStatus[i].m_thread_id)
		{
			SetThreadPriority(m_pThreadHandler[i], nPriority);
			return;
		}
	}
}
#endif

void XThread::UpdateThreadStatus()
{
	pthread_t id = GetThreadID();
	for (unsigned int i = 0; i < m_nThreadNum; i++)
	{
		if (id == m_pThreadStatus[i].m_thread_id)
		{
			m_pThreadStatus[i].m_count += 1;
			break;
		}
	}
}

int XThread::handle_timeout(ITimerCallbackBase *pTimerParam)
{
	for (unsigned int i = 0; i < m_nThreadNum; i++)
	{
		if (m_pThreadStatus[i].m_count != (long)0)
			m_pThreadStatus[i].m_count = 0;
		else
			OnThreadFrozen(m_pThreadStatus[i].m_thread_id);
	}
	return 0;
}
