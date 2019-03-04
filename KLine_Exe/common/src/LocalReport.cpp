//LocalReport.cpp
#include "stdafx.h"
#include "LocalReport.h"
#include "KLine_Exe.h"

///////////////////////////////////////////////////////////////////////////////////////////////
CLocalReport::CLocalReport()
{
}

CLocalReport::~CLocalReport()
{
}

bool CLocalReport::AddReport(string &tn/*日志时间*/, string &mn/*模块名称*/, int tid/*线程ID*/,
							string &fn/*文件名称*/, int ln/*行号*/, string &sr/*日志*/, int lv/*级别*/)
{	
	//去掉日志后面的换行
	int nPos = (int)sr.rfind("\n");
	if (nPos >= 0)
		sr = sr.substr(0, nPos);

	tagLog* pLog = new tagLog;
	memset(pLog, 0, sizeof(tagLog));

	strcpy_s(pLog->szDevName, 50, "本设备");
	strcpy_s(pLog->szTime, 50, tn.c_str());
	strcpy_s(pLog->szModule, 50, mn.c_str());
	pLog->dwThreadID = tid;
	sprintf_s(pLog->szLogPos, 50, "%s(%d)", fn.c_str(), ln);
	strcpy_s(pLog->szMsg, 1024, sr.c_str());
	pLog->bLocal = TRUE;


	if ((lv & 0xFF) == RPT_DEBUG)
		strcpy_s(pLog->szLogType, 50, "RPT_DEBUG");
	else if ((lv & 0xFF) == RPT_INFO)
		strcpy_s(pLog->szLogType, 50, "RPT_INFO");
	else if ((lv & 0xFF) == RPT_WARNING)
		strcpy_s(pLog->szLogType, 50, "RPT_WARNING");
	else if ((lv & 0xFF) == RPT_ERROR)
		strcpy_s(pLog->szLogType, 50, "RPT_ERROR");
	else if ((lv & 0xFF) == RPT_CRITICAL)
		strcpy_s(pLog->szLogType, 50, "RPT_CRITICAL");
	else if ((lv & 0xFF) == RPT_HEARTBEAT)
		strcpy_s(pLog->szLogType, 50, "RPT_HEARTBEAT");
	else if ((lv & 0xFF) == RPT_ADDI_INFO)
		strcpy_s(pLog->szLogType, 50, "RPT_ADDI_INFO");
	else if ((lv & 0xFF) == RPT_VERSION)
		strcpy_s(pLog->szLogType, 50, "RPT_VERSION");
	else
		strcpy_s(pLog->szLogType, 50, "LT_UNKNOWN");

	theApp.AddLog(lv, pLog);
	return true;
}