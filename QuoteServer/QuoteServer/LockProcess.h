#pragma once
#include "atlconv.h" 

class  LockProcess
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
    HANDLE m_mtx;
};

inline LockProcess::LockProcess()
{
    m_mtx = NULL;
}

inline LockProcess::LockProcess(const char *lpName)
{
    Initialize(lpName);
}

inline LockProcess::~LockProcess()
{
    if (m_mtx)
        ::CloseHandle(m_mtx);
}

inline bool LockProcess::Initialize(const char *lpName)
{
	USES_CONVERSION; 
    //m_mtx = ::CreateMutex(NULL, FALSE, lpName);
	m_mtx = ::CreateMutex(NULL, FALSE, A2W(lpName));
    if (m_mtx == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
        m_mtx = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, A2W(lpName));
    if (m_mtx == NULL)
        return false;
    return TRUE;
}

inline void LockProcess::lock()
{
    ::WaitForSingleObject(m_mtx, INFINITE);
}

inline void LockProcess::unlock()
{
    ::ReleaseMutex(m_mtx);
}

inline bool LockProcess::WaitRead()
{
    lock();
    return true;
}

inline bool LockProcess::WaitWrite()
{
    lock();
    return true;
}

inline void LockProcess::EndRead()
{
    unlock();
}

inline void LockProcess::EndWrite()
{
    unlock();
}
