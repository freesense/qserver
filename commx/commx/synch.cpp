
#include "stdafx.h"
#include "../public/synch.h"

////////////////////////////////////////////////////////////////////////////////////////////////
atomint::atomint()
{
#ifdef _WIN32
	::InterlockedExchange(&ref, 0);
#elif defined _POSIX_C_SOURCE
	atomic_set(&ref, 0);
#endif
}

atomint::atomint(const atomic_t &vi)
{
#ifdef _WIN32
	::InterlockedExchange(&ref, vi);
#elif defined _POSIX_C_SOURCE
	atomic_set(&ref, atomic_read(&vi));
#endif
}

atomint::atomint(atomint &ai)
{
#ifdef _WIN32
	::InterlockedExchange(&ref, ai.value());
#elif defined _POSIX_C_SOURCE
	atomic_set(&ref, ai.value());
#endif
}

atomint& atomint::operator=(const atomic_t &vi)
{
#ifdef _WIN32
	::InterlockedExchange(&ref, vi);
#elif defined _POSIX_C_SOURCE
	atomic_set(&ref, atomic_read(&vi));
#endif
	return *this;
}

atomint& atomint::operator=(atomint &ai)
{
#ifdef _WIN32
	::InterlockedExchange(&ref, ai.value());
#elif defined _POSIX_C_SOURCE
	atomic_set(&ref, ai.value());
#endif
	return *this;
}

long atomint::value()
{
#ifdef _WIN32
	return ref;
#elif defined _POSIX_C_SOURCE
	return atomic_read(&ref);
#endif
}

atomint::operator long()
{
	return value();
}

bool atomint::operator==(long ovalue)
{
#ifdef _WIN32
	return (ovalue == ref);
#elif defined _POSIX_C_SOURCE
	return (ovalue == atomic_read(&ref));
#endif
}

bool atomint::operator!=(long ovalue)
{
#ifdef _WIN32
	return (ovalue != ref);
#elif defined _POSIX_C_SOURCE
	return (ovalue != atomic_read(&ref));
#endif
}

bool atomint::operator>(long ovalue)
{
#ifdef _WIN32
	return (ref > ovalue);
#elif defined _POSIX_C_SOURCE
	return (atomic_read(&ref) > ovalue);
#endif
}

bool atomint::operator<(long ovalue)
{
#ifdef _WIN32
	return (ref < ovalue);
#elif defined _POSIX_C_SOURCE
	return (atomic_read(&ref) < ovalue);
#endif
}

bool atomint::operator==(atomint &ai)
{
#ifdef _WIN32
	return (ai.value() == ref);
#elif defined _POSIX_C_SOURCE
	return (ai.value() == atomic_read(&ref));
#endif
}

bool atomint::operator!=(atomint &ai)
{
#ifdef _WIN32
	return (ai.value() != ref);
#elif defined _POSIX_C_SOURCE
	return (ai.value() != atomic_read(&ref));
#endif
}

bool atomint::operator>(atomint &ai)
{
#ifdef _WIN32
	return (ref > ai.value());
#elif defined _POSIX_C_SOURCE
	return (atomic_read(&ref) > ai.value());
#endif
}

bool atomint::operator<(atomint &ai)
{
#ifdef _WIN32
	return (ref < ai.value());
#elif defined _POSIX_C_SOURCE
	return (atomic_read(&ref) < ai.value());
#endif
}

atomint& atomint::operator++()
{
#ifdef _WIN32
	::InterlockedIncrement(&ref);
#elif defined _POSIX_C_SOURCE
	atomic_inc(&ref);
#endif
	return *this;
}

atomint& atomint::operator--()
{
#ifdef _WIN32
	::InterlockedDecrement(&ref);
#elif defined _POSIX_C_SOURCE
	return atomic_dec(&ref);
#endif
	return *this;
}

atomint& atomint::operator+=(long ovalue)
{
#ifdef _WIN32
	::InterlockedExchangeAdd(&ref, ovalue);
#elif defined _POSIX_C_SOURCE
	atomic_add_return(ovalue, &ref);
#endif
	return *this;
}

