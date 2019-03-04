
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
	XThread(unsigned short check_time = 0);		// check_time!=0时，将启动定时器定时检查线程运行情况，单位为秒

	virtual unsigned int open(unsigned int initFlag = 0, unsigned int trdnum = 1);
	virtual void close() = 0;
	void wait();

	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);
	virtual unsigned int __stdcall svc(void* lpParam) = 0;
	virtual int OnThreadFrozen(int thread_id) {return 0;};	// 线程计数器停止更新后将被定时器回调本函数

	void run();
	void pause();
	pthread_t GetThreadID();
	pthread_t GetThreadID(unsigned int idx);

#ifdef _WIN32
	void SetThreadPriority(pthread_t tid, int nPriority);
#endif

protected:
	void UpdateThreadStatus();			// 提供给派生类在svc中调用，用以更新线程运行计数器，应用可以判断线程是否还在正常运行

#ifdef _WIN32
	unsigned long *m_pThreadHandler;
#endif
	_xthread_status *m_pThreadStatus;	// 线程ID及状态

	unsigned int m_nThreadNum;			// 成功创建的线程数
	unsigned short m_nCheckTime;		// 线程检查时间间隔(秒)
	long m_lTimerId;					// 线程检查定时器id
};

#endif
