/************************************************************************************
*
* �ļ���  : NTService.h
*
* �ļ�����: NT������
*
* ������  : dingjing, 2009-02-27
*
* �汾��  : 1.0
*
* �޸ļ�¼:
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

	virtual void Run();    //�����������ض���
	virtual void Stop();   //

private:

	//Attributes
	std::string            m_strServName;  //��������
	SERVICE_STATUS         m_status;
	SERVICE_STATUS_HANDLE  m_hServiceStatus;
	DWORD                  m_dwThreadID;

};
/////////////////////////////////////////////////////////////////////////////////////
#endif