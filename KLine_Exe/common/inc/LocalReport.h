//LocalReport.h
#ifndef _LOCALREPORT_H_
#define _LOCALREPORT_H_
#include "../../public/commx/report.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
class CLocalReport : public CReportBase
{
public:
	CLocalReport();
	virtual ~CLocalReport();

	virtual bool AddReport(string &tn/*日志时间*/, string &mn/*模块名称*/, int tid/*线程ID*/,
		string &fn/*文件名称*/, int ln/*行号*/, string &sr/*日志*/, int lv/*级别*/ = RPT_IGNORE);
protected:

private:

};
///////////////////////////////////////////////////////////////////////////////////////////////////////
#endif