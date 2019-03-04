#include "stdafx.h"
#include <string>
#include "DataStore.h"
#include "workthread.h"

void FormatBgTime(char * c_pDes,char * c_pDate)
{
	time_t  l_cur_time ;
	struct tm * ltm_time = NULL ;
	char  lc_time[20] ;
	memset(c_pDes,0,30);
	time(&l_cur_time) ;
	ltm_time = localtime(&l_cur_time) ;
	sprintf(c_pDes,"%4d-%02d-%02d ",ltm_time->tm_year + 1900,ltm_time->tm_mon +1,ltm_time->tm_mday) ;
	strncat(c_pDes,c_pDate,2);
	strcat(c_pDes,":");
	strncat(c_pDes,c_pDate+2,2);
	strcat(c_pDes,":000");
	return ;
}

CConsignOfStock::CConsignOfStock(std::string c_strSymbol,int c_nMarket,RINDEX * c_pIdx)
{
	m_strSymbol = c_strSymbol;
	m_strName = "";
	m_nMarket = c_nMarket;
	m_pIdx = c_pIdx;
	m_bCacl = false;
	m_unZtjg = 0;
	m_unDtjg = 0;
	m_unZrsp = 0;
	m_unPrice = 0;
	Quote * pQuote = gFarm.GetQuote(m_pIdx->idxQuote);
	if (pQuote)
	{
		m_pQuote = pQuote;
		m_unPrice = pQuote->zjjg;
		m_strSymbol = pQuote->szStockCode;
		m_strName = pQuote->szStockName;
		m_unZrsp = pQuote->zrsp;
		m_unZtjg = pQuote->ztjg;
		m_unDtjg = pQuote->dtjg;
	}
	else
		m_pQuote = NULL;
};

CConsignOfStock::~CConsignOfStock(void)
{
};

