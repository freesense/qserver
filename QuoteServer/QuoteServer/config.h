
#ifndef __QUOTE_FARM_CONFIG_H__
#define __QUOTE_FARM_CONFIG_H__

#include "stdafx.h"
#include "..\\..\\public\\xml\\xmlparser.h"
#include <vector>
#include <string>
#include <map>
#include <deque>
#include "PlugManager.h"

class CConfig : public TiXmlParser
{
public:
	struct _feed
	{
		std::string strName;
		bool bStatus;
		std::string strAddress;
	};

	struct _market
	{
		struct quote_time_slice
		{
			unsigned short hhmm_b;
			unsigned short hhmm_e;
			unsigned short blank_min;			// 该时间段相对于开盘时间要减去的中间休市的分钟数
		};

		enum MKT_STATUS {CLOSED = 0, OPENED};
		unsigned short nOpenTime;
		unsigned short nCloseTime;
		unsigned short nMinPerDay;				// 每交易日的开市分钟数，计算量比需要这个数据
		std::deque<quote_time_slice> dqTime;	// 开市时间段
		short nTz;
		std::string strMarketCode;
		MKT_STATUS status;
	};

	typedef std::map<std::string, unsigned int> CHKMKT;

	CConfig();
	~CConfig();

	virtual void OnNode(TiXmlElement *pElm, NODEWAY way);
	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	void CheckTaskOnTime(unsigned short nTime, CHKMKT &chkmkt);
	int GetQuoteMin(const char *lpMarket, unsigned short nTime);
	int GetMinPerDay(const char *lpMarket);
	int GetTz(const char *lpMarket);

	unsigned int m_nMarketCount;
	unsigned int m_nSymbolCount;
	unsigned int m_nTickCount;
	unsigned int m_nMinCount;
	unsigned short m_nTaskCount;
	std::vector<_feed> m_vFeed;
	std::vector<_market> m_vMarket;
	std::map<unsigned int, unsigned int> m_mpMemPool;

	std::string m_strHistoryPath;
	std::string m_strHistoryWeightPath;//历史还权K线
	std::string m_strDetailPath;

	//QuoteServer
public:
	bool    m_bRunWithQuote;
	int		m_nLineTest;
	int		m_nAcceptOut;
	int		m_nTimeOut;

	std::string m_strQuoteServerAddr;
	unsigned int m_unQuoteServerPort;
	std::string m_strQuoteFarmAddr;
	unsigned int m_unQuoteFarmPort;

	//监控
	int	m_nDevTimeout;
	std::string m_strDevServIP;
	int m_nDevServPort;
	std::string m_strAppName;
	std::string m_strSviceAddr;
	//调试
	bool m_bDebug;
	bool m_bDumpData;
private:
	inline unsigned int ConvertHHMM2Min(unsigned short nTime);
	inline unsigned int GetMinCount(unsigned short nBegin, unsigned short nEnd);

	unsigned int m_tmp;
	_feed m_tmpFeed;
	_market m_tmpMarket;
	SPlugItem * m_pPlugItem;
};

extern CConfig g_cfg;

#endif
