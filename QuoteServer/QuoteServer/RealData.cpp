#include "StdAfx.h"
#include "atlconv.h"
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "RealData.h"
#include "qs_data_struct.h"
#include "farm.h"
#include "..\\..\\public\\data_struct.h"
#include "wordspell.h"
#include "..\\plug.h"
#include "..\\..\\public\\commx\\synch.h"
#include "..\\..\\public\\commx\\report.h"
#include "DataPool.h"
#include "config.h"

extern CPlugManager * g_pPlugs;
extern CDataFarm * g_pDataFarm;
extern CWordSpell * g_pSpell;
extern CDataPool  g_DataPool;

extern Quote * GetSymbolQuote(char * c_pSymbol);
#define MODULE_NAME "QS"
CRealData::CRealData(void)
: m_unRepLen(0)
, m_unReqLen(0)
, m_pReqBuf(NULL)
, m_pRepBuf(NULL)
, m_unReqBufLen(0)
, m_unRepBufLen(0)
{
	m_pReqBuf = (char *)malloc(4096 * 10);
	m_pRepBuf = (char *)malloc(1024 * 512 * 4);
	m_unRepBufLen = 1024 * 512 * 4;
}

CRealData::~CRealData(void)
{
	free(m_pReqBuf);
	free(m_pRepBuf);
}

void CRealData::Reset()
{
	m_unRepLen = 0;
	m_unReqLen = 0;
}
bool CRealData::PharseReq(char * c_pReqBuf,unsigned short &c_usFuncNo,unsigned int & c_unReqLen)
{
	PPACKETHEAD  pReq;
	pReq = (PPACKETHEAD)c_pReqBuf;
	c_unReqLen = pReq->m_commxHead.GetLength() + sizeof(unsigned int) * 2;//*((unsigned int *)(c_pReqBuf + sizeof(unsigned int))) + sizeof(unsigned int) * 2;
	c_usFuncNo = pReq->m_commxHead.GetUS(pReq->m_usFuncNo);//(*((unsigned short *)(c_pReqBuf + sizeof(unsigned int) * 2)));
	return true;
}
// 处理请求，结果放在m_szBuffer中，长度放在m_unLen中
bool CRealData::GetData(char * c_pReq)
{
	PPACKETHEAD  pReq;
	pReq = (PPACKETHEAD)c_pReq;
	//首先复制请求包头到返回包
	//REPORT("QS",T("组织数据包开始\n"),RPT_INFO);
	memcpy(m_pRepBuf,c_pReq,sizeof(PACKETHEAD));
	int nRet = MakeData(m_pRepBuf,c_pReq);
	if (nRet > 0)
		m_unRepLen = nRet;
	else
		m_unRepLen = 0;
	//REPORT("QS",T("组织数据包结束%d\n",m_unRepLen),RPT_INFO);
	if (m_unRepLen > 0)
		return true;
	else
		return false;
}

char * CRealData::GetRepBuf(void)
{
	return m_pRepBuf;
}

unsigned int CRealData::GetRepLen(void)
{
	return m_unRepLen;
}

// 根据请求生成数据
int CRealData::MakeData(char * c_pBuf, char * c_pReq)
{
	PPACKETHEAD pHead;
	PPACKETHEAD pRepHead = (PPACKETHEAD)c_pBuf;
	REQ_ITEM ReqItem;
	pHead = (PPACKETHEAD)c_pReq;

	char *pBuf = c_pBuf;
	char *pReq = c_pReq;
	int iTotalLen = 0;
	int iReq = 0;
	int iPassLen;
	int iRetLen = 0;
	iPassLen = 0;
	//REPORT("QS",T("MakeData PREPHEAD [%d][%d][%d]\n",pRepHead->m_commxHead.GetVersion(),pRepHead->m_commxHead.GetEncoding(),pRepHead->m_commxHead.GetByteorder()),RPT_INFO);
	pReq += sizeof(PACKETHEAD);
	pBuf += sizeof(PACKETHEAD);//返回数据区
	//cout<<"m_usReqCount ="<< pHead->m_commxHead.GetUS(pHead->m_usReqCount)<<"::"<<pHead->m_usFuncNo<<"::" << sizeof(PACKETHEAD) << endl;
	while(iReq < pHead->m_commxHead.GetUS(pHead->m_usReqCount))//逐个处理
	{
		//cout<<"type="<<*((unsigned short *)pReq)<<" ORDER= "<<pHead->m_commxHead.GetByteorder()<<endl;
		ReqItem.m_unType = pHead->m_commxHead.GetUS(*((unsigned short *)pReq));
		ReqItem.m_usStockCount = pHead->m_commxHead.GetUS(*((unsigned short *)(pReq + sizeof(unsigned short))));
		ReqItem.m_pStocks = pReq + sizeof(unsigned short) * 2;
		//cout<<"m_unType ="<< ReqItem.m_unType<< endl;
		//REPORT("QS",T("处理类型[%d]\n",ReqItem.m_unType & 0x0FFF),RPT_INFO);
		switch(ReqItem.m_unType & 0x0FFF)
		{
		case LINETESTREQ://线路测试
			pHead->m_usFuncNo = pHead->m_commxHead.GetUS(LINETESTREP);
			iRetLen = MakeLINETESTREP(pBuf,(char *)&ReqItem,iPassLen);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case INITDATA:
			m_unRepLen = pBuf - c_pBuf;
			iRetLen = MakeINITDATA(pBuf,(char *)&ReqItem,iPassLen);
			//cout<<"iRetLen ="<< iRetLen<< endl;
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case TRACEDATA:
			iRetLen = MakeTRACEDATA(pBuf,(char *)&ReqItem,iPassLen);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case REPORTDATA:
			m_unRepLen = pBuf - c_pBuf;
			iRetLen = MakeREPORTDATA(pBuf,(char *)&ReqItem,iPassLen);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case REALMINSDATA:
			iRetLen = MakeREALMINSDATA(pBuf,(char *)&ReqItem,iPassLen);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case REALMINSDATA2:
			iRetLen = MakeREALMINSDATA2(pBuf,(char *)&ReqItem,iPassLen);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case HISKDAYDATA:
			iRetLen = MakeHISKDATA(pBuf,(char *)&ReqItem,iPassLen,HISKDAYDATA);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case HISKWEEKDATA:
			iRetLen = MakeHISKDATA(pBuf,(char *)&ReqItem,iPassLen,HISKWEEKDATA);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		case HISKMONTHDATA:
			iRetLen = MakeHISKDATA(pBuf,(char *)&ReqItem,iPassLen,HISKMONTHDATA);
			if (iRetLen <= 0)
				return iRetLen;
			break;
		default:
			return 0;
		}
		iReq++;
		iTotalLen += iRetLen;
		pBuf += iRetLen;
		pReq += iPassLen;
		iPassLen = 0;
		//REPORT("QS",T("处理类型[%d] Over\n",ReqItem.m_unType & 0x0FFF),RPT_INFO);
	}
	//REPORT("QS",T("MakeData PREPHEAD [%d][%d][%d]\n",pRepHead->m_commxHead.GetVersion(),pRepHead->m_commxHead.GetEncoding(),pRepHead->m_commxHead.GetByteorder()),RPT_INFO);
	pRepHead->m_commxHead.SetLength(iTotalLen + sizeof(unsigned short) * 2);
	return iTotalLen + sizeof(PACKETHEAD);
}

int CRealData::MakeINITDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen)
{
	int iRetLen = 0;
	char * pBuf;
	char acSymbol[10];
	PREQ_ITEM pItem=(PREQ_ITEM)c_pReqItem;
	pBuf = c_pDataBuf;
	
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	m_unRepLen += sizeof(unsigned short) * 2;
	
	int iRet = 0;
	int iLen = 0;
	SDataItem * pDataItem;
	g_DataPool.m_rwlock.WaitRead();
	sprintf(acSymbol,"INIT_%d_%d",pHead->GetEncoding(),pHead->GetByteorder());
	pDataItem = g_DataPool.ReadData(acSymbol);
	if (pDataItem)
	{
		iLen = pDataItem->m_nDataLen;
		if (g_cfg.m_bDebug)
		REPORT("QS",T("FIND INIT[%s][%d]\n",acSymbol,iLen),RPT_INFO);
		memcpy(pBuf,pDataItem->m_pBuf,pDataItem->m_nDataLen);
		g_DataPool.m_rwlock.EndRead();
	}
	else
	{
		g_DataPool.m_rwlock.EndRead();
		CQuoteData QuoteData(this);
		//REPORT("QS",T("组织数据包开始for_each\n"),RPT_INFO);
		iRet = g_pDataFarm->m_hashIdx.for_each(QuoteData);
		//REPORT("QS",T("组织数据包结束for_each iRet=%d,[%d][%d]\n",iRet,pHead->GetUS(INITDATA),pHead->GetUS(iRet)),RPT_INFO);
		
		*((unsigned short *)(pBuf)) = pHead->GetUS(INITDATA);
		*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(iRet);
		if (pHead->GetEncoding() == CommxHead::ANSI)
		iLen = sizeof(unsigned short) * 2 + (STOCK_CODE_LEN + STOCK_NAME_LEN + sizeof(unsigned int) + 4) * iRet;
		else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
			{
				iLen = sizeof(unsigned short) * 2 + (STOCK_CODE_LEN * 2 + STOCK_NAME_LEN * 2 + sizeof(unsigned int) + 4) * iRet;
			}
			else
			if (pHead->GetEncoding() == CommxHead::UTF8)
			{
				iLen = sizeof(unsigned short) * 2 + (STOCK_CODE_LEN * 3 + STOCK_NAME_LEN * 3 + sizeof(unsigned int) + 4) * iRet;
			}
		if (iLen > 1024 * 10)
		{
			g_DataPool.m_rwlock.WaitWrite();
			g_DataPool.PutData(acSymbol,pBuf,iLen + 1,iLen);
			if (g_cfg.m_bDebug)
				REPORT("QS",T("PUT INIT[%s][%d]\n",acSymbol,iLen),RPT_INFO);
			g_DataPool.m_rwlock.EndWrite();
		}
	}
	c_iPassLen = sizeof(unsigned short) * 2;
	//return sizeof(unsigned short) * 2 + (STOCK_CODE_LEN + STOCK_NAME_LEN + sizeof(unsigned int) + 4) * iRet;
	return iLen;
}

