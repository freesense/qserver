
#include "stdafx.h"
#include "tradematch.h"
#include "workthread.h"
#include <algorithm>
#include <assert.h>
#include <WinSock2.h>
#include "../../public/commx/highperformancecounter.h"

#pragma warning(disable:4267)

#define QFGET(F, B) (((F)[((B)/32)]) & (1 << ((B)%32)))
#define IsMin(x, y, z) ((x) && (!(y) || (y) && (x < y)) && (!(z) || (z) && (x < z)))

extern void FormatBgTime(char * c_pDes,char * c_pDate);
////////////////////////////////////////////////////////////////////////////////////////////
int CheckSymbol(char * c_acSymbol)
{
	int n;
	//if (strlen(c_acSymbol) == 6)
	if (SYMBOL_LEN == 6)
		n = atoi(c_acSymbol);
	else
	{
		//if (strncmp(c_acSymbol + 6,".SH",3) == 0 || strncmp(c_acSymbol + 6,".SZ",3) == 0)
		if (strstr(c_acSymbol,".SH") || strstr(c_acSymbol,".SZ"))
			return  -1;
		char buf[7];
		strncpy(buf,c_acSymbol,6);
		buf[6] = 0;
		n = atoi(buf);
	}
	if (n >= 1 && n <= 9999)
		return 2;
	if (n >= 300000 && n <= 309999)
		return 2;
	if (n > 200000 && n <= 209999)
		return 2;
	if (n >= 30000 && n <= 39999)
		return 2;
	if (n >= 600000 && n <= 600999)
		return 1;
	if (n >= 601000 && n <= 601999)
		return 1;
	if (n >= 609000 && n <= 609999)
		return 1;
	if (n >= 900000 && n <= 999999)
		return 1;
	if (n >= 580000 && n <= 580999)
		return 1;
	return -1;
		//<品种 code="md_szag" name="深圳Ａ股">[03]0\d{4}.sz</品种> 000001-009999,300001-309999
		//<品种 code="md_szbg" name="深圳Ｂ股">20\d{4}.sz</品种>	200001-209999
		//<品种 code="md_szwa" name="深圳权证">03\d{4}.sz</品种>	030001-039999
		//<品种 code="md_shag" name="上海Ａ股">60[019]\d{3}.sh</品种> 600001-600999,601001-601999,609001-609999
		//<品种 code="md_shbg" name="上海Ｂ股">9\d{5}.sh</品种>		900000-999999
		//<品种 code="md_shwa" name="上海权证">580\d{3}.sh</品种>	580001-580999
}
////////////////////////////////////////////////////////////////////////////////////////////
long _find_symbol::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	m_pIdx = pri;
	return 0;
}

bool _find_symbol::find(std::string *lpSymbol)
{
	m_pIdx = NULL;
	gFarm.m_hashIdx.cast(*lpSymbol,*this,true);
	if (m_pIdx)
		return true;
	else
		return false;
}

