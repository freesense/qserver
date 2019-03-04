//WorkThread.h
#ifndef _WORKTHREAD_H_
#define _WORKTHREAD_H_

///////////////////////////////////////////////////////////////////
typedef unsigned int(_stdcall *PTHREADFUN)(void* pParam);
class CWorkThread
{
public:
	CWorkThread();
	CWorkThread(PTHREADFUN pThreadFun, void* param);
	virtual ~CWorkThread();

	bool Start();
	void Stop();
protected:

private:
	void*   m_hThread;         //线程句柄
	void*   m_pParam;

	PTHREADFUN   m_pThreadFun; //函数指针
};
///////////////////////////////////////////////////////////////////
#endif