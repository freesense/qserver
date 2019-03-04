// QuoteServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdio.h>
#include <time.h>
#include "iomodel.h"
#include <windows.h>
#include <iostream>
#include "tchar.h"
#include <atlconv.h>
#include <atlbase.h>

#include "farm.h"
#include "realdata.h"
#include "config.h"
#include "wordspell.h"
#include "plugmanager.h"
#include "..\\..\\public\\commx\\synch.h"
#include "..\\..\\public\\commx\\report.h"
#include "DataPool.h"
#include "..\\..\\public\\devmonitor\\ReportServer.h"
#include "..\\..\\public\\devmonitor\\DevMonitor.h"

using namespace std;
#pragma comment(lib, "ws2_32.lib")

#ifndef _DEBUG
	#define RUNASSERVICE
#endif
#ifdef RUNASSERVICE
void Init();
void   ReconfigureService(LPWSTR   lpServiceName,   LPWSTR   lpDesc);
BOOL IsInstalled();
BOOL Install();
BOOL Uninstall();
BOOL IsStopped();
BOOL Start();
BOOL Stop();
void LogEvent(LPCTSTR pszFormat, ...);
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);

#define SAFE_CLOSESCM(handle) {if(handle){::CloseServiceHandle(handle); handle = NULL;} }

TCHAR szServiceName[] = _T("QuoteServer");
BOOL bInstall;
SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;
DWORD dwThreadID;
#endif

CDataFarm  * g_pDataFarm;
CWordSpell * g_pSpell;
CompletionPortModel * g_pIo;
CPlugManager * g_pPlugs;
CDataPool	g_DataPool;
CIndexSummary g_IndexSummary;

CReportFile * g_pReportFile;
std::string g_strVer;//版本信息

#define LOGNAME "quotesever"
#define MODULE_NAME "QS"

