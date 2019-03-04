//StockInfo.cpp
#include "stdafx.h"
#include "StockInfo.h"
#include <process.h>

CStockInfo g_StockInfo;
///////////////////////////////////////////////////////////////////////////////////////////////////
CStockInfo::CStockInfo()
{
	m_nUpdateDate = 0;
	m_nBufsize = 40*1024;
	m_pStockInfo = (char*)mpnew(m_nBufsize);
	m_nStockCount = 0;
}

CStockInfo::~CStockInfo()
{
	if (m_pStockInfo != NULL)
		mpdel(m_pStockInfo);
}

void CStockInfo::UpdateStockInfo()
{
	unsigned int nCurDate = 0;
	unsigned int nCurTime = 0;
	GetCurDateTime(nCurDate, nCurTime);

	if ( m_nUpdateDate == 0 || (nCurTime >= 930 && m_nUpdateDate != nCurDate) ) //明天9：30更新股票信息
	{
		SendInitRequest();
	}

	DumpMemPool();
}

void CStockInfo::GetCurDateTime(unsigned int& nCurDate, unsigned int& nCurTime)
{
	time_t  timeCur;
	time(&timeCur);
	tm tmCur;
	localtime_s(&tmCur, &timeCur);

	nCurDate = (tmCur.tm_year+1900)*10000 + (tmCur.tm_mon+1)*100 + tmCur.tm_mday;
	nCurTime = tmCur.tm_hour*100 + tmCur.tm_min;
}

void CStockInfo::SendInitRequest()
{
	REPORT(MN, T("向行情服务器发送初始化请求\n"), RPT_INFO);
	struct _new_req
	{
		CommxHead head;
		unsigned short funcno1;
		unsigned short reqnum;
		unsigned short funcno2;
	} *preq;

	preq = (_new_req*)mpnew(sizeof(_new_req));
	memset(preq, 0x00, sizeof(_new_req));
	preq->head.Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	preq->head.SerialNo = 0;
	preq->head.Length = sizeof(_new_req) - sizeof(CommxHead);
	preq->reqnum = 1;
	preq->funcno1 = preq->funcno2 = PT_INIT;
	CHECK_RUN(-1 == g_pcq->Write(preq, sizeof(_new_req)), MN, T("向行情服务器发送请求失败\n"), RPT_ERROR, return);
}

void CStockInfo::Answer(char *lpData, unsigned int nLength)
{
	WGUARD(LockSingle, m_lockStockInfo, grd);

	unsigned short* pStockCount = (unsigned short*)(lpData+6);
	REPORT(MN, T("获得股票个数%d\n", *pStockCount), RPT_INFO);
	if (*pStockCount <= 0)
		return;

	unsigned int nCurDate = 0;
	unsigned int nCurTime = 0;
	GetCurDateTime(nCurDate, nCurTime);
	m_nUpdateDate = nCurDate;

	m_nStockCount = *pStockCount;
	unsigned int nNeedBufSize = m_nStockCount * sizeof(tagStockInfo);
	if (nNeedBufSize > m_nBufsize)
	{ 
		if (m_pStockInfo != NULL)
			mpdel(m_pStockInfo);
	}

	m_pStockInfo = (char*)mpnew(nNeedBufSize);
	m_nBufsize = nNeedBufSize;
	memcpy(m_pStockInfo, lpData+8, m_nBufsize);
}

char* CStockInfo::GetStockCode(char *pSpell, int& nStockCount)
{
	WGUARD(LockSingle, m_lockStockInfo, grd);
	if (m_pStockInfo == NULL)
		return NULL;

	int nBufStockCount = 20;
	char* pStockCodeBuf = (char*)mpnew(sizeof(tagStockInfo)*nBufStockCount);
	char* pStockCodeBufPos = pStockCodeBuf;

	tagStockInfo* pStockInfo = (tagStockInfo*)m_pStockInfo;
	int nCount = 0;
	char szSpell[50];
	for(int i=0; i<(int)m_nStockCount; i++)
	{
		memset(szSpell, 0, 50);
		memcpy(szSpell, pStockInfo[i].szSpell, 4);
		if (_stricmp(szSpell, pSpell) == 0)
		{
			nCount++;
			if (nCount > nBufStockCount)
			{
				char* pNewStockCodeBuf = (char*)mpnew(nCount+10);
				memcpy(pNewStockCodeBuf, pStockCodeBuf, sizeof(tagStockInfo)*nBufStockCount);
				mpdel(pStockCodeBuf);
				pStockCodeBuf = pNewStockCodeBuf;
				pStockCodeBufPos = pStockCodeBuf + sizeof(tagStockInfo)*nBufStockCount;
			}
			memcpy(pStockCodeBufPos, &pStockInfo[i], sizeof(tagStockInfo));
			pStockCodeBufPos += sizeof(tagStockInfo);
		}
	}

	nStockCount = nCount; 
	return pStockCodeBuf;
}

unsigned short CStockInfo::GetMktType(char* pStockCode)
{
	unsigned short nMktType = 0;
	WGUARD(LockSingle, m_lockStockInfo, grd);
	if (m_pStockInfo == NULL)
		return 0;

	tagStockInfo* pStockInfo = (tagStockInfo*)m_pStockInfo;
	char szCode[16];
	for(int i=0; i<(int)m_nStockCount; i++)
	{
		memset(szCode, 0, 16);
		memcpy(szCode, pStockInfo[i].szCode, 16);

		char* pMty = strstr(szCode, ".");
		pMty[0] = 0;
		pMty += 1;

		unsigned int nCodeSrc = atoi(pStockCode);
		unsigned int nCode = atoi(szCode);
		if (_strcmpi(pStockCode, szCode) != 0)
			continue;

		if (strcmp(pMty, "sz") == 0 || strcmp(pMty, "SZ") == 0)
		{
			if(nCode < 3000)
				nMktType = SZ_AStock;
			else if (nCode < 100000)
				nMktType = SZ_Other;
			else if (nCode < 160000)
				nMktType = SZ_Bond;
			else if (nCode < 200000)
				nMktType = SZ_Fund;
			else if (nCode < 300000)
				nMktType = SZ_BStock;
			else
				nMktType = SZ_Index;
		}
		else if (strcmp(pMty, "sh") == 0 || strcmp(pMty, "SH") == 0)
		{
			if(nCode < 696)
				nMktType = SH_Index;
			else if (nCode < 500001)
				nMktType = SH_Bond;
			else if (nCode < 600000)
				nMktType = SH_Fund;
			else if (nCode < 700000)
				nMktType = SH_AStock;
			else if (nCode < 900000)
				nMktType = SH_Other;
			else
				nMktType = SH_BStock;
		}
		else if (strcmp(pMty, "hk") == 0 || strcmp(pMty, "HK") == 0)
		{
			nMktType = 8196;
		}
		break;
	}
	return nMktType;
}