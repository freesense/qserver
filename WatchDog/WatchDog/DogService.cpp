
#include "stdafx.h"
#include "DogService.h"
#include <iostream>
#include <shellapi.h>
#include   <windows.h> 
#include "KillProcess.h"
#include "../../public/commx/xdll.h"
#include "ReportLog.h"
#include "../../public/auth/FilePath.h"
#include <string>
using namespace std;

IServerAuthProxy *g_pAuthProxy = NULL;

char szVer[100] = {0};
typedef void(*pfnGetModuleVer)(char** pBuf);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuthService::OnUserLogin(const char *lpUser)
{
}

void CAuthService::OnUserLogout(const char *lpUser, const char *lpNoticeMessage)
{
}

void CAuthService::OnExecute(const char *lpExecutor, const char *lpParams, const char *lpWorkDir)
{
	REPORT(MODULE_NAME, T("ShellExecute lpExecutor=%s lpParams=%s lpWorkDir=%s\n",
		lpExecutor, lpParams, lpWorkDir), RPT_INFO);
	ShellExecute(NULL, "open", lpExecutor, lpParams, lpWorkDir, SW_SHOW);
}

void CAuthService::OnStop(const char *lpExecutor, const char *lpParams, const char *lpWorkDir)
{
	{//测试 删除dat文件
		DWORD dwProcessID = atoi(lpParams);
		if (dwProcessID == 0xFFFFFFFF)
		{
			string strDatFile = lpWorkDir;
			REPORT(MODULE_NAME, T("删除dat文件\n"), RPT_INFO);
			DeleteFile((strDatFile + "\\index.dat").c_str());
			DeleteFile((strDatFile + "\\mink.dat").c_str());
			DeleteFile((strDatFile + "\\quote.dat").c_str());
			DeleteFile((strDatFile + "\\tick.dat").c_str());
			return;
		}
	}
	//ad by dj
	CKillProcess kill;
	if (!kill.KillProc(atoi(lpParams), lpExecutor))
	{
		REPORT(MODULE_NAME, T("Stop App[%s]:%d ProcessID:0x%x failed\n",
		lpExecutor, atoi(lpParams)), RPT_ERROR);
	}
}

void CAuthService::OnDownloadXml(const char *lpDevName, const char *lpPath)
{
	REPORT(MODULE_NAME, T("DownloadXml DevName:%s Path:%s\n", lpDevName, lpPath), RPT_INFO);

	char* pData = NULL;
	char* pDesBuf = NULL;
	DWORD dwLowSize = 0;
	if (!CFilePath::IsFileExist(lpPath))
	{
		REPORT(MODULE_NAME, T("文件[%s]不存在\n", lpPath), RPT_WARNING);
	}
	else
	{
		HANDLE hFile = CreateFile(lpPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			REPORT(MODULE_NAME, T("Open file[%s]failed, ErrCode:%\n", lpPath, GetLastError()), RPT_ERROR);
		}
		else
		{
			dwLowSize = GetFileSize(hFile, NULL);
			if (dwLowSize != INVALID_FILE_SIZE)
			{
				pData = new char[dwLowSize];
				memset(pData, 0, dwLowSize);
				DWORD dwReaded = 0;
				ReadFile(hFile, pData, dwLowSize, &dwReaded, NULL);
				if(dwReaded != dwLowSize)
				{
					REPORT(MODULE_NAME, T("ReadFile failed, ErrCode:%\n", GetLastError()), RPT_ERROR);
				}
			}

			CloseHandle(hFile);
		}
	}

	std::string s;
	std::string strFileName = CFilePath::GetFileName(lpPath);
	if (pData == NULL)
		s = m_xml.SendXmlData(lpDevName, "", 0, strFileName.c_str());
	else
	{
		int nBufSize = dwLowSize*2+1;
		pDesBuf = new char[nBufSize];
		memset(pDesBuf, 0, nBufSize);
		ChangeStringToBinString(pDesBuf, pData, dwLowSize);
		delete[]pData;
		s = m_xml.SendXmlData(lpDevName, pDesBuf, nBufSize, strFileName.c_str());
	}

	int length= (int)s.length()+sizeof(CommxHead);
	char *szBuffer = new char[length];
	CommxHead *pHead = (CommxHead*)szBuffer;
	memset(szBuffer, 0x00, length);
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = (int)s.length();
	memcpy(szBuffer + sizeof(CommxHead), s.c_str(),pHead->Length);
	g_pAuthProxy->OnTransmit(szBuffer, sizeof(CommxHead) + pHead->Length);
	delete[]szBuffer;
	delete[]pDesBuf;
}

void CAuthService::OnUploadXml(const char *lpPath, const char *pDataBuf, int nDataSize)
{
	REPORT(MODULE_NAME, T("UploadXml Path:%s\n", lpPath), RPT_INFO);
	//写入文件
	HANDLE hFile = CreateFile(lpPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		REPORT(MODULE_NAME, T("Open file[%s]failed, ErrCode:%d\n", lpPath, GetLastError()), RPT_ERROR);
	}

	DWORD dwWirtten = 0;
	WriteFile(hFile, pDataBuf, nDataSize, &dwWirtten, NULL);
	if (dwWirtten != nDataSize)
	{
		REPORT(MODULE_NAME, T("WriteFile failed, ErrCode:%d\n", GetLastError()), RPT_ERROR);
	}
	CloseHandle(hFile);
}

