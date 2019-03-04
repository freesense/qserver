//HK_STK02_Qind.cpp
#include "stdafx.h"
#include "HK_STK02_Qind.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   125
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CHK_STK02_Qind::CHK_STK02_Qind()
{
}

CHK_STK02_Qind::~CHK_STK02_Qind()
{
}

BOOL CHK_STK02_Qind::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagHQIndex);
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
		for(int i=2; i<125; i++)
		{
			m_pFieldInfo[i].FieLen = sizeof(double);
			m_pFieldInfo[i].BegPos = nPos;
			nPos += sizeof(double);
		}
	}
	return Requery();
}

void CHK_STK02_Qind::Close()
{
	CBaseFile::Close();
}

BOOL CHK_STK02_Qind::Requery()
{
	char szSql[500];
	sprintf_s(szSql, 500, "SELECT HK_STK02_Qind.*, HK_STK03_Fnind.*, HK_STK04_Anst.*,\
		HK_STK08_Ggar.* FROM HK_STK02_Qind, HK_STK03_Fnind, HK_STK04_Anst, HK_STK08_Ggar\
		WHERE HK_STK02_Qind.SEC_CD=HK_STK03_Fnind.SEC_CD AND HK_STK03_Fnind.SEC_CD=\
		HK_STK04_Anst.SEC_CD AND HK_STK04_Anst.SEC_CD=HK_STK08_Ggar.SEC_CD AND \
		HK_STK08_Ggar.Sta_prd='CUR' AND HK_STK02_Qind.IS_VLD=1 AND HK_STK03_Fnind.IS_VLD=1 \
		AND HK_STK04_Anst.IS_VLD=1 AND HK_STK08_Ggar.IS_VLD=1");

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

	tagHQIndex hqIndex;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&hqIndex, 0, sizeof(tagHQIndex));
		char* pBufPos = (char*)&hqIndex;

		memcpy(pBufPos, MT_HK, 2);
		pBufPos += 2;

		char szSec_cd[8];
		g_database.GetCollect("SEC_CD", (LPCTSTR)szSec_cd, 8);
		memcpy(pBufPos, szSec_cd, 8);
		pBufPos += 8;

		char szColumn[10];
		double value = 0;
		for(int i=1; i<=47; i++)
		{
			if (i == 4)
				i = 9;
			else if (i == 28)
				i = 31;

			sprintf_s(szColumn, 10, "cs02%02d", i);
			g_database.GetCollect(szColumn, value);
			memcpy(pBufPos, &value, sizeof(double));
			pBufPos += sizeof(double);
		}

		for(int i=1; i<=62; i++)
		{
			if (i == 4)
				i = 11;

			sprintf_s(szColumn, 10, "cs03%02d", i);
			g_database.GetCollect(szColumn, value);
			memcpy(pBufPos, &value, sizeof(double));
			pBufPos += sizeof(double);
		}

		for(int i=1; i<=20; i++)
		{
			sprintf_s(szColumn, 10, "cs04%02d", i);
			g_database.GetCollect(szColumn, value);
			memcpy(pBufPos, &value, sizeof(double));
			pBufPos += sizeof(double);
		}
		g_database.GetCollect("Avg_Value", hqIndex.Avg_Value);
		g_database.GetCollect("Tbu_Num", hqIndex.Tbu_Num);
		g_database.GetCollect("Buy_Num", hqIndex.Buy_Num);
		g_database.GetCollect("Neu_Num", hqIndex.Neu_Num);
		g_database.GetCollect("Red_Num", hqIndex.Red_Num);

		g_database.GetCollect("Sel_Num", hqIndex.Sel_Num);
		g_database.GetCollect("Avg_Price", hqIndex.Avg_Price);
		g_database.GetCollect("Hig_Price", hqIndex.Hig_Price);
		g_database.GetCollect("Low_Price", hqIndex.Low_Price);

		if(!SetRecordBuffer((const char*)&hqIndex))
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
void CHK_STK02_Qind::LogEvent(LPCTSTR lpFormat, ...)
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
BOOL CHK_STK02_Qind::WriteData()
{
	return TRUE;
}