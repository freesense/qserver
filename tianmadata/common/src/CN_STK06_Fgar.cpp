//CN_STK06_Fgar.cpp
#include "stdafx.h"
#include "CN_STK06_Fgar.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   9
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CCN_STK06_Fgar::CCN_STK06_Fgar()
{
}

CCN_STK06_Fgar::~CCN_STK06_Fgar()
{
}

BOOL CCN_STK06_Fgar::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagPayOffCollectEx);
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
		m_pFieldInfo[0].FieLen = 8;		   //股票代码
		m_pFieldInfo[0].BegPos = nPos;
		nPos += 8;
		m_pFieldInfo[1].FieLen = 6;
		m_pFieldInfo[1].BegPos = nPos;
		nPos += 6;
		for(int i=2; i<=4; i++)
		{
			m_pFieldInfo[i].FieLen = 4;
			m_pFieldInfo[i].BegPos = nPos;
			nPos += 4;
		}
		for(int i=5; i<=7; i++)
		{
			m_pFieldInfo[i].FieLen = sizeof(double);
			m_pFieldInfo[i].BegPos = nPos; 
			nPos += sizeof(double);
		}
		m_pFieldInfo[8].FieLen = sizeof(unsigned short);
		m_pFieldInfo[8].BegPos = nPos; 
	}
	return Requery();
}

void CCN_STK06_Fgar::Close()
{
	CBaseFile::Close();
}

BOOL CCN_STK06_Fgar::Requery()
{
	if (!g_database.OpenTable("CN_STK06_Fgar"))
	{
		REPORT(MN, T("OpenTable(\"CN_STK06_Fgar\") failed\n"), RPT_ERROR);
		return FALSE;
	}
	m_nRecordCount = g_database.GetRecordCount();

	if(!AllocateMemory())
	{
		g_database.CloseTable();
		Close();
		return FALSE;
	}

	tagPayOffCollectEx Info;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&Info, 0, sizeof(tagPayOffCollectEx));
		g_database.GetCollect("SEC_CD", (LPCTSTR)Info.SEC_CD, 8);

		g_database.GetCollect("IND_ID", (LPCTSTR)Info.collect.IND_ID, 6);
		g_database.GetCollect("Rpt_Yr", (LPCTSTR)Info.collect.Rpt_Yr, 4);
		g_database.GetCollect("Rpt_Prd", (LPCTSTR)Info.collect.Rpt_Prd, 4);
		g_database.GetCollect("Sta_Prd", (LPCTSTR)Info.collect.Sta_Prd, 4);
		g_database.GetCollect("Avg_Value", Info.collect.Avg_Value);
		g_database.GetCollect("Hig_Value", Info.collect.Hig_Value);
		g_database.GetCollect("Low_Value", Info.collect.Low_Value);
		g_database.GetCollect("Ana_Num", Info.collect.Ana_Num);

		if(!SetRecordBuffer((const char*)&Info))
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
void CCN_STK06_Fgar::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}*/

BOOL CCN_STK06_Fgar::WriteData()
{
	return TRUE;
}

int CCN_STK06_Fgar::GetStockRecordCount(char* pStock)
{
	int nRecordCount = 0;
	CBaseFile::MoveFirst();
	tagPayOffCollectEx Info;
	while(!CBaseFile::IsEOF())
	{
		memset(&Info, 0, sizeof(tagPayOffCollectEx));
		if (!CBaseFile::GetRecordToBuffer((char*)&Info))
		{
			REPORT(MN, T("GetRecordToBuffer() failed\n"), RPT_ERROR);
		}
		else
		{
			if (_stricmp(pStock, (const char*)Info.SEC_CD) == 0)
				nRecordCount++;
		}
		CBaseFile::MoveNext();
	}
	return nRecordCount;
}