int CRealData::MakeLINETESTREP(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen)
{
	int iRetLen = 0;
	char * pBuf;
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	PREQ_ITEM pItem=(PREQ_ITEM)c_pReqItem;
	pBuf = c_pDataBuf;
	*((unsigned short *)pBuf) = pHead->GetUS(LINETESTREP);
	*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(pItem->m_usStockCount);
	pBuf += sizeof(unsigned short) * 2;
	
	return iRetLen + sizeof(unsigned short) * 2;
}

int CRealData::MakeREPORTDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen)
{
	/*
	Quote * pQuote = GetSymbolQuote("000001.SH");
	if (pQuote)
	{
		REPORT(LockSingle,MODULE_NAME,T("000001.SH GetSymbolQuote<%d> %d %d %d\n",pQuote->jrkp,pQuote->zjjg,pQuote->zrsp,pQuote->zgjg),RPT_INFO);
	}
	else
		REPORT(LockSingle,MODULE_NAME,T("000001.SH GetSymbolQuote return NULL\n"),RPT_INFO);

	pQuote = GetSymbolQuote("600036.sh");
	if (pQuote)
	{
		REPORT(LockSingle,MODULE_NAME,T("600036.sh GetSymbolQuote<%d> %d %d %d\n",pQuote->jrkp,pQuote->zjjg,pQuote->zrsp,pQuote->zgjg),RPT_INFO);
	}
	else
		REPORT(LockSingle,MODULE_NAME,T("600036.sh GetSymbolQuote return NULL\n"),RPT_INFO);
	*/
	Quote * pQuote399106 = GetSymbolQuote("399106.SZ");
	Quote * pQuote399305 = GetSymbolQuote("399305.SZ");
	Quote * pQuote000011 = GetSymbolQuote("000011.SH");
	
	int iRetLen = 0;
	char * pBuf;
	unsigned short usRealCount = 0;//实际返回股票个数字
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	PREQ_ITEM pItem=(PREQ_ITEM)c_pReqItem;
	pBuf = c_pDataBuf;
	*((unsigned short *)pBuf) = pHead->GetUS(pItem->m_unType);
	*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(pItem->m_usStockCount);//先设置为请求值
	
	char acSymbol[STOCK_CODE_LEN + 1];
	//if (pHead->GetUS(pItem->m_usStockCount) == 0)
	if (pItem->m_usStockCount > 500)
		return -1;
	if (pItem->m_usStockCount == 0)
	{
		m_unRepLen += sizeof(unsigned short) * 2;

		CQuoteData QuoteData(this,1);
		int iRet = 0;
		iRet = g_pDataFarm->m_hashIdx.for_each(QuoteData);
		*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(iRet);
		c_iPassLen = sizeof(unsigned short) * 2;
		if (pHead->GetEncoding() == CommxHead::ANSI)
		return sizeof(unsigned short) * 2 + (STOCK_CODE_LEN + STOCK_NAME_LEN + 40 * sizeof(unsigned int)) * iRet;
		else
		if (pHead->GetEncoding() == CommxHead::UTF16LE)
		return sizeof(unsigned short) * 2 + ((STOCK_CODE_LEN + STOCK_NAME_LEN)* 2 + 40 * sizeof(unsigned int)) * iRet;
		else
		if (pHead->GetEncoding() == CommxHead::UTF8)
		return sizeof(unsigned short) * 2 + ((STOCK_CODE_LEN + STOCK_NAME_LEN)* 3 + 40 * sizeof(unsigned int)) * iRet;
	}
	else
	{		
		pBuf += sizeof(unsigned short) * 2;
		
		for(int i=0; i<pItem->m_usStockCount; i++)
		{
			memset(acSymbol,0,sizeof(acSymbol));
			pHead->GetStr(pItem->m_pStocks + c_iPassLen,STOCK_CODE_LEN,acSymbol,STOCK_CODE_LEN);
			if (strlen(acSymbol) < 6)
				return -1;

			//获取m_pIdx
			Quote * pQuote;
			int nCount = 0;
			int n=0;
			//CQuoteData QuoteData(this);
			
			if (g_pDataFarm->m_hashIdx.cast(acSymbol,*this,true) > 0)
			{
				
				//if (QuoteData.m_Idx.idxQuote > 0)
				if (true)
				{
					pQuote = g_pDataFarm->GetQuote(this->m_Idx.idxQuote);
					if (pQuote)
					{
						//这时候才确认
						SummaryInfo * pSumInfo;
						pSumInfo = g_IndexSummary.GetIndex(pQuote->szStockCode);

						pHead->PutStr(acSymbol,pBuf + iRetLen,STOCK_CODE_LEN);
						if (pHead->GetEncoding() == CommxHead::ANSI)
							iRetLen += STOCK_CODE_LEN;
						else
						if (pHead->GetEncoding() == CommxHead::UTF16LE)
							iRetLen += STOCK_CODE_LEN * 2;
						else
						if (pHead->GetEncoding() == CommxHead::UTF8)
							iRetLen += STOCK_CODE_LEN * 3;
						
						pHead->PutStr(pQuote->szStockName,pBuf + iRetLen,STOCK_NAME_LEN);
						if (pHead->GetEncoding() == CommxHead::ANSI)
							iRetLen += STOCK_NAME_LEN;
						else
						if (pHead->GetEncoding() == CommxHead::UTF16LE)
							iRetLen += STOCK_NAME_LEN * 2;
						else
						if (pHead->GetEncoding() == CommxHead::UTF8)
							iRetLen += STOCK_NAME_LEN * 3;

						//返回的股票数字加一
						usRealCount++;
						
						*((unsigned int *)(pBuf + iRetLen)) = pHead->GetUI(pQuote->zrsp);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int))) = pHead->GetUI(pQuote->jrkp);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 2)) = pHead->GetUI(pQuote->ztjg);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 3)) = pHead->GetUI(pQuote->dtjg);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 4)) = pHead->GetUI(pQuote->syl1);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 5)) = pHead->GetUI(pQuote->syl2);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 6)) = pHead->GetUI(pQuote->zgjg);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 7)) = pHead->GetUI(pQuote->zdjg);
						if (pQuote->zjjg > 0)
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 8)) = pHead->GetUI(pQuote->zjjg);
						else
						if (pQuote->jrkp > 0)
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 8)) = pHead->GetUI(pQuote->jrkp);
						else
							*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 8)) = pHead->GetUI(pQuote->zrsp);

						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 9)) = pHead->GetUI(pQuote->zjcj);
						unsigned int je=0,sl=0;
						je = pQuote->cjje;
						sl = pQuote->cjsl;
						if (strcmp(pQuote->szStockCode,"000001.SH") == 0)
						{
							if (pQuote000011)
							{
								je += pQuote000011->cjje;
								sl += pQuote000011->cjsl;
							}
						}
						else
						if (strcmp(pQuote->szStockCode,"399001.SZ") == 0)
						{
							if (pQuote399305 && pQuote399106)
							{
								je = pQuote399106->cjje + pQuote399305->cjje;
								je = pQuote399106->cjsl + pQuote399305->cjsl;
							}
							//g_StockKindManager.Summary("md_szwa",je,sl);
							//*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 10)) = pHead->GetUI(pQuote399106->cjsl + sl);
							//*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 11)) = pHead->GetUI(pQuote399106->cjje + je);
							//REPORT(MODULE_NAME,T("REPORTDATA 399001[%d][%d][%d][%d]\n",je,sl,pQuote399106->cjsl,pQuote399106->cjsl),RPT_INFO);
						}
						else
						if (strcmp(pQuote->szStockCode,"399002.SZ") == 0 || strcmp(pQuote->szStockCode,"399003.SZ") == 0 || strcmp(pQuote->szStockCode,"399005.SZ") == 0)
						{
							if (pQuote399305)
							{
								//je += pQuote399305->cjje;
								//je += pQuote399305->cjsl;
							}
						}
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 10)) = pHead->GetUI(sl);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 11)) = pHead->GetUI(je);

						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 12)) = pHead->GetUI(pQuote->cjbs);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 13)) = pHead->GetUI(pQuote->BP1);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 14)) = pHead->GetUI(pQuote->BM1);
						if (pSumInfo)
							*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 14)) = pHead->GetUI(pSumInfo->m_usUp);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 15)) = pHead->GetUI(pQuote->BP2);						
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 16)) = pHead->GetUI(pQuote->BM2);
						if (pSumInfo)
							*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 16)) = pHead->GetUI(pSumInfo->m_usNoChange);

						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 17)) = pHead->GetUI(pQuote->BP3);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 18)) = pHead->GetUI(pQuote->BM3);
						if (pSumInfo)
							*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 18)) = pHead->GetUI(pSumInfo->m_usDown);

						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 19)) = pHead->GetUI(pQuote->BP4);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 20)) = pHead->GetUI(pQuote->BM4);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 21)) = pHead->GetUI(pQuote->BP5);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 22)) = pHead->GetUI(pQuote->BM5);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 23)) = pHead->GetUI(pQuote->SP1);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 24)) = pHead->GetUI(pQuote->SM1);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 25)) = pHead->GetUI(pQuote->SP2);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 26)) = pHead->GetUI(pQuote->SM2);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 27)) = pHead->GetUI(pQuote->SP3);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 28)) = pHead->GetUI(pQuote->SM3);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 29)) = pHead->GetUI(pQuote->SP4);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 30)) = pHead->GetUI(pQuote->SM4);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 31)) = pHead->GetUI(pQuote->SP5);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 32)) = pHead->GetUI(pQuote->SM5);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 33)) = pHead->GetUI(pQuote->day5pjzs);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 34)) = pHead->GetUI(pQuote->pjjg);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 35)) = pHead->GetUI(pQuote->wb);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 36)) = pHead->GetUI(pQuote->lb);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 37)) = pHead->GetUI(pQuote->np);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 38)) = pHead->GetUI(pQuote->wp);
						if (pQuote->zjjg > 0)
							*((int *)(pBuf + iRetLen + sizeof(unsigned int) * 39)) = pHead->GetUI((int)pQuote->zjjg - (int)pQuote->zrsp);
						else
							if (pQuote->jrkp > 0)
								*((int *)(pBuf + iRetLen + sizeof(unsigned int) * 39)) = pHead->GetUI((int)pQuote->jrkp - (int)pQuote->zrsp);
							else
								*((int *)(pBuf + iRetLen + sizeof(unsigned int) * 39)) = 0;
						*((int *)(pBuf + iRetLen + sizeof(unsigned int) * 40)) = pHead->GetUI(pQuote->zdf);
						*((int *)(pBuf + iRetLen + sizeof(unsigned int) * 41)) = pHead->GetUI(pQuote->zf);
						iRetLen += sizeof(unsigned int) * 42;
					}
				}
			}
			if (pHead->GetEncoding() == CommxHead::ANSI)
				c_iPassLen += STOCK_CODE_LEN;
			else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
				c_iPassLen += STOCK_CODE_LEN * 2;
			else
			if (pHead->GetEncoding() == CommxHead::UTF8)
				c_iPassLen += STOCK_CODE_LEN * 3;

		}
	}
	*((unsigned short *)(c_pDataBuf + sizeof(unsigned short))) = pHead->GetUS(usRealCount);//先设置为真正值
	c_iPassLen += sizeof(unsigned short) * 2;
	return iRetLen + sizeof(unsigned short) * 2;
}
int CRealData::MakeTRACEDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen)
{
	int iRetLen = 0;
	char * pBuf;
	unsigned short usRealCount = 0;
	unsigned short usRealStockCount = 0;
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	PREQ_ITEM pItem=(PREQ_ITEM)c_pReqItem;
	pBuf = c_pDataBuf;
	*((unsigned short *)pBuf) = pHead->GetUS(pItem->m_unType);
	*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(pItem->m_usStockCount);
	pBuf += sizeof(unsigned short) * 2;
	if (pItem->m_usStockCount > 10)
		return -1;

	char acSymbol[STOCK_CODE_LEN + 1];
	unsigned short usBeginTime;
	for(int i=0; i<pItem->m_usStockCount; i++)
	{

		memset(acSymbol,0,sizeof(acSymbol));
		pHead->GetStr(pItem->m_pStocks + c_iPassLen,STOCK_CODE_LEN,acSymbol,STOCK_CODE_LEN);
		if (strlen(acSymbol) < 6)
			return -1;

		usBeginTime = pHead->GetUS(*(unsigned short *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN));

		//获取m_pIdx
		TickBlock * pTickBlock;
		int nCount = 0;
		int n=0;
		//CQuoteData QuoteData(this);
		//if (g_pDataFarm->m_hashIdx.cast(acSymbol,QuoteData,true) > 0)
		if (g_pDataFarm->m_hashIdx.cast(acSymbol,*this,true) > 0)
		{
			//if (QuoteData.m_Idx.idxQuote >= 0)
			if (true)
			{
				usRealStockCount++;
				usRealCount = 0;
				pHead->PutStr(acSymbol,pBuf + iRetLen,STOCK_CODE_LEN);
				if (pHead->GetEncoding() == CommxHead::ANSI)
					iRetLen += STOCK_CODE_LEN;
				else
				if (pHead->GetEncoding() == CommxHead::UTF16LE)
					iRetLen += STOCK_CODE_LEN * 2;
				else
				if (pHead->GetEncoding() == CommxHead::UTF8)
					iRetLen += STOCK_CODE_LEN * 3;

				pTickBlock = g_pDataFarm->GetTick(this->m_Idx.idxTick);
				*((unsigned short *)(pBuf + iRetLen)) = pHead->GetUS(this->m_Idx.cntTick);//记录个数
				unsigned short * punCount = ((unsigned short *)(pBuf + iRetLen));
				iRetLen += sizeof(unsigned short);
				while(nCount < this->m_Idx.cntTick)
				{
					if (pTickBlock->unit[n].Time >= usBeginTime)
					{
						*((unsigned int *)(pBuf + iRetLen)) = pHead->GetUI(pTickBlock->unit[n].Time);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int))) = pHead->GetUI(pTickBlock->unit[n].Price);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 2)) = pHead->GetUI(pTickBlock->unit[n].Volume);
						*((unsigned char *)(pBuf + iRetLen + sizeof(unsigned int) * 3)) = pTickBlock->unit[n].Way;
						iRetLen += sizeof(unsigned int) * 3 + sizeof(unsigned char);
						usRealCount++;
					}
					n++;
					nCount++;
					if (n == TICK_PERBLOCK)//当前BLOCK已经读完
					{
						pTickBlock = g_pDataFarm->GetTick(pTickBlock->next);
						n = 0;
					}
				}
				//cout<<"usRealCount="<<usRealCount<<endl;
				*punCount = pHead->GetUS(usRealCount);//记录个数
			}
		}
		if (pHead->GetEncoding() == CommxHead::ANSI)
		c_iPassLen += STOCK_CODE_LEN + sizeof(unsigned short);
		else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
		c_iPassLen += STOCK_CODE_LEN * 2+ sizeof(unsigned short);
		else
			if (pHead->GetEncoding() == CommxHead::UTF8)
		c_iPassLen += STOCK_CODE_LEN * 3 + sizeof(unsigned short);

	}
	*((unsigned short *)(c_pDataBuf + sizeof(unsigned short))) = pHead->GetUS(usRealStockCount);//先设置为真正值
	c_iPassLen += sizeof(unsigned short) * 2;
	return iRetLen + sizeof(unsigned short) * 2;
}
int CRealData::MakeREALMINSDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen)
{
	int iRetLen = 0;
	char * pBuf;
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	unsigned short usRealCount = 0;
	PREQ_ITEM pItem=(PREQ_ITEM)c_pReqItem;
	pBuf = c_pDataBuf;
	*((unsigned short *)pBuf) = pHead->GetUS(pItem->m_unType);
	*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(pItem->m_usStockCount);
	pBuf += sizeof(unsigned short) * 2;

	char acSymbol[STOCK_CODE_LEN + 1];
	if (g_cfg.m_bDebug)
		REPORT("QS",T("REALMINS %d\n",pItem->m_usStockCount),RPT_INFO);
	if (pItem->m_usStockCount > 10)
		return -1;
	for(int i=0; i<pItem->m_usStockCount; i++)
	{
		memset(acSymbol,0,sizeof(acSymbol));
		pHead->GetStr(pItem->m_pStocks + c_iPassLen,STOCK_CODE_LEN,acSymbol,STOCK_CODE_LEN);
		if (strlen(acSymbol) < 6)
			return -1;

		//获取m_pIdx
		MinBlock * pMinBlock;
		int nCount = 0;
		int n=0;
		//CQuoteData QuoteData(this);
		//REPORT("QS",T("REALMINS TO cast[%s]\n",acSymbol),RPT_INFO);
		if (g_pDataFarm->m_hashIdx.cast(acSymbol,*this,true) > 0)
		{
			//if (QuoteData.m_Idx.idxQuote >= 0)
			if (true)
			{
				pHead->PutStr(acSymbol,pBuf + iRetLen,STOCK_CODE_LEN);

				if (pHead->GetEncoding() == CommxHead::ANSI)
					iRetLen += STOCK_CODE_LEN;
				else
					if (pHead->GetEncoding() == CommxHead::UTF16LE)
						iRetLen += STOCK_CODE_LEN * 2;
					else
						if (pHead->GetEncoding() == CommxHead::UTF8)
							iRetLen += STOCK_CODE_LEN * 3;
				usRealCount++;
	
				//REPORT("QS",T("[QuoteData.m_Idx.idxMinK=%d]\n",m_Idx.idxMinK),RPT_INFO);
				pMinBlock = g_pDataFarm->GetMinK(m_Idx.idxMinK);
				
				*((unsigned short *)(pBuf + iRetLen)) = pHead->GetUS(m_Idx.cntMinK);//记录个数
				iRetLen += sizeof(unsigned short);
				while(nCount < m_Idx.cntMinK)
				{
					//REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d]\n",n,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
					*((unsigned int *)(pBuf + iRetLen)) = pHead->GetUI(pMinBlock->unit[n].Time);
					*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int))) = pHead->GetUI(pMinBlock->unit[n].OpenPrice);
					*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 2)) = pHead->GetUI(pMinBlock->unit[n].MaxPrice);
					*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 3)) = pHead->GetUI(pMinBlock->unit[n].MinPrice);
					*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 4)) = pHead->GetUI(pMinBlock->unit[n].NewPrice);
					*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 5)) = pHead->GetUI(pMinBlock->unit[n].Volume);
					*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 6)) = pHead->GetUI(pMinBlock->unit[n].AvgPrice);
					iRetLen += sizeof(unsigned int) * 7;
					n++;
					nCount++;
					if (n == MINK_PERBLOCK)//当前BLOCK已经读完
					{
						pMinBlock = g_pDataFarm->GetMinK(pMinBlock->next);
						n = 0;
					}
				}
			}
			else
			{
				*((unsigned short *)(pBuf + iRetLen)) = 0;//记录个数
				iRetLen += sizeof(unsigned short);
			}
		}
		else
		{
				*((unsigned short *)(pBuf + iRetLen)) = 0;//记录个数
				iRetLen += sizeof(unsigned short);
		}
		//sprintf(pBuf + iRetLen,"0123456789");
		if (pHead->GetEncoding() == CommxHead::ANSI)
		c_iPassLen += STOCK_CODE_LEN;
		else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
		c_iPassLen += STOCK_CODE_LEN * 2;
		else
			if (pHead->GetEncoding() == CommxHead::UTF8)
		c_iPassLen += STOCK_CODE_LEN * 3;
	}
	*((unsigned short *)(c_pDataBuf + sizeof(unsigned short))) = pHead->GetUS(usRealCount);//先设置为真正值
	c_iPassLen += sizeof(unsigned short) * 2;
	return iRetLen + sizeof(unsigned short) * 2;
}
//两日分时
int CRealData::MakeREALMINSDATA2(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen)
{
	int iRetLen = 0;
	int iRet;
	char * pBuf;
	unsigned int unDateA,unDateB;
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	PREQ_ITEM pItem=(PREQ_ITEM)c_pReqItem;
	pBuf = c_pDataBuf;
	*((unsigned short *)pBuf) = pHead->GetUS(pItem->m_unType);
	*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(pItem->m_usStockCount);
	pBuf += sizeof(unsigned short) * 2;
	
	char acSymbol[STOCK_CODE_LEN + 1];
	if (pItem->m_usStockCount > 10)
		return -1;
	for(int i=0; i<pItem->m_usStockCount; i++)
	{
		memset(acSymbol,0,sizeof(acSymbol));
		pHead->GetStr(pItem->m_pStocks + c_iPassLen,STOCK_CODE_LEN,acSymbol,STOCK_CODE_LEN);
		if (strlen(acSymbol) < 6)
			return -1;

		if (pHead->GetEncoding() == CommxHead::ANSI)
		{
			unDateA = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN)));//分时前一个交易日
			unDateB = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN + sizeof(unsigned int))));//分时当前交易日
		}
		else
		if (pHead->GetEncoding() == CommxHead::UTF16LE)
		{
			unDateA = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN * 2)));//分时前一个交易日
			unDateB = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN * 2 + sizeof(unsigned int))));//分时当前交易日
		}
		else
		if (pHead->GetEncoding() == CommxHead::UTF8)
		{
			unDateA = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN * 3)));//分时前一个交易日
			unDateB = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN * 3 + sizeof(unsigned int))));//分时当前交易日
		}

		pHead->PutStr(acSymbol,pBuf + iRetLen,STOCK_CODE_LEN);

		if (pHead->GetEncoding() == CommxHead::ANSI)
			iRetLen += STOCK_CODE_LEN;
		else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
			iRetLen += STOCK_CODE_LEN * 2;
		else
			if (pHead->GetEncoding() == CommxHead::UTF8)
			iRetLen += STOCK_CODE_LEN * 3;


		if (unDateA == 0)//如果是0则取前一个交易日
		{
			unDateA = GetPreDate();
		}
		*(unsigned int *)(pBuf + iRetLen) = pHead->GetUI(unDateA);
		iRetLen += sizeof(unsigned int);
		iRet = GetRealMinData(pBuf + iRetLen,acSymbol,unDateA);
		if (iRet > 0)
		{
			iRetLen += iRet;
		}

		*(unsigned int *)(pBuf + iRetLen) = pHead->GetUI(unDateB);
		iRetLen += sizeof(unsigned int);
		if (unDateB > 0)
		{
			iRet = GetRealMinData(pBuf + iRetLen,acSymbol,unDateB);
			if (iRet > 0)
			{
				iRetLen += iRet;
			}
		}
		else
		{
			//获取m_pIdx
			MinBlock * pMinBlock;
			int nCount = 0;
			int n=0;
			//CQuoteData QuoteData(this);
			if (g_pDataFarm->m_hashIdx.cast(acSymbol,*this,true) > 0)
			{
				//if (QuoteData.m_Idx.idxQuote >= 0)
				if (true)
				{
					pMinBlock = g_pDataFarm->GetMinK(m_Idx.idxMinK);
					*((unsigned short *)(pBuf + iRetLen)) = pHead->GetUS(m_Idx.cntMinK);//记录个数
					iRetLen += sizeof(unsigned short);
					while(nCount < m_Idx.cntMinK)
					{
						*((unsigned int *)(pBuf + iRetLen)) = pHead->GetUI(pMinBlock->unit[n].Time);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int))) = pHead->GetUI(pMinBlock->unit[n].OpenPrice);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 2)) = pHead->GetUI(pMinBlock->unit[n].MaxPrice);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 3)) = pHead->GetUI(pMinBlock->unit[n].MinPrice);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 4)) = pHead->GetUI(pMinBlock->unit[n].NewPrice);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 5)) = pHead->GetUI(pMinBlock->unit[n].Volume);
						*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 6)) = pHead->GetUI(pMinBlock->unit[n].AvgPrice);
						iRetLen += sizeof(unsigned int) * 7;
						n++;
						nCount++;
						if (n == MINK_PERBLOCK)//当前BLOCK已经读完
						{
							pMinBlock = g_pDataFarm->GetMinK(pMinBlock->next);
							n = 0;
						}
					}
				}
				else
				{
					*((unsigned short *)(pBuf + iRetLen)) = 0;//记录个数
					iRetLen += sizeof(unsigned short);
				}
			}
			else
			{
				*((unsigned short *)(pBuf + iRetLen)) = 0;//记录个数
				iRetLen += sizeof(unsigned short);
			}
		}
		if (pHead->GetEncoding() == CommxHead::ANSI)
			c_iPassLen += STOCK_CODE_LEN + sizeof(unsigned int) * 2;
		else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
			c_iPassLen += STOCK_CODE_LEN * 2 + sizeof(unsigned int) * 2;
		else
			if (pHead->GetEncoding() == CommxHead::UTF8)
			c_iPassLen += STOCK_CODE_LEN * 3 + sizeof(unsigned int) * 2;
	}
	c_iPassLen += sizeof(unsigned short) * 2;
	return iRetLen + sizeof(unsigned short) * 2;
}

