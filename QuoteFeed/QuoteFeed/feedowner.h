
#ifndef __QUOTE_FEED_OWNER_H__
#define __QUOTE_FEED_OWNER_H__

#include "../../public/commx/synch.h"
#include "../../public/feedinterface.h"
#include "feedservice.h"
#include "Log.h"
#include "config.h"
#include "../../public/devmonitor/ReportServer.h"

class CFeedOwner : public IFeedOwner
{
public:
	CFeedOwner(CFeedServer *pServer)
		: m_pServer(pServer)
	{
		m_nPackCount = 0;
		m_nSendSize  = 0;
		m_nPreTick = 0;
		m_nMinSize = 0;

		::InitializeCriticalSection(&g_csCount);
	}

	virtual ~CFeedOwner()
	{
		::DeleteCriticalSection(&g_csCount);
	}

	virtual void Quote(const char *lpData, unsigned int nLength, void *lpParam)
	{
		if (g_cfg.m_bUserDBF)
			m_log.ParseDataToLog(lpData, nLength + sizeof(long));  //4���ֽڵı���λ
		else
			Sleep(5000);  //��QuoteFarm���Ӻ��ٷ������ݣ��������ݶ�ʧ

		::EnterCriticalSection(&g_csCount);
		m_nSendSize += (double)(nLength) / 1024 / 1024;
		m_nPackCount++;
		m_nMinSize += (nLength);
		::LeaveCriticalSection(&g_csCount);

		m_pServer->SendQuote((CFeedService*)lpParam, (void*)lpData, nLength);
	}

    virtual void Report(const char *mn, int tid, const char *fn, int ln, const char *sr, int lv)
	{
		::EnterCriticalSection(&g_csCount);
		unsigned long nCurTick = ::GetTickCount();
		lv &= ~RPT_IGNORE;
// 		time_t now;
// 		time(&now);
// 		struct tm *lt = localtime(&now);
// 
// 		if (m_nPreTick != 0)
// 		{
// 			REPORT(MN, T("%m%d %H:%M:%S\n", lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec), RPT_ADDI_INFO|RPT_IGNORE);
// 			if (nCurTick-m_nPreTick >= 60000)
// 				m_nMinSize = 0;
// 		}
// 		m_nPreTick = nCurTick;

		if (nCurTick - m_nPreTick >= 1000*60)
		{
			if (m_nPreTick != 0)
			{
				REPORT(MN, T("���ݴ�С:[%.3f KB] ���ݰ�����:[%d] �ɹ������ֽ�:[%.3f M] �������ֽ�:[%.3f M]  \n",
					(double)m_nMinSize/1024, m_nPackCount, g_nSendSuccess, m_nSendSize), RPT_ADDI_INFO/*|RPT_IGNORE*/);
				m_nMinSize = 0;
			}
			m_nPreTick = nCurTick;
		}
		::LeaveCriticalSection(&g_csCount);
		CReportBox::Instance()->AddReport(mn, tid, fn, ln, string(sr), lv);
	}

private:
	LockSingle m_lockSerial;
	CFeedServer *m_pServer;
	CLog m_log;

	//Test info
	UINT      m_nPackCount;	  //�������ݰ��Ĵ���
	double    m_nSendSize;	  //�����ֽ���
	unsigned long m_nMinSize; //ÿ���ӷ��͵��ֽ���

	unsigned long m_nPreTick;
};

#endif
