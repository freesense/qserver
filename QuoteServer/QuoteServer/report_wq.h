
#ifndef __REPORT_BOX_H__
#define __REPORT_BOX_H__

#pragma warning(disable:4786)
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/timeb.h>
#include <time.h>
#include <ctype.h>
#ifdef _POSIX_C_SOURCE
#include <pthread.h>
#endif

using std::string;
using std::vector;
using std::ofstream;
using std::cout;
using std::endl;
using std::ios;

#define RPT_DEBUG		0
#define RPT_INFO		1
#define RPT_LOG			3
#define RPT_WARNING		4
#define RPT_ERROR		6
#define RPT_CRITICAL	8

inline string T(const char *fmt, ...)
{
	char szMsg[1024];
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	vsprintf(szMsg, fmt, arg_ptr);
	va_end(arg_ptr);
	return szMsg;
}

//直接写cout
template <class L>
class CReportBase : public L
{
public:
	virtual ~CReportBase<L>() {};
	virtual bool AddReport(string &tn/*日志时间*/, string &mn/*模块名称*/, int tid/*线程ID*/,
		string &fn/*文件名称*/, int ln/*行号*/, string &sr/*日志*/, int lv/*级别*/ = -1)
	{
		if (lv != RPT_LOG)
		{
			cout << tn << "[" << mn << "]" << tid << "@" << lv << "@" << fn << "(" << ln << "): " << sr;
		}
		return true;	//继续向下传递报告
	}
};

//写入日志文件，每天一个
template <class L>
class CReportFile : public CReportBase<L>
{
public:
	CReportFile(const char *lpszFileName)
		: m_FileNamePrefix(lpszFileName)
	{
	}

	virtual bool AddReport(string &tn/*日志时间*/, string &mn/*模块名称*/, int tid/*线程ID*/,
		string &fn/*文件名称*/, int ln/*行号*/, string &sr/*日志*/, int lv/*级别*/ = -1)
	{
		if (lv >= RPT_LOG)
		{
			NewFileName(tn);
			m_of << tn.substr(tn.find(" ") + 1) << "[" << mn << "]" << tid << "@" << lv << "@" << fn << "(" << ln << "): " << sr;
		}
		return true;	//继续向下传递报告
	}

private:
	void NewFileName(string &tn)
	{
		string sdate = tn.substr(0, tn.find(" "));
		if (sdate != m_FileNameDate)
		{
			if (m_of.is_open())
				m_of.close();
			char *pszFileName = new char[m_FileNamePrefix.length() + 13];
			sprintf(pszFileName, "%s%s.log", m_FileNamePrefix.c_str(), sdate.c_str());
			m_of.open(pszFileName, ios::out|ios::app);
			delete []pszFileName;
			m_FileNameDate = sdate;
		}
	}
	ofstream m_of;
	string m_FileNamePrefix;
	string m_FileNameDate;
};

template <class L>
class CReportBox : public L
{
public:
	static CReportBox<L>* Instance()
	{
		if (m_pInstance == NULL)
			m_pInstance = new CReportBox<L>;
		return m_pInstance;
	}

	static void Instance(CReportBox<L> *prb)
	{
		Close();
		m_bFlag = false;
		m_pInstance = prb;
	}