int CRealData::MakeHISKDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen, unsigned short c_usType)
{
	int iRetLen = 0;
	char * pBuf;
	unsigned short usRealCount = 0;
	unsigned int unDateBegin;
	unsigned int unDateEnd;
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	PREQ_ITEM pItem=(PREQ_ITEM)c_pReqItem;
	pBuf = c_pDataBuf;
	*((unsigned short *)pBuf) = pHead->GetUS(pItem->m_unType);
	*((unsigned short *)(pBuf + sizeof(unsigned short))) = pHead->GetUS(pItem->m_usStockCount);
	pBuf += sizeof(unsigned short) * 2;
	
	char acSymbol[STOCK_CODE_LEN + 1];
	unsigned short usWeight = 0;//还权标志
	if (g_cfg.m_bDebug)
		REPORT("QS",T("STOCKCOUNT=%d[%d]\n",pItem->m_usStockCount,pHead->GetUS(pItem->m_usStockCount)),RPT_INFO);
	if (pItem->m_usStockCount > 10)
		return -1;
	for(int i=0; i<pItem->m_usStockCount; i++)
	{
		memset(acSymbol,0,sizeof(acSymbol));
		pHead->GetStr(pItem->m_pStocks + c_iPassLen,STOCK_CODE_LEN,acSymbol,STOCK_CODE_LEN);
		if (strlen(acSymbol) < 6)
			return -1;

		//REPORT("QS",T("STOCKS=[%s]\n",acSymbol),RPT_INFO);	
		//
		pHead->PutStr(acSymbol,pBuf + iRetLen,STOCK_CODE_LEN);
		if (pHead->GetEncoding() == CommxHead::ANSI)
			iRetLen += STOCK_CODE_LEN;
		else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
			iRetLen += STOCK_CODE_LEN * 2;
		else
			if (pHead->GetEncoding() == CommxHead::UTF8)
			iRetLen += STOCK_CODE_LEN * 3;

		if (pHead->GetEncoding() == CommxHead::ANSI)
		{
			unDateBegin = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks+ c_iPassLen + STOCK_CODE_LEN)));
			unDateEnd = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN + sizeof(unsigned int))));
			//读取还权标志
			usWeight = pHead->GetUS(*((unsigned short *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN * 1 + 2 * sizeof(unsigned int))));
		}
		else
		if (pHead->GetEncoding() == CommxHead::UTF16LE)
		{
			unDateBegin = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks +c_iPassLen+ STOCK_CODE_LEN * 2)));
			unDateEnd = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks +c_iPassLen+ STOCK_CODE_LEN * 2 + sizeof(unsigned int))));
			//读取还权标志
			usWeight = pHead->GetUS(*((unsigned short *)(pItem->m_pStocks +c_iPassLen+ STOCK_CODE_LEN * 2 + 2 * sizeof(unsigned int))));
		}
		else
		if (pHead->GetEncoding() == CommxHead::UTF8)
		{
			unDateBegin = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks +c_iPassLen+ STOCK_CODE_LEN * 3)));
			unDateEnd = pHead->GetUI(*((unsigned int *)(pItem->m_pStocks +c_iPassLen+ STOCK_CODE_LEN * 3 + sizeof(unsigned int))));
			//读取还权标志
			usWeight = pHead->GetUS(*((unsigned short *)(pItem->m_pStocks + c_iPassLen + STOCK_CODE_LEN * 3 + 2 * sizeof(unsigned int))));
		}
		//设置还权标志
		
		*(unsigned short *)(pBuf + iRetLen) = pHead->GetUS(usWeight);
		if (g_cfg.m_bDebug)
		REPORT("QS",T("还权标志=[%d][%d]\n",usWeight,*(unsigned short *)(pBuf + iRetLen)),RPT_INFO);	
		iRetLen += sizeof(unsigned short);
		unsigned int nCount = 0;
		char szFile[MAX_PATH];
		char szHisFile[MAX_PATH];
