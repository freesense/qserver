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

	virtual bool AddReport(string &tn/*��־ʱ��*/, string &mn/*ģ������*/, int tid/*�߳�ID*/,
		string &fn/*�ļ�����*/, int ln/*�к�*/, string &sr/*��־*/, int lv/*����*/ = RPT_IGNORE);
protected:

private:

};
///////////////////////////////////////////////////////////////////////////////////////////////////////
#endif