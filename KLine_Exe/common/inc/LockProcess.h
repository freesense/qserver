//CLockProcess.h
//
#ifndef _LOCKPROCESS_H_
#define _LOCKPROCESS_H_

/////////////////////////////////////////////////////////////////////////////
class CLockProcess
{
public:
	CLockProcess();
	CLockProcess(const char *lpName);
	virtual ~CLockProcess();

	void lock();
	void unlock();
	bool WaitRead();
	bool WaitWrite();
	void EndRead();
	void EndWrite();
	bool Initialize(const char *lpName);
private:
	HANDLE   m_mtx;

};

/////////////////////////////////////////////////////////////////////////////
#endif