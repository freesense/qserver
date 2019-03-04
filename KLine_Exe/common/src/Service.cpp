//Service.cpp
//
#include "stdafx.h"
#include "Service.h"
#include "KLine_Exe.h"

#define SAFE_CLOSESCM(handle) {if(handle){::CloseServiceHandle(handle); handle = NULL;} }

CService* g_pService = NULL;
////////////////////////////////////////////////////////////////////////////////////////
CService::CService()
{
	m_hServiceStatus            = NULL;
	m_status.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
	m_status.dwCurrentState     = SERVICE_STOPPED;
	m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	m_status.dwWin32ExitCode    = 0;
	m_status.dwServiceSpecificExitCode = 0;

	m_dwThreadID = 0;
}

CService::~CService()
{
}

void CService::GetLastError(UINT nLine)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ::GetLastError(), 0, (LPTSTR)&lpMsgBuf, 0, NULL);
	
	CString strCppName = __FILE__;
	strCppName = CValidFunction::GetExeFileName(strCppName);

	REPORT(MN, T("ErrMsg:%s\n", lpMsgBuf), RPT_ERROR);
	LocalFree(lpMsgBuf);
}

BOOL CService::IsInstalled(CString strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL)
		return FALSE;

	SC_HANDLE hService = ::OpenService(hSCM, strServName, SERVICE_QUERY_CONFIG);
	if(hService == NULL)
	{
		SAFE_CLOSESCM(hService);
		return FALSE;
	}
	
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CService::Install(CString strServName)
{
	if(IsInstalled(strServName))
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL)
	{
		GetLastError(__LINE__);
		return FALSE;
	}

	TCHAR szFilePath[_MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

	SC_HANDLE hService = ::CreateService(hSCM, strServName, strServName, 
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, NULL, NULL, NULL);

	if(hService == NULL)
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CService::Uninstall(CString strServName)
{
	if(!IsInstalled(strServName))
		return TRUE;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(hSCM == NULL)
	{
		GetLastError(__LINE__);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, strServName, SERVICE_STOP | DELETE);

	if(hService == NULL)
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SERVICE_STATUS status;
	::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	if (!::DeleteService(hService))
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hService);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);

	return TRUE;
}


BOOL CService::IsServiceStopped(CString strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(hSCM == NULL)
	{
		GetLastError(__LINE__);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, strServName, SERVICE_STOP | DELETE);
	if(hService == NULL)
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SERVICE_STATUS	status = {0};
	if (!QueryServiceStatus(hService, &status))
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);

	return (SERVICE_STOPPED == status.dwCurrentState);
}

BOOL CService::StartService(CString strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(hSCM == NULL)
	{
		GetLastError(__LINE__);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, strServName, SERVICE_ALL_ACCESS);
	if(hService == NULL)
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	if (!::StartService(hService,0,NULL))
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hService);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CService::StopService(CString strServName)
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(hSCM == NULL)
	{
		GetLastError(__LINE__);
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, strServName, SERVICE_STOP | DELETE);
	if(hService == NULL)
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SERVICE_STATUS	status = {0};
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
	{
		GetLastError(__LINE__);
		SAFE_CLOSESCM(hService);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

BOOL CService::StartServiceMain(CString strServName, CService* pService)
{
	ASSERT(pService != NULL);
	
	g_pService = pService;

	m_strServName = strServName;
	LPTSTR pszSevName;
	pszSevName = strServName.GetBuffer(0);
	strServName.ReleaseBuffer();

	SERVICE_TABLE_ENTRY st[] = 
	{
		{pszSevName, _ServiceMain},
		{NULL, NULL}
	};

	if(!StartServiceCtrlDispatcher(st))
	{
		GetLastError(__LINE__);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void CService::_ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	g_pService->ServiceMain(dwArgc, lpszArgv);
}

void CService::ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	m_dwThreadID = GetCurrentThreadId();
	m_hServiceStatus = RegisterServiceCtrlHandler(m_strServName, _Handler);
	if(m_hServiceStatus == NULL)
	{
		GetLastError(__LINE__);
		return;
	}

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

void CService::_Handler(DWORD dwOpcode)
{
	g_pService->Handler(dwOpcode);
}

void CService::Handler(DWORD dwOpcode)
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

void CService::SetServiceStatus(DWORD dwState)
{
	m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

void CService::Run()
{
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		DispatchMessage(&msg);
	}
}

void CService::Stop()
{
	PostThreadMessage(m_dwThreadID, WM_QUIT, 0, 0);
}

/*void CService::LogEvent(LPCTSTR pFormat, ...)
{
	char chMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(chMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);

	theApp.LogEvent(RPT_DEBUG, chMsg);
}*/