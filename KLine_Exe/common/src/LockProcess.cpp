//CLockProcess.cpp
#include "stdafx.h"
#include "LockProcess.h"

//////////////////////////////////////////////////////////////////
CLockProcess::CLockProcess()
{
    m_mtx = NULL;
}

CLockProcess::CLockProcess(const char *lpName)
{
    Initialize(lpName);
}

CLockProcess::~CLockProcess()
{
    if (m_mtx)
        ::CloseHandle(m_mtx);
}

bool CLockProcess::Initialize(const char *lpName)
{
    m_mtx = ::CreateMutex(NULL, FALSE, lpName);
    if (m_mtx == NULL && GetLastError() == ERROR_ALREADY_EXISTS)
        m_mtx = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, lpName);
    if (m_mtx == NULL)
        return false;
    return TRUE;
}

void CLockProcess::lock()
{
    ::WaitForSingleObject(m_mtx, INFINITE);
}

void CLockProcess::unlock()
{
    ::ReleaseMutex(m_mtx);
}

bool CLockProcess::WaitRead()
{
    lock();
    return true;
}

bool CLockProcess::WaitWrite()
{
    lock();
    return true;
}

void CLockProcess::EndRead()
{
    unlock();
}

void CLockProcess::EndWrite()
{
    unlock();
}
