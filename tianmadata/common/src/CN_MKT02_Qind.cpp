//CN_MKT02_Qind.cpp
#include "stdafx.h"
#include "CN_MKT02_Qind.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   24
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CCN_MKT02_Qind::CCN_MKT02_Qind()
{
}

CCN_MKT02_Qind::~CCN_MKT02_Qind()
{
}

BOOL CCN_MKT02_Qind::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagIDXIndex);
		m_nFieldCount = FIELD_COUNT;
		if(m_pFieldInfo != NULL)
		{
			delete []m_pFieldInfo;
			m_pFieldInfo = NULL;
		}

		m_pFieldInfo = new tagFieldInfo[FIELD_COUNT];
		if (m_pFieldInfo == NULL)
		{
			REPORT(MN, T("new tagFieldInfo failed\n"), RPT_ERROR);
			return FALSE;
		}
		int nPos = 0;
		m_pFieldInfo[0].FieLen = 2;
		m_pFieldInfo[0].BegPos = nPos;
		nPos += 2;
		m_pFieldInfo[1].FieLen = 8;
		m_pFieldInfo[1].BegPos = nPos;
		nPos += 8;
		for(int i=2; i<5; i++)
		{
			m_pFieldInfo[i].FieLen = 10;
			m_pFieldInfo[i].BegPos = nPos;
			nPos += 10;
		}
		for(int i=5; i<24; i++)
		{
			m_pFieldInfo[i].FieLen = sizeof(double);
			m_pFieldInfo[i].BegPos = nPos;
			nPos += sizeof(double);
		}
	}
	return Requery();
}

void CCN_MKT02_Qind::Close()
{
	CBaseFile::Close();
}

BOOL CCN_MKT02_Qind::Requery()
{
	char szSql[400];
	sprintf_s(szSql, 400, "SELECT a.*, b.* FROM CN_MKT02_Qind AS a LEFT OUTER JOIN  CN_MKT03_Fnind AS b on a.idx_cd=b.idx_cd \
		WHERE a.IS_VLD=1 AND b.IS_VLD=1");

	if (!g_database.OpenTableUseSql(szSql))
	{
		REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
		return FALSE;
	}
	m_nRecordCount = g_database.GetRecordCount();

	if(!AllocateMemory())
	{
		g_database.CloseTable();
		Close();
		return FALSE;
	}

	tagIDXIndex index;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&index, 0, sizeof(tagIDXIndex));
		char* pBufPos = (char*)&index;

		memcpy(pBufPos, MT_CN, 2);
		pBufPos += 2;

		char szSec_cd[8];
		g_database.GetCollect("IDX_CD", (LPCTSTR)szSec_cd, 8);
		memcpy(pBufPos, szSec_cd, 8);
		pBufPos += 8;

		g_database.GetCollect("cm0201", (LPCTSTR)index.cm0201, 10);
		pBufPos += 10;
		g_database.GetCollect("cm0202", (LPCTSTR)index.cm0202, 10);
		pBufPos += 10;
		g_database.GetCollect("cm0203", (LPCTSTR)index.cm0203, 10);
		pBufPos += 10;

		char szColumn[10];
		double value = 0;
		for(int i=11; i<=26; i++)
		{
			if (i == 20)
				i = 21;

			sprintf_s(szColumn, 10, "cm02%02d", i);
			g_database.GetCollect(szColumn, value);
			memcpy(pBufPos, &value, sizeof(double));
			pBufPos += sizeof(double);
		}

		for(int i=1; i<=4; i++)
		{
			sprintf_s(szColumn, 10, "cm03%02d", i);
			g_database.GetCollect(szColumn, value);
			memcpy(pBufPos, &value, sizeof(double));
			pBufPos += sizeof(double);
		}
		
		//发送数据
		if(!SetRecordBuffer((const char*)&index))
		{
			REPORT(MN, T("SetRecordBuffer() failed\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		CBaseFile::MoveNext();
		g_database.MoveNext();
	}

	g_database.CloseTable();
	return TRUE;
}
/*
void CCN_MKT02_Qind::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}  */

BOOL CCN_MKT02_Qind::WriteData()
{
	return TRUE;
}