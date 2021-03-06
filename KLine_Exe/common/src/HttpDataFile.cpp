//HttpDataFile.cpp
//
#include "stdafx.h"
#include "HttpDataFile.h"

#define FIELD_COUNT       10
/////////////////////////////////////////////////////////////////////////////
CHttpDataFile::CHttpDataFile()
{
	m_hFile = NULL;
}

CHttpDataFile::~CHttpDataFile()
{
}

BOOL CHttpDataFile::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
					  DWORD dwCreationDisposition)
{
	if(m_hFile)
		Close();

	m_hFile = CreateFile(pFileName, dwDesiredAccess, dwShareMode
		, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

	if(m_hFile == INVALID_HANDLE_VALUE)
	{
		REPORT_RUN(MN, T("Open file failed, ErrCode:%d\n", GetLastError()), RPT_ERROR, return FALSE);
	}

	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagHttpKFileInfo);
		m_nFieldCount = FIELD_COUNT;
		if(m_pFieldInfo != NULL)
		{
			delete m_pFieldInfo;
			m_pFieldInfo = NULL;
		}

		m_pFieldInfo = new tagFieldInfo[FIELD_COUNT];
		for (int i=0; i<FIELD_COUNT; i++)
		{
			m_pFieldInfo[i].FieLen = sizeof(tagHttpKFileInfo) / FIELD_COUNT;
			m_pFieldInfo[i].BegPos = i*m_pFieldInfo[i].FieLen;
		}
		   
		DWORD dwLowSize = GetFileSize(m_hFile, NULL);
		if (dwLowSize != INVALID_FILE_SIZE)
			m_nRecordCount = dwLowSize / sizeof(tagHttpKFileInfo);
	}

	m_hLockFile.SetFileHandle(m_hFile);
	return Requery();
}

void CHttpDataFile::Close()
{
	m_hLockFile.UnlockFile();

	if(m_hFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}

	CBaseFile::Close();
}

BOOL CHttpDataFile::Requery()
{
	BOOL bRet = m_hLockFile.ReadLockFile();
	if(!bRet)
	{
		REPORT_RUN(MN, T("lock file failed\n"), RPT_ERROR, {Close(); return FALSE;});
	}
	
	SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
	//
	if(!AllocateMemory())
	{
		m_hLockFile.UnlockFile();
		Close();
		return FALSE;
	}
	//
	
	DWORD dwReaded = 0;
	ReadFile(m_hFile, m_pRecordBuf, m_nRecordBufLen, &dwReaded, NULL);
	if(dwReaded != m_nRecordBufLen)
	{
		REPORT(MN, T("read record failed\n"), RPT_ERROR);
		m_hLockFile.UnlockFile();
		Close();
		return FALSE;
	}
	
	bRet = m_hLockFile.UnlockFile();
	if(!bRet)
	{
		REPORT_RUN(MN, T("unlock file failed\n"), RPT_ERROR, {Close(); return FALSE;});
	}
	
	MoveFirst();
	return TRUE;
}

BOOL CHttpDataFile::WriteData()
{
	if (!IsOpen())
		return FALSE;

	DWORD dwSizeLow = GetFileSize(m_hFile, NULL);
	DWORD dwNumLow = dwSizeLow > m_nRecordBufLen ? dwSizeLow : m_nRecordBufLen;

	DWORD dwWritten = 0;

	CLockFile  hLockFile(m_hFile);
	BOOL bRet = hLockFile.WriteLockFile(0, 0, dwNumLow, 0);
	if(!bRet)
	{
		REPORT_RUN(MN, T("lock file failed, ErrCode:%\n", GetLastError()), RPT_ERROR, return FALSE);
	}

	SetFilePointer(m_hFile,NULL,NULL,FILE_BEGIN);  
	SetEndOfFile(m_hFile);

	SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
	if (!WriteFile(m_hFile, m_pRecordBuf, m_nRecordBufLen, &dwWritten, NULL))
	{
		hLockFile.UnlockFile(0, 0, dwNumLow, 0);
		DWORD dwErr = GetLastError();
		return FALSE;
	}

	bRet = hLockFile.UnlockFile(0, 0, dwNumLow, 0);
	if(!bRet)
	{
		REPORT_RUN(MN, T("unlock file failed, ErrCode:%\n", GetLastError()), RPT_ERROR, return FALSE);
	}

	if (dwWritten != m_nRecordBufLen)
		return FALSE;
	
	return TRUE;
}

BOOL CHttpDataFile::WriteDataToEnd()
{
	if (!IsOpen())
		return FALSE;

	DWORD dwSizeLow = GetFileSize(m_hFile, NULL);
	DWORD dwNumLow = dwSizeLow + m_nRecordBufLen;

	DWORD dwWritten = 0;

	CLockFile  hLockFile(m_hFile);
	BOOL bRet = hLockFile.WriteLockFile(0, 0, dwNumLow, 0);
	if(!bRet)
	{
		REPORT_RUN(MN, T("lock file failed, ErrCode:%\n", GetLastError()), RPT_ERROR, return FALSE);
	}

	//SetFilePointer(m_hFile, dwSizeLow, 0, FILE_BEGIN);
	if (!WriteFile(m_hFile, m_pRecordBuf, m_nRecordBufLen, &dwWritten, NULL))
	{
		hLockFile.UnlockFile(0, 0, dwNumLow, 0);
		DWORD dwErr = GetLastError();
		return FALSE;
	}

	bRet = hLockFile.UnlockFile(0, 0, dwNumLow, 0);
	if(!bRet)
	{
		REPORT_RUN(MN, T("unlock file failed, ErrCode:%\n", GetLastError()), RPT_ERROR, return FALSE);
	}

	if (dwWritten != m_nRecordBufLen)
		return FALSE;
	
	return TRUE;
}

BOOL CHttpDataFile::AddRecord(tagHttpKFileInfo* pHttpInfo)
{
	MoveFirst();
	tagHttpKFileInfo oldInfo = {0};
	while(!IsEOF())
	{
		GetRecordToBuffer((char*)&oldInfo);
		if (oldInfo.nDate == pHttpInfo->nDate)
		{
			if (!SetRecordBuffer((const char*)pHttpInfo))
				return FALSE;
			
			if (!WriteData())
				return FALSE;
			else
				return TRUE;
		}
		MoveNext();
	}

	if (!AllocateMemoryForSave(1))
		return FALSE;
	if (!SetRecordBuffer((const char*)pHttpInfo))
		return FALSE;
	if (!WriteDataToEnd())
		return FALSE;

	return TRUE;
}