/*		char   buffer[MAX_PATH];   
		GetCurrentDirectory   (   MAX_PATH,   buffer   );
*/
		char acDayKFile[MAX_PATH];
		if (usWeight == 0)
		{
			sprintf(acDayKFile, "%s\\current\\%s\\%s", g_cfg.m_strHistoryPath.c_str(),HISK_DAY, acSymbol);
			if(c_usType == HISKDAYDATA)
			{
				sprintf(szFile, "%s\\current\\%s\\%s", g_cfg.m_strHistoryPath.c_str(),HISK_DAY, acSymbol);
				sprintf(szHisFile, "%s\\history\\%s\\%s", g_cfg.m_strHistoryPath.c_str(),HISK_DAY, acSymbol);
			}
			else
			if(c_usType == HISKWEEKDATA)
			{
				sprintf(szFile, "%s\\current\\%s\\%s",g_cfg.m_strHistoryPath.c_str(), HISK_WEK, acSymbol);
				sprintf(szHisFile, "%s\\history\\%s\\%s", g_cfg.m_strHistoryPath.c_str(),HISK_WEK, acSymbol);
			}
			else
			if (c_usType == HISKMONTHDATA)
			{
				sprintf(szFile, "%s\\current\\%s\\%s",g_cfg.m_strHistoryPath.c_str(), HISK_MON, acSymbol);
				sprintf(szHisFile, "%s\\history\\%s\\%s", g_cfg.m_strHistoryPath.c_str(),HISK_MON, acSymbol);
			}
		}
		else
		{
			sprintf(acDayKFile, "%s\\current\\%s\\%s", g_cfg.m_strHistoryWeightPath.c_str(),HISK_DAY, acSymbol);
			if(c_usType == HISKDAYDATA)
			{
				sprintf(szFile, "%s\\current\\%s\\%s", g_cfg.m_strHistoryWeightPath.c_str(),HISK_DAY, acSymbol);
				sprintf(szHisFile, "%s\\history\\%s\\%s", g_cfg.m_strHistoryWeightPath.c_str(),HISK_DAY, acSymbol);
			}
			else
			if(c_usType == HISKWEEKDATA)
			{
				sprintf(szFile, "%s\\current\\%s\\%s",g_cfg.m_strHistoryWeightPath.c_str(), HISK_WEK, acSymbol);
				sprintf(szHisFile, "%s\\history\\%s\\%s", g_cfg.m_strHistoryWeightPath.c_str(),HISK_WEK, acSymbol);
			}
			else
			if (c_usType == HISKMONTHDATA)
			{
				sprintf(szFile, "%s\\current\\%s\\%s",g_cfg.m_strHistoryWeightPath.c_str(), HISK_MON, acSymbol);
				sprintf(szHisFile, "%s\\history\\%s\\%s", g_cfg.m_strHistoryWeightPath.c_str(),HISK_MON, acSymbol);
			}
		}
		/*获取日K线的最后日期*/
		char * pToFind;
		pToFind = strstr(acDayKFile,".SH");
		if (pToFind)
			strcpy(pToFind,".shidx");
		else
		{
			pToFind = strstr(acDayKFile,".SZ");
			if (pToFind)
				strcpy(pToFind,".szidx");
			else
			{
				pToFind = strstr(acDayKFile,".HK");
				if (pToFind)
					strcpy(pToFind,".hkidx");
			}
		}

		int nDayKLastDate = 0;
		FILE * fK;
		fK = fopen(acDayKFile,"rb");
		if (fK)
		{
			fseek(fK,0,SEEK_END);
			int nFileLen = ftell(fK);
			if (nFileLen > sizeof(KLINE))
			{
				fseek(fK,nFileLen-sizeof(KLINE),SEEK_SET);
				KLINE kLine;
				if (fread(&kLine,1,sizeof(KLINE),fK))
				{
					nDayKLastDate = kLine.day;
				}
			}
			fclose(fK);
		}

		/*结束*/
		if (g_cfg.m_bDebug)
			REPORT("QS",T("FILE=[%s],日线最后日期是[%d]\n",szFile,nDayKLastDate),RPT_INFO);
		SDataItem DataItem;
		//g_DataPool.m_rwlock.WaitWrite();
		//转换文件名称SH->shidx, SZ->szidx, HK->hkidx
		char * pFind;
		pFind = strstr(szFile,".SH");
		if (pFind)
			strcpy(pFind,".shidx");
		else
		{
			pFind = strstr(szFile,".SZ");
			if (pFind)
				strcpy(pFind,".szidx");
			else
			{
				pFind = strstr(szFile,".HK");
				if (pFind)
					strcpy(pFind,".hkidx");
			}
		}
		pFind = strstr(szHisFile,".SH");
		if (pFind)
			strcpy(pFind,".shidx");
		else
		{
			pFind = strstr(szHisFile,".SZ");
			if (pFind)
				strcpy(pFind,".szidx");
			else
			{
				pFind = strstr(szHisFile,".HK");
				if (pFind)
					strcpy(pFind,".hkidx");
			}
		}

		//pDataItem = g_DataPool.ReadData(szFile);
		
		//if (!pDataItem)
		if (true)
		{
			if (g_cfg.m_bDebug)
			REPORT("QS",T("FILE=[%s]\n",szFile),RPT_INFO);	
			FILE *fp = fopen(szFile, "rb");			
			
			if (fp)	
			{
				if (g_cfg.m_bDebug)
				REPORT("QS",T("OPEN OK FILE=[%s]\n",szFile),RPT_INFO);	
				fseek(fp, 0, SEEK_END);
				nCount = ftell(fp) / sizeof(KLINE);
				fseek(fp, 0, SEEK_SET);
				DataItem.m_nBufLen = (nCount + 2) * sizeof(KLINE);//留两个空白
				DataItem.m_nDataLen = nCount * sizeof(KLINE);//真实的
				DataItem.m_pBuf = (char *)malloc(DataItem.m_nBufLen);
				memset(DataItem.m_pBuf,0,DataItem.m_nBufLen);
				//fread(pBuf + iRetLen,sizeof(KLINE),nCount,fp);
				fread(DataItem.m_pBuf,sizeof(KLINE),nCount,fp);
				fclose(fp);
				//pDataItem = g_DataPool.PutData(szFile,pBuf + iRetLen,nCount * sizeof(KLINE),nCount * sizeof(KLINE));
			}
			else
			{
				DataItem.m_nBufLen = 2 * sizeof(KLINE);//留两个空白
				DataItem.m_nDataLen = 0;//真实的
				DataItem.m_pBuf = (char *)malloc(DataItem.m_nBufLen);
				memset(DataItem.m_pBuf,0,DataItem.m_nBufLen);
			}
		}

		//检查是否需要读入history目录下的数据
		bool bNeed = false;
		if (DataItem.m_nDataLen == 0)
			bNeed = true;
		else
		if (unDateBegin > 0)
		{
			KLINE * pFirstLine;
			pFirstLine = (KLINE *)(DataItem.m_pBuf);
			if (pFirstLine->day > unDateBegin)
				bNeed = true;
		}
		if (bNeed)
		{//读入历史数据
			FILE *fp = fopen(szHisFile, "rb");
			char *pNew;
			if (fp)	
			{
				if (g_cfg.m_bDebug)
				REPORT("QS",T("OPEN OK HIS FILE=[%s]\n",szHisFile),RPT_INFO);	
				fseek(fp, 0, SEEK_END);
				nCount = ftell(fp) / sizeof(KLINE);
				fseek(fp, 0, SEEK_SET);
				pNew = (char *)malloc(nCount * sizeof(KLINE) + DataItem.m_nBufLen);
				memset(pNew,0,nCount * sizeof(KLINE));
				fread(pNew,sizeof(KLINE),nCount,fp);
				fclose(fp);
				memcpy(pNew + sizeof(KLINE) * nCount,DataItem.m_pBuf,DataItem.m_nDataLen);
				DataItem.m_nDataLen += nCount * sizeof(KLINE);
				DataItem.m_nBufLen += nCount * sizeof(KLINE);
				free(DataItem.m_pBuf);
				DataItem.m_pBuf = pNew;
			}
		}
		MarketStatus * pMarketStatus;
		char * pMarket;
		pMarket = strstr(acSymbol,".");
		if (pMarket)//对日线、周线、月线补上当日的行情
		{
			pMarketStatus = g_pDataFarm->GetMarketStatus(pMarket+1);//找到行情
			if (pMarketStatus)//找到
			{
				if (g_cfg.m_bDebug)
					REPORT("QS",T("开盘时间=%d 收盘时间=%d\n",pMarketStatus->dateOpen,pMarketStatus->dateClose),RPT_DEBUG);
				//if (usWeight == 0 && pMarketStatus->dateOpen > 0 && pMarketStatus->dateClose == 0)//已经开盘，还没有收盘，还权K线不做当日处理
				if (pMarketStatus->dateOpen > 0 && (pMarketStatus->dateClose == 0 ||pMarketStatus->dateClose<pMarketStatus->dateOpen ||  nDayKLastDate!=pMarketStatus->dateClose))//已经开盘，还没有收盘，还权K线不做当日处理
				{
					KLINE * pLastKLine;
					unsigned int unKDate;
					int nOP = 0;
					if (DataItem.m_nDataLen < sizeof(KLINE))
					{
						nOP = 1;//增加
						unKDate = GetKDate(0,pMarketStatus->dateOpen,c_usType);//计算KLine中对应周期的日、周、月
					}
					else
					{
						pLastKLine = (KLINE *)(DataItem.m_pBuf + DataItem.m_nDataLen - sizeof(KLINE));
						
						unKDate = GetKDate(pLastKLine->day,pMarketStatus->dateOpen,c_usType);//计算KLine中对应周期的日、周、月
						if (unKDate != pLastKLine->day)
							nOP = 1;//增加
						else
							nOP = 2;//修改，只有周线和月线才会用的修改
					}
					Quote * pQuote;
					if (g_pDataFarm->m_hashIdx.cast(acSymbol,*this,true) > 0)
					{
						pQuote = g_pDataFarm->GetQuote(this->m_Idx.idxQuote);
						//REPORT("QS",T("[%d][%s]开盘价格=%d \n",nOP,acSymbol,pQuote->jrkp),RPT_DEBUG);
						if (pQuote && (pQuote->jrkp > 0))
						{						
							if (nOP == 1)//增加
							{
								DataItem.m_nDataLen += sizeof(KLINE);
								pLastKLine = (KLINE *)(DataItem.m_pBuf + DataItem.m_nDataLen - sizeof(KLINE));
								pLastKLine->day = unKDate;
								pLastKLine->amount = pQuote->cjje;
								pLastKLine->close = pQuote->zjjg;
								pLastKLine->high = pQuote->zgjg;
								pLastKLine->low = pQuote->zdjg;
								pLastKLine->open = pQuote->jrkp;
								pLastKLine->volume = pQuote->cjsl;
							}
							if (nOP == 2 && (c_usType == HISKWEEKDATA || c_usType == HISKMONTHDATA))//周线，月线修改
							{
								pLastKLine = (KLINE *)(DataItem.m_pBuf + DataItem.m_nDataLen - sizeof(KLINE));
								pLastKLine->amount += pQuote->cjje;
								pLastKLine->close = pQuote->zjjg;
								if (pLastKLine->high < pQuote->zgjg)
									pLastKLine->high = pQuote->zgjg;
								if (pLastKLine->low > pQuote->zdjg)
									pLastKLine->low = pQuote->zdjg;
								pLastKLine->volume += pQuote->cjsl;
							}
						}
					}
				}
			}
		}
		if (DataItem.m_nDataLen >= sizeof(KLINE))	
		{
			KLINE * pK;
			int nBegin,nEnd;
			nCount = DataItem.m_nDataLen / sizeof(KLINE);//合计元素
			if (unDateBegin > 0)
			{
				for(nBegin=0; (unsigned int)nBegin<nCount; nBegin++)
				{
					pK = (KLINE *)(DataItem.m_pBuf + nBegin * sizeof(KLINE));
					if (pK->day >= unDateBegin)//指定开始日期
					{
						break;//找到了，否则定位nBegin=nCount
					}
				}
			}
			else
			{
				nBegin = 0;
			}
			
			if (unDateEnd > 0)
			{
				for(nEnd=nBegin; (unsigned int)nEnd<nCount; nEnd++)
				{
					pK = (KLINE *)(DataItem.m_pBuf + nEnd * sizeof(KLINE));
					if (pK->day >= unDateEnd)
					{
						break;
					}
				}
			}
			else
			{
				nEnd = nCount - 1;
			}
			if ((unsigned int)nBegin >= nCount)
				nBegin = nCount - 1;
			if ((unsigned int)nEnd >= nCount)
				nEnd = nCount - 1;
			if (nCount > 0)
				nCount = nEnd - nBegin + 1;
			*((unsigned int *)(pBuf + iRetLen)) = pHead->GetUI(nCount);
			iRetLen += sizeof(unsigned int);
			memcpy(pBuf + iRetLen,DataItem.m_pBuf + nBegin * sizeof(KLINE),nCount * sizeof(KLINE));//CHANGE
			DataItem.m_nBufLen = 0;
			DataItem.m_nDataLen = 0;
			free(DataItem.m_pBuf);
			DataItem.m_pBuf = NULL;
			for(int i=0; (unsigned int)i<nCount; i++)//CHANGE
			{
				KLINE *p;
				p = (KLINE *)(pBuf + iRetLen + i * sizeof(KLINE));
				p->day = pHead->GetUI(p->day);
				p->open = pHead->GetUI(p->open);
				p->high = pHead->GetUI(p->high);
				p->low = pHead->GetUI(p->low);
				p->close = pHead->GetUI(p->close);
				p->volume = pHead->GetUI(p->volume);
				p->amount = pHead->GetUI(p->amount);
			}
			iRetLen += nCount * sizeof(KLINE);			
		}
		else
		{
			*((unsigned int *)(pBuf + iRetLen)) = 0;
			iRetLen += sizeof(unsigned int);
		}
		//g_DataPool.m_rwlock.EndWrite();

		if (pHead->GetEncoding() == CommxHead::ANSI)
		c_iPassLen += STOCK_CODE_LEN + sizeof(unsigned int) * 2 + sizeof(unsigned short);
		else
			if (pHead->GetEncoding() == CommxHead::UTF16LE)
		c_iPassLen += STOCK_CODE_LEN * 2 + sizeof(unsigned int) * 2 + sizeof(unsigned short);
		else
			if (pHead->GetEncoding() == CommxHead::UTF8)
		c_iPassLen += STOCK_CODE_LEN * 3+ sizeof(unsigned int) * 2 + sizeof(unsigned short);

	}

	c_iPassLen += sizeof(unsigned short) * 2;
	return iRetLen + sizeof(unsigned short) * 2;
}

