/************************************************************************************
*
* 文件名  : NTService.h
*
* 文件描述: NT服务类
*
* 创建人  : dingjing, 2009-02-27
*
* 版本号  : 1.0
*
* 修改记录:
*
************************************************************************************/
#ifndef _NTSERVICE_H_
#define _NTSERVICE_H_

/////////////////////////////////////////////////////////////////////////////////////
class CNTService
{
public:
	CNTService();
	virtual ~CNTService();

	BOOL IsInstalled(std::string strServName);
	BOOL Install(std::string strServName);
	BOOL Uninstall(std::string strServName);
	BOOL IsServiceStopped(std::string strServName);
	BOOL StartService(std::string strServName);
	BOOL StopService(std::string strServName);
	BOOL StartServiceMain(std::string strServName, CNTService* pService);

protected:
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
	void ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
	static void WINAPI _Handler(DWORD dwOpcode);
	void Handler(DWORD dwOpcode);
	void SetServiceStatus(DWORD dwState);

	virtual void Run();    //在派生类中重定义
	virtual void Stop();   //

private:

	//Attributes
	std::string            m_strServName;  //服务名称
	SERVICE_STATUS         m_status;
	SERVICE_STATUS_HANDLE  m_hServiceStatus;
	DWORD                  m_dwThreadID;

};
/////////////////////////////////////////////////////////////////////////////////////
#endif