void CConsignOfStock::ResetQuote()
{
	_find_symbol find_symbol;
	if (find_symbol.find(&m_strSymbol))
	{
		m_pIdx = find_symbol.m_pIdx;
		m_pQuote = gFarm.GetQuote(m_pIdx->idxQuote);
	}
	else
	{
		REPORT(MN,T("ResetQuote 失败:[%s]\n",m_strSymbol.c_str()),RPT_INFO);
		m_pIdx = NULL;
		m_pQuote = NULL;
	}
	
}
int CConsignOfStock::AddConsign(Consign * c_pConsign)
{
	if((gnTradeTime >= 570 && gnTradeTime < 690) || (gnTradeTime >= 780 && gnTradeTime < 900))//交易时间段内
	{
		if (m_pQuote)
		{
			if (c_pConsign->m_nFlag == 1 && m_pQuote->BP1>0 && c_pConsign->m_nPrice<=m_pQuote->BP1)
			{
				REPORT(MN,T("立即卖单:[委托价格:%d][买一价:%d]\n",c_pConsign->m_nPrice,m_pQuote->BP1),RPT_INFO);
				FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
				c_pConsign->m_nStatus = 1;		
				c_pConsign->m_nPrice = m_pQuote->BP1;
				gConsignQueue.Push(c_pConsign);
				gSumInfo.AddBargain(1);
				return 0;
			}
			else
			//if (c_pConsign->m_nFlag == 0 && m_pQuote->SP1>0 && c_pConsign->m_nPrice>=m_pQuote->SP1)
			if (c_pConsign->m_nFlag == 0 && m_pQuote->SP1>0 && m_pQuote->SP2>0 && c_pConsign->m_nPrice>=m_pQuote->SP1)
			{
				REPORT(MN,T("立即买单:[委托价格:%d][卖一价:%d][卖二价:%d]\n",c_pConsign->m_nPrice,m_pQuote->SP1,m_pQuote->SP2),RPT_INFO);
				FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
				c_pConsign->m_nStatus = 1;
				c_pConsign->m_nPrice = m_pQuote->SP1;
				gConsignQueue.Push(c_pConsign);
				gSumInfo.AddBargain(1);
				return 0;
			}
		}
	}
	m_rwLock.WaitWrite();
	PriceUnit * pPriceUnit;
	if (c_pConsign->m_nFlag == 0)
	{
		if (m_mapPriceUnitBuy.find(c_pConsign->m_nPrice) == m_mapPriceUnitBuy.end())//没找到
		{
			pPriceUnit = new PriceUnit();
			pPriceUnit->m_nPrice = c_pConsign->m_nPrice;
			m_mapPriceUnitBuy[pPriceUnit->m_nPrice] = pPriceUnit;//新建一条
			pPriceUnit->m_listConsign.push_back(c_pConsign);
		}
		else
		{	
			m_mapPriceUnitBuy[c_pConsign->m_nPrice]->m_listConsign.push_back(c_pConsign);
		}
		m_rwLock.EndWrite();
		return 0;
	}
	else
	{
		if (m_mapPriceUnitSale.find(c_pConsign->m_nPrice) == m_mapPriceUnitSale.end())//没找到
		{
			pPriceUnit = new PriceUnit();
			pPriceUnit->m_nPrice = c_pConsign->m_nPrice;
			m_mapPriceUnitSale[pPriceUnit->m_nPrice] = pPriceUnit;//新建一条
			pPriceUnit->m_listConsign.push_back(c_pConsign);
		}
		else
		{	
			m_mapPriceUnitSale[c_pConsign->m_nPrice]->m_listConsign.push_back(c_pConsign);
		}
		m_rwLock.EndWrite();
		return 0;
	}
	m_rwLock.EndWrite();
	return -1;
}
int CConsignOfStock::AddConsignOld(Consign * c_pConsign)
{
	int nRet = CheckPrice(c_pConsign->m_nPrice);
//	if ((nRet == -1 && c_pConsign->m_nFlag == 0)//买单低于跌停价格
//		||(nRet == 1 && c_pConsign->m_nFlag == 1))//卖单高于涨停价
	if (nRet != 0)
	{
		REPORT(MN,T("价格限制:[%d][%d][现价:%d]\n",m_unDtjg,m_unZtjg,c_pConsign->m_nPrice),RPT_INFO);
		FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
		c_pConsign->m_nStatus = 4;		
		gConsignQueue.Push(c_pConsign);
		gSumInfo.AddBargain(1);
		return -1;
	}
	if((gnTradeTime >= 570 && gnTradeTime < 690) || (gnTradeTime >= 780 && gnTradeTime < 900))//交易时间段内
	{
	if ((m_unPrice == c_pConsign->m_nPrice && m_unPrice == m_unZtjg && c_pConsign->m_nFlag == 1)
	|| (m_unPrice == c_pConsign->m_nPrice && m_unPrice == m_unDtjg && c_pConsign->m_nFlag == 0))
	{
		REPORT(MN,T("立即买卖价格:[涨停:%d][跌停:%d][委托价格:%d][现价:%d]\n",m_unDtjg,m_unZtjg,c_pConsign->m_nPrice,m_unPrice),RPT_INFO);
		FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
		c_pConsign->m_nStatus = 1;		
		gConsignQueue.Push(c_pConsign);
		gSumInfo.AddBargain(1);
		return 0;
	}
	//如果高于当前价格，价差在2分或以上的买单，低于当前价格，价差在2分或以上的卖单，立即成交。
	if ((c_pConsign->m_nFlag == 0 && (m_unPrice > 0) && (c_pConsign->m_nPrice - (int)m_unPrice)>=20)//买单
		|| (c_pConsign->m_nFlag == 1 && m_unPrice > 0 && ((int)m_unPrice - c_pConsign->m_nPrice )>=20))//卖单
	{
		REPORT(MN,T("立即买卖价格:[委托价格:%d][现价:%d][买卖:%d]\n",c_pConsign->m_nPrice,m_unPrice,c_pConsign->m_nFlag),RPT_INFO);
		FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
		/*if (c_pConsign->m_nFlag == 0)
			c_pConsign->m_nPrice = m_unPrice + 20;
		else
			c_pConsign->m_nPrice = m_unPrice - 20;*/
		c_pConsign->m_nStatus = 1;		
		gConsignQueue.Push(c_pConsign);
		gSumInfo.AddBargain(1);
		return 0;
	}
	}
	m_rwLock.WaitWrite();
	PriceUnit * pPriceUnit;
	if (c_pConsign->m_nFlag == 0)
	{
		if (m_mapPriceUnitBuy.find(c_pConsign->m_nPrice) == m_mapPriceUnitBuy.end())//没找到
		{
			pPriceUnit = new PriceUnit();
			pPriceUnit->m_nPrice = c_pConsign->m_nPrice;
			m_mapPriceUnitBuy[pPriceUnit->m_nPrice] = pPriceUnit;//新建一条
			pPriceUnit->m_listConsign.push_back(c_pConsign);
		}
		else
		{	
			m_mapPriceUnitBuy[c_pConsign->m_nPrice]->m_listConsign.push_back(c_pConsign);
		}
		m_rwLock.EndWrite();
		return 0;
	}
	else
	{
		if (m_mapPriceUnitSale.find(c_pConsign->m_nPrice) == m_mapPriceUnitSale.end())//没找到
		{
			pPriceUnit = new PriceUnit();
			pPriceUnit->m_nPrice = c_pConsign->m_nPrice;
			m_mapPriceUnitSale[pPriceUnit->m_nPrice] = pPriceUnit;//新建一条
			pPriceUnit->m_listConsign.push_back(c_pConsign);
		}
		else
		{	
			m_mapPriceUnitSale[c_pConsign->m_nPrice]->m_listConsign.push_back(c_pConsign);
		}
		m_rwLock.EndWrite();
		return 0;
	}
	m_rwLock.EndWrite();
	return -1;
}

