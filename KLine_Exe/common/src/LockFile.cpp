// LockFile.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "LockFile.h"

//////////////////////////////////////////////////////////////////////////////////////////////

BOOL CLockFile::ReadLockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh)
{
	OVERLAPPED overlapped;
	overlapped.Offset = dwOffsetLow;
	overlapped.OffsetHigh = dwOffsetHigh;
	overlapped.hEvent = NULL;

	return ::LockFileEx(m_hFile, NULL, 0, dwNumLow, dwNumHigh, &overlapped);
}

BOOL CLockFile::WriteLockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh)
{
	OVERLAPPED overlapped;
	overlapped.Offset = dwOffsetLow;
	overlapped.OffsetHigh = dwOffsetHigh;
	overlapped.hEvent = NULL;

	return ::LockFileEx(m_hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, dwNumLow, dwNumHigh, &overlapped);
}

BOOL CLockFile::ReadLockFile()
{
	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = GetFileSize(m_hFile, &dwSizeHigh);

	return ReadLockFile(0, 0, dwSizeLow, dwSizeHigh);
}

BOOL CLockFile::WriteLockFile()
{
	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = GetFileSize(m_hFile, &dwSizeHigh);

	return WriteLockFile(0, 0, dwSizeLow, dwSizeHigh);
}

BOOL CLockFile::UnlockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh)
{
	OVERLAPPED overlapped;
	overlapped.Offset = dwOffsetLow;
	overlapped.OffsetHigh = dwOffsetHigh;
	overlapped.hEvent = NULL;

	return ::UnlockFileEx(m_hFile, 0, dwNumLow, dwNumHigh, &overlapped);
}

BOOL CLockFile::UnlockFile()
{
	DWORD dwSizeHigh = 0;
	DWORD dwSizeLow = GetFileSize(m_hFile, &dwSizeHigh);

	return UnlockFile(0, 0, dwSizeLow, dwSizeHigh);
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
CCriticalSection::CCriticalSection()
{
	::InitializeCriticalSection(&m_cs);
}

CCriticalSection::~CCriticalSection()
{
	::DeleteCriticalSection(&m_cs);
}

void CCriticalSection::Lock()
{
	::EnterCriticalSection(&m_cs);
}

void CCriticalSection::Unlock()
{
	::LeaveCriticalSection(&m_cs);
}