int CRealData::MakeLINETESTREQ(char * c_pDataBuf)
{
	PPACKETHEAD pHead;
	pHead = (PPACKETHEAD)c_pDataBuf;
	pHead->m_commxHead.Prop = pHead->m_commxHead.MakeProp(0x03,CommxHead::ANSI,CommxHead::HOSTORDER);
	pHead->m_commxHead.SerialNo = 1;
	pHead->m_usFuncNo = LINETESTREQ;
	pHead->m_usReqCount = 1;

	*((unsigned short *)(c_pDataBuf + sizeof(PACKETHEAD))) = LINETESTREQ;
	*((unsigned short *)(c_pDataBuf + sizeof(PACKETHEAD) + sizeof(unsigned short))) = 0;
	pHead->m_commxHead.SetLength(sizeof(unsigned short) * 4);

	return sizeof(PACKETHEAD) + sizeof(unsigned short) * 2;
}

int CRealData::MakeWrongReturn(char * c_pReq,char * c_pErrMsg, int c_nLen)
{
	PPACKETHEAD pHead;
	memcpy(m_pRepBuf,c_pReq,sizeof(CommxHead));//copy commxhead
	pHead = (PPACKETHEAD)this->m_pRepBuf;
	pHead->m_usFuncNo = pHead->m_commxHead.GetUS(0xFFFF);
	int nLen;
	memset(m_pRepBuf + sizeof(CommxHead) + sizeof(unsigned short),0,this->m_unRepBufLen - sizeof(CommxHead) - sizeof(unsigned short));
	nLen = pHead->m_commxHead.PutStr(c_pErrMsg,m_pRepBuf + sizeof(CommxHead) + sizeof(unsigned short),c_nLen);
	pHead->m_commxHead.SetLength(nLen + sizeof(unsigned short));
	this->m_unRepLen = nLen + sizeof(unsigned short) + sizeof(CommxHead);
	return m_unRepLen;
}
//取上一交易日的分时走势
int CRealData::GetRealMinData(char * c_pBuf,char * c_pSymbol,unsigned int c_unDate)
{
	char * pBuf = c_pBuf;
	int iRetLen = 0;
	*((unsigned short *)(pBuf)) = 0;
	iRetLen += sizeof(unsigned short);
	CommxHead * pHead = (CommxHead *)m_pRepBuf;
	HANDLE hFile;
	HANDLE hMappingFile;
	char * lpData;
	IndexHead indexHead;				//文件头结构
	int nSize;
	char acFile[MAX_PATH];
	sprintf(acFile,"%s\\%d\\index.dat",g_cfg.m_strDetailPath.c_str(),c_unDate);
	//REPORT("QS",T("GetRealMinData file=[%s]\n",acFile),RPT_INFO);
	USES_CONVERSION;
	hFile = CreateFile(A2W(acFile), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return iRetLen;
	}
	nSize = GetFileSize(hFile,0);
	hMappingFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, nSize, NULL);
	if (!hMappingFile)
	{
		CloseHandle(hFile);
		return iRetLen;
	}

	lpData = (char*)MapViewOfFile(hMappingFile, FILE_MAP_READ, 0, 0, 0);
	memcpy(&indexHead, lpData, sizeof(IndexHead));
	typedef CHashTable<std::string, RINDEX, LockProcess, HashAllocateMMapFile<std::string, RINDEX> > HASHIDX;

	HASHIDX hashIdx;					//品种索引哈希表
	hashIdx.malloc(indexHead.nBucketCount, indexHead.nSymbolCount);
	hashIdx.mapmem(lpData + indexHead.nIndexOffset, NULL, 0);
	
	CMMapFile<MinBlock> tbMinK;		//保存分钟K线
	sprintf(acFile,"%s\\%d\\mink.dat",g_cfg.m_strDetailPath.c_str(),c_unDate);
	//REPORT("QS",T("GetRealMinData file=[%s][%s]\n",acFile,c_pSymbol),RPT_INFO);
	tbMinK.mapfile(acFile, indexHead.nMinkCount);
	
	//获取m_pIdx
	MinBlock * pMinBlock;
	int nCount = 0;
	int n=0;
	if (hashIdx.cast(c_pSymbol,*this,true) > 0)
	{
		//REPORT("QS",T("hashIdx.cast success %d %d\n",m_Idx.idxQuote,m_Idx.cntMinK),RPT_INFO);
		if (m_Idx.idxQuote >= 0)
		{
			pMinBlock = tbMinK[m_Idx.idxMinK];
			*((unsigned short *)(pBuf)) = (unsigned short)pHead->GetUS(m_Idx.cntMinK);//记录个数
			while(nCount < m_Idx.cntMinK)
			{
				*((unsigned int *)(pBuf + iRetLen)) = pHead->GetUI(pMinBlock->unit[n].Time);
				*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int))) = pHead->GetUI(pMinBlock->unit[n].OpenPrice);
				*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 2)) = pHead->GetUI(pMinBlock->unit[n].MaxPrice);
				*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 3)) = pHead->GetUI(pMinBlock->unit[n].MinPrice);
				*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 4)) = pHead->GetUI(pMinBlock->unit[n].NewPrice);
				*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 5)) = pHead->GetUI(pMinBlock->unit[n].Volume);
				*((unsigned int *)(pBuf + iRetLen + sizeof(unsigned int) * 6)) = pHead->GetUI(pMinBlock->unit[n].AvgPrice);
				iRetLen += sizeof(unsigned int) * 7;
				n++;
				nCount++;
				if (n == MINK_PERBLOCK)//当前BLOCK已经读完
				{
					pMinBlock = tbMinK[pMinBlock->next];
					n = 0;
				}
			}
		}
	}
	UnmapViewOfFile(lpData);
	CloseHandle(hMappingFile);
	CloseHandle(hFile);

	return iRetLen;
}
//获取前一工作日
unsigned int CRealData::GetPreDate()
{
	time_t now;
	time(&now);
	if (g_cfg.m_bDebug)
	REPORT("QS",T("GetPreDate Now=[%d]\n",now),RPT_INFO);
	DWORD dwSeconds=3600 * 24;
	
	struct tm *lt = localtime(&now);
	
	//SYSTEMTIME   dt;   
	//::GetSystemTime(&dt);   
	//::GetLocalTime(&dt);

	unsigned int unToday;
	char acFile[MAX_PATH];
	int n = 1;
	for(n = 1; n < 30; n++)
	{
		now -= dwSeconds;
		//REPORT("QS",T("GetPreDate Now=[%d]\n",now),RPT_INFO);
		lt = localtime(&now);
		unToday = (lt->tm_year + 1900) * 10000 + (lt->tm_mon + 1) * 100 + lt->tm_mday;
		sprintf(acFile,"%s\\%d\\index.dat",g_cfg.m_strDetailPath.c_str(),unToday);
		//REPORT("QS",T("GetPreDate[%d]\n",unToday),RPT_INFO);
		if (_access(acFile,0) != -1)
			return unToday;
	}
	return 0;
}
int CQuoteData::OnElement(std::string  * c_pKey,RINDEX * c_pValue)
	{
		RINDEX * pIdx;
		Quote * pQuote;
		char acSpell[5];
		int nCount = 0;
		CommxHead * pHead = (CommxHead *)m_pRealData->m_pRepBuf;
		pIdx = c_pValue;
		if (pIdx)
		{
			pQuote = g_pDataFarm->GetQuote(pIdx->idxQuote);
			if (pQuote)
			{
				if (m_nType == 0)
				{
					pHead->PutStr(pQuote->szStockCode,m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,STOCK_CODE_LEN);
					//strcpy(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,pQuote->szStockCode);//CHANGECODE
					if (pHead->GetEncoding() == CommxHead::ANSI)
						m_pRealData->m_unRepLen += STOCK_CODE_LEN;
					else
					if (pHead->GetEncoding() == CommxHead::UTF16LE)
						m_pRealData->m_unRepLen += STOCK_CODE_LEN * 2;
					else
					if (pHead->GetEncoding() == CommxHead::UTF8)
						m_pRealData->m_unRepLen += STOCK_CODE_LEN * 3;

					pHead->PutStr(pQuote->szStockName,m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,STOCK_NAME_LEN);
					//strcpy(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,pQuote->szStockName);
					if (pHead->GetEncoding() == CommxHead::ANSI)
						m_pRealData->m_unRepLen += STOCK_NAME_LEN;
					else
					if (pHead->GetEncoding() == CommxHead::UTF16LE)
						m_pRealData->m_unRepLen += STOCK_NAME_LEN * 2;
					else
					if (pHead->GetEncoding() == CommxHead::UTF8)
						m_pRealData->m_unRepLen += STOCK_NAME_LEN * 3;
					memset(acSpell,0,5);
					//g_pSpell->WordToSpell(pQuote->szStockName,acSpell,strlen(pQuote->szStockName));
					std::string py;
					py = g_pDataFarm->GetHzpy(pQuote->szStockName);
					if (strstr(pQuote->szStockName,"银行"))
					{
						int nPos = py.find("YX",2);
						py.replace(nPos,2,"YH");
					}
					strncpy(acSpell,py.c_str(),4);
					pHead->PutStr(acSpell,m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,4);
					//strcpy(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,acSpell);
					if (pHead->GetEncoding() == CommxHead::ANSI)
						m_pRealData->m_unRepLen += 4;
					else
					if (pHead->GetEncoding() == CommxHead::UTF16LE)
						m_pRealData->m_unRepLen += 4 * 2;
					else
					if (pHead->GetEncoding() == CommxHead::UTF8)
						m_pRealData->m_unRepLen += 4 * 3;
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen)) = pHead->GetUI(pQuote->zrsp);
					m_pRealData->m_unRepLen += sizeof(unsigned int);
				}
				else
				if (m_nType == 1)
				{
					//strcpy(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,pQuote->szStockCode);//CHANGECODE
					pHead->PutStr(pQuote->szStockCode,m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,STOCK_CODE_LEN);
					if (pHead->GetEncoding() == CommxHead::ANSI)
						m_pRealData->m_unRepLen += STOCK_CODE_LEN;
					else
					if (pHead->GetEncoding() == CommxHead::UTF8)
						m_pRealData->m_unRepLen += STOCK_CODE_LEN * 2;
					else
					if (pHead->GetEncoding() == CommxHead::UTF16LE)
						m_pRealData->m_unRepLen += STOCK_CODE_LEN * 3;

					pHead->PutStr(pQuote->szStockName,m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen,STOCK_NAME_LEN);
					if (pHead->GetEncoding() == CommxHead::ANSI)
						m_pRealData->m_unRepLen += STOCK_NAME_LEN;
					else
					if (pHead->GetEncoding() == CommxHead::UTF8)
						m_pRealData->m_unRepLen += STOCK_NAME_LEN * 2;
					else
					if (pHead->GetEncoding() == CommxHead::UTF16LE)
						m_pRealData->m_unRepLen += STOCK_NAME_LEN * 3;

					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen)) = pHead->GetUI(pQuote->zrsp);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int))) = pHead->GetUI(pQuote->jrkp);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 2)) = pHead->GetUI(pQuote->ztjg);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 3)) = pHead->GetUI(pQuote->dtjg);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 4)) = pHead->GetUI(pQuote->syl1);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 5)) = pHead->GetUI(pQuote->syl2);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 6)) = pHead->GetUI(pQuote->zgjg);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 7)) = pHead->GetUI(pQuote->zdjg);
					if (pQuote->zjjg > 0)
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 8)) = pHead->GetUI(pQuote->zjjg);
					else
						if (pQuote->jrkp)
							*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 8)) = pHead->GetUI(pQuote->jrkp);
						else
							*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 8)) = pHead->GetUI(pQuote->zrsp);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 9)) = pHead->GetUI(pQuote->zjcj);
					unsigned int je=0,sl=0;
					Quote *pQuote000011,*pQuote399106,*pQuote399305;
					je = pQuote->cjje;
					sl = pQuote->cjsl;

					if (strcmp(pQuote->szStockCode,"000001.SH") == 0
						||strcmp(pQuote->szStockCode,"399001.SZ") == 0
						||strcmp(pQuote->szStockCode,"399002.SZ") == 0
						||strcmp(pQuote->szStockCode,"399003.SZ") == 0
						||strcmp(pQuote->szStockCode,"399005.SZ") == 0)
					{
						pQuote000011 = GetSymbolQuote("000011.SH");
						pQuote399106 = GetSymbolQuote("399106.SZ");
						pQuote399305 = GetSymbolQuote("399305.SZ");

						if (strcmp(pQuote->szStockCode,"000001.SH") == 0)
						{
							if (pQuote000011)
							{
								je += pQuote000011->cjje;
								sl += pQuote000011->cjsl;
							}
						}
						else
						if (strcmp(pQuote->szStockCode,"399001.SZ") == 0)
						{
							if (pQuote399305 && pQuote399106)
							{
								je = pQuote399106->cjje + pQuote399305->cjje;
								je = pQuote399106->cjsl + pQuote399305->cjsl;
							}
						}
						else
						if (strcmp(pQuote->szStockCode,"399002.SZ") == 0 || strcmp(pQuote->szStockCode,"399003.SZ") == 0 || strcmp(pQuote->szStockCode,"399005.SZ") == 0)
						{
							if (pQuote399305)
							{
								//je += pQuote399305->cjje;
								//je += pQuote399305->cjsl;
							}
						}
					}
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 10)) = pHead->GetUI(sl);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 11)) = pHead->GetUI(je);

					//*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 10)) = pHead->GetUI(pQuote->cjsl);
					//*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 11)) = pHead->GetUI(pQuote->cjje);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 12)) = pHead->GetUI(pQuote->cjbs);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 13)) = pHead->GetUI(pQuote->BP1);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 14)) = pHead->GetUI(pQuote->BM1);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 15)) = pHead->GetUI(pQuote->BP2);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 16)) = pHead->GetUI(pQuote->BM2);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 17)) = pHead->GetUI(pQuote->BP3);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 18)) = pHead->GetUI(pQuote->BM3);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 19)) = pHead->GetUI(pQuote->BP4);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 20)) = pHead->GetUI(pQuote->BM4);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 21)) = pHead->GetUI(pQuote->BP5);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 22)) = pHead->GetUI(pQuote->BM5);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 23)) = pHead->GetUI(pQuote->SP1);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 24)) = pHead->GetUI(pQuote->SM1);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 25)) = pHead->GetUI(pQuote->SP2);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 26)) = pHead->GetUI(pQuote->SM2);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 27)) = pHead->GetUI(pQuote->SP3);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 28)) = pHead->GetUI(pQuote->SM3);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 29)) = pHead->GetUI(pQuote->SP4);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 30)) = pHead->GetUI(pQuote->SM4);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 31)) = pHead->GetUI(pQuote->SP5);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 32)) = pHead->GetUI(pQuote->SM5);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 33)) = pHead->GetUI(pQuote->day5pjzs);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 34)) = pHead->GetUI(pQuote->pjjg);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 35)) = pHead->GetUI(pQuote->wb);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 36)) = pHead->GetUI(pQuote->lb);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 37)) = pHead->GetUI(pQuote->np);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 38)) = pHead->GetUI(pQuote->wp);
					if (pQuote->zjjg > 0)
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 39)) = pHead->GetUI(pQuote->zjjg - pQuote->zrsp);
					else
						if (pQuote->jrkp > 0)
						*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 39)) = pHead->GetUI(pQuote->jrkp - pQuote->zrsp);
						else
						*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 39)) = 0;
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 40)) = pHead->GetUI(pQuote->zdf);
					*((unsigned int *)(m_pRealData->m_pRepBuf + m_pRealData->m_unRepLen + sizeof(unsigned int) * 41)) = pHead->GetUI(pQuote->zf);
					m_pRealData->m_unRepLen += sizeof(unsigned int) * 41;
				}
				return 1;
			}
		}
		return 0;
	}