atomint& atomint::operator-=(long ovalue)
{
#ifdef _WIN32
	::InterlockedExchangeAdd(&ref, -1 * ovalue);
#elif defined _POSIX_C_SOURCE
	atomic_sub_return(ovalue, &ref);
#endif
	return *this;
}

atomint& atomint::operator+(long ovalue)
{
#ifdef _WIN32
	::InterlockedExchangeAdd(&ref, ovalue);
#elif defined _POSIX_C_SOURCE
	atomic_inc(ovalue, &ref);
#endif
	return *this;
}

atomint& atomint::operator-(long ovalue)
{
#ifdef _WIN32
	::InterlockedExchangeAdd(&ref, -1 * ovalue);
#elif defined _POSIX_C_SOURCE
	atomic_dec(ovalue, &ref);
#endif
	return *this;
}

//////////////////////////////////////////////////////////////////////////////////
LockSingle::LockSingle()
{
#ifdef _WIN32
	::InitializeCriticalSection(&m_cs);
#elif defined _POSIX_C_SOURCE
	pthread_mutex_init(&m_cs, NULL);
#endif
}

LockSingle::~LockSingle()
{
#ifdef _WIN32
	::DeleteCriticalSection(&m_cs);
#elif defined _POSIX_C_SOURCE
	pthread_mutex_destroy(&m_cs);
#endif
}

void LockSingle::lock()
{
#ifdef _WIN32
	::EnterCriticalSection(&m_cs);
#elif defined _POSIX_C_SOURCE
	pthread_mutex_lock(&m_cs);
#endif
}

void LockSingle::unlock()
{
#ifdef _WIN32
	::LeaveCriticalSection(&m_cs);
#elif defined _POSIX_C_SOURCE
	pthread_mutex_unlock(&m_cs);
#endif
}

bool LockSingle::WaitRead()
{
	lock();
	return true;
}

bool LockSingle::WaitWrite()
{
	lock();
	return true;
}

void LockSingle::EndRead()
{
	unlock();
}

void LockSingle::EndWrite()
{
	unlock();
}

//////////////////////////////////////////////////////////////////////////////////////
LockProcess::LockProcess()
{
	m_mtx = NULL;
}

LockProcess::LockProcess(const char *lpName)
{
	Initialize(lpName);
}

LockProcess::~LockProcess()
{
	if (m_mtx)
	{
		::CloseHandle(m_mtx);
		m_mtx = NULL;
	}
}

bool LockProcess::Initialize(const char *lpName)
{
	m_mtx = ::CreateMutex(NULL, FALSE, lpName);
	if (m_mtx == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
		m_mtx = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpName);
	if (m_mtx == NULL)
		return false;
	return true;
}

void LockProcess::lock()
{
	::WaitForSingleObject(m_mtx, INFINITE);
}

void LockProcess::unlock()
{
	::ReleaseMutex(m_mtx);
}

bool LockProcess::WaitRead()
{
	lock();
	return true;
}

bool LockProcess::WaitWrite()
{
	lock();
	return true;
}

void LockProcess::EndRead()
{
	unlock();
}

void LockProcess::EndWrite()
{
	unlock();
}

//////////////////////////////////////////////////////////////////////////////////////
LockRW::LockRW()
{
	m_nWaitingReaders = 0;
	m_nWaitingWriters = 0;

#ifdef _WIN32
	m_nActive = 0;
	m_nReadTimeout = INFINITE;
	m_nWriteTimeout = INFINITE;
	m_hsemReaders = CreateSemaphore(NULL, 0, MAXLONG, NULL);
	m_hsemWriters = CreateSemaphore(NULL, 0, MAXLONG, NULL);
	InitializeCriticalSection(&m_cs);
#elif defined _POSIX_C_SOURCE
	pthread_mutex_init(&cnt_mutex, NULL);
	pthread_cond_init(&rw_cond, NULL);
#endif
}

