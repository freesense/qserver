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
	return ReadLockFile(0, 0, -1, -1);
}

BOOL CLockFile::WriteLockFile()
{
	return WriteLockFile(0, 0, -1, -1);
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
	return UnlockFile(0, 0, -1, -1);
}