	static void Close()
	{
		if (m_pInstance && m_bFlag)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

	//输出16进制数据
	void HexDump(const char *szmn, int tid, const char *szFile, int ln, const void *lpData, unsigned int nLength)
	{
		time_t now;
		time(&now);
		struct tm *lt = localtime(&now);
		struct timeb timebuffer;
		ftime(&timebuffer);
		char szTime[24];	//format: 2000-11-11 09:09:09.001
		sprintf(szTime, "%4d%02d%02d %02d:%02d:%02d.%03d",
			lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, timebuffer.millitm);
		szTime[23] = 0x00;
		string tn = szTime;

		string fn = szFile;
		fn = fn.substr(fn.rfind('\\') + 1);

		string mn = szmn;
		char szAlpha[4], szSeeable[16], szTitle[21];
		if (nLength > 1024)
			nLength = 1024;
		sprintf(szTitle, "Dump %d bytes:\n", nLength);
		string sr = szTitle;

		for (int m = 0; m < nLength; m++)
		{
			short idx = m%16;
			unsigned char c = ((char*)lpData)[m];
			sprintf(szAlpha, "%02X ", c);
			sr.append(szAlpha, 3);

			if (isprint(c))
				szSeeable[idx] = c;
			else
				szSeeable[idx] = '.';
			if (idx == 15)
			{
				sr += "-> ";
				sr.append(szSeeable , idx+1);
				sr += "\n";
			}
			else if (m == nLength - 1)
			{
				sr.append((16 - idx - 1) * 3, ' ');
				sr += "-> ";
				sr.append(szSeeable , idx+1);
				sr += "\n";
			}
		}

		for (int i = 0; i < m_vReportUnit.size(); i++)
		{
			m_vReportUnit[i]->lock();
			bool bFlag = m_vReportUnit[i]->AddReport(tn, mn, tid, fn, ln, sr, RPT_INFO);
			m_vReportUnit[i]->unlock();
		}
	}

	void AddReport(const char *szmn/*模块名称*/, int tid/*线程ID*/, const char *szFile/*文件名称*/,
		int ln/*行号*/, string sr/*日志*/, int lv/*级别*/ = -1)
	{
		//获得当前时间
		time_t now;
		time(&now);
		struct tm *lt = localtime(&now);
		struct timeb timebuffer;
		ftime(&timebuffer);
		char szTime[24];	//format: 2000-11-11 09:09:09.001
		sprintf(szTime, "%4d%02d%02d %02d:%02d:%02d.%03d",
			lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, timebuffer.millitm);
		szTime[23] = 0x00;
		string tn = szTime;

		//获得文件名，不包括路径
		string fn = szFile;
		fn = fn.substr(fn.rfind('\\') + 1);

		string mn = szmn;

		for (int i = 0; i < m_vReportUnit.size(); i++)
		{
			m_vReportUnit[i]->lock();
			bool bFlag = m_vReportUnit[i]->AddReport(tn, mn, tid, fn, ln, sr, lv);
			m_vReportUnit[i]->unlock();

			//职责链不继续向下传递
			if (!bFlag)
				break;
		}
	}

	void AddReportUnit(CReportBase<L> *pUnit)
	{
		lock();
		m_vReportUnit.push_back(pUnit);
		unlock();
	}

private:
	CReportBox<L>() {};
	~CReportBox<L>()
	{
		for (int i = 0; i < m_vReportUnit.size(); i++)
			delete m_vReportUnit[i];
	}

	static CReportBox<L> *m_pInstance;
	static bool m_bFlag;
	vector<CReportBase<L>*> m_vReportUnit;
};

template <class L> CReportBox<L>* CReportBox<L>::m_pInstance = NULL;
template <class L> bool CReportBox<L>::m_bFlag = true;

#ifdef _WIN32
#define REPORT(L, M, R, Level) \
	CReportBox<L>::Instance()->AddReport(M, GetCurrentThreadId(), __FILE__, __LINE__, R, Level)
#define HEXDUMP(L, M, D, Len) //CReportBox<L>::Instance()->HexDump(M, GetCurrentThreadId(), __FILE__, __LINE__, D, Len);
#elif defined _POSIX_C_SOURCE
#define REPORT(L, M, R, Level) \
	CReportBox<L>::Instance()->AddReport(M, (int)pthread_self(), __FILE__, __LINE__, R, Level)
#define HEXDUMP(L, M, D, Len) //CReportBox<L>::Instance()->HexDump(M, (int)pthread_self(), __FILE__, __LINE__, D, Len);
#endif

#endif
