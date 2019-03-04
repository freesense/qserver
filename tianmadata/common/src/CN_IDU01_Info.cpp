//CN_IDU01_Info.cpp
#include "stdafx.h"
#include "CN_IDU01_Info.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   2
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CCN_IDU01_Info::CCN_IDU01_Info()
{
}

CCN_IDU01_Info::~CCN_IDU01_Info()
{
}

BOOL CCN_IDU01_Info::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagIndustryInfo);
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
		m_pFieldInfo[0].FieLen = 10;
		m_pFieldInfo[0].BegPos = nPos;
		nPos += 10;
		m_pFieldInfo[1].FieLen = 40;
		m_pFieldInfo[1].BegPos = nPos;
	}
	return Requery();
}

void CCN_IDU01_Info::Close()
{
	CBaseFile::Close();
}

BOOL CCN_IDU01_Info::Requery()
{
	if (!g_database.OpenTable("CN_IDU01_Info"))
	{
		REPORT_RUN(MN, T("OpenTable(\"CN_IDU01_Info\") failed\n"), RPT_ERROR, return FALSE;);
	}
	m_nRecordCount = g_database.GetRecordCount();

	if(!AllocateMemory())
	{
		g_database.CloseTable();
		Close();
		return FALSE;
	}

	tagIndustryInfo iduInfo;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&iduInfo, 0, sizeof(tagIndustryInfo));
		g_database.GetCollect("IDU_CD", (LPCTSTR)iduInfo.Idu_cd, 10);
		g_database.GetCollect("IDU_NM", (LPCTSTR)iduInfo.Idu_nm, 40);

		if(!SetRecordBuffer((const char*)&iduInfo))
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
void CCN_IDU01_Info::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}*/

BOOL CCN_IDU01_Info::WriteData()
{
	return TRUE;
}