int CConsignOfStock::CancelConsign(Consign * c_pConsign)
{
	m_rwLock.WaitWrite();
	std::list<Consign *>::iterator iter;
	//REPORT(MN,T("CancelConsign %d->%d\n",c_pConsign->m_nId,c_pConsign->m_nCancelId),RPT_INFO);
	if (c_pConsign->m_nFlag == 0)
	{
		//REPORT(MN,T("CancelConsign A\n"),RPT_INFO);
		if (m_mapPriceUnitBuy.find(c_pConsign->m_nPrice) != m_mapPriceUnitBuy.end())//找到
		{
			//REPORT(MN,T("CancelConsign B\n"),RPT_INFO);
			for(iter=m_mapPriceUnitBuy[c_pConsign->m_nPrice]->m_listConsign.begin(); iter!=m_mapPriceUnitBuy[c_pConsign->m_nPrice]->m_listConsign.end(); iter++)
			{
				if ((*iter)->m_nId == c_pConsign->m_nCancelId)
				{//找到了
					REPORT(MN,T("CancelConsign Find %d [%d]\n",c_pConsign->m_nCancelId,m_mapPriceUnitBuy[c_pConsign->m_nPrice]->m_listConsign.size()),RPT_INFO);
					(*iter)->m_nCancelId = c_pConsign->m_nId;
					(*iter)->m_nStatus = 2;
					
					FormatBgTime((*iter)->m_acBgDate,gacTradeTime);//撤单时间
			
					c_pConsign->m_nStatus = 2;
					FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
					
					gConsignQueue.Push(c_pConsign);
					gSumInfo.AddBargain(2);
					gConsignQueue.Push(*iter);
					m_mapPriceUnitBuy[c_pConsign->m_nPrice]->m_listConsign.erase(iter);
					REPORT(MN,T("CancelConsign erase %d [%d]\n",c_pConsign->m_nCancelId,m_mapPriceUnitBuy[c_pConsign->m_nPrice]->m_listConsign.size()),RPT_INFO);
					m_rwLock.EndWrite();
					return true;
				}
			}
		}
	}
	else
	{
		if (m_mapPriceUnitSale.find(c_pConsign->m_nPrice) != m_mapPriceUnitSale.end())//找到
		{
			for(iter=m_mapPriceUnitSale[c_pConsign->m_nPrice]->m_listConsign.begin(); iter!=m_mapPriceUnitSale[c_pConsign->m_nPrice]->m_listConsign.end(); iter++)
			{
				if ((*iter)->m_nId == c_pConsign->m_nCancelId)
				{//找到了
					REPORT(MN,T("CancelConsign Find %d [%d]\n",c_pConsign->m_nCancelId,m_mapPriceUnitSale[c_pConsign->m_nPrice]->m_listConsign.size()),RPT_INFO);
					(*iter)->m_nCancelId = c_pConsign->m_nId;
					(*iter)->m_nStatus = 2;
					FormatBgTime((*iter)->m_acBgDate,gacTradeTime);//撤单时间
					
					FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
					
					c_pConsign->m_nStatus = 2;
					gConsignQueue.Push(c_pConsign);
					gConsignQueue.Push(*iter);
					gSumInfo.AddBargain(2);
					m_mapPriceUnitSale[c_pConsign->m_nPrice]->m_listConsign.erase(iter);
					REPORT(MN,T("CancelConsign erase %d [%d]\n",c_pConsign->m_nCancelId,m_mapPriceUnitSale[c_pConsign->m_nPrice]->m_listConsign.size()),RPT_INFO);
					m_rwLock.EndWrite();
					return true;
				}
			}
		}
	}
	c_pConsign->m_nStatus = 3;
	FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
	
	gConsignQueue.Push(c_pConsign);
	gSumInfo.AddBargain(1);
	m_rwLock.EndWrite();
	return -1;
}
int CConsignOfStock::CheckPrice(unsigned int c_unPrice)
{
	//return 0;
	if (m_unZrsp == 0)//没有收盘价格，不判断
		return 0;
	if (m_strName.length() == 0)//没有名称，不判断
		return 0;
	if (strncmp(m_strSymbol.c_str(),"60",2) != 0)//不是上海市场,则按照传进来的涨跌停价格来判断
	{
		if (m_unZtjg == 0 || m_unDtjg == 0)//限制价格为0，不判断
			return 0;
		if (c_unPrice < m_unDtjg)//小于跌停价
			return -1;
		if (c_unPrice > m_unZtjg)//大于涨停价
			return 1;
		return 0;
	}
	//上海市场
	float fzf,fdf;
	if (strnicmp(m_strName.c_str(),"ST",2) == 0 || strnicmp(m_strName.c_str(),"*ST",3) == 0 || strnicmp(m_strName.c_str(),"SST",3) == 0)//判断是否ST,SST,*ST
	{
		fzf = 0.05;
		fdf = -0.05;
	}
	else
	{
		fzf = 0.10;
		fdf = -0.10;
	}
	m_unZtjg = (float)m_unZrsp * float(1 + fzf);
	m_unDtjg = (float)m_unZrsp * float(1 + fdf);
	
	unsigned nLeft;
	nLeft = m_unZtjg%10;
	if (nLeft >=5)
		m_unZtjg += 10 - nLeft;
	else
		m_unZtjg -= nLeft;

	nLeft = m_unDtjg%10;
	if (nLeft >=5)
		m_unDtjg += 10 - nLeft;
	else
		m_unDtjg -= nLeft;

	if (c_unPrice < m_unDtjg)//小于跌停价
		return -1;
	if (c_unPrice > m_unZtjg)//大于涨停价
		return 1;

	return 0;
}
void CConsignOfStock::TradeMatch()
{
	char acDate[5];
	std::map<int,PriceUnit *>::iterator iter;
	std::list<Consign *>::iterator iterConsign;

	sprintf(acDate,"%02d%02d",gnTradeTime/60 % 100,gnTradeTime%60);
	acDate[4] = 0;
	if (m_pQuote == NULL)
		return;
	if(m_mapPriceUnitBuy.size() == 0 && m_mapPriceUnitSale.size() == 0)
		return;
	
	m_rwLock.WaitWrite();
	if (m_pQuote->SP1 > 0 && m_pQuote->SP2 > 0)
	for(iter=m_mapPriceUnitBuy.begin(); iter!=m_mapPriceUnitBuy.end(); iter++)
	{
		if ((*iter).first >= m_pQuote->SP1)//
		{			
			for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
			{
				(*iterConsign)->m_nStatus = 1;
				FormatBgTime((*iterConsign)->m_acBgDate,acDate);
				(*iterConsign)->m_nPrice = m_pQuote->SP1;
				REPORT(MN,T("to TradeMatch [买单:%d[委托价:%d][卖一价:%d][卖二价:%d][时间:%d]]\n",(*iterConsign)->m_nId,(*iterConsign)->m_nPrice,m_pQuote->SP1,m_pQuote->SP2,gnTradeTime),RPT_INFO);
				gConsignQueue.Push(*iterConsign);				
			}
			gSumInfo.AddBargain((*iter).second->m_listConsign.size());
			(*iter).second->m_listConsign.clear();
		}
	}
	if (m_pQuote->BP1 > 0)
	for(iter=m_mapPriceUnitSale.begin(); iter!=m_mapPriceUnitSale.end(); iter++)
	{
		if ((*iter).first <= m_pQuote->BP1)//
		{			
			for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
			{
				(*iterConsign)->m_nStatus = 1;
				FormatBgTime((*iterConsign)->m_acBgDate,acDate);
				(*iterConsign)->m_nPrice = m_pQuote->BP1;
				REPORT(MN,T("to TradeMatch [卖单:%d[委托价:%d][买一价:%d][时间:%d]]\n",(*iterConsign)->m_nId,(*iterConsign)->m_nPrice,m_pQuote->BP1,gnTradeTime),RPT_INFO);
				gConsignQueue.Push(*iterConsign);				
			}
			gSumInfo.AddBargain((*iter).second->m_listConsign.size());
			(*iter).second->m_listConsign.clear();
		}
	}
	m_rwLock.EndWrite();
}