void MainProc()
{
	CReportBase *pr0 = new CReportBase;
	CReportFile *pr1 = new CReportFile(LOGNAME);
	g_pReportFile = pr1;
	CReportBox::Instance()->AddReportUnit(pr0);
	CReportBox::Instance()->AddReportUnit(pr1);

	CReportServer *pr2 = new CReportServer();
	CReportBox::Instance()->AddReportUnit(pr2);//for monitor
	g_strVer = "$QUOTESERVER#2.1.0$";//本应用的版本号

	g_pSpell = new CWordSpell();
	g_pSpell->ReadVoiceLib("");

	g_pPlugs = new CPlugManager();
//#ifndef RUNASSERVICE
//	g_cfg.Load("E:\\debug\\QuoteFarm.xml");
//	g_cfg.Load("E:\\debug\\QuoteServer.xml");
//#else
	g_cfg.Load(".\\qextend.xml");
	g_cfg.Load(".\\QuoteFarm.xml");
	g_cfg.Load(".\\QuoteServer.xml");
	
//#endif
	//g_Plugs.AddItem("D:\\今日投资\\新行情服务\\QuoteServer\\debug\\PlugTest.dll");

	//设备监控
#ifndef RUNASSERVICE
	cout<<"m_nDevTimeout="<<g_cfg.m_nDevTimeout <<endl;
	cout<<"m_strDevServIP="<<g_cfg.m_strDevServIP <<endl;
	cout<<"m_nDevServPort="<<g_cfg.m_nDevServPort <<endl;
	cout<<"m_strAppName="<<g_cfg.m_strAppName <<endl;
	cout<<"m_strSviceAddr="<<g_cfg.m_strSviceAddr <<endl;
#endif
	REPORT(MODULE_NAME,T("初始化设备监控模块\n"),RPT_ERROR);
	BOOL bRet = g_DevMonitor.Init(g_cfg.m_strDevServIP,g_cfg.m_nDevServPort,g_cfg.m_strAppName);
	if(!bRet)
		REPORT(MODULE_NAME,T("初始化设备监控模块失败%d!!!\n",g_DevMonitor.m_bInitSuccess),RPT_ERROR);
	else
		g_strVer += g_DevMonitor.m_strVer;
	//end
	g_cfg.m_nDevTimeout = g_DevMonitor.GetTimeOut();
	int iRet;
	iRet = g_pPlugs->InitAllItems();

	g_IndexSummary.MakeReq6001();

	REPORT(MODULE_NAME,T("Oninit ret = %d\n",iRet),RPT_WARNING);
	REPORT(MODULE_NAME,T("版本信息：%s\n",g_strVer.c_str()),RPT_VERSION);
	g_cfg.m_bRunWithQuote = false;
	REPORT(MODULE_NAME,T("MarketCount = %d\n",g_cfg.m_nMarketCount),RPT_WARNING);
	g_pDataFarm = new CDataFarm();
	
	if (!g_pDataFarm->mapfile(FILE_INDEX,g_cfg.m_nMarketCount,g_cfg.m_nSymbolCount/2,g_cfg.m_nSymbolCount,g_cfg.m_nTickCount,g_cfg.m_nMinCount))
	{
		REPORT(MODULE_NAME,T("Mapfile failed，run without quote!!!\n"),RPT_INFO);
	}
	else
	g_cfg.m_bRunWithQuote = true;
	
	REPORT(MODULE_NAME,T("Mapfile success，run quote!!!\n"),RPT_INFO);
	g_pIo = new CompletionPortModel();
	g_pIo->Init();

	g_pIo->SetLineTestTime(g_cfg.m_nLineTest);
	g_pIo->SetTimeout(g_cfg.m_nAcceptOut,g_cfg.m_nTimeOut);
	//g_pIo.ConnectToFarmer((char *)g_cfg.m_strQuoteFarmAddr.c_str(),g_cfg.m_unQuoteFarmPort);
	g_pIo->AllocEventMessage();
	if (FALSE == g_pIo->PostAcceptEx())
	{
		return;
	}
	if (FALSE == g_pIo->CreateMyThreadLoop())
	{
		return;
	}
	/*
	char aBuf[30];
	char bBuf[60];
	char cBuf[60];
	strcpy(aBuf,"我是测试项目12345我是测试单元");
	//ansi to unicode
	MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,aBuf,30,(LPWSTR)(bBuf),60);
	cout<<"bBuf count="<<wcslen((LPWSTR)bBuf)<<" len="<<lstrlen(LPWSTR(bBuf))<<endl;
	
	//unicode to utf8
	int u8Len = WideCharToMultiByte(CP_UTF8, NULL, (LPCWSTR)bBuf, wcslen((LPCWSTR)bBuf), NULL, 0, NULL, NULL);	
	cout<<"utf8 len=" << u8Len << endl;
	memset(cBuf,0,sizeof(cBuf));
	WideCharToMultiByte(CP_UTF8, NULL, (LPCWSTR)bBuf, wcslen((LPCWSTR)bBuf), cBuf, u8Len, NULL, NULL);
    //最后加上'\0'
    cBuf[u8Len] = '\0';

	//utf8 to unicode
	memset(bBuf,0,sizeof(bBuf));
	int wcsLen = MultiByteToWideChar(CP_UTF8, NULL, cBuf, strlen(cBuf), NULL, 0);
	cout<<"wcslen ="<<wcslen<<endl;
    //转换
    MultiByteToWideChar(CP_UTF8, NULL, cBuf, strlen(cBuf), (LPWSTR)bBuf, wcsLen);
    //最后加上'\0'
    ((LPWSTR)bBuf)[wcsLen] = '\0';	

	memset(aBuf,0,sizeof(aBuf));
	WideCharToMultiByte(CP_ACP,0,(LPCWSTR)bBuf,-1,aBuf,30,NULL,NULL);//-1这个参数表示取源字符串的实际长度，30这个参数表示目标缓冲区最大长度
	cout<<aBuf<<" len="<<strlen(aBuf)<<endl;
	*/
	//g_pIo->ThreadLoop();
#ifndef RUNASSERVICE
	while(true)
	{
		char acInput[101];
		memset(acInput,0,101);
		::gets_s(acInput,100);
		if (stricmp(acInput,"exit") == 0)
		{
			g_pIo->m_bExit = true;
			break;
		}
	}
	int nCount = 0;
	while(!g_pIo->m_bThreadLoopExit)
	{
		Sleep(1000);
		if (nCount++ > 10)
			break;
	}
#else
	while(!g_pIo->m_bExit)
	{
		Sleep(1000);
	}
	int nCount = 0;
	while(!g_pIo->m_bThreadLoopExit)
	{
		Sleep(1000);
		if (nCount++ > 10)
			break;
	}
#endif
	g_pIo->UnInit();
	delete g_pIo;
	delete g_pDataFarm;
	delete g_pPlugs;
	delete g_pSpell;
	if (g_DevMonitor.m_bInitSuccess)
		g_DevMonitor.Uninit();
	REPORT(MODULE_NAME,T("程序结束\n"),RPT_INFO);
	delete pr2;
	delete pr1;
	delete pr0;
	//CReportBox::Instance()->Close();
	return;
}

