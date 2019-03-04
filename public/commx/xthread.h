
#ifndef __XTHREAD_H__
#define __XTHREAD_H__

#include "commxapi.h"
#include "synch.h"
#include "commxbase.h"

#ifdef _WIN32
	typedef unsigned int pthread_t;
#elif defined _POSIX_C_SOURCE
	#include <pthread.h>
	#define __stdcall
#endif

class COMMXAPI XThread : public ITimerBase
{
	typedef unsigned(__stdcall XThread::*Thread)(void*);

	struct _xthread_status
	{
		pthread_t m_thread_id;
		atomint m_count;
	};

public:
	XThread(unsigned short check_time = 0);		// check_time!=0ʱ����������ʱ����ʱ����߳������������λΪ��

	virtual unsigned int open(unsigned int initFlag = 0, unsigned int trdnum = 1);
	virtual void close() = 0;
	void wait();

	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);
	virtual unsigned int __stdcall svc(void* lpParam) = 0;
	virtual int OnThreadFrozen(int thread_id) {return 0;};	// �̼߳�����ֹͣ���º󽫱���ʱ���ص�������

	void run();
	void pause();
	pthread_t GetThreadID();
	pthread_t GetThreadID(unsigned int idx);

#ifdef _WIN32
	void SetThreadPriority(pthread_t tid, int nPriority);
#endif

protected:
	void UpdateThreadStatus();			// �ṩ����������svc�е��ã����Ը����߳����м�������Ӧ�ÿ����ж��߳��Ƿ�����������

#ifdef _WIN32
	unsigned long *m_pThreadHandler;
#endif
	_xthread_status *m_pThreadStatus;	// �߳�ID��״̬

	unsigned int m_nThreadNum;			// �ɹ��������߳���
	unsigned short m_nCheckTime;		// �̼߳��ʱ����(��)
	long m_lTimerId;					// �̼߳�鶨ʱ��id
};

#endif
