
#ifndef __WATCH_DOG_SERVICE_H__
#define __WATCH_DOG_SERVICE_H__

#include "NTService.h"
#include "../../public/devmonitor/proxy_auth_server_base.h"
#include "dog_socket.h"

class CAuthService : public IServerAuthBase
{
public:
	virtual void OnUserLogin(const char *lpUser);
	virtual void OnUserLogout(const char *lpUser, const char *lpNoticeMessage);
	virtual void OnExecute(const char *lpExecutor, const char *lpParams, const char *lpWorkDir);
	virtual void OnStop(const char *lpExecutor, const char *lpParams, const char *lpWorkDir);
	virtual void OnDownloadXml(const char *lpDevName, const char *lpPath);
	virtual void OnUploadXml(const char *lpPath, const char *pDataBuf, int nDataSize);

	void ChangeStringToBinString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize);
private:
	CAuthProtoclXML  m_xml;
};

class CDogService : public CNTService
{
public:
	CDogService();
	virtual void Run();

private:
	CAuthService m_objAuthService;
	CDogListener m_listener;
};

extern IServerAuthProxy *g_pAuthProxy;

#endif
