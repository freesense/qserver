
#include "stdafx.h"
#include "farmtimer.h"
#include "../../public/devmonitor/ReportServer.h"
#include "../../public/commx/mery.h"

CFarmTimer::CFarmTimer(CSharedServer *pServer, CDataFarm *pFarm)
{
	m_pServer = pServer;
	m_pFarm = pFarm;
	m_nSelfTestCount = 0;
	memset(m_szHeartbeatBuffer, 0x00, sizeof(m_szHeartbeatBuffer));
	CommxHead *pHead = (CommxHead*)m_szHeartbeatBuffer;
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = sizeof(unsigned short)+sizeof(time_t);
}

int CFarmTimer::handle_timeout(ITimerBase::ITimerCallbackBase *pTimerParam)
{
	DumpMemPool();

	time_t *pNow = (time_t*)(m_szHeartbeatBuffer + sizeof(CommxHead) + sizeof(unsigned short));
	time(pNow);

	/** �����е�Feed���������� [5/22/2009 xinl] */
	WGUARD(LockSingle, g_cfg.m_feedLock, grd);
	for (unsigned int i = 0; i != g_cfg.m_vFeed.size(); i++)
	{
		if (g_cfg.m_vFeed[i].bConnected)
		{
			CFeedSocket *pHandle = (CFeedSocket*)g_cfg.m_vFeed[i].pHandler;
			int iFeedHeart = pHandle->Write(&m_szHeartbeatBuffer, sizeof(m_szHeartbeatBuffer));
			CHECK_RUN(-1 ==iFeedHeart, MN, T("��QuoteFeed������ֹͣ���ؽ�����\n"), RPT_WARNING, pHandle->Close());
		}
	}

	/** ���Farm״̬��Ϣ�����ϱ�������� [5/22/2009 xinl] */
	float fSpeed = (float)(g_nFeedRecvLength.value()) / 1024;
	g_nTotalRecv += g_nFeedRecvLength;
	g_nFeedRecvLength = 0;
	REPORT(MN, T("���ݴ�С:[%.3f KB] ���ݰ�����:[%d] �ɹ������ֽ�:[%.3f M] Symbol:[%d]\n",
		fSpeed, g_nFeedPacketCount.value(), ((float)g_nTotalRecv.value())/1024/1024, m_pFarm->GetHashSize()),
		RPT_ADDI_INFO|RPT_IGNORE);

	/** �Լ� [5/22/2009 xinl] */
	if (++m_nSelfTestCount >= g_cfg.m_nSelfTestCount)
	{
		m_nSelfTestCount = 0;
		m_pFarm->SelfCheck("sz|sh|hk");
	}

	return 0;
}