int CRealData::GetMinKData(char * c_pSymbol,char * c_pRetBuf,int c_nMaxLen)
{
	//获取m_pIdx
	MinBlock * pMinBlock;
	int nCount = 0;
	int n=0;
	int iRetLen;

	if (g_pDataFarm->m_hashIdx.cast(c_pSymbol,*this,true) > 0)
	{
		//if (QuoteData.m_Idx.idxQuote >= 0)
		if (true)
		{
			pMinBlock = g_pDataFarm->GetMinK(m_Idx.idxMinK);
			iRetLen = 0;
			while(nCount < m_Idx.cntMinK)
			{
				*((unsigned int *)(c_pRetBuf + iRetLen)) = pMinBlock->unit[n].Time;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int))) = pMinBlock->unit[n].OpenPrice;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int) * 2)) = pMinBlock->unit[n].MaxPrice;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int) * 3)) = pMinBlock->unit[n].MinPrice;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int) * 4)) = pMinBlock->unit[n].NewPrice;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int) * 5)) = pMinBlock->unit[n].Volume;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int) * 6)) = pMinBlock->unit[n].AvgPrice;
				iRetLen += sizeof(unsigned int) * 7;
				n++;
				nCount++;
				if (n == MINK_PERBLOCK)//当前BLOCK已经读完
				{
					pMinBlock = g_pDataFarm->GetMinK(pMinBlock->next);
					n = 0;
				}
			}
			return m_Idx.cntMinK;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}