void CConsignOfStock::OnNewTick(Quote * quote,TickUnit * tickunit)
{
	int nNewPrice;//最近成交价格
	char acDate[5];
	std::map<int,PriceUnit *>::iterator iter;
	std::list<Consign *>::iterator iterConsign;
	//获取最新的成交价格和时间
	_find_symbol find_symbol;
	if (m_pIdx)
	{
		find_symbol.m_pIdx = m_pIdx;
	}
	TickUnit * pUnit;
	pUnit = tickunit;
	//pUnit = find_symbol.getlasttickunit(); 不需要自己去取
	//REPORT(MN,T("OnNewTick %s\n",this->m_strSymbol.c_str()),RPT_INFO);
	if (pUnit == NULL)
		return;
	//REPORT(MN,T("OnNewTick %s %d b=%d s=%d\n",this->m_strSymbol.c_str(),pUnit->Price,m_mapPriceUnitBuy.size(),m_mapPriceUnitSale.size()),RPT_INFO);
	nNewPrice = pUnit->Price;
	sprintf(acDate,"%02d%02d",pUnit->Time/60 % 100,pUnit->Time%60);
	acDate[4] = 0;

	m_strName = quote->szStockName;
	m_unZrsp = quote->zrsp;
	m_unZtjg = quote->ztjg;
	m_unDtjg = quote->dtjg;
	m_unPrice = tickunit->Price;//现价

	REPORT(MN,T("OnNewTick (%s)(q:zrsp:%d)(q:ztjg:%d)(q:dtjg:%d)(zt:%d)(tickprice:%d)(dt:%d)\n",quote->szStockCode,quote->zrsp,quote->ztjg,quote->dtjg,m_unZtjg,nNewPrice,m_unDtjg),RPT_INFO);
	int nRet;
	m_rwLock.WaitWrite();
	for(iter=m_mapPriceUnitBuy.begin(); iter!=m_mapPriceUnitBuy.end(); iter++)
	{
		//REPORT(MN,T("BUY (%d)>(%d) (z:%d)(d:%d)\n",(*iter).first,nNewPrice,quote->ztjg,quote->dtjg),RPT_INFO);
		//if (m_nZtjg > 0 && m_nDtjg > 0 && ((*iter).first < m_nDtjg || (*iter).first > m_nZtjg))
		//if (m_nZtjg > 0 && m_nDtjg > 0 && (*iter).first < m_nDtjg)
		nRet = CheckPrice((*iter).first);
		//if (nRet < 0)//买单低于跌停价格
		if (nRet != 0)//价格小于跌停价格或者大于涨停价格
		{
			for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
			{
				(*iterConsign)->m_nStatus = 4;
				FormatBgTime((*iterConsign)->m_acBgDate,acDate);
				
				gConsignQueue.Push(*iterConsign);
				REPORT(MN,T("ConsignID=%d 买单价格限制 %d\n",(*iterConsign)->m_nId,(*iterConsign)->m_nPrice),RPT_INFO);
			}
			gSumInfo.AddBargain((*iter).second->m_listConsign.size());
			(*iter).second->m_listConsign.clear();
		}
		else
		{
			if (m_unZtjg == 0 || nNewPrice<m_unZtjg)//如果ticker价格是涨停价格，则不做撮合买单
			{
				if ((*iter).first > nNewPrice || ((*iter).first == m_unDtjg && nNewPrice == m_unDtjg))//
				{			
					for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
					{
						(*iterConsign)->m_nStatus = 1;
						FormatBgTime((*iterConsign)->m_acBgDate,acDate);
				
						//if ((*iterConsign)->m_nPrice - nNewPrice > 10)//价格差超过1分钱
						if ((*iterConsign)->m_nPrice != m_unDtjg || nNewPrice!=m_unDtjg)//现价是跌停价，并且委托价格是跌停价
							(*iterConsign)->m_nPrice = nNewPrice;
						REPORT(MN,T("to OnNewTick [%d[%d][%d][%d]]\n",(*iterConsign)->m_nId,(*iterConsign)->m_nPrice,tickunit->Price,tickunit->Time),RPT_INFO);
						gConsignQueue.Push(*iterConsign);				
					}
					gSumInfo.AddBargain((*iter).second->m_listConsign.size());
					(*iter).second->m_listConsign.clear();
				}
			}
		}
	}

	for(iter=m_mapPriceUnitSale.begin(); iter!=m_mapPriceUnitSale.end(); iter++)
	{
		//REPORT(MN,T("SALE (%d)<(%d) (z:%d)(d:%d)\n",(*iter).first,nNewPrice,quote->ztjg,quote->dtjg),RPT_INFO);
		nRet = CheckPrice((*iter).first);
		//if (nRet > 0)//卖单大于涨停价格
		if (nRet != 0)//价格小于跌停价格或者大于涨停价格
		{
			for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
			{
				(*iterConsign)->m_nStatus = 4;
				FormatBgTime((*iterConsign)->m_acBgDate,acDate);
				
				gConsignQueue.Push(*iterConsign);
				REPORT(MN,T("ConsignID=%d 卖单价格限制 %d\n",(*iterConsign)->m_nId,(*iterConsign)->m_nPrice),RPT_INFO);
			}
			gSumInfo.AddBargain((*iter).second->m_listConsign.size());
			(*iter).second->m_listConsign.clear();
		}
		else
			if (m_unDtjg == 0 || nNewPrice>m_unDtjg)//如果ticker价格是跌停价格，则不做撮合卖单
			{
				if ((*iter).first < nNewPrice || (nNewPrice==m_unZtjg && (*iter).first == m_unZtjg))//卖单大于当前价格，或者卖单价格为涨停价格，并且当前价格是涨停价格
				{
					for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
					{
						(*iterConsign)->m_nStatus = 1;
						FormatBgTime((*iterConsign)->m_acBgDate,acDate);
						REPORT(MN,T("to OnNewTick [%d[%d][%d][%d]]B\n",(*iterConsign)->m_nId,(*iterConsign)->m_nPrice,tickunit->Price,tickunit->Time),RPT_INFO);
						if ((*iterConsign)->m_nPrice != m_unZtjg || nNewPrice!=m_unZtjg)
							(*iterConsign)->m_nPrice = nNewPrice;
						gConsignQueue.Push(*iterConsign);
					}
					gSumInfo.AddBargain((*iter).second->m_listConsign.size());
					(*iter).second->m_listConsign.clear();
				}
			}
	}
	m_rwLock.EndWrite();
}
void CConsignOfStock::OnDelete()
{
	std::map<int,PriceUnit *>::iterator iter;
	std::list<Consign *>::iterator iterConsign;
	//获取最新的成交价格和时间

	m_rwLock.WaitWrite();
	for(iter=m_mapPriceUnitBuy.begin(); iter!=m_mapPriceUnitBuy.end(); iter++)
	{
		for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
		{
			//REPORT(MN,T("delete %d %s\n",(*iterConsign)->m_nId,(*iterConsign)->m_acSymbol),RPT_INFO);
			delete (*iterConsign);
		}
		(*iter).second->m_listConsign.clear();
	}
	for(iter=m_mapPriceUnitSale.begin(); iter!=m_mapPriceUnitSale.end(); iter++)
	{
		for(iterConsign=(*iter).second->m_listConsign.begin();iterConsign!=(*iter).second->m_listConsign.end(); iterConsign++)
		{
			//REPORT(MN,T("delete %d %s\n",(*iterConsign)->m_nId,(*iterConsign)->m_acSymbol),RPT_INFO);
			delete (*iterConsign);
		}
		(*iter).second->m_listConsign.clear();
	}
	m_rwLock.EndWrite();
}
int CConsignOfStock::cast(std::string  * c_pKey, RINDEX * c_pValue,bool c_bFlag)
{
	m_pIdx = c_pValue;
	return 0;
}

