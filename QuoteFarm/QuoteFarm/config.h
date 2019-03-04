
#ifndef __QUOTE_FARM_CONFIG_H__
#define __QUOTE_FARM_CONFIG_H__

#include "stdafx.h"
#include "../../public/data_struct.h"
#include "../../public/xml/xmlparser.h"
#include <vector>
#include <string>
#include <map>
#include <deque>

class CConfig : public TiXmlParser
{
public:
	enum MKT_SECOND {CLOSED = 0,				/** 市场收盘 [6/12/2009 xinl] */
					 OPENED,					/** 市场开盘 [6/12/2009 xinl] */
					 CHECK,						/** 市场自检 [6/12/2009 xinl] */
					 UNKNOWN,					/** 市场状态未知 [6/12/2009 xinl] */
	};

	struct _pattern
	{
		std::string PatternCode;		/*分类代码*/
		std::string RegexPattern;		/*分类的正则表达式*/
	};

	struct _feed
	{
		std::string strName;
		std::string strAddress;
		void *pHandler;							// 连接对象指针
		int bStatus;							// 数据源状态，“正常”or“暂停”or“备用”
		bool bConnected;						// 连接状态
	};

	struct _market
	{
		struct quote_time_slice
		{
			unsigned short hhmm_b;
			unsigned short hhmm_e;
			unsigned short blank_min;			// 该时间段相对于开盘时间要减去的中间休市的分钟数
		};

		unsigned short nOpenTime;
		unsigned short nCloseTime;
		unsigned short nMinPerDay;				// 每交易日的开市分钟数，计算量比需要这个数据
		std::deque<quote_time_slice> dqTime;	// 开市时间段
		short nTz;
		std::string strMarketCode;

		MKT_SECOND status;
		unsigned int nOpenDate;					// 开盘日期
		unsigned int nCloseDate;				// 收盘日期

		char szWeekRest[8];						// 每周的休息日

		/** 为了计算指数均价增加下面两个字段 [7/31/2009 xinl] */
		unsigned int nSumPrevClose;				/** 昨收价之和，昨收为0以现价代替 [7/31/2009 xinl] */
		unsigned int nSumLastPrice;				/** 现价之和，现价为0以昨收价代替 [7/31/2009 xinl] */
	};

	struct _addin
	{
		enum TIMER_STATUS {WAIT = 0, TRIGGERED};
		bool bQuoteNotify;						// 是否需要行情变化通知
		std::string strPath;					// 插件路径
		unsigned int pfnTimer;					// 插件中的定时器回调函数指针
		std::map<short, TIMER_STATUS> mpTimer;	// 定时器
		bool bStatus;							// 插件状态，“正常”or“暂停”
	};

	struct _weight
	{
		std::string script;						// 还权脚本及方法
		std::string params;						// 还权参数
		unsigned short nWeightTime;				// 更新还权数据的时间
		unsigned int day;						// 还权更新日期
		bool bEnable;							// 是否允许更新
	};

	/** 定义各种行情事件的触发结构 [6/29/2009 xinl] */
	typedef std::multimap<std::string, std::string> CHKMKT;

	CConfig();
	~CConfig();

	virtual void OnNode(TiXmlElement *pElm, NODEWAY way);
	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	void CheckTaskOnTime(time_t nTime, CHKMKT &chkmkt);
	int GetQuoteMin(const char *lpMarket, unsigned short nTime);
	int GetMinPerDay(const char *lpMarket);
	int GetTz(const char *lpMarket);
	int ConvergenceTime(const char *lpMarket, unsigned short nTime);
	MKT_SECOND GetMarketStatus(const char *lpMarket);

	inline unsigned int GetMarketDay(time_t now, const char *lpMarket);
	inline unsigned int GetMarketTime(time_t now, const char *lpMarket);
	inline unsigned short GetMarketWeek(time_t now, const char *lpMarket);

	const char* GetPattern(const char *symbol);

	/**@brief 更新内存映射文件中的市场状态结构
	
	从xml文件中更新到内存映射文件中，使两处的状态一致
	@param pms 内存映射的市场状态
	 */
	void UpdateMarketStatus(MarketStatus *pms);

	void ActivateFeed(const char *lpName, void *pFeedHandle);
	void DisableFeed(void *pFeedHandle);

	std::string m_strHolidayPath;
	unsigned int m_nSymbolCount;
	unsigned int m_nTickCount;
	unsigned int m_nMinCount;
	unsigned short m_nTaskCount;
	std::string m_strCheckAddress;
	LockSingle m_feedLock;
	std::vector<_feed> m_vFeed;
	std::vector<_market> m_vMarket;
	std::vector<_addin> m_vAddin;
	std::vector<_weight> m_vWeight;
	std::map<unsigned int, unsigned int> m_mpMemPool;

	unsigned int m_nHeartbeat;						/** 心跳时间，秒 [5/22/2009 xinl] */
	unsigned int m_nSelfTestCount;					/** 自检心跳次数 [5/22/2009 xinl] */

	std::string m_strXmlFile;						// xml配置文件名

	//监控模块使用
	unsigned short m_nDevServPort;
	std::string m_strDevServIP, m_strAppName;

	std::string m_strHisPath;						// 历史文件保存的绝对路径
	std::vector<_pattern> m_vPattern;				// 证券代码品种模式

private:
	inline unsigned int ConvertHHMM2Min(unsigned short nTime);
	inline unsigned int GetMinCount(unsigned short nBegin, unsigned short nEnd);

	void ChangeOpenCloseDate(const char *mktcode, unsigned int date, bool isOpen);
	void ChangeDetailDate(unsigned int date);
	void ChangeWeightDate(unsigned int date);

	std::map<std::string, std::string> m_mapHoliday;
	unsigned int m_tmp, m_mode;
	_feed m_tmpFeed;
	_market m_tmpMarket;
	_addin m_tmpAddin;
	_pattern m_tmpPattern;
	_weight m_tmpWeight;

	unsigned int m_nHolidayDay;						// 已更新休市文件的日期
	int m_iDetailDays;								// 历史明细保存的天数
	unsigned short m_nDetailTime;					// 保存历史明细的时间
	unsigned int m_nDetailDate;						// 最近保存明细的日期
};

extern CConfig g_cfg;

#endif
