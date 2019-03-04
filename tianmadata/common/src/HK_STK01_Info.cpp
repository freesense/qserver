//HK_STK01_Info.cpp
#include "stdafx.h"
#include "HK_STK01_Info.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   12
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CHK_STK01_Info::CHK_STK01_Info()
{
}

CHK_STK01_Info::~CHK_STK01_Info()
{
}

BOOL CHK_STK01_Info::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagStockInfo);
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
		m_pFieldInfo[0].FieLen = 8;
		m_pFieldInfo[0].BegPos = nPos;
		nPos += 8;
		m_pFieldInfo[1].FieLen = 10;
		m_pFieldInfo[1].BegPos = nPos;
		nPos += 10;
		m_pFieldInfo[2].FieLen = 10;
		m_pFieldInfo[2].BegPos = nPos;
		nPos += 10;
		m_pFieldInfo[3].FieLen = 10;
		m_pFieldInfo[3].BegPos = nPos;
		nPos += 10;
		m_pFieldInfo[4].FieLen = 10;
		m_pFieldInfo[4].BegPos = nPos; 
		nPos += 10;
		m_pFieldInfo[5].FieLen = 2;
		m_pFieldInfo[5].BegPos = nPos;
		nPos += 2;
		m_pFieldInfo[6].FieLen = 1;
		m_pFieldInfo[6].BegPos = nPos; 
		nPos += 1;
		m_pFieldInfo[7].FieLen = 16;
		m_pFieldInfo[7].BegPos = nPos; 
		nPos += 16;
		m_pFieldInfo[8].FieLen = 16;
		m_pFieldInfo[8].BegPos = nPos; 
		nPos += 16;
		m_pFieldInfo[9].FieLen = 4;
		m_pFieldInfo[9].BegPos = nPos; 
		nPos += 4;
		m_pFieldInfo[10].FieLen = 4;
		m_pFieldInfo[10].BegPos = nPos;
		nPos += 4;
		m_pFieldInfo[11].FieLen = 4;
		m_pFieldInfo[11].BegPos = nPos; 
	}
	return Requery();
}

void CHK_STK01_Info::Close()
{
	CBaseFile::Close();
}

BOOL CHK_STK01_Info::Requery()
{
	if (!g_database.OpenTable("HK_STK01_Info"))
	{
		REPORT(MN, T("OpenTable(\"HK_STK01_Info\") failed\n"), RPT_ERROR);
		return FALSE;
	}
	m_nRecordCount = g_database.GetRecordCount();

	if(!AllocateMemory())
	{
		g_database.CloseTable();
		Close();
		return FALSE;
	}

	tagStockInfo stockInfo;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&stockInfo, 0, sizeof(tagStockInfo));
		g_database.GetCollect("SEC_CD", (LPCTSTR)stockInfo.Sec_cd, 8);
		g_database.GetCollect("SEC_ID", (LPCTSTR)stockInfo.Sec_id, 10);
		g_database.GetCollect("ORG_ID", (LPCTSTR)stockInfo.Org_id, 10);
		g_database.GetCollect("IDU_CD", (LPCTSTR)stockInfo.Idu_cd, 10);
		g_database.GetCollect("AREA_CD", (LPCTSTR)stockInfo.Area_cd, 10);
		g_database.GetCollect("VAR_CL", (LPCTSTR)stockInfo.Var_cl, 2);
		g_database.GetCollect("MKT_CL", (LPCTSTR)stockInfo.Mkt_cl, 1);
		g_database.GetCollect("SEC_SNM", (LPCTSTR)stockInfo.Sec_snm, 16);
		g_database.GetCollect("SEC_ESNM", (LPCTSTR)stockInfo.Sec_esnm, 16);
		g_database.GetCollect("SEC_SSNM", (LPCTSTR)stockInfo.Sec_ssnm, 4);
		
		SYSTEMTIME time_iss;
		if (!g_database.GetCollect("ISS_DT", &time_iss))
			stockInfo.nIss_dt = 0;
		else
			stockInfo.nIss_dt = time_iss.wYear*10000 + time_iss.wMonth*100 + time_iss.wDay;

		SYSTEMTIME time_mkt;
		if (!g_database.GetCollect("MKT_DT", &time_mkt))
			stockInfo.nMkt_dt = 0;
		else
			stockInfo.nMkt_dt = time_mkt.wYear*10000 + time_mkt.wMonth*100 + time_mkt.wDay;

		if(!SetRecordBuffer((const char*)&stockInfo))
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
void CHK_STK01_Info::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}  */

BOOL CHK_STK01_Info::WriteData()
{
	return TRUE;
}