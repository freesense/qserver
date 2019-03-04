
#ifndef __QUOTE_FARM_TIMER_H__
#define __QUOTE_FARM_TIMER_H__

#include "../../public/commx/commxbase.h"
#include "communication.h"
#include "farm.h"

class CFarmTimer : public ITimerBase
{
public:
	CFarmTimer(CSharedServer *pServer, CDataFarm *pFarm);
	virtual int handle_timeout(ITimerBase::ITimerCallbackBase *pTimerParam);
	char m_szHeartbeatBuffer[sizeof(CommxHead)+sizeof(unsigned short)+sizeof(time_t)];

	CDataFarm *m_pFarm;
	CSharedServer *m_pServer;

	ITimerCallbackBase m_tcb;				/** 心跳定时器 [5/22/2009 xinl] */
	unsigned int m_nSelfTestCount;			/** 自检心跳计数器 [5/22/2009 xinl] */
};

#endif
