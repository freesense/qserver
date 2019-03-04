//ReportServer.h
#ifndef _REPORTSERVER_H_
#define _REPORTSERVER_H_

#include "../commx/report.h"
#include <map>
#include <string>

///////////////////////////////////////////////////////////////////////////////////
#ifndef RPT_HEARTBEAT
#define	RPT_HEARTBEAT  0xAA	//������־����������߳�
#endif

#ifndef RPT_ADDI_INFO      //������Ϣ�����Ա����豸��״̬����Ϣ
#define	RPT_ADDI_INFO  0x90
#endif

#ifndef RPT_VERSION		   //ģ��İ汾��Ϣ
#define RPT_VERSION    0xAB
#endif

typedef	std::map<unsigned long, unsigned long> CThreadMap;

class CReportServer : public CReportBase
{
public:
	CReportServer();
	virtual ~CReportServer();

	virtual bool AddReport(string &tn/*��־ʱ��*/, string &mn/*ģ������*/, int tid/*�߳�ID*/,
		string &fn/*�ļ�����*/, int ln/*�к�*/, string &sr/*��־*/, int lv/*����*/ = RPT_IGNORE);

	void AddMonitorThread(unsigned long dwThreadID);

protected:
	std::string GetCurTimeString();

private:
	unsigned long       m_dwPreTick;
};

///////////////////////////////////////////////////////////////////////////////////
#endif