#ifndef RUNASSERVICE
void main()
{
	MainProc();
	exit(0);
}
#else

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	Init();

	dwThreadID = ::GetCurrentThreadId();

    SERVICE_TABLE_ENTRY st[] =
    {
        { szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

	if (stricmp(lpCmdLine, "-i") == 0)
	{
		Install();
	}
	else if (stricmp(lpCmdLine, "-u") == 0)
	{
		Uninstall();
	}
	else if (stricmp(lpCmdLine, "-o") == 0)
	{
		Start();
	}
	else if (stricmp(lpCmdLine, "-t") == 0)
	{
		Stop();
	}
	else
	{
		if (!::StartServiceCtrlDispatcher(st))
		{
			LogEvent(_T("Register Service Main Function Error!"));
		}
	}

	return 0;
}
//*********************************************************
//Functiopn:			Init
//Description:			初始化
//Calls:				main
//Called By:				
//Table Accessed:				
//Table Updated:				
//Input:				
//Output:				
//Return:				
//Others:				
//History:				
//			<author>niying <time>2006-8-10		<version>		<desc>
//*********************************************************
void Init()
{
    hServiceStatus = NULL;
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = SERVICE_STOPPED;
    status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    status.dwWin32ExitCode = 0;
    status.dwServiceSpecificExitCode = 0;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
}
char * ExtractDirFromFullPath(char * path)
{
 char *p = path;
 char *tag = p;
 while (*p)
 {
  if ( (*p) == '\\' )
   tag = p;
  ++p;
 }
 *tag = '\0';
 return path;
}


//*********************************************************
//Functiopn:			ServiceMain
//Description:			服务主函数，这在里进行控制对服务控制的注册
//Calls:
//Called By:
//Table Accessed:
//Table Updated:
//Input:
//Output:
//Return:
//Others:
//History:
//			<author>niying <time>2006-8-10		<version>		<desc>
//*********************************************************
void WINAPI ServiceMain()
{
	TCHAR acPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,acPath);
	
	GetModuleFileName(NULL,acPath,MAX_PATH);
	LogEvent(_T("Start Now!"));
	char aBuf[MAX_PATH];
	USES_CONVERSION;
	strcpy(aBuf,W2A(acPath));
	ExtractDirFromFullPath(aBuf);
	SetCurrentDirectory(A2W(aBuf));
    // Register the control request handler
    status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	//注册服务控制
    hServiceStatus = RegisterServiceCtrlHandler(szServiceName, ServiceStrl);
    if (hServiceStatus == NULL)
    {
        LogEvent(_T("Handler not installed"));
        return;
    }
    SetServiceStatus(hServiceStatus, &status);

    status.dwWin32ExitCode = S_OK;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
	status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatus, &status);

	MainProc();

	//LogEvent(_T("Service stopped a"));
	//LogEvent(_T("Service stopped b"));
	//CReportBox::Instance()->Close();
	//LogEvent(_T("Service stopped c"));
    status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(hServiceStatus, &status);
    //LogEvent(_T("Service stopped"));
}

//*********************************************************
//Functiopn:			ServiceStrl
//Description:			服务控制主函数，这里实现对服务的控制，
//						当在服务管理器上停止或其它操作时，将会运行此处代码
//Calls:
//Called By:
//Table Accessed:
//Table Updated:
//Input:				dwOpcode：控制服务的状态
//Output:
//Return:
//Others:
//History:
//			<author>niying <time>2006-8-10		<version>		<desc>
//*********************************************************
void WINAPI ServiceStrl(DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
		LogEvent(_T("normal stop"));
		PostQueuedCompletionStatus(g_pIo->m_hCOP, 0, NULL, NULL);
		g_pIo->m_bExit = true;
		status.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(hServiceStatus, &status);
        PostThreadMessage(dwThreadID, WM_CLOSE, 0, 0);
        break;
    case SERVICE_CONTROL_PAUSE:
        break;
    case SERVICE_CONTROL_CONTINUE:
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        break;
    default:
        LogEvent(_T("Bad service request"));
    }
}
//*********************************************************
//Functiopn:			IsInstalled
//Description:			判断服务是否已经被安装
//Calls:
//Called By:
//Table Accessed:
//Table Updated:
//Input:
//Output:
//Return:
//Others:
//History:
//			<author>niying <time>2006-8-10		<version>		<desc>
//*********************************************************
BOOL IsInstalled()
{
    BOOL bResult = FALSE;

	//打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
		//打开服务
        SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

//*********************************************************
//Functiopn:			Install
//Description:			安装服务函数
//Calls:
//Called By:
//Table Accessed:
//Table Updated:
//Input:
//Output:
//Return:
//Others:
//History:
//			<author>niying <time>2006-8-10		<version>		<desc>
//*********************************************************
BOOL Install()
{
    if (IsInstalled())
        return TRUE;

	//打开服务控制管理器
    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
        return FALSE;
    }

    // Get the executable file path
    TCHAR szFilePath[MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, MAX_PATH);

	//创建服务
    SC_HANDLE hService = ::CreateService(
        hSCM, szServiceName, szServiceName,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        szFilePath, NULL, NULL, _T(""), NULL, NULL);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't create service"), szServiceName, MB_OK);
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

	ReconfigureService(szServiceName,_T("行情对外服务器"));
    return TRUE;
}