TickUnit * _find_symbol::getlasttickunit()
{
	if (m_pIdx)
	{
		REPORT(MN,T("cntTick[%d]\n",m_pIdx->cntTick),RPT_INFO);
		if (m_pIdx->cntTick == 0)
		{
			gbIsTradeTime = false;
			return NULL;
		}
		gbIsTradeTime = true;
		TickBlock * p;
		int nIndex;
		p = gFarm.GetTick(m_pIdx->idxTick);
		if (p)
		{
			nIndex = m_pIdx->cntTick;
			while(p->next)
			{
				p = gFarm.GetTick(p->next);
				nIndex -= TICK_PERBLOCK;
			}
			return &(p->unit[nIndex]);
		}
		return NULL;
	}
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////
//获取当前的工作状态
int GetWorkState()
{
	std::string strSymbol("000001.SH");
	_find_symbol find_symbol;
	if (find_symbol.find(&strSymbol))
	{
		TickUnit * pUnit;
		Quote * quote;
		quote = gFarm.GetQuote(find_symbol.m_pIdx->idxQuote);
		pUnit = find_symbol.getlasttickunit();
		if (pUnit)
		{
			gnTradeTime = pUnit->Time;
			sprintf(gacTradeTime,"%02d%02d",pUnit->Time / 60 % 100,pUnit->Time % 60);
			REPORT(MN,T("上证指数最新交易时间%s %s %d [%4s]\n",quote->szStockCode,quote->szStockName,pUnit->Time,gacTradeTime),RPT_INFO);
		}
		return 0;
	}
	else
	{
		REPORT(MN,T("取上证指数最新交易时间失败\n"),RPT_INFO);
	}
	return -1;
	
}

////////////////////////////////////////////////////////////////////////////////////////////
void CWorkThread::close()
{
	//memset(&m_head, 0x00, sizeof(CommxHead));
	//gQueue.Push(&m_head);
}

unsigned int CWorkThread::svc(void* lpParam)
{
	//初始化数据库
	int len = 0;
	int timeout = INFINITE;
	int nHeart = 0;
	bool bFirstRun = true;
	m_dwPreTick = GetTickCount();

	DWORD dwNow = GetTickCount();
	char acSql[1024];
	m_nLastConsignId = 0;
	int m_nRecordCount = 0;
	bool bWaitEmpty = false;
	bool bInit = false;
	::CoInitialize(NULL);
	if (gDBRead.Open(cfg.m_strServerIP,cfg.m_strDataBase,cfg.m_strUser,cfg.m_strPass))
	{
		REPORT(MN,T("(Read)连接数据库成功！\n"),RPT_INFO);
	}
	else
		REPORT(MN,T("(Read)连接数据库失败[%s][%s][%s][%s]！\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);

	while (!gbExitFlag)
	{
		Sleep(1000);
		if (gDBRead.m_bIsConnected == false)
		{
			REPORT(MN,T("等待建立数据库连接(Read)\n"),RPT_INFO);
			gDBRead.Reconnect();
		}
		DWORD dwCurTick = GetTickCount();
		if (dwCurTick - m_dwPreTick >= (DWORD)cfg.m_nDevTimeout * 1000)
		{
			REPORT(MN, T(""), RPT_HEARTBEAT|RPT_IGNORE);
			m_dwPreTick = dwCurTick;
		}

		time_t now;
		time(&now);
		struct tm *lt = localtime(&now);
		int nTime;
		nTime =	lt->tm_hour * 100 + lt->tm_min;
		sprintf(gacTradeTime,"%02d%02d",lt->tm_hour,lt->tm_min);
		
		if (gbWorkDay && nTime > cfg.m_nStopSvcBegin && nTime < cfg.m_nStopSvcEnd)
		{//在清算时间区间内
			gDataStore.Clear();//清除未成交的委托
			m_nLastConsignId = 0;//从0开始
			bWaitEmpty = true;
			gbWorkDay = false;
			gnTradeTime = 0;
			Sleep(1000);
			continue;
		}
		
		if (bInit == false)
		{
			gnTradeTime = 0;
			_find_symbol find_symbol;
			std::string strToFind = "000001.SH";
			if (find_symbol.find(&strToFind))
			{
				REPORT(MN,T("FARM已经开盘完成 30秒后开始工作\n"),RPT_INFO);
				Sleep(1000 * 30);
				bInit = true;
			}
			else
			{
				REPORT(MN,T("等待FARM开盘完成\n"),RPT_INFO);
				Sleep(1000 * 30);
				continue;
			}
		}
		if (dwCurTick - dwNow >= 1000)
		{
			dwNow = dwCurTick;
			//读取委托表
			//sprintf(acSql,"select a.*,b.consignid as existid from UserVTradeConsign as a LEFT JOIN UserVTradeBargain  as b on a.consignId=b.consignId where a.consignid>%d order by consignId asc",m_nLastConsignId);
			if (bWaitEmpty)
				sprintf(acSql,"select count(*) cnt from UserVTradeConsign");
			else
			{
				if (bFirstRun)
				{
					sprintf(acSql,"select a.*,b.consignid bid,b.consignprice bprice,b.consignamount bamount,b.consignflag bflag,b.F9000 bF9000,b.F2000 bF2000,c.consignid done from UserVTradeConsign as a LEFT JOIN UserVTradeConsign as b on a.cancelid=b.consignid LEFT JOIN UserVTradeBargain as c on a.consignid=c.consignid where a.consignid>%d order by a.consignid  asc",m_nLastConsignId);
				}
				else
					sprintf(acSql,"select a.*,b.consignid bid,b.consignprice bprice,b.consignamount bamount,b.consignflag bflag,b.F9000 bF9000,b.F2000 bF2000 from UserVTradeConsign as a LEFT JOIN UserVTradeConsign as b on a.cancelid=b.consignid where a.consignid>%d order by a.consignid  asc",m_nLastConsignId);
			}
			//sprintf(acSql,"select * from UserVTradeConsign where consignId>%d order by consignId",m_nLastConsignId);
			//REPORT(MN, T("SQL[%s]\n",acSql), RPT_INFO);
			if (gDBRead.OpenTableUseSql(acSql))
			{
				if (bWaitEmpty)
				{
					m_nRecordCount = 0;
					gDBRead.GetCollect("cnt",m_nRecordCount);
					gDBRead.CloseTable();
					if (m_nRecordCount == 0)
					{
						bWaitEmpty = false;
						REPORT(MN,T("委托表已经被清空[%d],开始接受委托\n",m_nRecordCount),RPT_INFO);
						m_nLastConsignId = 0;
					}
					else
					{
						REPORT(MN,T("等待委托表被清空[%d]\n",m_nRecordCount),RPT_INFO);
						if (nTime > 1800)
						{
							REPORT(MN,T("委托表 没有被被清空[%d]，仍然开始工作，ID=%d\n",m_nRecordCount,m_nLastConsignId),RPT_INFO);
							bWaitEmpty = false;
						}
					}
					continue;
				}
				Consign * pItem;
				int nExistId;
				m_nRecordCount = 0;
				//REPORT(MN,T("RECORD STATE [%d][%d]\n",gDBRead.IsBOF(),gDBRead.IsEOF()),RPT_INFO);
				int nNowId;
				while (!gDBRead.IsBOF() && !gDBRead.IsEOF())
				//while(!gDBRead.IsEOF())
				{
					if (bFirstRun)
					{
						nExistId = 0;
						gDBRead.GetCollect("done",nExistId);
						if (nExistId > 0)//已经处理过了
						{
							gDBRead.GetCollect("consignId",nExistId);
							gDBRead.MoveNext();
							if (m_nLastConsignId < nExistId)
								m_nLastConsignId = nExistId;					
							continue;
						}
					}
					m_nRecordCount++;
					gSumInfo.AddConsign(1);
					pItem = new Consign();
					memset(pItem->m_acBgDate,0,sizeof(pItem->m_acBgDate));
					pItem->m_acDate[23] = 0;
					pItem->m_acSymbol[SYMBOL_LEN] = 0;
					gDBRead.GetCollect("consignId",pItem->m_nId);
					nNowId = pItem->m_nId;
					REPORT(MN,T("GetRecord [%d]\n",pItem->m_nId),RPT_INFO);
					gDBRead.GetCollect("consignDate",pItem->m_acDate,23);
					//REPORT(MN,T("GetRecord[%s]\n",pItem->m_acDate),RPT_INFO);
					double d;
					gDBRead.GetCollect("cancelId",pItem->m_nCancelId);
					gDBRead.GetCollect("type",pItem->m_nType);
					
					if (pItem->m_nType == CONSIGN_CANCEL)
					{
						gDBRead.GetCollect("bAmount",pItem->m_nAmount);
						gDBRead.GetCollect("bF9000",pItem->m_nUserId);
						gDBRead.GetCollect("bF2000",pItem->m_acSymbol,SYMBOL_LEN);
						gDBRead.GetCollect("bPrice",d);
						pItem->m_nPrice = (d * 1000);
						gDBRead.GetCollect("bflag",pItem->m_nFlag);
						//REPORT(MN,T("ID=%d CID=%d AMOUNT=%d F9000=%d F2000=%s PRICE=%d FLAG=%d\n",pItem->m_nId,pItem->m_nCancelId,pItem->m_nAmount,pItem->m_nUserId,pItem->m_acSymbol,pItem->m_nPrice,pItem->m_nFlag),RPT_INFO);
					}
					else
					{
						gDBRead.GetCollect("consignPrice",d);
						pItem->m_nPrice = (d * 1000);
						gDBRead.GetCollect("consignAmount",pItem->m_nAmount);

						gDBRead.GetCollect("F9000",pItem->m_nUserId);
						gDBRead.GetCollect("F2000",pItem->m_acSymbol,SYMBOL_LEN);
						gDBRead.GetCollect("consignflag",pItem->m_nFlag);
					}
					if (pItem->m_nPrice <= 0)
					{
						if (pItem->m_nType == CONSIGN_CANCEL)
							pItem->m_nStatus = 3;
						else
							pItem->m_nStatus = 4;
						FormatBgTime(pItem->m_acBgDate,gacTradeTime);
						
						gConsignQueue.Push(pItem);
						gSumInfo.AddBargain(1);
					}
					else
					{
						gDataStore.AddConsign(pItem);
						//gSumInfo.AddBargain(1);
					}
					if (m_nLastConsignId < nNowId)
						m_nLastConsignId = nNowId;
					gDBRead.MoveNext();
					//REPORT(MN,T("RECORD STATE [%d][%d][%d]\n",gDB.IsBOF(),gDB.IsEOF(),m_nRecordCount),RPT_INFO);
				}
				gDBRead.CloseTable();
				if (bFirstRun)
					bFirstRun = false;
			}
			else
			{
				gDBRead.Close();
			}
			if (m_nRecordCount > 0)
				REPORT(MN, T("WorkThread record  %d id=%d\n",m_nRecordCount,m_nLastConsignId), RPT_INFO);
			else
			if (++nHeart % 30 == 0)
			{
				REPORT(MN, T("WorkThread runging\n"), RPT_INFO);
				nHeart = 0;
			}
		}
	}
	::CoUninitialize();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
void CWriteThread::close()
{
	//memset(&m_head, 0x00, sizeof(CommxHead));
	//gQueue.Push(&m_head);
}

unsigned int CWriteThread::svc(void* lpParam)
{
	//初始化数据库
	int len = 0;
	int timeout = INFINITE;
	Consign * pConsign;
	m_dwPreTick = GetTickCount();

	DWORD dwNow = GetTickCount();
	char acSql[1024];
	m_nLastConsignId = 0;
	int m_nRecordCount = 0;
	::CoInitialize(NULL);
	if (gDBWrite.Open(cfg.m_strServerIP,cfg.m_strDataBase,cfg.m_strUser,cfg.m_strPass))
	{
		REPORT(MN,T("(Write)连接数据库成功！\n"),RPT_INFO);
	}
	else
		REPORT(MN,T("(Write)连接数据库失败[%s][%s][%s][%s]！\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);

	while (!gbExitFlag)
	{
		DWORD dwCurTick = GetTickCount();
		if (gDBWrite.m_bIsConnected == false)
		{
			REPORT(MN,T("等待建立数据库连接(Write)\n"),RPT_INFO);
			gDBWrite.Reconnect();
			continue;
		}
		timeout = 1000;
		time_t  l_cur_time ;
		struct tm * ltm_time = NULL ;
		time(&l_cur_time) ;
		ltm_time = localtime(&l_cur_time) ;
		int nTime = ltm_time->tm_hour * 60 + ltm_time->tm_min;
		
		int nSize = -1;
		//if (gbTradeTime)
		
		if (nTime >= 570)
		{
			//REPORT(MN,T("gConsignQueueWait Pop\n"),RPT_INFO);
			nSize = gConsignQueueWait.Pop(pConsign, timeout);
			//REPORT(MN,T("gConsignQueueWait Pop %d\n",nSize),RPT_INFO);
		}

		if (nSize == -1)
			nSize = gConsignQueue.Pop(pConsign, timeout);
		
		if (-1 != nSize)
		{
			//REPORT(MN,T("OK Get record[%d]\n",pConsign->m_nId),RPT_INFO);
			
			//if (pConsign->m_nStatus == 1 && !gbTradeTime)
			if (pConsign->m_nStatus == 1 && nTime<570)
			{
				REPORT(MN,T("id=%d,time=%s,push to waitqueue\n",pConsign->m_nId,pConsign->m_acDate),RPT_INFO);
				gConsignQueueWait.Push(pConsign);
				continue;
			}
			if (pConsign->m_nType == CONSIGN_CANCEL && pConsign->m_nStatus == 5)
				pConsign->m_nStatus = 3;
			sprintf(acSql,"insert into UserVTradeBargain (consignId,bargainDate,bargainPrice,bargainAmount,status,F9000,F2000) values (%d,'%4s',%.3f,%d,%d,%d,'%s')",pConsign->m_nId,pConsign->m_acBgDate,float(pConsign->m_nPrice)/1000,pConsign->m_nAmount,pConsign->m_nStatus,pConsign->m_nUserId,pConsign->m_acSymbol);
			//REPORT(MN,T("insert sql[%s]\n",acSql),RPT_INFO);
			if (gDBWrite.RunSql(acSql))
			{
				delete pConsign;
			}
			else
			{
				REPORT(MN, T("Write To DB Fail\n"), RPT_INFO);
				gConsignQueue.Push(pConsign);
				gDBWrite.Close();
			}
			REPORT(MN, T("gConsignQueue left size %d\n",nSize), RPT_INFO);
		}
		
	}
	::CoUninitialize();
	return 0;
}

unsigned int CInfoThread::svc(void* lpParam)
{
	//初始化数据库

	DWORD dwNow = GetTickCount();
	DWORD dwRun;
	dwRun = dwNow;
	while (!gbExitFlag)
	{
		if((gnTradeTime >= 570 && gnTradeTime < 690) || (gnTradeTime >= 780 && gnTradeTime < 900))//交易时间段内
		{
			gDataStore.StockTradeMatch();
		}
		Sleep(1000);
		if(gSumInfo.m_nConsign > 0 || gSumInfo.m_nBargain > 0)
			REPORT(MN,T("最大(%d:%d),当前(%d:%d),队列(%d)\n",gSumInfo.m_nMaxConsign,gSumInfo.m_nMaxBargain,gSumInfo.m_nConsign,gSumInfo.m_nBargain,gConsignQueue.GetCount()),RPT_INFO);
		gSumInfo.SetNew();
		continue;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
void CInfoThread::close()
{
	//memset(&m_head, 0x00, sizeof(CommxHead));
	//gQueue.Push(&m_head);
}
