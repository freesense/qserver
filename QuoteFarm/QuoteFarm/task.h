
#ifndef __QUOTE_FARM_TASK_H__
#define __QUOTE_FARM_TASK_H__

#include "../../public/commx/synch.h"
#include "../../public/commx/xthread.h"
#include "../../public/protocol.h"
#include "farm.h"
#include "config.h"
#include "../../public/messagequeue.h"
#include "communication.h"
#include <string>

class CFarmTask : public XThread
{
	struct _NODE
	{
		FeedHead *pHead;
		unsigned int nLength;
		unsigned int nType;			/** 0-由过夜的文件读入的行情数据，1-实时行情数据 [7/6/2009 xinl] */
	};

	struct _find_symbol
	{
		int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);
		void OnQuoteData(unsigned int nCount, Quote *pq, TickUnit *ptick);

		CFarmTask *m_pTask;
		FeedHead *pHead;
		unsigned int nTotalPacketSize;
		unsigned short nFeedType;
		unsigned int nSize;
		unsigned int nNodeType;			/** @see _NODE [7/6/2009 xinl] */
		RINDEX *pIndex;
		int m_nChangeFlag[4];
	};

	/**@brief 计算市场指数的均价

	*/
	struct _avg_of_index
	{
		int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);
		int OnElement(std::string *pSymbol, RINDEX *pIdx);
		CDataFarm *pFarm;
		std::string strMktCode;
		std::vector<std::string> vecIndexCode;
		double nSumPrevClose;
		double nSumLastPrice;
		double scale;
	};

public:
	virtual unsigned int __stdcall svc(void* lpParam);
	virtual void close();
	void AddFeed(FeedHead *pHead, unsigned int nLen, unsigned int nType = 1);
	void SetFarmPtr(CDataFarm *pFarm);
	void SetSharedServer(CSharedServer *pServer);

	LockSingle m_lock;
	CConfig::CHKMKT m_chkmkt;

private:
	CDataFarm *m_pFarm;
	CSharedServer *m_pServer;
	CMessageQueue<_NODE> m_qFeed;
	DWORD m_dwPreTick;
};

extern CFarmTask gtask;

#endif