CDataStore::CDataStore(void)
{
}

CDataStore::~CDataStore(void)
{
	Clear();
}

void CDataStore::OnBegin()
{
}

void CDataStore::AddSymbol(const char *symbol, int *pFlag)
{
}

void CDataStore::UpdateSymbol(const char * symbol, int *pFlag,Quote *quote,TickUnit * tickunit)
{
	//REPORT(MN,T("to UpdateSymbol [%s][%d][%d][%d]\n",symbol,tickunit->Time,tickunit->Price,tickunit->Volume),RPT_INFO);
	//if (*pFlag & 0x0008)//成交量变化
	if (true)
	{
		if (strcmp(symbol,"000001.SH") == 0)
		{//跟踪上证指数
			//GetWorkState();
			if (gnTradeTime == 0)
				this->ResetQuote();//重新设置Quote 
			gnTradeTime = tickunit->Time;//当前交易时间
			sprintf(gacTradeTime,"%02d%02d",tickunit->Time / 60 % 100,tickunit->Time % 60);
			if (!gbWorkDay)
			{
			if (tickunit->Time > 580 && tickunit->Time < 890)
				gbWorkDay = true;
			}
			if (!gbTradeTime && tickunit->Time >= 580)
			{
				gbTradeTime = true;
				REPORT(MN,T("Set gbTradeTime=true\n"),RPT_INFO);
			}
			if (gbTradeTime && tickunit->Time < 580)
			{
				gbTradeTime = false;
				REPORT(MN,T("Set gbTradeTime=false\n"),RPT_INFO);
			}
			//REPORT(MN,T("to UpdateSymbol 上海综合指数 [%s][%d][%d][%d][%d]\n",symbol,tickunit->Time,tickunit->Price,tickunit->Volume,gbTradeTime),RPT_INFO);
		}
		else
		{
			/*不使用tick进行交易
			if (strstr(symbol,".SH") || strstr(symbol,".SZ"))
				return;
			m_lock.WaitWrite();
			std::string strSymbol;
			char acSymbol[SYMBOL_LEN + 1];
			strncpy(acSymbol,symbol,SYMBOL_LEN);
			acSymbol[SYMBOL_LEN] = 0;
			strSymbol = acSymbol;			
			if (m_mapStock.find(strSymbol) != m_mapStock.end())
			{
				if (tickunit->Price > 0)
				{
					m_mapStock[strSymbol]->OnNewTick(quote,tickunit);
				}
				else
					REPORT(MN,T("fail to UpdateSymbol [%s][%d][%d][%d]\n",symbol,tickunit->Time,tickunit->Price,tickunit->Volume),RPT_INFO);
			}
			m_lock.EndWrite();
			*/
		}
	}
}

