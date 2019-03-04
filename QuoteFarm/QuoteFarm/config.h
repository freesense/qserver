
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
	enum MKT_SECOND {CLOSED = 0,				/** �г����� [6/12/2009 xinl] */
					 OPENED,					/** �г����� [6/12/2009 xinl] */
					 CHECK,						/** �г��Լ� [6/12/2009 xinl] */
					 UNKNOWN,					/** �г�״̬δ֪ [6/12/2009 xinl] */
	};

	struct _pattern
	{
		std::string PatternCode;		/*�������*/
		std::string RegexPattern;		/*�����������ʽ*/
	};

	struct _feed
	{
		std::string strName;
		std::string strAddress;
		void *pHandler;							// ���Ӷ���ָ��
		int bStatus;							// ����Դ״̬����������or����ͣ��or�����á�
		bool bConnected;						// ����״̬
	};

	struct _market
	{
		struct quote_time_slice
		{
			unsigned short hhmm_b;
			unsigned short hhmm_e;
			unsigned short blank_min;			// ��ʱ�������ڿ���ʱ��Ҫ��ȥ���м����еķ�����
		};

		unsigned short nOpenTime;
		unsigned short nCloseTime;
		unsigned short nMinPerDay;				// ÿ�����յĿ��з�����������������Ҫ�������
		std::deque<quote_time_slice> dqTime;	// ����ʱ���
		short nTz;
		std::string strMarketCode;

		MKT_SECOND status;
		unsigned int nOpenDate;					// ��������
		unsigned int nCloseDate;				// ��������

		char szWeekRest[8];						// ÿ�ܵ���Ϣ��

		/** Ϊ�˼���ָ�������������������ֶ� [7/31/2009 xinl] */
		unsigned int nSumPrevClose;				/** ���ռ�֮�ͣ�����Ϊ0���ּ۴��� [7/31/2009 xinl] */
		unsigned int nSumLastPrice;				/** �ּ�֮�ͣ��ּ�Ϊ0�����ռ۴��� [7/31/2009 xinl] */
	};

	struct _addin
	{
		enum TIMER_STATUS {WAIT = 0, TRIGGERED};
		bool bQuoteNotify;						// �Ƿ���Ҫ����仯֪ͨ
		std::string strPath;					// ���·��
		unsigned int pfnTimer;					// ����еĶ�ʱ���ص�����ָ��
		std::map<short, TIMER_STATUS> mpTimer;	// ��ʱ��
		bool bStatus;							// ���״̬����������or����ͣ��
	};

	struct _weight
	{
		std::string script;						// ��Ȩ�ű�������
		std::string params;						// ��Ȩ����
		unsigned short nWeightTime;				// ���»�Ȩ���ݵ�ʱ��
		unsigned int day;						// ��Ȩ��������
		bool bEnable;							// �Ƿ��������
	};

	/** ������������¼��Ĵ����ṹ [6/29/2009 xinl] */
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

	/**@brief �����ڴ�ӳ���ļ��е��г�״̬�ṹ
	
	��xml�ļ��и��µ��ڴ�ӳ���ļ��У�ʹ������״̬һ��
	@param pms �ڴ�ӳ����г�״̬
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

	unsigned int m_nHeartbeat;						/** ����ʱ�䣬�� [5/22/2009 xinl] */
	unsigned int m_nSelfTestCount;					/** �Լ��������� [5/22/2009 xinl] */

	std::string m_strXmlFile;						// xml�����ļ���

	//���ģ��ʹ��
	unsigned short m_nDevServPort;
	std::string m_strDevServIP, m_strAppName;

	std::string m_strHisPath;						// ��ʷ�ļ�����ľ���·��
	std::vector<_pattern> m_vPattern;				// ֤ȯ����Ʒ��ģʽ

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

	unsigned int m_nHolidayDay;						// �Ѹ��������ļ�������
	int m_iDetailDays;								// ��ʷ��ϸ���������
	unsigned short m_nDetailTime;					// ������ʷ��ϸ��ʱ��
	unsigned int m_nDetailDate;						// ���������ϸ������
};

extern CConfig g_cfg;

#endif
