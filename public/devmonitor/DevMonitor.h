//DevMonitor.h
#ifndef _DEVMONITOR_H_
#define _DEVMONITOR_H_

#include "proxy_auth_server_base.h"
#include "auth_xml_protocol.h"
#include "../protocol.h"
#include "../commx/xdll.h"
#include "../commx/report.h"

#define MONITOR_NAME "MONITOR"

typedef void(*pfnGetModuleVer)(char** pBuf);
///////////////////////////////////////////////////////////////////////////////
class CDevMonitor
{
public:
	CDevMonitor();
	virtual ~CDevMonitor();

	bool Init(std::string strDevServerIP, unsigned int nDevServerPort,
		std::string strDevName);
	void Uninit();
	void SendLog(int nLevel, char* pLog);
	 
	unsigned int GetTimeOut(){return m_nTimeOut;};
public:
	int                m_nTimeOut;
	bool               m_bInitSuccess;
	std::string        m_strVer;
protected:

private:
	XDLL               m_xDll;
	pfnUninit          _UnInit;

	std::string        m_strDevName;

	CAuthProtoclXML    m_xml;
	IServerAuthProxy*  m_pAuthProxy;
};

//////////////////////////////////////////////////////////////////////////////
extern CDevMonitor g_DevMonitor;

#endif