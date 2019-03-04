//CN_Subject_Table.cpp
#include "stdafx.h"
#include "CN_Subject_Table.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT   5
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CCN_Subject_Table::CCN_Subject_Table()
{
	m_nCount16 = -1;
	m_nCount17 = -1;
	m_nCount18 = -1;
	m_nCount19 = -1;
	m_nCount20 = -1;
	m_nCount21 = -1;
	m_nCount22 = -1;
	m_nCount23 = -1;

	m_n16Pos = m_n17Pos = m_n18Pos = m_n19Pos = m_n20Pos = 0;
	m_n21Pos = m_n22Pos = m_n23Pos = 0;
}

CCN_Subject_Table::~CCN_Subject_Table()
{
}

BOOL CCN_Subject_Table::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagSubjectTableEx);
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
		m_pFieldInfo[1].FieLen = 10;
		m_pFieldInfo[1].BegPos = nPos;
		nPos += 10;
		m_pFieldInfo[2].FieLen = 40;
		m_pFieldInfo[2].BegPos = nPos;
		nPos += 40;
		m_pFieldInfo[3].FieLen = 40;
		m_pFieldInfo[3].BegPos = nPos;
		nPos += 40;
		m_pFieldInfo[4].FieLen = 10;
		m_pFieldInfo[4].BegPos = nPos; 
	}
	return Requery();
}

void CCN_Subject_Table::Close()
{
	CBaseFile::Close();
}

BOOL CCN_Subject_Table::Requery()
{
	m_nCount16 = g_database.GetTableRecordCount("CN_STK16_Fbsc_R");
	m_nCount17 = g_database.GetTableRecordCount("CN_STK17_FHind_R");
	m_nCount18 = g_database.GetTableRecordCount("CN_STK18_ASSET1_R");
	m_nCount19 = g_database.GetTableRecordCount("CN_STK19_ASSET2_R");
	m_nCount20 = g_database.GetTableRecordCount("CN_STK20_PROFIT1_R");
	m_nCount21 = g_database.GetTableRecordCount("CN_STK21_PROFIT2_R");
	m_nCount22 = g_database.GetTableRecordCount("CN_STK22_CASH1_R");
	m_nCount23 = g_database.GetTableRecordCount("CN_STK23_CASH2_R");

	m_nRecordCount = 0;
	if (m_nCount16 > 0)
		m_nRecordCount += m_nCount16;
	if (m_nCount17 > 0)
		m_nRecordCount += m_nCount17;
	if (m_nCount18 > 0)
		m_nRecordCount += m_nCount18;
	if (m_nCount19 > 0)
		m_nRecordCount += m_nCount19;
	if (m_nCount20 > 0)
		m_nRecordCount += m_nCount20;
	if (m_nCount21 > 0)
		m_nRecordCount += m_nCount21;
	if (m_nCount22 > 0)
		m_nRecordCount += m_nCount22;
	if (m_nCount23 > 0)
		m_nRecordCount += m_nCount23;
	
	if(!AllocateMemory())
	{
		Close();
		return FALSE;
	}
	
	if (!GetDataFromTable("CN_STK16_Fbsc_R", STK16_FBSC))
		m_nCount16 = 0;
	if (!GetDataFromTable("CN_STK17_FHind_R", STK17_FHIND))
		m_nCount17 = 0;
	if (!GetDataFromTable("CN_STK18_ASSET1_R", STK18_ASSET1))
		m_nCount18 = 0;
	if (!GetDataFromTable("CN_STK19_ASSET2_R", STK19_ASSET2))
		m_nCount19 = 0;
	if (!GetDataFromTable("CN_STK20_PROFIT1_R", STK20_PROFIT1))
		m_nCount20 = 0;
	if (!GetDataFromTable("CN_STK21_PROFIT2_R", STK21_PROFIT2))
		m_nCount21 = 0;
	if (!GetDataFromTable("CN_STK22_CASH1_R", STK22_CASH1))
		m_nCount22 = 0;
	if (!GetDataFromTable("CN_STK23_CASH2_R", STK23_CASH2))
		m_nCount23 = 0;

	return TRUE;
}
/*
void CCN_Subject_Table::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}  */

