//DevMonitor.cpp
//#include "stdafx.h"
#include "DevMonitor.h"

CDevMonitor g_DevMonitor;
//////////////////////////////////////////////////////////////////////////////////////
CDevMonitor::CDevMonitor()
{
	_UnInit         = NULL;
	m_pAuthProxy    = NULL;
	m_nTimeOut      = 60;
	m_bInitSuccess  = false;
}

CDevMonitor::~CDevMonitor()
{
}

bool CDevMonitor::Init(std::string strDevServerIP, unsigned int nDevServerPort, std::string strDevName)
{
	m_strDevName = strDevName;

	std::string strExecutePath = GetExecutePath();
	strExecutePath = strExecutePath.substr(0, strExecutePath.rfind("\\") + 1);

#ifdef _DEBUG
	strExecutePath += "proxy_auth_serverD.dll";
#else
	strExecutePath += "proxy_auth_server.dll";
#endif

	if (!m_xDll.Open(strExecutePath.c_str()))
		REPORT_RUN(MONITOR_NAME, T("载入DLL[%s]失败\n", strExecutePath.c_str()), RPT_ERROR, return false;);

	pfnInit   _Init = (pfnInit)m_xDll.Symbol("Initialize");
	CHECK_RUN(_Init == NULL, MONITOR_NAME, T("获得函数Initialize()指针失败\n"), RPT_ERROR, return false;);

    _UnInit = (pfnUninit)m_xDll.Symbol("Uninitialize");
	CHECK_RUN(_UnInit == NULL, MONITOR_NAME, T("获得函数Uninitialize()指针失败\n"), RPT_ERROR, return false;);

	pfnGetModuleVer _GetModuleVer = (pfnGetModuleVer)m_xDll.Symbol("GetModuleVer");
	CHECK_RUN(_GetModuleVer == NULL, MONITOR_NAME, T("获得函数GetModuleVer()指针失败\n"), RPT_ERROR, return false;);

	char* pVer = NULL;				  
	_GetModuleVer(&pVer);
	m_strVer = pVer;

	//初始化与DOG通信的dll
	m_pAuthProxy = _Init(m_nTimeOut, strDevServerIP.c_str(), nDevServerPort, NULL);
	CHECK_RUN(m_pAuthProxy == NULL, MONITOR_NAME, T("_Init() 失败\n"), RPT_ERROR, return false;);

	unsigned long dwProID = GetCurrentProcessId();
	char szProID[50] = {0};
	sprintf_s(szProID, 50, "%d", dwProID);
	int bRet = m_pAuthProxy->OnServerLogin(strDevName.c_str(), szProID);
	if (-1 == bRet)
		return false;

	m_bInitSuccess  = true;
	return true;
}

void CDevMonitor::Uninit()
{
	m_bInitSuccess  = false;
	if (_UnInit)
		_UnInit();
}

void CDevMonitor::SendLog(int nLevel, char* pLog)
{
	if (!m_bInitSuccess)
		return;
	string s = m_xml.Log(m_strDevName.c_str(), nLevel, pLog, "");
	int length= (int)s.length()+sizeof(CommxHead);
	char *szBuffer = new char[length];
	CommxHead *pHead = (CommxHead*)szBuffer;
	memset(pHead, 0x00, length);
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = (int)s.length();
	memcpy(szBuffer + sizeof(CommxHead), s.c_str(),pHead->Length);
	m_pAuthProxy->OnTransmit(szBuffer, sizeof(CommxHead) + pHead->Length);
	delete[]szBuffer;
}
