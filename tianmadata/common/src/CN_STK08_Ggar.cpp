//CN_STK08_Ggar.cpp
#include "stdafx.h"
#include "CN_STK08_Ggar.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   11
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CCN_STK08_Ggar::CCN_STK08_Ggar()
{
}

CCN_STK08_Ggar::~CCN_STK08_Ggar()
{
}

BOOL CCN_STK08_Ggar::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagCommentaryCollectEx);
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

		m_pFieldInfo[1].FieLen = 4;
		m_pFieldInfo[1].BegPos = nPos;
		nPos += 4;

		for(int i=2; i<11; i++)
		{
			m_pFieldInfo[i].FieLen = sizeof(double);
			m_pFieldInfo[i].BegPos = nPos;
			nPos += sizeof(double);
		}
	}
	return Requery();
}

void CCN_STK08_Ggar::Close()
{
	CBaseFile::Close();
}

BOOL CCN_STK08_Ggar::Requery()
{
	if (!g_database.OpenTable("CN_STK08_Ggar"))
	{
		REPORT(MN, T("OpenTable(\"CN_STK08_Ggar\") failed\n"), RPT_ERROR);
		return FALSE;
	}
	m_nRecordCount = g_database.GetRecordCount();

	if(!AllocateMemory())
	{
		g_database.CloseTable();
		Close();
		return FALSE;
	}

	tagCommentaryCollectEx info;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&info, 0, sizeof(tagCommentaryCollectEx));
		g_database.GetCollect("SEC_CD", (LPCTSTR)info.SEC_CD, 8);
		g_database.GetCollect("Sta_Prd", (LPCTSTR)info.collect.Sta_Prd, 4);

		g_database.GetCollect("Avg_Value", info.collect.Avg_Value);
		g_database.GetCollect("Tbu_Num", info.collect.Tbu_Num);
		g_database.GetCollect("Buy_Num", info.collect.Buy_Num);
		g_database.GetCollect("Neu_Num", info.collect.Neu_Num);
		g_database.GetCollect("Red_Num", info.collect.Red_Num);
		g_database.GetCollect("Sel_Num", info.collect.Sel_Num);
		g_database.GetCollect("Avg_Price", info.collect.Avg_Price);
		g_database.GetCollect("Hig_Price", info.collect.Hig_Price);
		g_database.GetCollect("Low_Price", info.collect.Low_Price);
		
		
		if(!SetRecordBuffer((const char*)&info))
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
void CCN_STK08_Ggar::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}  */

BOOL CCN_STK08_Ggar::WriteData()
{
	return TRUE;
}

int CCN_STK08_Ggar::GetStockRecordCount(const char* pStock)
{
	int nRecordCount = 0;
	CBaseFile::MoveFirst();
	tagCommentaryCollectEx Info;
	while(!CBaseFile::IsEOF())
	{
		memset(&Info, 0, sizeof(tagCommentaryCollectEx));
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