int CRealData::GetTraceData(char * c_pSymbol,char * c_pRetBuf,int c_nMaxLen)
{
	//获取m_pIdx
	int nCount = 0;
	int n=0;
	int iRetLen;
	
	if (g_pDataFarm->m_hashIdx.cast(c_pSymbol,*this,true) > 0)
	{
		//if (QuoteData.m_Idx.idxQuote >= 0)
		if (true)
		{
			TickBlock * pTickBlock;
			pTickBlock = g_pDataFarm->GetTick(m_Idx.idxTick);
			iRetLen = 0;
			while(nCount < m_Idx.cntTick)
			{
				*((unsigned int *)(c_pRetBuf + iRetLen)) = pTickBlock->unit[n].Time;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int))) = pTickBlock->unit[n].Price;
				*((unsigned int *)(c_pRetBuf + iRetLen + sizeof(unsigned int) * 2)) = pTickBlock->unit[n].Volume;
				*((unsigned char *)(c_pRetBuf + iRetLen + sizeof(unsigned int) * 3)) = pTickBlock->unit[n].Way;
				iRetLen += sizeof(unsigned int) * 3 + sizeof(unsigned char);
				n++;
				nCount++;
				if (n == TICK_PERBLOCK)//当前BLOCK已经读完
				{
					pTickBlock = g_pDataFarm->GetTick(pTickBlock->next);
					n = 0;
				}
			}
			return m_Idx.cntTick;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

int CRealData::GetHisKData(char * c_pSymbol,unsigned int c_unBeginDate,unsigned int c_unEndDate, unsigned short c_usType,unsigned short c_usRight,char * c_pRetBuf,unsigned int c_unMaxLen)
{
	int iRetLen = 0;
	char * pBuf;
	unsigned int unDateBegin;
	unsigned int unDateEnd;
	pBuf = c_pRetBuf;

	unDateBegin = c_unBeginDate;
	unDateEnd = c_unEndDate;
	unsigned int nCount = 0;
	char szFile[MAX_PATH];
	if(c_usType == HISKDAYDATA)
		sprintf(szFile, "%s/%s", HISK_DAY, c_pSymbol);
	else
	if(c_usType == HISKWEEKDATA)
		sprintf(szFile, "%s/%s", HISK_WEK, c_pSymbol);
	else
	if (c_usType == HISKMONTHDATA)
		sprintf(szFile, "%s/%s", HISK_MON, c_pSymbol);

//	REPORT(LockSingle,MODULE_NAME,T("Read file %s\n",szFile),RPT_INFO);
	FILE *fp = fopen(szFile, "rb");
	if (fp)	
	{
		fseek(fp, 0, SEEK_END);
		nCount = ftell(fp) / sizeof(KLINE);
		//REPORT(LockSingle,MODULE_NAME,T("k_line count=%d\n",nCount),RPT_INFO);
		fseek(fp, 0, SEEK_SET);
		*((unsigned int *)(pBuf + iRetLen)) = nCount;
		iRetLen += sizeof(unsigned int);
		fread(pBuf + iRetLen,sizeof(KLINE),nCount,fp);
		fclose(fp);
		return nCount;
	}
	else
	{
		return 0;
	}
}

