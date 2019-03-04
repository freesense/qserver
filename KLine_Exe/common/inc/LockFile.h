// LockFile.h : 头文件
//
#ifndef _LOCKFILE_H_
#define _LOCKFILE_H_

/**********************************************************************
参数说明:
dwOffsetLow  : The low-order word of the starting byte offset in the file where the lock should begin
dwOffsetHigh : The high-order word of the starting byte offset in the file where the lock should begin
dwNumLow     : The low-order word of the length of the byte range to be locked
dwNumHigh    : The high-order word of the length of the byte range to be locked
***********************************************************************/
class CLockFile
{
public:
	CLockFile(){};
	CLockFile(HANDLE hFile){ m_hFile = hFile; }
	virtual ~CLockFile(){};

	void SetFileHandle(HANDLE hFile){ m_hFile = hFile; }
    //ReadLockFile() 可以多个线程同时读，但不可以写
	BOOL ReadLockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh);
	//WriteLockFile() 只可以一个线程写，其他线程不可读也不可写
	BOOL WriteLockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh);
    //锁定整个文件
	BOOL ReadLockFile();
    //锁定整个文件
	BOOL WriteLockFile();
    //解锁（公用）
	BOOL UnlockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh);
	//解锁整个文件（公用）
	BOOL UnlockFile();

private:
	HANDLE           m_hFile;    //待锁定的文件句柄，可由CreateFile()获得
};

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
class CCriticalSection
{
public:
	CCriticalSection();
	virtual ~CCriticalSection();

	void Lock();
	void Unlock();
protected:

private:
	CRITICAL_SECTION   m_cs;

};
/////////////////////////////////////////////////////////////////////////////////////////
#endif