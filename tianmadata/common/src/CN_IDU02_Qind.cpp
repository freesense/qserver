//CN_IDU02_Qind.cpp
#include "stdafx.h"
#include "CN_IDU02_Qind.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   101
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CCN_IDU02_Qind::CCN_IDU02_Qind()
{
}

CCN_IDU02_Qind::~CCN_IDU02_Qind()
{
}

BOOL CCN_IDU02_Qind::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagIduIndex);
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
		for(int i=2; i<101; i++)
		{
			m_pFieldInfo[i].FieLen = sizeof(double);
			m_pFieldInfo[i].BegPos = nPos;
			nPos += sizeof(double);
		}
	}
	return Requery();
}

void CCN_IDU02_Qind::Close()
{
	CBaseFile::Close();
}

BOOL CCN_IDU02_Qind::Requery()
{
	char szSql[400];
	sprintf_s(szSql, 400, "SELECT CN_IDU02_Qind.*, CN_IDU03_Fnind.*, CN_IDU04_Anst.*\
	FROM CN_IDU02_Qind, CN_IDU03_Fnind, CN_IDU04_Anst WHERE \
	CN_IDU02_Qind.IDU_CD=CN_IDU03_Fnind.IDU_CD AND CN_IDU03_Fnind.IDU_CD=CN_IDU04_Anst.IDU_CD \
	AND CN_IDU02_Qind.IS_VLD=1  AND CN_IDU03_Fnind.IS_VLD=1 AND CN_IDU04_Anst.IS_VLD=1");

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

	tagIduIndex index;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&index, 0, sizeof(tagIduIndex));
		char* pBufPos = (char*)&index;

		memcpy(pBufPos, MT_CN, 2);
		pBufPos += 2;

		char szSec_cd[8];
		g_database.GetCollect("IDU_CD", (LPCTSTR)szSec_cd, 8);
		memcpy(pBufPos, szSec_cd, 8);
		pBufPos += 8;

		char szColumn[10];
		double value = 0;
		for(int i=1; i<=47; i++)
		{
			if (i == 3)
				i = 10;
			else if (i == 28)
				i = 31;

			sprintf_s(szColumn, 10, "ci02%02d", i);
			g_database.GetCollect(szColumn, value);
			memcpy(pBufPos, &value, sizeof(double));
			pBufPos += sizeof(double);
		}

		for(int i=1; i<=49; i++)
		{
			sprintf_s(szColumn, 10, "ci03%02d", i);
			g_database.GetCollect(szColumn, value);
			memcpy(pBufPos, &value, sizeof(double));
			pBufPos += sizeof(double);
		}

		for(int i=1; i<=13; i++)
		{
			sprintf_s(szColumn, 10, "ci04%02d", i);
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
void CCN_IDU02_Qind::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}
*/
BOOL CCN_IDU02_Qind::WriteData()
{
	return TRUE;
}