//ReportLog.h
#ifndef _REPORTLOG_H_
#define _REPORTLOG_H_

#include <map>
#include <string>
#include "../../public/commx/report.h"
#include "../../public/devmonitor/auth_xml_protocol.h"
///////////////////////////////////////////////////////////////////////////////////
#ifndef RPT_HEARTBEAT
#define	RPT_HEARTBEAT  0xAA	//心跳日志，用来监控线程
#endif

#ifndef RPT_ADDI_INFO      //附加信息，可以报告设备的状态等信息
#define	RPT_ADDI_INFO  0x90
#endif

#ifndef RPT_VERSION		   //模块的版本信息
#define RPT_VERSION    0xAB
#endif

typedef	std::map<unsigned long, unsigned long> CThreadMap;

class CReportLog : public CReportBase
{
public:
	CReportLog();
	virtual ~CReportLog();

	virtual bool AddReport(string &tn/*日志时间*/, string &mn/*模块名称*/, int tid/*线程ID*/,
		string &fn/*文件名称*/, int ln/*行号*/, string &sr/*日志*/, int lv/*级别*/ = RPT_IGNORE);

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
