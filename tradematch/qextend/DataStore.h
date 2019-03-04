#pragma once
#include<map>
#include<list>
//#include <hash_map> 
#include   <string> 

#include "../../public/data_struct.h"
#include "../../public/commx/synch.h"

#define CONSIGN_ADD			0
#define CONSIGN_CANCEL		1

#define SYMBOL_LEN			6

//统计信息
struct SumInfo
{
	int		m_nMaxConsign;//委托数量
	int		m_nMaxBargain;//成交数量
	int		m_nConsign;//委托数量
	int		m_nBargain;//成交数量
	LockProcess m_lock;	
	void	AddConsign(int c_nAdd)
	{
		m_lock.WaitWrite();
		m_nConsign += c_nAdd;
		m_lock.EndWrite();
	}
	void	AddBargain(int c_nAdd)
	{
		m_lock.WaitWrite();
		m_nBargain += c_nAdd;
		m_lock.EndWrite();
	}
	void	SetNew()
	{
		m_lock.WaitWrite();
		if (m_nConsign > m_nMaxConsign)
			m_nMaxConsign = m_nConsign;
		if (m_nBargain > m_nMaxBargain)
			m_nMaxBargain = m_nBargain;
		m_nBargain = 0;
		m_nConsign = 0;
		m_lock.EndWrite();
	}
};

//委托结构
struct Consign
{
	int		m_nId;				//委托单号
	char	m_acDate[30];		//委托时间
	int		m_nPrice;			//委托价格
	int		m_nAmount;			//委托数量
	int		m_nFlag;			//是买入，0买入，1卖出
	int		m_nType;			//委托类别,0委托，1撤单
	int		m_nCancelId;		//待撤单号
	int		m_nUserId;			//用户代码
	char	m_acSymbol[SYMBOL_LEN+1];		//证券代码

	char	m_acBgDate[30];		//bargainDate
	int		m_nStatus;			//当前状态
};
//报价单元
struct PriceUnit
{
	int		m_nPrice;
	std::list<Consign *> m_listConsign;
};

class CConsignOfStock
{
public:
	CConsignOfStock(std::string c_strSymbol,int c_nMarket,RINDEX * c_pIdx);
	~CConsignOfStock(void);

public:
	void ResetQuote();
	//增加一个交易,0成功增加,-1价格超过涨停价格,-2价格低过跌停价格
	int AddConsignOld(Consign * c_pConsign);
	int AddConsign(Consign * c_pConsign);
	//处理一个撤单,0撤单成功,-1失败
	int CancelConsign(Consign * c_pConsign);
	int CheckPrice(unsigned int c_unPrice);//0正常,-1低于跌停价,1高于涨停价
	//新的成交处理
	void TradeMatch();//根据买一和卖一进行撮合
	void OnNewTick(Quote *quote,TickUnit * tickunit);
	void OnDelete();//删除
	int cast(std::string  * c_pKey,RINDEX * c_pValue,bool c_bFlag);
public:
	std::string m_strSymbol;	//证券代码
	std::string m_strName;		//证券名称
	int			m_nMarket;		//1上海,2深圳
	RINDEX	  * m_pIdx;
	Quote	  * m_pQuote;		//
	bool		m_bCacl;		//计算涨停、跌停价格
	unsigned int			m_unZtjg;		//涨停价格
	unsigned int			m_unDtjg;		//跌停价格
	unsigned int			m_unPrice;		//现价
	unsigned int m_unZrsp;

	std::map<int,PriceUnit *> m_mapPriceUnitBuy;//委买队列
	std::map<int,PriceUnit *> m_mapPriceUnitSale;//委卖队列

	LockRW m_rwLock;
};

class CDataStore
{
public:
	CDataStore(void);
	~CDataStore(void);

public:
	void OnBegin();
	void AddSymbol(const char *symbol, int *pFlag);
	void UpdateSymbol(const char * symbol, int *pFlag, Quote *quote,TickUnit * tickunit);
	void DelSymbol(const char * symbol);
	void OnEnd();

	bool AddConsign(Consign * c_pConsign);//增加一个委托
	void ResetQuote();
	void StockTradeMatch();
	void Clear();
public:
	LockProcess m_lock;	
	std::map<std::string ,CConsignOfStock *> m_mapStock;
};