void CDataStore::DelSymbol(const char * symbol)
{
	REPORT(MN,T("to DelSymbol [%s]\n",symbol),RPT_INFO);
	m_lock.WaitWrite();
	std::string strSymbol;
	std::map<std::string,CConsignOfStock *>::iterator iter;
	char acSymbol[SYMBOL_LEN + 1];
	strncpy(acSymbol,symbol,SYMBOL_LEN);
	acSymbol[SYMBOL_LEN] = 0;
	strSymbol = acSymbol;
	//REPORT(MN,T("to DelSymbol [%s] A\n",strSymbol.c_str()),RPT_INFO);
	iter = m_mapStock.find(strSymbol);
	if (iter != m_mapStock.end())
	{
		//REPORT(MN,T("to DelSymbol [%s] B\n",strSymbol.c_str()),RPT_INFO);
		(*iter).second->OnDelete();
		delete (*iter).second;
		m_mapStock.erase(iter);
	}
	m_lock.EndWrite();
}

void CDataStore::OnEnd()
{
}


bool CDataStore::AddConsign(Consign * c_pConsign)//增加一个委托
{
	std::string str;
	str = c_pConsign->m_acSymbol;
	//REPORT(MN,T("To AddConsign %d %s>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n",c_pConsign->m_nId,str.c_str()),RPT_INFO);
	m_lock.WaitWrite();
	if (m_mapStock.find(str) != m_mapStock.end())//找到
	{
		//REPORT(MN,T("m_mapStock FIND %s\n",str.c_str()),RPT_INFO);
		if (c_pConsign->m_nType == CONSIGN_CANCEL)
			m_mapStock[str]->CancelConsign(c_pConsign);
		else
		{
			//REPORT(MN,T("m_mapStock FIND To AddConsign %d %s\n",c_pConsign->m_nId,str.c_str()),RPT_INFO);
			m_mapStock[str]->AddConsign(c_pConsign);
			//REPORT(MN,T("m_mapStock FIND OK AddConsign %d %s\n",c_pConsign->m_nId,str.c_str()),RPT_INFO);
		}
	}
	else
	{
		if (c_pConsign->m_nType == CONSIGN_CANCEL)
		{
			FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
			
			c_pConsign->m_nStatus = 5;//无此股票代码的委托
			gConsignQueue.Push(c_pConsign);
			gSumInfo.AddBargain(1);
			m_lock.EndWrite();
			return false;
		}
		int nRet;
		//REPORT(MN,T("m_mapStock Not FIND %s\n",str.c_str()),RPT_INFO);
		nRet = CheckSymbol(c_pConsign->m_acSymbol);
		REPORT(MN,T("CheckSymbol %s ret=%d\n",str.c_str(),nRet),RPT_INFO);
		if (nRet < 0)
		{
			//REPORT(MN,T("CheckSymbol %d %s ret=%d\n",c_pConsign->m_nId,str.c_str(),nRet),RPT_INFO);
			FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
			
			c_pConsign->m_nStatus = 5;//不支持的证券代码
			gConsignQueue.Push(c_pConsign);
			m_lock.EndWrite();
			return false;
		}
		std::string strToFind;
		if (str.length() == 6)
		{
			if (nRet == 1)
				strToFind = str + ".sh";
			else
				strToFind = str + ".sz";
		}
		else
			strToFind = str;
		_find_symbol find_symbol;
		if (find_symbol.find(&strToFind))
		{
			//REPORT(MN,T("farm find success %s\n",strToFind.c_str()),RPT_INFO);
			CConsignOfStock * pItem;
			pItem = new CConsignOfStock(str,nRet,find_symbol.m_pIdx);
			m_mapStock[str] = pItem;
			pItem->AddConsign(c_pConsign);
		}
		else
		{
			//REPORT(MN,T("farm find fail %d %s\n",c_pConsign->m_nId,strToFind.c_str()),RPT_INFO);
			FormatBgTime(c_pConsign->m_acBgDate,gacTradeTime);
			
			c_pConsign->m_nStatus = 5;//无此股票号码
			gConsignQueue.Push(c_pConsign);
			gSumInfo.AddBargain(1);
		}
	}
	m_lock.EndWrite();
	return true;
}
void CDataStore::ResetQuote()
{
	m_lock.WaitWrite();
	std::map<std::string,CConsignOfStock *>::iterator iter;
	iter = m_mapStock.begin();
	while(iter!=m_mapStock.end())
	{
		iter->second->ResetQuote();
		iter++;
	}
	m_lock.EndWrite();
}

