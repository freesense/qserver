//HK_MKT01_Idx.cpp
#include "stdafx.h"
#include "HK_MKT01_Idx.h"
#include "TMDdef.h"
#include "DataBaseEx.h"
////////////////////////////////////////////////////////////////////////
#define FIELD_COUNT     8
#define CODE_LEN        8
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

CHK_MKT01_Idx::CHK_MKT01_Idx()
{
}

CHK_MKT01_Idx::~CHK_MKT01_Idx()
{
}

BOOL CHK_MKT01_Idx::Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
						  DWORD dwCreationDisposition)
{
	{//Init base class variable
		m_bIsOpen     = TRUE;
		m_nRecordLen  = sizeof(tagMkt_Idx);
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
		m_pFieldInfo[1].FieLen = 2;
		m_pFieldInfo[1].BegPos = nPos;
		nPos += 2;
		m_pFieldInfo[2].FieLen = 1;
		m_pFieldInfo[2].BegPos = nPos;
		nPos += 1;
		m_pFieldInfo[3].FieLen = 16;
		m_pFieldInfo[3].BegPos = nPos;
		nPos += 16;
		m_pFieldInfo[4].FieLen = 16;
		m_pFieldInfo[4].BegPos = nPos; 
		nPos += 16;
		m_pFieldInfo[5].FieLen = 4;
		m_pFieldInfo[5].BegPos = nPos;
		nPos += 4;
		m_pFieldInfo[6].FieLen = 2;
		m_pFieldInfo[6].BegPos = nPos; 
		nPos += 2;
		m_pFieldInfo[7].FieLen = 4;
		m_pFieldInfo[7].BegPos = nPos; 
	}
	return Requery();
}

void CHK_MKT01_Idx::Close()
{
	FreeMemory();
	CBaseFile::Close();
}

BOOL CHK_MKT01_Idx::Requery()
{
	m_nDataSize = 0;
	FreeMemory();
	if (!g_database.OpenTable("HK_MKT01_Idx"))
	{
		REPORT(MN, T("OpenTable(\"HK_MKT01_Idx\") failed\n"), RPT_ERROR);
		return FALSE;
	}
	m_nRecordCount = g_database.GetRecordCount();

	if(!AllocateMemory())
	{
		g_database.CloseTable();
		Close();
		return FALSE;
	}

	tagMkt_Idx mktIdx;
	while(!g_database.IsEOF())
	{
		if (CBaseFile::IsEOF())
		{
			REPORT(MN, T("SetRecordBuffer()将越界\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		memset(&mktIdx, 0, sizeof(tagMkt_Idx));
		g_database.GetCollect("IDX_CD", (LPCTSTR)mktIdx.Idx_cd, 10);
		//g_database.GetCollect("VAR_CL", (LPCTSTR)mktIdx.Var_cl, 2);
		//g_database.GetCollect("MKT_CL", (LPCTSTR)mktIdx.Mkt_cl, 1);
		//g_database.GetCollect("IDX_SNM", (LPCTSTR)mktIdx.Idx_snm, 16);
		//g_database.GetCollect("IDX_ESNM", (LPCTSTR)mktIdx.Idx_esnm, 16);
		//g_database.GetCollect("IDX_SSNM", (LPCTSTR)mktIdx.Idx_ssnm, 4);

		g_database.GetCollect("IDX_NM", (LPCTSTR)mktIdx.Idx_snm, 16);
		g_database.GetCollect("IDX_ENM", (LPCTSTR)mktIdx.Idx_esnm, 16);
		//mktIdx.pStockCode = g_database.GetMkt_Idx("HK_MKT04_Samp", (const char*)mktIdx.Idx_cd,
		//	(int&)mktIdx.nRecordCount);

		//m_nDataSize += sizeof(tagIndexInfo) + mktIdx.nRecordCount*CODE_LEN;
		if(!SetRecordBuffer((const char*)&mktIdx))
		{
			REPORT(MN, T("SetRecordBuffer() failed\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		CBaseFile::MoveNext();
		g_database.MoveNext();
	}

	g_database.CloseTable();

	//获取指数样本股票
	if (!g_database.OpenTable("HK_MKT04_Samp"))
	{
		REPORT(MN, T("OpenTable(\"HK_MKT04_Samp\") failed\n"), RPT_ERROR);
		return FALSE;
	}
	//////////////////////////////////////////////////////////////
	CBaseFile::MoveFirst();
	while(!CBaseFile::IsEOF())
	{
		memset(&mktIdx, 0, sizeof(tagMkt_Idx));
		if(!GetRecordToBuffer((char*)&mktIdx))
		{
			REPORT(MN, T("GetRecordToBuffer() failed\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		//获得股票的个数
		g_database.MoveFirst();
		int nCount = 0;
		while(!g_database.IsEOF())
		{
			char szIdxCode[10];
			g_database.GetCollect("IDX_CD", (LPCTSTR)szIdxCode, 10);
			if (_stricmp(szIdxCode, (const char*)mktIdx.Idx_cd) == 0)
				nCount++;

			g_database.MoveNext();
		}
		mktIdx.nRecordCount = nCount;

		if (nCount <= 0)  //如果没有股票就查下一个
		{
			m_nDataSize += sizeof(tagIndexInfo);
			CBaseFile::MoveNext();
			continue;
		}

		const int nCodeLen = 8;
		char* pStockCode = new char[nCodeLen*nCount];
		if (pStockCode == NULL)
		{
			REPORT(MN, T("分配内存[pStockCode]失败\n"), RPT_ERROR);
			g_database.CloseTable();
			return NULL;
		}
		char* pBufPos = pStockCode;
		//
		g_database.MoveFirst();
		while(!g_database.IsEOF())
		{
			char szIdxCode[10];
			g_database.GetCollect("IDX_CD", (LPCTSTR)szIdxCode, 10);
			if (_stricmp(szIdxCode, (const char*)mktIdx.Idx_cd) == 0)
			{
				char szStockCode[nCodeLen];
				g_database.GetCollect("SEC_CD", (LPCTSTR)szStockCode, nCodeLen);
				memcpy(pBufPos, szStockCode, nCodeLen);
				pBufPos += nCodeLen;
			}

			g_database.MoveNext();
		}
		//
		mktIdx.pStockCode = pStockCode;
		m_nDataSize += sizeof(tagIndexInfo) + mktIdx.nRecordCount*CODE_LEN;
		if(!SetRecordBuffer((const char*)&mktIdx))
		{
			REPORT(MN, T("SetRecordBuffer() failed\n"), RPT_ERROR);
			g_database.CloseTable();
			return FALSE;
		}
		CBaseFile::MoveNext();
	}
	//////////////////////////////////////////////////////////////
	CBaseFile::MoveFirst();
	g_database.CloseTable();

	return TRUE;
}

/*void CHK_MKT01_Idx::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}  */

BOOL CHK_MKT01_Idx::WriteData()
{
	return TRUE;
}

void CHK_MKT01_Idx::FreeMemory()
{
	CBaseFile::MoveFirst();
	tagMkt_Idx mktIdx;
	while(!IsEOF())
	{
		memset(&mktIdx, 0, sizeof(tagMkt_Idx));
		if (GetRecordToBuffer((char*)&mktIdx))
		{
			if (mktIdx.pStockCode != NULL)
			{
				delete []mktIdx.pStockCode;
				mktIdx.pStockCode = NULL;
			}
		}
		CBaseFile::MoveNext();
	}
}

int CHK_MKT01_Idx::GetDataSize()
{
	return m_nDataSize;
}	