BOOL CCN_Subject_Table::WriteData()
{
	return TRUE;
}

BOOL CCN_Subject_Table::GetDataFromTable(LPCTSTR lpTableName, unsigned short nTableNum)
{
	tagSubjectTableEx info;
	if (!g_database.OpenTable(lpTableName))
	{
		REPORT(MN, T("OpenTable(\"%s\") failed\n"), RPT_ERROR);
		return FALSE;
	}

	switch(nTableNum)
	{
	case STK16_FBSC:
		m_n16Pos = CBaseFile::m_nRecordPos;
		break;
	case STK17_FHIND:
		m_n17Pos = CBaseFile::m_nRecordPos;
		break;
	case STK18_ASSET1:
		m_n18Pos = CBaseFile::m_nRecordPos;
		break;
	case STK19_ASSET2:
		m_n19Pos = CBaseFile::m_nRecordPos;
		break;
	case STK20_PROFIT1:
		m_n20Pos = CBaseFile::m_nRecordPos;
		break;
	case STK21_PROFIT2:
		m_n21Pos = CBaseFile::m_nRecordPos;
		break;
	case STK22_CASH1:
		m_n22Pos = CBaseFile::m_nRecordPos;
		break;
	case STK23_CASH2:
		m_n23Pos = CBaseFile::m_nRecordPos;
		break;
	}
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&info, 0, sizeof(tagSubjectTableEx));
		info.TableNum = nTableNum;
		g_database.GetCollect("IND_ID", (LPCTSTR)info.table.IND_ID, 10);
		g_database.GetCollect("IND_NM", (LPCTSTR)info.table.IND_NM, 40);
		g_database.GetCollect("IND_ENM", (LPCTSTR)info.table.IND_ENM, 40);
		if (nTableNum != STK16_FBSC)
			g_database.GetCollect("Mea_Unit", (LPCTSTR)info.table.Mea_Unit, 10);
		
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

int CCN_Subject_Table::MovePos(unsigned short nTableNum)
{
	int nRecordCount = 0;
	CBaseFile::MoveFirst();
	switch(nTableNum)
	{
	case STK16_FBSC:
		{
			if (m_nCount16 > 0)
			{
				nRecordCount = m_nCount16;
				CBaseFile::Move(m_n16Pos);
			}
			break;
		}
	case STK17_FHIND:
		{
			if (m_nCount17 > 0)
			{
				nRecordCount = m_nCount17;
				CBaseFile::Move(m_n17Pos);
			}
			break;
		}
	case STK18_ASSET1:
		{
			if (m_nCount18 > 0)
			{
				nRecordCount = m_nCount18;
				CBaseFile::Move(m_n18Pos);
			}
			break;
		}
	case STK19_ASSET2:
		{
			if (m_nCount19 > 0)
			{
				nRecordCount = m_nCount19;
				CBaseFile::Move(m_n19Pos);
			}
			break;
		}
	case STK20_PROFIT1:
		{
			if (m_nCount20 > 0)
			{
				nRecordCount = m_nCount20;
				CBaseFile::Move(m_n20Pos);
			}
			break;
		}
	case STK21_PROFIT2:
		{
			if (m_nCount21 > 0)
			{
				nRecordCount = m_nCount21;
				CBaseFile::Move(m_n21Pos);
			}
			break;
		}
	case STK22_CASH1:
		{
			if (m_nCount22 > 0)
			{
				nRecordCount = m_nCount22;
				CBaseFile::Move(m_n22Pos);
			}
			break;
		}
	case STK23_CASH2:
		{
			if (m_nCount23 > 0)
			{
				nRecordCount = m_nCount23;
				CBaseFile::Move(m_n23Pos);
			}
			break;
		}
	default:
		REPORT(MN, T("不能处理的科目表编码\n"), RPT_ERROR);
	}

	return nRecordCount;
}