int CRealData::cast(std::string  * c_pKey, RINDEX * c_pValue,bool c_bFlag)
{
	//int iRetLen = 0;
	//REPORT("QS",T("cast[%s][%d][idxMink=%d][idxQuote=%d][idxTick=%d]\n",c_pKey->c_str(),c_pValue->cntMinK,c_pValue->idxMinK,c_pValue->idxQuote,c_pValue->idxTick),RPT_INFO);

	//MinBlock * pMinBlock;
	//int n=0;
	/*
	for(int k=0; k<8000; k++)
	{
	pMinBlock = g_pDataFarm->GetMinK(k);
	n = 0;
	REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d][%d][%d]\n",k,n,pMinBlock->next,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	n = 1;
	REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d][%d][%d]\n",k,n,pMinBlock->next,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	n = 2;
	REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d][%d][%d]\n",k,n,pMinBlock->next,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	n = 3;
	REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d][%d][%d]\n",k,n,pMinBlock->next,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	n = 4;
	REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d][%d][%d]\n",k,n,pMinBlock->next,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	n = 5;
	REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d][%d][%d]\n",k,n,pMinBlock->next,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	}*/
	//pMinBlock = g_pDataFarm->GetMinK(c_pValue->idxMinK);
	//REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d]\n",n,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	//n = 1;
	//REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d]\n",n,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);
	//n = 2;
	//REPORT("QS",T("[%d][%d][%d][%d][%d][%d][%d]\n",n,pMinBlock->unit[n].Time,pMinBlock->unit[n].OpenPrice,pMinBlock->unit[n].MaxPrice,pMinBlock->unit[n].MinPrice,pMinBlock->unit[n].NewPrice,pMinBlock->unit[n].Volume),RPT_INFO);

	//TickBlock * pTickBlock;
	//pTickBlock = g_pDataFarm->GetTick(c_pValue->idxTick);
	//REPORT("QS",T("<%d><%d><%d><%d>\n",c_pValue->idxQuote,c_pValue->idxTick,pTickBlock->unit[0].Time,pTickBlock->unit[0].Price),RPT_INFO);
	memcpy(&m_Idx,c_pValue,sizeof(RINDEX));
	return 0;
}

int CQuoteData::cast(std::string  * c_pKey, RINDEX * c_pValue,bool c_bFlag)
{
	memcpy(&m_Idx,c_pValue,sizeof(RINDEX));
	return 0;
}
unsigned int CRealData::GetKDate(unsigned int c_unKDate,unsigned int c_unDate,unsigned short c_usType)
{
	if (c_unKDate == 0)
		return c_unDate;
	if (c_usType == HISKDAYDATA)
		return c_unDate;
	if (c_usType == HISKWEEKDATA)
	{
		if (IsSameWeek(c_unKDate,c_unDate))
			return c_unKDate;
		else
			return c_unDate;
	}
	if (c_usType == HISKMONTHDATA)
	{
		if (IsSameMonth(c_unKDate,c_unDate))
			return c_unKDate;
		else
			return c_unDate;
	}
	return c_unKDate;
}

bool CRealData::IsSameWeek(unsigned int uiNow, unsigned int uiPrev)
{//比较今日和所给定日期是否在同一周内
	tm	stPrevTime,stNowTime;
	memset(&stPrevTime,0,sizeof(tm));
	stPrevTime.tm_year=uiPrev/10000-1900;
	stPrevTime.tm_mon=uiPrev%10000/100-1;
	stPrevTime.tm_mday=uiPrev%100;
	time_t stPrev=mktime(&stPrevTime);

	memset(&stNowTime,0,sizeof(tm));
	stNowTime.tm_year=uiNow/10000-1900;
	stNowTime.tm_mon=uiNow%10000/100-1;
	stNowTime.tm_mday=uiNow%100;
	time_t stNow=mktime(&stNowTime);

	if(stNow>stPrev && difftime(stNow,stPrev)>=7*24*3600)
		return	false;
	else if(stPrev>stNow && difftime(stPrev,stNow)>=7*24*3600)
		return	false;
	if(stNowTime.tm_wday<stPrevTime.tm_wday && stNow>stPrev)
		return	false;
	else if(stNowTime.tm_wday>stPrevTime.tm_wday && stNow<stPrev)
		return	false;
	return	true;
}

bool CRealData::IsSameMonth(unsigned int uiNow, unsigned int uiPrev)
{//比较今日和所给定日期是否在同一月内
	unsigned char ucNowMonth=uiNow%10000/100;
	unsigned char ucPrevMonth=uiPrev%10000/100;
	if(uiNow/10000 != uiPrev/10000)
		return false;
	if(ucNowMonth!=ucPrevMonth)
		return false;
	return	true;
}

void CIndexSummary::SendReq6001()
{
	std::map<std::string,SummaryInfo *>::iterator iter;
	for(iter=m_mapSum.begin(); iter!=m_mapSum.end(); iter++)
	{
		SMsgID pID;
		pID.m_unID = 0XF0000000 + iter->second->m_usCode;
		unsigned short usFuncNo = 6001;
		g_pPlugs->SendRequest(&pID,usFuncNo,iter->second->m_acReq,42);
	}						
}

void CIndexSummary::OnRep6001(unsigned int c_unId,char * c_pRep)
{
	PPACKETHEAD pHead = (PPACKETHEAD)c_pRep;
	if (pHead->m_usFuncNo != 6001)
		return;

	std::map<std::string,SummaryInfo *>::iterator iter;
	for(iter=m_mapSum.begin(); iter!=m_mapSum.end(); iter++)
	{
		if (c_unId - 0xF0000000 == iter->second->m_usCode)
		{
			iter->second->m_usUp = *(unsigned int *)(c_pRep + 10);
			iter->second->m_usDown = *(unsigned int *)(c_pRep + 10 + sizeof(unsigned int));
			iter->second->m_usNoChange = *(unsigned int *)(c_pRep + 10 + sizeof(unsigned int) * 2);
		}
	}
}
bool	CStockKind::AddQuote(std::string c_strSymbol,Quote * c_pQuote)
{
	if (c_pQuote)
		m_mapQuote[c_strSymbol] = c_pQuote;
	else
	{
		CQuoteData quoteData;
		if (g_pDataFarm->m_hashIdx.cast(c_strSymbol,quoteData,true))
		{
			Quote *pQuote;
			pQuote = g_pDataFarm->GetQuote(quoteData.m_Idx.idxQuote);
			//if (pQuote)
			//	REPORT(MODULE_NAME,T("[%s][%d][%d]\n",pQuote->szStockCode,pQuote->cjje,pQuote->cjsl),RPT_INFO);
			m_mapQuote[c_strSymbol] = pQuote;
		}
	}
	return true;
}

int CStockKind::AddSymbols(char *c_pBuf,int c_nLen)
{
	char * pSrc, * pFind;
	char acSymbol[10];
	int nCount = 0;
	int nLen = c_nLen;
	pSrc = c_pBuf;
	while(nLen > 0)
	{
		pFind = strstr(pSrc,"|");
		if (pFind)
		{
			if (pFind - pSrc > 9)
			{
				//REPORT(MODULE_NAME,T("SYMBOL OUT OF RANGE[%d]\n",nLen),RPT_INFO);
				break;
			}
			else
			{
				strncpy(acSymbol,pSrc,pFind - pSrc);
				acSymbol[pFind - pSrc] = 0;
			}
			acSymbol[9] = 0;
			nLen -= pFind - pSrc - 1;
			//REPORT(MODULE_NAME,T("SYMBOL[%s][%d]\n",acSymbol,nLen),RPT_INFO);
			pSrc = pFind + 1;
		}
		else
		{
			//REPORT(MODULE_NAME,T("SYMBOL NOT FOUND[%d]\n",nLen),RPT_INFO);
			break;
			/*
			if (strlen(pSrc) > 9)
			{
				strcpy(acSymbol,pSrc);
				acSymbol[strlen(pSrc)] = 0;
			}
			else
				strncpy(acSymbol,pSrc,9);
			acSymbol[9] = 0;
			pSrc += strlen(pSrc);*/
		}
		std::string strSymbol = acSymbol;
		this->AddQuote(strSymbol,NULL);
		nCount++;
	}
	return nCount;
}

int	CStockKind::SendReq6004()
{
	char acBuf[128];
	SMsgID pID;
	pID.m_unID = m_usId;
	unsigned short usFuncNo = 6004;
	memset(acBuf,0,sizeof(acBuf));
	PPACKETHEAD pHead;
	pHead = (PPACKETHEAD)(acBuf);
	pHead->m_commxHead.Prop = pHead->m_commxHead.MakeProp(0x03,CommxHead::ANSI,CommxHead::HOSTORDER);
	pHead->m_commxHead.SerialNo = m_usId;
	pHead->m_usFuncNo = 6004;//取分类信息
	strcpy(acBuf + 10,m_strKindCode.c_str());//2BHL + H + 32S
	pHead->m_commxHead.SetLength(2 + 32);
	if (g_cfg.m_bDebug)
	REPORT(MODULE_NAME,T("Send 6004[%d]\n",pID.m_unID),RPT_INFO);
	return g_pPlugs->SendRequest(&pID,usFuncNo,acBuf,42);
}

int CStockKind::OnReq6004(unsigned short c_usId,char * c_pRep)
{
	PPACKETHEAD pHead = (PPACKETHEAD)c_pRep;
	if (pHead->m_commxHead.SerialNo != this->m_usId)
		return 0;
	ClearSymbols();
	if (g_cfg.m_bDebug)
	REPORT(MODULE_NAME,T("[6004][%d]\n",pHead->m_commxHead.GetLength()),RPT_INFO);
	AddSymbols(c_pRep + 10,pHead->m_commxHead.GetLength()-2);
	return 1;
}
