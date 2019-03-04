//Service.h
//

#ifndef _SERVICE_H_
#define _SERVICE_H_

#include <winsvc.h>
/////////////////////////////////////////////////////////////////////////////////////
class CService
{
public:
	CService();
	virtual ~CService();

	BOOL IsInstalled(CString strServName);
	BOOL Install(CString strServName);
	BOOL Uninstall(CString strServName);
	BOOL StartServiceMain(CString strServName, CService* pService);
	BOOL StartService(CString strServName);
	BOOL StopService(CString strServName);
	BOOL IsServiceStopped(CString strServName);
protected:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
	static void WINAPI _Handler(DWORD dwOpcode);
	void ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
	void Handler(DWORD dwOpcode);
	void SetServiceStatus(DWORD dwState);
	//void CService::LogEvent(LPCTSTR pFormat, ...);

	virtual void Run();    //在派生类中重定义
	virtual void Stop();   //

private:
	void GetLastError(UINT nLine);

	//Attributes
	CString                m_strServName;  //服务名称
	SERVICE_STATUS         m_status;
	SERVICE_STATUS_HANDLE  m_hServiceStatus;
	DWORD                  m_dwThreadID;
};

/////////////////////////////////////////////////////////////////////////////////////
#endif