void CAuthService::ChangeStringToBinString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize)
{
	char* pDesBufPos = (char*)lpDesBuf;
	char* pSrcBufPos = (char*)lpSrcBuf;

	char szTemp[10];
	for(int i=0; i<nSrcBufSize; i++)
	{
		sprintf_s(szTemp, 10, "%02x", (unsigned char)pSrcBufPos[i]);
		memcpy(pDesBufPos, szTemp, 2);
		pDesBufPos += 2;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDogService::CDogService()
	: CNTService("WatchDog")
{
}

void CDogService::Run()
{
	::InitializeCriticalSection(&g_csDevList);
	//提升权限
	HANDLE   hToken;  
	LUID   sedebugnameValue;  
	TOKEN_PRIVILEGES   tkp;  

	if (!OpenProcessToken(GetCurrentProcess(),  
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))  
	{  

		REPORT(MODULE_NAME, T("OpenProcessToken() failed with error %d\n", GetLastError()), RPT_ERROR);
		return;  
	}  

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))  
	{ 
		REPORT(MODULE_NAME, T("LookupPrivilegeValue() failed with error %d\n", GetLastError()), RPT_ERROR);  
		CloseHandle(hToken);  
		return;  
	}  

	tkp.PrivilegeCount           = 1;  
	tkp.Privileges[0].Luid       = sedebugnameValue;  
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))  
	{  
		REPORT(MODULE_NAME, T("AdjustTokenPrivileges() failed with error %d\n", GetLastError()), RPT_ERROR);  
		return;  
	}  
	CloseHandle(hToken);
	///////////////////////////////////////////////////////////////////////////////////////

	XDLL xDll;

#ifdef _DEBUG
	gExecutePath += "proxy_auth_serverD.dll";
#else
	gExecutePath += "proxy_auth_server.dll";
#endif

	if (!xDll.Open(gExecutePath.c_str()))
		REPORT_RUN(MODULE_NAME, T("载入DLL[%s]失败\n", gExecutePath.c_str()), RPT_ERROR, return;);

	pfnInit _Initialize = (pfnInit)xDll.Symbol("Initialize");
	CHECK_RUN(_Initialize == NULL, MODULE_NAME, T("获得函数Initialize()指针失败,装载代理库失败\n"), RPT_ERROR, return;);

	pfnUninit _UnInitialize = (pfnUninit)xDll.Symbol("Uninitialize");
	CHECK_RUN(_UnInitialize == NULL, MODULE_NAME, T("获得函数Uninitialize()指针失败,装载代理库失败\n"), RPT_ERROR, return;);

	pfnGetModuleVer _GetModuleVer = (pfnGetModuleVer)xDll.Symbol("GetModuleVer");
	CHECK_RUN(_GetModuleVer == NULL, MODULE_NAME, T("获得函数GetModuleVer()指针失败\n"), RPT_ERROR, return;);
	//初始化与控制中心通信的dll
	char* pDllVer = NULL;
	_GetModuleVer(&pDllVer);
	strcpy_s((char*)szVer,	100, pDllVer);
	int nLen = strlen(szVer);
	strcpy_s((char*)szVer+nLen, 100-nLen, DOGVER);

	char* pCommxVer = NULL;
	CommxVer(&pCommxVer);
	if (strstr(szVer, pCommxVer) == NULL)
	{
		nLen = strlen(szVer);
		strcpy_s((char*)szVer+nLen, 100-nLen, pCommxVer);
	}

	g_pAuthProxy = _Initialize(TIME_OUT, cfg.m_strCCAddress.c_str(), cfg.m_usCCPort, &m_objAuthService);
	if (!g_pAuthProxy)
	{
		REPORT(MODULE_NAME, T("初始化代理库失败\n"), RPT_ERROR);
		return;
	}

	REPORT(MODULE_NAME, T("%s\n", szVer), RPT_VERSION);
	//add OnServerLogin()
	SendDevList();

	start_comm_loop(1);

	//初始化socket监听
	const char *lpAddress = cfg.m_strServiceAddress.c_str();
	CHECK_RUN(-1 == m_listener.Listen(lpAddress, 2), MODULE_NAME,
		T("服务监听[%s]失败\n", lpAddress),	RPT_ERROR, {stop_comm_loop(); _UnInitialize(); return;});

#ifdef CONSOLEAPP
	char cInput;
	while (1)
	{
		cin >> cInput;
		if (cInput == 'q')
			break;
	}
#else
	while (m_bIsRunning)
		Sleep(100);
#endif

	CReportBox::Instance()->Close(g_pLog);

	//释放资源
	stop_comm_loop();
	_UnInitialize();
	::DeleteCriticalSection(&g_csDevList);
}
