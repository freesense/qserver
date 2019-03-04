//BaseFile.cpp
//
#include "stdafx.h"
#include "BaseFile.h"
#include "../../public/CharTranslate.h"
#include "KLine_Exe.h"

///////////////////////////////////////////////////////////////////////////////
CBaseFile::CBaseFile()
{
	m_pRecordBuf    = NULL;
	m_nRecordBufLen = 0;
	m_nRecordPos    = NULL;
	m_pCurRcdBuf    = NULL;
	m_nRecordCount  = 0;
	m_nRecordLen    = 0;
	m_bIsOpen       = FALSE;
	m_pFieldInfo    = NULL;
}

CBaseFile::~CBaseFile()
{
	Close();
}

void CBaseFile::MoveFirst()
{
	m_nRecordPos = 0; 
	m_pCurRcdBuf = m_pRecordBuf;
}

void CBaseFile::MoveLast()
{
	m_nRecordPos = m_nRecordCount - 1;
	m_pCurRcdBuf = m_pRecordBuf + m_nRecordLen * (m_nRecordCount - 1);
}

void CBaseFile::MoveNext()
{
	m_nRecordPos++;
	m_pCurRcdBuf += m_nRecordLen;
}

void CBaseFile::MovePrev()
{
	m_nRecordPos--;
	m_pCurRcdBuf -= m_nRecordLen;
}

void CBaseFile::Move(long nRows)
{
	if(nRows < 0)
	{
		m_nRecordPos -= nRows;
		m_pCurRcdBuf -= m_nRecordLen * nRows;
	}
	else if(nRows > 0)
	{
		m_nRecordPos += nRows;
		m_pCurRcdBuf += m_nRecordLen * nRows;
	}
}

BOOL CBaseFile::IsBOF()
{
	if(m_nRecordPos < 0 || m_pCurRcdBuf < m_pRecordBuf) 
		return TRUE;
	return FALSE;
}

BOOL CBaseFile::IsEOF()
{
	if(m_nRecordPos > (int)(m_nRecordCount - 1 )
		|| m_pCurRcdBuf > m_pRecordBuf + m_nRecordLen * (m_nRecordCount - 1))
		return TRUE;

	return FALSE;
}

BOOL CBaseFile::GetRecordToBuffer(char *pDesBuf)
{
	if(!IsOpen() || m_nRecordCount == 0 || IsBOF() || IsEOF())
	{
		pDesBuf[0] = 0;
		return FALSE;
	}

	memcpy(pDesBuf, m_pCurRcdBuf, m_nRecordLen);
	return TRUE;
}

BOOL CBaseFile::SetRecordBuffer(const char *pDesBuf)
{
	if(!IsOpen() || pDesBuf == NULL || IsBOF() || IsEOF())
		return FALSE;

	memcpy(m_pCurRcdBuf, pDesBuf, m_nRecordLen);
	return TRUE;
}

BOOL CBaseFile::GetAllRecordToBuffer(char *pDesBuf)
{
	if(!IsOpen() || m_nRecordCount == 0 || IsBOF() || IsEOF())
	{
		pDesBuf[0] = 0;
		return FALSE;
	}

	memcpy(pDesBuf, m_pRecordBuf, m_nRecordBufLen);
	return TRUE;
}

BOOL CBaseFile::SetAllRecordToBuffer(const char *pDesBuf)
{
	if(!IsOpen() || pDesBuf == NULL)
		return FALSE;

	MoveFirst();
	memcpy(m_pRecordBuf, pDesBuf, m_nRecordBufLen);
	
	return TRUE;
}

int CBaseFile::GetFieldValue(unsigned int nIndex, char* pValue)
{
	if(!IsOpen() || m_nRecordCount == 0 || IsBOF() || IsEOF() || m_pFieldInfo == NULL)
		return -1;

	if(m_nFieldCount <= 0)
		return -1;
	if(nIndex < 0 || nIndex > (m_nFieldCount - 1) )
		return -1;

	int nFieldSize = m_pFieldInfo[nIndex].FieLen;
	memcpy(pValue, m_pCurRcdBuf+m_pFieldInfo[nIndex].BegPos, nFieldSize); 
	pValue[nFieldSize] = 0;

	return nFieldSize;
}

int CBaseFile::GetFieldValue(unsigned int nIndex, unsigned int& nValue)
{
	if(!IsOpen() || m_nRecordCount == 0 || IsBOF() || IsEOF() || m_pFieldInfo == NULL)
		return -1;

	if(m_nFieldCount <= 0)
		return -1;
	if(nIndex < 0 || nIndex > (m_nFieldCount - 1) )
		return -1;

	int nFieldSize = m_pFieldInfo[nIndex].FieLen;
	memcpy(&nValue, m_pCurRcdBuf+m_pFieldInfo[nIndex].BegPos, nFieldSize); 

	return nFieldSize;
}

void CBaseFile::Close()
{
	if (m_pRecordBuf != NULL)
	{
		delete m_pRecordBuf;
		m_pRecordBuf = NULL;
	}

	m_pCurRcdBuf    = NULL;
	m_nRecordBufLen = 0;
	m_nRecordPos    = 0;
	m_nRecordCount  = 0;
	m_nRecordLen    = 0;
	m_nFieldCount   = 0;
	m_bIsOpen       = FALSE;

	if (m_pFieldInfo != NULL)
	{
		delete m_pFieldInfo;
		m_pFieldInfo = NULL;
	}
}

/*void CBaseFile::LogEvent(WORD nLogType, LPCTSTR pFormat, ...)
{
	char chMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(chMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);

	theApp.LogEvent(nLogType, chMsg);
}*/

BOOL CBaseFile::AllocateMemory()
{
	unsigned int nSize = m_nRecordLen * m_nRecordCount;
	if (nSize == 0)
		return TRUE;
	if(m_nRecordBufLen != nSize)
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
		REPORT_RUN(MN, T("给记录分配内存失败!\n"), RPT_ERROR, return FALSE);
	}

	return TRUE;
}

BOOL CBaseFile::AllocateMemoryForSave(unsigned int nRecordCount)
{
	m_nRecordCount = nRecordCount;
	unsigned int nSize = m_nRecordLen * m_nRecordCount;
	if(m_nRecordBufLen != nSize)
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
		REPORT_RUN(MN, T("给记录分配内存失败!\n"), RPT_ERROR, return FALSE);
	}
	
	MoveFirst();
	return TRUE;
}