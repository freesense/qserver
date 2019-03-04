//ReportLog.cpp
#include "stdafx.h"
#include "ReportLog.h"
#include "../../public/protocol.h"
#include "../../public/devmonitor/proxy_auth_server_base.h"

CThreadMap  g_mapThread;
extern IServerAuthProxy *g_pAuthProxy;
CReportLog *g_pLog = NULL;

////////////////////////////////////////////////////////////////////////////////
CReportLog::CReportLog()
{
	m_dwPreTick = GetTickCount();
}

CReportLog::~CReportLog()
{
	g_mapThread.clear();
}

bool CReportLog::AddReport(string &tn/*日志时间*/, string &mn/*模块名称*/, int tid/*线程ID*/,
		string &fn/*文件名称*/, int ln/*行号*/, string &sr/*日志*/, int lv/*级别*/)
{
	char szLog[1024] = {0};

	CThreadMap::iterator find = g_mapThread.find(tid);
	if (find != g_mapThread.end())
	{
		find->second = ::GetTickCount();
	}
	else //根据模块名添加需要监控的线程ID
	{
		if (lv == RPT_HEARTBEAT)
		{
			std::string strTime = GetCurTimeString();
			sprintf_s(szLog, 1024, "%s[%s]%d@%d@%s(%d):%s", strTime.c_str(), MODULE_NAME, tid, RPT_INFO,
				"ReportLog", __LINE__, (T("Add Monitor Thread:0x%x\n", tid)).c_str());
			SendLog(RPT_INFO, szLog);

			AddMonitorThread(tid);
		}
	}

	//if (lv != RPT_IGNORE && lv != RPT_HEARTBEAT)
	{
		int nPos = (int)sr.rfind("\n");
		if (nPos >= 0)
			sr = sr.substr(0, nPos);

		sprintf_s(szLog, 1024, "%s[%s]%d@%d@%s(%d):%s", tn.c_str(), mn.c_str(), tid, lv,
			fn.c_str(), ln, sr.c_str());

		SendLog(lv, szLog);
	}

	//检测是否有超时的线程
	unsigned long dwCurTick = ::GetTickCount();
	if (dwCurTick - m_dwPreTick >= (unsigned long)TIME_OUT*1000)
	{
		m_dwPreTick = dwCurTick;

		CThreadMap::iterator pos = g_mapThread.begin();
		bool bTimeOut = false;
		while (pos != g_mapThread.end())
		{
			//两倍超时时间内没有日志，则超时， 不发送心跳日志到设备服务器
			if (dwCurTick - pos->second > (unsigned long)TIME_OUT*1000*2) 
			{
				bTimeOut = true;

				std::string strTime = GetCurTimeString();
				sprintf_s(szLog, 1024, "%s[%s]%d@%d@%s(%d):%s", strTime.c_str(), MODULE_NAME, tid, RPT_WARNING,
					"ReportLog", __LINE__, (T("Thread:0x%x Time out\n", pos->first)).c_str());
				
				SendLog(RPT_WARNING, szLog);
			}
			pos++;
		}
		if (bTimeOut)
			return true;
		
		std::string strTime = GetCurTimeString();
		sprintf_s(szLog, 1024, "%s[%s]%d@%d@%s(%d):%s", strTime.c_str(), MODULE_NAME, tid, RPT_HEARTBEAT,
			"ReportLog", __LINE__, "HeartBeat\n");
		SendLog(RPT_HEARTBEAT, szLog);
	}
	return true;
}

void CReportLog::AddMonitorThread(unsigned long dwThreadID)
{
	unsigned long dwTickCount = ::GetTickCount();
	g_mapThread.insert(std::make_pair(dwThreadID, dwTickCount));
}

std::string CReportLog::GetCurTimeString()
{
	time_t now;
	time(&now);
	struct tm *lt = localtime(&now);
	struct timeb timebuffer;
	ftime(&timebuffer);
	char szTime[24] = {0};	//format: 2000-11-11 09:09:09.001
	sprintf_s(szTime, 24, "%4d%02d%02d %02d:%02d:%02d.%03d", lt->tm_year+1900, 
		lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, timebuffer.millitm);
	
	string tn = szTime;
	return tn;
}

void CReportLog::SendLog(int nLevel, char* pLog)
{
	string s = m_xml.Log(MODULE_NAME, nLevel, pLog, "");
	int length= (int)s.length()+sizeof(CommxHead);
	char *szBuffer = new char[length];
	CommxHead *pHead = (CommxHead*)szBuffer;
	memset(pHead, 0x00, length);
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = (int)s.length();
	memcpy(szBuffer + sizeof(CommxHead), s.c_str(),pHead->Length);
	g_pAuthProxy->OnTransmit(szBuffer, sizeof(CommxHead) + pHead->Length);
	delete[]szBuffer;
}
