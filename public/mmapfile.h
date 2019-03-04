
#ifndef __MMAP_FILE_H__
#define __MMAP_FILE_H__

#pragma warning(disable:4786)

#include <assert.h>
#include <utility>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
	#ifndef _WINDOWS_
		#include <windows.h>
	#endif
#elif defined _POSIX_C_SOURCE
	#include <fcntl.h>
	#include <sys/mman.h>
	#include <unistd.h>
#endif

template <class T>
class CMMapFile
{
public:
	bool mapfile(const char* lpszFileName, unsigned int nCount)
	{
		m_nTotalBlock = nCount;
		unsigned long nSize = sizeof(m_pnIndex) + sizeof(T) * nCount;

		bool bFileExists = true;
		struct stat fstat;
		if (-1 == stat(lpszFileName, &fstat))
			bFileExists = false;

#ifdef _WIN32
#ifdef UNICODE
		m_hFile = CreateFileA(lpszFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		m_hFile = CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
		m_hMappingFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, nSize, NULL);
		m_lpData = (char*)MapViewOfFile(m_hMappingFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#elif defined _POSIX_C_SOURCE
		m_fd = open(lpszFileName, O_RDWR|O_CREAT);
		m_lpData = (char*)mmap(0, nSize, PROT_READ|PROT_READ, MAP_SHARED, m_fd, 0);
#endif

		if (!m_lpData)
			return false;

		m_pnIndex = (int*)m_lpData;
		if (!bFileExists)
			*m_pnIndex = 0;

		return true;
	}

	~CMMapFile()
	{
#ifdef _WIN32
		if (m_lpData)
			UnmapViewOfFile(m_lpData);
		if (m_hMappingFile)
			CloseHandle(m_hMappingFile);
		if (m_hFile)
			CloseHandle(m_hFile);
#elif defined _POSIX_C_SOURCE
		munmap(m_lpData);
		close(m_fd);
#endif
	}

	inline int GetIndex()
	{
		return *m_pnIndex;
	}

	inline T* operator[](unsigned int index)
	{
		return (T*)(m_lpData + sizeof(m_pnIndex) + index * sizeof(T));
	}

	inline unsigned int GetFreeSize()
	{
		return m_nTotalBlock - (*m_pnIndex);
	}

	int GetNextIndex(unsigned int nBlock = 1)
	{
		if (*m_pnIndex == -1)
			return -1;

		int i = *m_pnIndex;
		*m_pnIndex += nBlock;
		if ((unsigned)(*m_pnIndex) >= m_nTotalBlock)
			*m_pnIndex = -1;
		return i;
	}

private:
#ifdef _WIN32
	HANDLE m_hFile;					//文件句柄
	HANDLE m_hMappingFile;			//映射句柄
#elif defined _POSIX_C_SOURCE
	int m_fd;						//文件描述符
#endif

	char* m_lpData;					//映射的内存首地址
	int* m_pnIndex;					//空闲内存块索引
	unsigned m_nTotalBlock;			//总共的内存块数量
};

#endif
