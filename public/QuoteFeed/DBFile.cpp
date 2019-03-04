// DBFile.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "DBFile.h"
#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////////////////////
CDBFile::CDBFile()
{
	m_hFile         = NULL;
	m_pFileHead     = NULL;
	m_pRecordBuf    = NULL;
	m_pCurRcdBuf    = NULL;
	m_nRecordBufLen = 0;
	m_nFileHeadLen  = 0;
	m_nRecordPos    = 0;
}

CDBFile::~CDBFile()
{
	Close();
}

BOOL CDBFile::Open(const char* pDBName)
{
	m_strFilePath = pDBName;
	if(m_hFile)
		Close();
								  
	m_hFile = CreateFile(pDBName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE
		, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(m_hFile == INVALID_HANDLE_VALUE)
	{
		char szMsg[MSG_BUF_LEN];
		sprintf_s(szMsg, MSG_BUF_LEN, "open dbf failed ErrCode:%d\n", GetLastError());
		REPORT_Ex(szMsg, RPT_ERROR);
		return FALSE;
	}

	//m_hLockFile.SetFileHandle(m_hFile); bydj2008-12-4
	//return Requery();
	return TRUE;
}

BOOL CDBFile::Reopen()
{
	REPORT_Ex((T("重新打开dbf文件:%s\n", m_strFilePath.c_str())).c_str(), RPT_WARNING);
	BOOL bRet = Open(m_strFilePath.c_str());
	if (bRet)
		REPORT_Ex((T("重新打开dbf文件成功:%s\n", m_strFilePath.c_str())).c_str(), RPT_INFO);
	else
		REPORT_Ex((T("重新打开dbf文件失败:%s\n", m_strFilePath.c_str())).c_str(), RPT_ERROR);

	return bRet;
}

//读取dbf文件数据到缓冲区
BOOL CDBFile::Requery()
{
	//bydj2008-12-4
	/*BOOL bRet = m_hLockFile.ReadLockFile();
	if(!bRet)
	{
		REPORT_Ex("lock file failed\n", RPT_ERROR);
		Close();
		return FALSE;
	}
	*/
	
	SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
	DWORD dwReaded = 0;
	memset(&m_fHdr, 0, sizeof(FILEHEADER));
	ReadFile(m_hFile, &m_fHdr, sizeof(FILEHEADER), &dwReaded, NULL);
	if(dwReaded != sizeof(FILEHEADER))
	{
		REPORT_Ex((T("read file flag failed with code:%d\n", GetLastError())).c_str(), RPT_ERROR);
		/*if (!m_hLockFile.UnlockFile())	 //bydj2008-12-4
		{
			REPORT_Ex("unlock file failed\n", RPT_ERROR);
		}
		*/
		Reopen();
		return FALSE;
	}
	
	if(m_fHdr.flag != 0x03) //dbf文件第一个字节为0x03
	{
		REPORT_Ex("invalid DBF file\n", RPT_ERROR);
		/*if (!m_hLockFile.UnlockFile())  //bydj2008-12-4
		{
			REPORT_Ex("unlock file failed\n", RPT_ERROR);
		}  */
		Close();
		return FALSE;
	}
	SetFilePointer(m_hFile, 0, 0, FILE_BEGIN);
	//
	if(!AllocateMemory())
	{
		/*if (!m_hLockFile.UnlockFile()) //bydj2008-12-4
		{
			REPORT_Ex("unlock file failed\n", RPT_ERROR);
		}*/
		Close();
		return FALSE;
	}
	//
	memset(m_pFileHead, 0, m_nFileHeadLen);
	ReadFile(m_hFile, m_pFileHead, m_nFileHeadLen, &dwReaded, NULL);
	if(dwReaded != m_nFileHeadLen)
	{
		REPORT_Ex((T("read file flag header with code:%d\n", GetLastError())).c_str(), RPT_ERROR);
		/*if (!m_hLockFile.UnlockFile()) //bydj2008-12-4
		{
			REPORT_Ex("unlock file failed\n", RPT_ERROR);
		}*/
		Reopen();
		return FALSE;
	}
	
	memset(m_pRecordBuf, 0, m_nRecordBufLen);
	ReadFile(m_hFile, m_pRecordBuf, m_nRecordBufLen, &dwReaded, NULL);
	if(dwReaded != m_nRecordBufLen)
	{
		REPORT_Ex((T("read record failed with code:%d, dwReaded=%d, m_nRecordBufLen=%d\n",
			GetLastError(), dwReaded, m_nRecordBufLen)).c_str(), RPT_ERROR);
		/*if (!m_hLockFile.UnlockFile()) //bydj2008-12-4
		{
			REPORT_Ex("unlock file failed\n", RPT_ERROR);
		}*/
		Reopen();
		return FALSE;
	}
	
	/*bRet = m_hLockFile.UnlockFile();	//bydj2008-12-4
	if(!bRet)
	{
		REPORT_Ex("unlock file failed\n", RPT_ERROR);
		Close();
		return FALSE;
	}*/
	
	MoveFirst();
	return TRUE;
}

void CDBFile::Close()
{
	//m_hLockFile.UnlockFile(); //bydj2008-12-4

	if(m_hFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}

	if(m_pFileHead)
	{
		delete[] m_pFileHead;
		m_pFileHead = NULL;
	}

	if(m_pRecordBuf)
	{
		delete[] m_pRecordBuf;
		m_pRecordBuf = NULL;
	}
	m_pCurRcdBuf    = NULL;
	m_nRecordBufLen = 0;
	m_nFileHeadLen  = 0;
	m_nRecordPos    = 0;
}

BOOL CDBFile::IsOpen()
{
	if(m_hFile)
		return TRUE;

	return FALSE;
}

//给文件头缓冲区m_pFileHead和记录缓冲区m_pRecordBuf分配内存并初始化
BOOL CDBFile::AllocateMemory()
{
	unsigned int nSize = m_fHdr.RecordLen * m_fHdr.RecordSum;
	if(m_nRecordBufLen < nSize)
	{
		if(m_pRecordBuf)
			delete[] m_pRecordBuf;
		m_pRecordBuf = new char[nSize];
		m_nRecordBufLen = nSize;
	}

	if(m_pRecordBuf)
		memset(m_pRecordBuf, 0, m_nRecordBufLen);
	else
	{
		REPORT_Ex((T("给记录分配内存失败,内存大小：%d\n", nSize)).c_str(), RPT_ERROR);
		return FALSE;
	}

	unsigned int nFHeaderSize = m_fHdr.dbfHeadSize;
	if(m_nFileHeadLen <	nFHeaderSize)
	{
		if(m_pFileHead)
			delete[] m_pFileHead;
		m_pFileHead = new char[nFHeaderSize]; 
		m_nFileHeadLen = nFHeaderSize;
	}
	if(m_pFileHead)
		memset(m_pFileHead, 0, m_nFileHeadLen);
	else
	{
		REPORT_Ex((T("给文件头分配内存失败,内存大小:%d\n", nFHeaderSize)).c_str(), RPT_ERROR);
		return FALSE;
	}

	return TRUE;
}

//返回记录的总数
long CDBFile::GetRecordCount()
{
	if(!IsOpen())
		return -1;

	return m_fHdr.RecordSum;
}

//返回一条记录的大小
long CDBFile::GetRecordSize()
{
	if(!IsOpen())
		return -1;

	return m_fHdr.RecordLen;
}

//返回所有记录的大小
long CDBFile::GetAllRecordSize()
{
	if(!IsOpen())
		return -1;

	return m_fHdr.RecordLen * m_fHdr.RecordSum;
}

//获得当前记录的内容复制到 pDesBuf 中
BOOL CDBFile::GetRecordToBuffer(char *pDesBuf)
{
	if(!IsOpen() || m_fHdr.RecordSum == 0 || IsBOF() || IsEOF())
	{
		pDesBuf[0] = 0;
		return FALSE;
	}

	memcpy(pDesBuf, m_pCurRcdBuf, m_fHdr.RecordLen);
	return TRUE;
}

BOOL CDBFile::GetAllRecordToBuffer(char *pDesBuf)
{
	if(!IsOpen() || m_fHdr.RecordSum == 0 || IsBOF() || IsEOF())
	{
		pDesBuf[0] = 0;
		return FALSE;
	}

	memcpy(pDesBuf, m_pRecordBuf, m_nRecordBufLen);
	return TRUE;
}  

int CDBFile::GetFieldValue(short nIndex, char* pValue)
{
	if(!IsOpen() || m_fHdr.RecordSum == 0 || IsBOF() || IsEOF())
		return -1;

	int nFieldCount = GetFieldCount();
	if(nFieldCount <= 0)
		return -1;
	if(nIndex < 0 || nIndex > (nFieldCount - 1) )
		return -1;

	FIELD *pField = (FIELD *)(m_pFileHead + sizeof(FILEHEADER));
	int nFieldSize = pField[nIndex].FieLen;
	memcpy(pValue, m_pCurRcdBuf+pField[nIndex].BegPos, nFieldSize); 
	pValue[nFieldSize] = 0;

	return nFieldSize;
}

int CDBFile::GetFieldValue(const char* pFieldName, char* pValue)
{
	if(!IsOpen() || m_fHdr.RecordSum == 0 || IsBOF() || IsEOF())
		return -1;

	int nFieldCount = GetFieldCount();
	if(nFieldCount <= 0)
		return -1;

	FIELD *pField = (FIELD *)(m_pFileHead + sizeof(FILEHEADER));
	int nFieldSize = 0;
	for(int i=0; i<nFieldCount; i++)
	{
		if(_stricmp(pFieldName, pField[i].FieName) == 0)
		{
			nFieldSize = pField[i].FieLen;
			memcpy(pValue, m_pCurRcdBuf+pField[i].BegPos, nFieldSize); 
			pValue[nFieldSize] = 0;
			return nFieldSize;
		}
	}

	return -1;
}

int CDBFile::GetFieldValue(const char* pFieldName, int& nValue)
{
	char szValue[50];
	int nFieldSize = GetFieldValue(pFieldName, szValue);
	if(nFieldSize == -1)
		return -1;
						   
	nValue = atoi(szValue);
	return nFieldSize;
}

/*************************************************************************
1.先找到记录中对应的字段stricmp(pDescr[i].FieName,strFieName)
2.在比较对应的值memcmp(m_pRecordBuffer + pDescr[i].BegPos, pRecord + 
  pDescr[i].BegPos,nFieLen)
3.如果不等返回最新的数据，否则NULL
*************************************************************************/
char* CDBFile::CompFieldBuffer(const char* pFieldName, const char* pRecord)
{
	if(pFieldName == NULL || pRecord == NULL)
		return NULL;
	if(!IsOpen() || m_fHdr.RecordSum == 0 || IsBOF() || IsEOF())
		return NULL;

	int nFieldCount = GetFieldCount();
	if(nFieldCount <= 0)
		return NULL;

	FIELD *pField = (FIELD *)(m_pFileHead + sizeof(FILEHEADER));
	int nFieldSize = 0;
	for(int i=0; i<nFieldCount; i++)
	{
		if(_stricmp(pFieldName, pField[i].FieName) == 0)
		{
			//nFieLen = pField[i].FieLen;

			if(memcmp(m_pCurRcdBuf+pField[i].BegPos, pRecord + pField[i].BegPos, pField[i].FieLen) != 0)
				return m_pCurRcdBuf+pField[i].BegPos;
			else
				return NULL;
		}
	}

	return NULL;
}


/*************************************************************************
从一个记录中获得相关字段的值
*************************************************************************/
int CDBFile::GetFieldValue(const char* pFieldName, const char* pRecord, char* pValue)
{
	int nFieldCount = GetFieldCount();
	if(nFieldCount <= 0)
		return -1;

	FIELD *pField = (FIELD *)(m_pFileHead + sizeof(FILEHEADER));
	int nFieldSize = 0;
	for(int i=0; i<nFieldCount; i++)
	{
		if(_stricmp(pFieldName, pField[i].FieName) == 0)
		{
			nFieldSize = pField[i].FieLen;
			memcpy(pValue, pRecord+pField[i].BegPos, nFieldSize); 
			pValue[nFieldSize] = 0;
			return nFieldSize;
		}
	}

	return -1;
}