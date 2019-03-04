
#include "stdafx.h"
#include "../public/report.h"
#include <deque>
#include <algorithm>
#include <direct.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CReportBase::AddReport(string &tn, string &mn, int tid, string &fn, int ln, string &sr, int lv)
{
	if (lv & RPT_IGNORE)
		return true;

	if (lv & RPT_IMPORTANT)
		cout << ">>> ";
	if (0 == (lv & RPT_NOPREFIX))
	{
		cout << tn.substr(tn.find(" ") + 1) << "[" << mn << "]" << tid << "@" << lv << "@";
		if (lv < RPT_WARNING)
			cout << ": ";
		else
			cout << fn << "(" << ln << "): ";
	}

	cout << sr;
	return true;	//继续向下传递报告
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CReportFile::CReportFile(const char *lpszFileName, unsigned int nDay)
: m_FileNamePrefix(lpszFileName), m_nDay(nDay)
{
	_mkdir("log");
}

void CReportFile::Clean()
{/** 本函数中不能使用REPORT宏，会有异常 [1/8/2010 xinl] */
	if (m_nDay == -1)
		return;

	char szFile[MAX_PATH];
	string relative_path = "log/";
	std::deque<std::string> vfs;
	sprintf(szFile, "log/%s????????.log", m_FileNamePrefix.c_str());

#ifdef _WIN32
	WIN32_FIND_DATA fd;
	HANDLE hd = ::FindFirstFile(szFile, &fd);
	if (hd == INVALID_HANDLE_VALUE)
		return;
	vfs.push_back(relative_path + fd.cFileName);
	while (::FindNextFile(hd, &fd))
		vfs.push_back(relative_path + fd.cFileName);
	::FindClose(hd);
#else defined _POSIX_C_SOURCE
#endif

	std::sort(vfs.begin(), vfs.end());
	while (vfs.size() > m_nDay)
	{
#ifdef _WIN32
		remove(vfs.front().c_str());
#else defined _POSIX_C_SOURCE
#endif
		vfs.pop_front();
	}
}

void CReportFile::NewFileName(string &tn)
{
	string sdate = tn.substr(0, tn.find(" "));
	if (sdate != m_FileNameDate)
	{
		if (m_of.is_open())
			m_of.close();
		char *pszFileName = new char[m_FileNamePrefix.length() + 17];
		sprintf(pszFileName, "log/%s%s.log", m_FileNamePrefix.c_str(), sdate.c_str());
		m_of.open(pszFileName, ios::out|ios::app);
		delete []pszFileName;
		m_FileNameDate = sdate;
		Clean();
	}
}

bool CReportFile::AddReport(string &tn, string &mn, int tid, string &fn, int ln, string &sr, int lv)
{
	if (lv & RPT_IGNORE)
		return true;

	NewFileName(tn);
	if (lv & RPT_IMPORTANT)
		m_of << ">>> ";
	if (0 == (lv & RPT_NOPREFIX))
	{
		m_of << tn.substr(tn.find(" ") + 1) << "[" << mn << "]" << tid << "@" << lv << "@";
		if (lv < RPT_WARNING)
			m_of << ": ";
		else
			m_of << fn << "(" << ln << "): ";
	}

	m_of << sr;
	m_of.flush();
	return true;	//继续向下传递报告
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CReportBox* CReportBox::m_pInstance = NULL;

CReportBox* CReportBox::Instance()
{
	if (m_pInstance == NULL)
		m_pInstance = new CReportBox;
	return m_pInstance;
}

void CReportBox::Instance(CReportBox *prb)
{
	Close();
	m_pInstance = prb;
}

void CReportBox::Close(CReportBase *pr)
{
	if (m_pInstance)
	{
		if (!pr)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
		else
		{
			m_pInstance->m_lock.lock();
			for (deque<CReportBase*>::iterator iter = m_pInstance->m_vReportUnit.begin();
				iter != m_pInstance->m_vReportUnit.end(); iter++)
			{
				if (*iter == pr)
				{
					delete (*iter);
					m_pInstance->m_vReportUnit.erase(iter);
					break;
				}
			}
			m_pInstance->m_lock.unlock();
		}
	}
}

CReportBox::CReportBox()
{}

CReportBox::~CReportBox()
{
	m_lock.lock();
	for (unsigned int i = 0; i < m_vReportUnit.size(); i++)
		delete m_vReportUnit[i];
	m_vReportUnit.clear();
	m_lock.unlock();
}

void CReportBox::HexDump(const char *szmn, int tid, const char *szFile, int ln,
						 const void *lpData, unsigned int nLength, string &sr, int lv)
{
	if (!lpData || !nLength || (lv & RPT_IGNORE))
		return;

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

	string sprint = sr;
	sprintf(szTitle, "(Dump %d bytes)\n", nLength);
	sprint += szTitle;

	for (unsigned int m = 0; m < nLength; m++)
	{
		short idx = m%16;
		unsigned char c = ((char*)lpData)[m];
		sprintf(szAlpha, "%02X ", c);
		sprint.append(szAlpha, 3);

		if (isprint(c))
			szSeeable[idx] = c;
		else
			szSeeable[idx] = '.';
		if (idx == 15)
		{
			sprint += "-> ";
			sprint.append(szSeeable , idx+1);
			sprint += "\n";
		}
		else if (m == nLength - 1)
		{
			sprint.append((16 - idx - 1) * 3, ' ');
			sprint += "-> ";
			sprint.append(szSeeable , idx+1);
			sprint += "\n";
		}
	}

	for (unsigned int i = 0; i < m_vReportUnit.size(); i++)
	{
		m_vReportUnit[i]->m_lock.lock();
		bool bFlag = m_vReportUnit[i]->AddReport(tn, mn, tid, fn, ln, sprint, lv);
		m_vReportUnit[i]->m_lock.unlock();
	}
}

void CReportBox::AddReport(const char *szmn, int tid, const char *szFile, int ln, string &sr, int lv)
{
	if (sr.length() == 0)
		return;

	string fn = szFile;
	string mn = szmn;
	string tn;

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
	tn = szTime;

	//获得文件名，不包括路径
	fn = fn.substr(fn.rfind('\\') + 1);

	m_lock.lock();
	for (unsigned int i = 0; i < m_vReportUnit.size(); i++)
	{
		m_vReportUnit[i]->m_lock.lock();
		bool bFlag = m_vReportUnit[i]->AddReport(tn, mn, tid, fn, ln, sr, lv);
		m_vReportUnit[i]->m_lock.unlock();

		//职责链不继续向下传递
		if (!bFlag)
			break;
	}
	m_lock.unlock();
}

void CReportBox::AddReportUnit(CReportBase *pUnit)
{
	m_lock.lock();
	m_vReportUnit.push_back(pUnit);
	m_lock.unlock();
}
