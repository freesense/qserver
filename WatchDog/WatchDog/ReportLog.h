//ReportLog.h
#ifndef _REPORTLOG_H_
#define _REPORTLOG_H_

#include <map>
#include <string>
#include "../../public/commx/report.h"
#include "../../public/devmonitor/auth_xml_protocol.h"
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

class CReportLog : public CReportBase
{
public:
	CReportLog();
	virtual ~CReportLog();

	virtual bool AddReport(string &tn/*��־ʱ��*/, string &mn/*ģ������*/, int tid/*�߳�ID*/,
		string &fn/*�ļ�����*/, int ln/*�к�*/, string &sr/*��־*/, int lv/*����*/ = RPT_IGNORE);

	void AddMonitorThread(unsigned long dwThreadID);

protected:
	std::string GetCurTimeString();
	void SendLog(int nLevel, char* pLog);

private:
	unsigned long       m_dwPreTick;
	CAuthProtoclXML     m_xml;
};

extern CReportLog *g_pLog;

///////////////////////////////////////////////////////////////////////////////////
#endif
