// LockFile.h : ͷ�ļ�
//
#ifndef _LOCKFILE_H_
#define _LOCKFILE_H_

/**********************************************************************
����˵��:
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
    //ReadLockFile() ���Զ���߳�ͬʱ������������д
	BOOL ReadLockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh);
	//WriteLockFile() ֻ����һ���߳�д�������̲߳��ɶ�Ҳ����д
	BOOL WriteLockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh);
    //���������ļ�
	BOOL ReadLockFile();
    //���������ļ�
	BOOL WriteLockFile();
    //���������ã�
	BOOL UnlockFile(DWORD dwOffsetLow, DWORD dwOffsetHigh, DWORD dwNumLow, DWORD dwNumHigh);
	//���������ļ������ã�
	BOOL UnlockFile();

private:
	HANDLE           m_hFile;    //���������ļ����������CreateFile()���
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