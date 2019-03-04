//NTService.cpp
#include "stdafx.h"
#include "NTService.h"

CNTService* g_pService = NULL;

#define SAFE_CLOSESCM(handle) {if(handle){::CloseServiceHandle(handle); handle = NULL;} }
////////////////////////////////////////////////////////////////////////////////////
CNTService::CNTService()
{
	m_hServiceStatus            = NULL;
	m_status.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
	m_status.dwCurrentState     = SERVICE_STOPPED;
	m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	m_status.dwWin32ExitCode    = 0;
	m_status.dwServiceSpecificExitCode = 0;

	m_dwThreadID = 0;
}

CNTService::~CNTService()
{
}

BOOL CNTService::IsInstalled(std::string strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL)
		return FALSE;

	SC_HANDLE hService = ::OpenService(hSCM, strServName.c_str(), SERVICE_QUERY_CONFIG);
	if(hService == NULL)
	{
		SAFE_CLOSESCM(hService);
		return FALSE;
	}
	
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CNTService::Install(std::string strServName)
{
	if(IsInstalled(strServName.c_str()))
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	CHECK_RUN(hSCM == NULL, MN, T("OpenSCManager() failed with error code:%d\n", GetLastError()), RPT_ERROR, return FALSE;);

	TCHAR szFilePath[_MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

	SC_HANDLE hService = ::CreateService(hSCM, strServName.c_str(), strServName.c_str(), 
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, NULL, NULL, NULL);

	CHECK_RUN(hService == NULL, MN, T("CreateService() failed with error code:%d\n", GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hSCM); return FALSE;});

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CNTService::Uninstall(std::string strServName)
{
	if(!IsInstalled(strServName.c_str()))
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	CHECK_RUN(hSCM == NULL, MN, T("OpenSCManager() failed with error code:%d\n", GetLastError()), RPT_ERROR, return FALSE;);

	SC_HANDLE hService = ::OpenService(hSCM, strServName.c_str(), SERVICE_STOP | DELETE);

	CHECK_RUN(hService == NULL, MN, T("OpenService() failed with error code:%d\n", GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hSCM); return FALSE;});

	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	CHECK_RUN(!::DeleteService(hService), MN, T("DeleteService() failed with error code:%d\n", GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hService); SAFE_CLOSESCM(hSCM); return FALSE;});

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);

	return TRUE;
}

BOOL CNTService::IsServiceStopped(std::string strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	CHECK_RUN(hSCM == NULL, MN, T("OpenSCManager() failed with error code:%d\n", GetLastError()), RPT_ERROR, return FALSE;);
	
	SC_HANDLE hService = ::OpenService(hSCM, strServName.c_str(), SERVICE_STOP | DELETE);
	CHECK_RUN(hService == NULL, MN, T("OpenService() failed with error code:%d\n", GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hSCM); return FALSE;});
	
	SERVICE_STATUS	status = {0};
	CHECK_RUN(!QueryServiceStatus(hService, &status), MN, T("QueryServiceStatus() failed with error code:%d\n",
		GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hService); SAFE_CLOSESCM(hSCM); return FALSE;});

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);

	return (SERVICE_STOPPED == status.dwCurrentState);
}

BOOL CNTService::StartService(std::string strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	CHECK_RUN(hSCM == NULL, MN, T("OpenSCManager() failed with error code:%d\n", GetLastError()), RPT_ERROR, return FALSE;);
	

	SC_HANDLE hService = ::OpenService(hSCM, strServName.c_str(), SERVICE_ALL_ACCESS);
	CHECK_RUN(hService == NULL, MN, T("OpenService() failed with error code:%d\n", GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hSCM); return FALSE;});

	CHECK_RUN(!::StartService(hService,0,NULL), MN, T("StartService() failed with error code:%d\n",
		GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hService); SAFE_CLOSESCM(hSCM); return FALSE;});

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CNTService::StopService(std::string strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	CHECK_RUN(hSCM == NULL, MN, T("OpenSCManager() failed with error code:%d\n", GetLastError()), RPT_ERROR, return FALSE;);
	
	SC_HANDLE hService = ::OpenService(hSCM, strServName.c_str(), SERVICE_STOP | DELETE);
	CHECK_RUN(hService == NULL, MN, T("OpenService() failed with error code:%d\n", GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hSCM); return FALSE;});

	SERVICE_STATUS	status = {0};
	CHECK_RUN(!ControlService(hService, SERVICE_CONTROL_STOP, &status), MN, T("ControlService() failed with error code:%d\n",
		GetLastError()), RPT_ERROR, {SAFE_CLOSESCM(hService); SAFE_CLOSESCM(hSCM); return FALSE;});

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CNTService::StartServiceMain(std::string strServName, CNTService* pService)
{
	if (pService == NULL)
		return FALSE;
	
	g_pService = pService;

	m_strServName = strServName;
	LPTSTR pszSevName;
	pszSevName = (LPTSTR)strServName.c_str();

	SERVICE_TABLE_ENTRY st[] = 
	{
		{pszSevName, _ServiceMain},
		{NULL, NULL}
	};

	CHECK_RUN(!StartServiceCtrlDispatcher(st), MN, T("StartServiceCtrlDispatcher() failed with error code:%d\n", GetLastError()), RPT_ERROR, return FALSE;);
	
	return TRUE;
}

void CNTService::_ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	g_pService->ServiceMain(dwArgc, lpszArgv);
}

void CNTService::ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	m_dwThreadID = GetCurrentThreadId();
	m_hServiceStatus = RegisterServiceCtrlHandler(m_strServName.c_str(), _Handler);
	
	CHECK_RUN(m_hServiceStatus == NULL, MN, T("RegisterServiceCtrlHandler() failed with error code:%d\n", GetLastError()), RPT_ERROR, {});

	SetServiceStatus(SERVICE_START_PENDING);
	
	m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
	
	SetServiceStatus(SERVICE_RUNNING);

	// When the Run function returns, the service has stopped.
	REPORT(MN, T("service running!\n"), RPT_INFO);
	Run();
	REPORT(MN, T("service stop!\n"), RPT_INFO);
	SetServiceStatus(SERVICE_STOPPED);
}

void CNTService::_Handler(DWORD dwOpcode)
{
	g_pService->Handler(dwOpcode);
}

void CNTService::Handler(DWORD dwOpcode)
{
	switch(dwOpcode) 
	{
	case SERVICE_CONTROL_STOP:
		{	
			SetServiceStatus(SERVICE_STOP_PENDING);
			//*************反初始化代码********************
			Stop();
			//*********************************************
			REPORT(MN, T("stop service!\n"), RPT_INFO);
			break;
		}
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;
	default:
		REPORT(MN, T("Bad service request!\n"), RPT_INFO);
	}
}

void CNTService::SetServiceStatus(DWORD dwState)
{
	m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

void CNTService::Run()
{
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		DispatchMessage(&msg);
	}
}

void CNTService::Stop()
{
	PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);
}