//*********************************************************
//Functiopn:			Uninstall
//Description:			删除服务函数
//Calls:
//Called By:
//Table Accessed:
//Table Updated:
//Input:
//Output:
//Return:
//Others:
//History:
//			<author>niying <time>2006-8-10		<version>		<desc>
//*********************************************************
BOOL Uninstall()
{
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), szServiceName, MB_OK);
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't open service"), szServiceName, MB_OK);
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

	//删除服务
    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

    LogEvent(_T("Service could not be deleted"));
    return FALSE;
}
BOOL IsStopped()
{
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(hSCM == NULL)
	{
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);
	if(hService == NULL)
	{
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SERVICE_STATUS	status = {0};
	if (!QueryServiceStatus(hService, &status))
	{
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);

	return (SERVICE_STOPPED == status.dwCurrentState);
}

BOOL Start()
{
	//if (!IsStopped())
	//	return true;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(hSCM == NULL)
	{
		//LogEvent(_T("here Start a"));
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_ALL_ACCESS);
	if(hService == NULL)
	{
		//LogEvent(_T("here Start b"));
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	if (!::StartService(hService,0,NULL))
	{
		//LogEvent(_T("here Start d"));
		SAFE_CLOSESCM(hService);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	//LogEvent(_T("here Start e"));
	return TRUE;
}

BOOL Stop()
{
	if (IsStopped())
	{
		return true;
	}
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(hSCM == NULL)
	{
		return FALSE;
	}

	SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_STOP | DELETE);
	if(hService == NULL)
	{
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SERVICE_STATUS	status = {0};
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &status))
	{
		SAFE_CLOSESCM(hService);
		SAFE_CLOSESCM(hSCM);
		return FALSE;
	}

	SAFE_CLOSESCM(hService);
	SAFE_CLOSESCM(hSCM);
	return TRUE;
}

//*********************************************************
//Functiopn:			LogEvent
//Description:			记录服务事件
//Calls:
//Called By:
//Table Accessed:
//Table Updated:
//Input:
//Output:
//Return:
//Others:
//History:
//			<author>niying <time>2006-8-10		<version>		<desc>
//*********************************************************
void LogEvent(LPCTSTR pFormat, ...)
{
    TCHAR    chMsg[256];
    HANDLE  hEventSource;
    LPTSTR  lpszStrings[1];
    va_list pArg;

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

    lpszStrings[0] = chMsg;
	
	hEventSource = RegisterEventSource(NULL, szServiceName);
	if (hEventSource != NULL)
	{
		ReportEvent(hEventSource, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, (LPCTSTR*) &lpszStrings[0], NULL);
		DeregisterEventSource(hEventSource);
	}
}


void   ReconfigureService(LPWSTR   lpServiceName,   LPWSTR   lpDesc)
  {     
      SC_HANDLE   schSCManager   =   ::OpenSCManager(NULL,   NULL,   SC_MANAGER_ALL_ACCESS);   
	  //USES_CONVERSION;
      if   (schSCManager   !=   NULL)   
      {   
          //   Need   to   acquire   database   lock   before   reconfiguring.     
          SC_LOCK   sclLock   =   LockServiceDatabase(schSCManager);     
          if   (sclLock   !=   NULL)     
          {     
              //   Open   a   handle   to   the   service.     
              SC_HANDLE   schService   =   OpenService(     
  schSCManager,                       //   SCManager   database     
  lpServiceName,                     //   name   of   service     
  SERVICE_CHANGE_CONFIG);   //   need   CHANGE   access     
    
              if   (schService   !=   NULL)     
              {   
  SERVICE_DESCRIPTION   sdBuf;   
  sdBuf.lpDescription   =   lpDesc;   
  if(   ChangeServiceConfig2(   
  schService, SERVICE_CONFIG_DESCRIPTION,   
  &sdBuf)   )   
  {   
      //MessageBox(NULL,"Change   SUCCESS","",MB_SERVICE_NOTIFICATION);     
  }   
  CloseServiceHandle(schService);     
              }   
              UnlockServiceDatabase(sclLock);     
          }     
          CloseServiceHandle(schSCManager);     
      }   
  }

#endif