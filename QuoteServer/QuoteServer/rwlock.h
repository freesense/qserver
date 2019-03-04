
#ifndef __MULTI_READ_SINGLE_WRITE_H__
#define __MULTI_READ_SINGLE_WRITE_H__

#ifdef _WIN32
class RWLock
{
public:
	RWLock()
	{
		m_nWaitingReaders = 0;
		m_nWaitingWriters = 0;
		m_nActive = 0;
		m_nReadTimeout = INFINITE;
		m_nWriteTimeout = INFINITE;
		m_hsemReaders = CreateSemaphore(NULL, 0, MAXLONG, NULL);
		m_hsemWriters = CreateSemaphore(NULL, 0, MAXLONG, NULL);
		InitializeCriticalSection(&m_cs);
	};
	~RWLock()
	{
		m_nWaitingReaders = 0;
		m_nWaitingWriters = 0;
		m_nActive = 0;
		DeleteCriticalSection(&m_cs);
		CloseHandle(m_hsemReaders);
		CloseHandle(m_hsemWriters);
	};

	//阻塞模式等候读资源
	bool WaitRead()
	{
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
	};

	//阻塞模式等候写资源
	bool WaitWrite()
	{
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
	};

	void EndRead()
	{
		done();
	}

	void EndWrite()
	{
		done();
	}

	void SetTimeout(long nRead, long nWrite)
	{
		m_nReadTimeout = nRead;
		m_nWriteTimeout = nWrite;
	}

private:
	//读写完毕调用释放锁
	void done()
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
	};

	//等候读的用户数
	long m_nWaitingReaders;
	//等候写的用户数
	long m_nWaitingWriters;
	//工作中的用户数
	long m_nActive;
	//等候读写锁的时间
	long m_nReadTimeout, m_nWriteTimeout;

	CRITICAL_SECTION m_cs;
	HANDLE m_hsemReaders;
	HANDLE m_hsemWriters;

	RWLock(const RWLock&);
    RWLock& operator= (const RWLock&);
};
#elif defined _POSIX_C_SOURCE
#include <pthread.h>
class RWLock
{
public :
    RWLock(): rd_cnt(0),wr_cnt(0)
    {
        pthread_mutex_init(&cnt_mutex, NULL);
        pthread_cond_init(&rw_cond, NULL);
    }

    ~RWLock()
    {
        pthread_mutex_destroy(&cnt_mutex);
        pthread_cond_destroy(&rw_cond);
    }

    void WaitRead()
    {
        pthread_mutex_lock(&cnt_mutex);
        while (wr_cnt >0)
            pthread_cond_wait(&rw_cond,&cnt_mutex);
        rd_cnt++;
        pthread_mutex_unlock(&cnt_mutex);
    }

    void EndRead()
    {
        pthread_mutex_lock(&cnt_mutex);
        rd_cnt--;
        if (0 == rd_cnt)
            pthread_cond_signal(&rw_cond);
        pthread_mutex_unlock(&cnt_mutex);
    }

    void WaitWrite()
    {
        pthread_mutex_lock(&cnt_mutex);
        while (rd_cnt+wr_cnt>0)
            pthread_cond_wait(&rw_cond,&cnt_mutex);
        wr_cnt++;
        pthread_mutex_unlock(&cnt_mutex);
    }

    void EndWrite()
    {
        pthread_mutex_lock(&cnt_mutex);
        wr_cnt--;
        pthread_cond_broadcast(&rw_cond);
        pthread_mutex_unlock(&cnt_mutex);
    }

private :
    pthread_mutex_t cnt_mutex;
    pthread_cond_t rw_cond;
    int rd_cnt, wr_cnt;

    RWLock(const RWLock&);
    RWLock& operator= (const RWLock&);
};
#endif

#endif
