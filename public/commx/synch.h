
#ifndef __SYNCH_H__
#define __SYNCH_H__

#include "commxapi.h"

#ifdef _WIN32
#include <windows.h>
typedef long atomic_t;
#elif defined _POSIX_C_SOURCE
#include <pthread.h>
#endif

class COMMXAPI atomint
{
public:
	atomint();

	atomint(const atomic_t &vi);
	atomint& operator=(const atomic_t &vi);

	atomint(atomint &ai);
	atomint& operator=(atomint &ai);

	long value();
	operator long();

	bool operator==(long ovalue);
	bool operator!=(long ovalue);
	bool operator>(long ovalue);
	bool operator<(long ovalue);

	bool operator==(atomint &ovalue);
	bool operator!=(atomint &ai);
	bool operator>(atomint &ai);
	bool operator<(atomint &ai);

	atomint& operator++();
	atomint& operator--();
	atomint& operator+=(long ovalue);
	atomint& operator-=(long ovalue);
	atomint& operator+(long ovalue);
	atomint& operator-(long ovalue);

private:
	atomic_t ref;
};

class COMMXAPI LockNone
{
public:
	void lock() {};
	void unlock() {};
	bool WaitRead() {return false;};
	bool WaitWrite() {return false;};
	void EndRead() {};
	void EndWrite() {};
};

class COMMXAPI LockSingle
{
public:
	LockSingle();
	~LockSingle();
	void lock();
	void unlock();
	bool WaitRead();
	bool WaitWrite();
	void EndRead();
	void EndWrite();

private:
#ifdef _WIN32
	CRITICAL_SECTION m_cs;
#elif defined _POSIX_C_SOURCE
	pthread_mutex_t m_cs;
#endif
};

class COMMXAPI LockProcess
{
public:
	LockProcess();
	LockProcess(const char *lpName);
	~LockProcess();

	bool Initialize(const char *lpName);
	void lock();
	void unlock();
	bool WaitRead();
	bool WaitWrite();
	void EndRead();
	void EndWrite();

private:
#ifdef _WIN32
	HANDLE m_mtx;
#elif defined _POSIX_C_SOURCE
#endif
};

class COMMXAPI LockRW
{
public:
	LockRW();
	~LockRW();

	void lock();
	void unlock();
	bool WaitRead();
	bool WaitWrite();
	void EndRead();
	void EndWrite();

private:
	long m_nWaitingReaders;			/**< 等候读的用户数 */
	long m_nWaitingWriters;			/**< 等候写的用户数 */

#ifdef _WIN32
	void done();
	void SetTimeout(long nRead, long nWrite);

	long m_nActive;					/**< 工作中的用户数 */
	long m_nReadTimeout, m_nWriteTimeout;	/**< 等候读写锁的时间 */
	
	CRITICAL_SECTION m_cs;
	HANDLE m_hsemReaders;
	HANDLE m_hsemWriters;
#elif defined _POSIX_C_SOURCE
	pthread_mutex_t cnt_mutex;
    pthread_cond_t rw_cond;
#endif

	LockRW(const LockRW&);
    LockRW& operator= (const LockRW&);
};

template <class T> class Guard
{
public:
	Guard(T &lock, bool bRead)
		: m_lock(lock), m_bRead(bRead)
	{
		if (m_bRead)
			m_lock.WaitRead();
		else
			m_lock.WaitWrite();
	}

	~Guard()
	{
		if (m_bRead)
			m_lock.EndRead();
		else
			m_lock.EndWrite();
	}

private:
	bool m_bRead;
	T &m_lock;
};

#define RGUARD(Type, Lock, Obj) Guard<Type> Obj(Lock, true)
#define WGUARD(Type, Lock, Obj) Guard<Type> Obj(Lock, false)

#endif