LockRW::~LockRW()
{
	m_nWaitingReaders = 0;
	m_nWaitingWriters = 0;

#ifdef _WIN32
	m_nActive = 0;
	DeleteCriticalSection(&m_cs);
	CloseHandle(m_hsemReaders);
	CloseHandle(m_hsemWriters);
#elif defined _POSIX_C_SOURCE
	pthread_mutex_destroy(&cnt_mutex);
	pthread_cond_destroy(&rw_cond);
#endif
}

void LockRW::lock()
{
	WaitWrite();
}

void LockRW::unlock()
{
	EndWrite();
}

bool LockRW::WaitRead()
{
#ifdef _WIN32
	EnterCriticalSection(&m_cs);
	bool fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));
	if (fResourceWritePending)
		m_nWaitingReaders++;
	else
		m_nActive++;
	LeaveCriticalSection(&m_cs);

	if (fResourceWritePending && WAIT_OBJECT_0 != WaitForSingleObject(m_hsemReaders, m_nReadTimeout))
	{
		::InterlockedDecrement(&m_nWaitingReaders);
		return false;
	}
	return true;
#elif defined _POSIX_C_SOURCE
	pthread_mutex_lock(&cnt_mutex);
	while (m_nWaitingWriters >0)
		pthread_cond_wait(&rw_cond, &cnt_mutex);
	m_nWaitingReaders++;
	pthread_mutex_unlock(&cnt_mutex);
	return true;
#endif
}

bool LockRW::WaitWrite()
{
#ifdef _WIN32
	EnterCriticalSection(&m_cs);
	bool fResourceOwned = (m_nActive != 0);
	if (fResourceOwned)
		m_nWaitingWriters++;
	else
		m_nActive = -1;
	LeaveCriticalSection(&m_cs);

	if (fResourceOwned && WAIT_OBJECT_0 != WaitForSingleObject(m_hsemWriters, m_nWriteTimeout))
	{
		::InterlockedDecrement(&m_nWaitingWriters);
		return false;
	}
	return true;
#elif defined _POSIX_C_SOURCE
	pthread_mutex_lock(&cnt_mutex);
	while (m_nWaitingReaders + m_nWaitingWriters > 0)
		pthread_cond_wait(&rw_cond, &cnt_mutex);
	m_nWaitingWriters++;
	pthread_mutex_unlock(&cnt_mutex);
	return true;
#endif
};

void LockRW::EndRead()
{
#ifdef _WIN32
	done();
#elif defined _POSIX_C_SOURCE
	pthread_mutex_lock(&cnt_mutex);
	m_nWaitingReaders--;
	if (0 == m_nWaitingReaders)
		pthread_cond_signal(&rw_cond);
	pthread_mutex_unlock(&cnt_mutex);
#endif
}

void LockRW::EndWrite()
{
#ifdef _WIN32
	done();
#elif defined _POSIX_C_SOURCE
	pthread_mutex_lock(&cnt_mutex);
	m_nWaitingWriters--;
	pthread_cond_broadcast(&rw_cond);
	pthread_mutex_unlock(&cnt_mutex);
#endif
}

#ifdef _WIN32

void LockRW::done()
{
	EnterCriticalSection(&m_cs);
	
	if (m_nActive > 0)
		m_nActive--;
	else
		m_nActive++;
	
	HANDLE hsem = NULL;
	long lCount = 1;
	
	if (m_nActive == 0)
	{
		if (m_nWaitingWriters > 0)
		{
			m_nActive = -1;
			m_nWaitingWriters--;
			hsem = m_hsemWriters;
		}
		else if (m_nWaitingReaders > 0)
		{
			m_nActive = m_nWaitingReaders;
			m_nWaitingReaders = 0;
			hsem = m_hsemReaders;
			lCount = m_nActive;
		}
	}
	
	LeaveCriticalSection(&m_cs);
	
	if (hsem != NULL)
		ReleaseSemaphore(hsem, lCount, NULL);
}

void LockRW::SetTimeout(long nRead, long nWrite)
{
	m_nReadTimeout = nRead;
	m_nWriteTimeout = nWrite;
}

#endif