void CDataStore::StockTradeMatch()
{
	m_lock.WaitWrite();
	std::map<std::string,CConsignOfStock *>::iterator iter;
	iter = m_mapStock.begin();
	while(iter!=m_mapStock.end())
	{
		iter->second->TradeMatch();
		iter++;
	}
	m_lock.EndWrite();
}
void CDataStore::Clear()
{
	m_lock.WaitWrite();
	std::map<std::string,CConsignOfStock *>::iterator iter;
	std::map<int,PriceUnit *>::iterator iterPriceUnit;
	std::list<Consign *>::iterator iterConsign;
	iter = m_mapStock.begin();
	while(iter!=m_mapStock.end())
	{
		iterPriceUnit = (*iter).second->m_mapPriceUnitBuy.begin();
		while(iterPriceUnit != (*iter).second->m_mapPriceUnitBuy.end())
		{
			for(iterConsign = (*iterPriceUnit).second->m_listConsign.begin();iterConsign != (*iterPriceUnit).second->m_listConsign.end();iterConsign++)
			{
				//REPORT(MN,T("delete consign %d\n",(*iterConsign)->m_nId),RPT_INFO);
				delete (*iterConsign);
			}
			(*iterPriceUnit).second->m_listConsign.clear();	
			//REPORT(MN,T("delete PriceUnit %d\n",(*iterPriceUnit).first),RPT_INFO);
			delete (*iterPriceUnit).second;
			iterPriceUnit = (*iter).second->m_mapPriceUnitBuy.erase(iterPriceUnit);
		}

		iterPriceUnit = (*iter).second->m_mapPriceUnitSale.begin();
		while(iterPriceUnit != (*iter).second->m_mapPriceUnitSale.end())
		{
			for(iterConsign = (*iterPriceUnit).second->m_listConsign.begin();iterConsign != (*iterPriceUnit).second->m_listConsign.end();iterConsign++)
			{
				//REPORT(MN,T("delete consign %d\n",(*iterConsign)->m_nId),RPT_INFO);
				delete (*iterConsign);
			}
			(*iterPriceUnit).second->m_listConsign.clear();	
			//REPORT(MN,T("delete PriceUnit %d\n",(*iterPriceUnit).first),RPT_INFO);
			delete (*iterPriceUnit).second;
			iterPriceUnit = (*iter).second->m_mapPriceUnitSale.erase(iterPriceUnit);
		}
		//REPORT(MN,T("delete ConsignOfStock %s\n",(*iter).first.c_str()),RPT_INFO);
		delete (*iter).second;
		iter = m_mapStock.erase(iter);
	}
	m_lock.EndWrite();
}