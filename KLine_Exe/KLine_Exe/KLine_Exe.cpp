// KLine_Exe.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#ifdef _DEBUG
//#include <vld.h>
#endif
#include "KLine_Exe.h"
#include "MainDlg.h"
#include "../../public/devmonitor/DevMonitor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CKLine_ExeApp

BEGIN_MESSAGE_MAP(CKLine_ExeApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CKLine_ExeApp ����

CKLine_ExeApp::CKLine_ExeApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
	m_nReserveCount = 0;
	m_nDevMonPort   = 32112;
	m_bInitCompleted = FALSE;
	m_bExit = FALSE;
	m_bUseDevMonitor = FALSE;
	m_bStartDevMon   = FALSE;
	m_nMaxRecord   = 2000; 
	m_pDlgInfoWnd  = NULL;
	m_pcheckThread = NULL;
}


// Ψһ��һ�� CKLine_ExeApp ����

CKLine_ExeApp theApp;


// CKLine_ExeApp ��ʼ��

void CKLine_ExeApp::InitDevMonitor()
{
	//*//�豸���
	std::string strDevServIP = m_strDevServIP.GetBuffer();
	m_strDevServIP.ReleaseBuffer();
	std::string strAppName = m_strAppName.GetBuffer();
	m_strAppName.ReleaseBuffer();

	BOOL bRet = g_DevMonitor.Init(strDevServIP, m_nDevServPort, strAppName);
	CHECK_RUN(!bRet, MN, T("��ʼ���豸���ģ��ʧ��\n"), RPT_ERROR, return;);

	char* pCommxVer = NULL;
	CommxVer(&pCommxVer);
	REPORT(MN, T("%s%s%s\n", KLINEVER, g_DevMonitor.m_strVer.c_str(), pCommxVer), RPT_VERSION);
	//end  //*/
}

void CKLine_ExeApp::UninitDevMonitor()
{
	//�豸���
		g_DevMonitor.Uninit();
	//end
}

BOOL CKLine_ExeApp::InitInstance()
{
	m_pRptServer = new CReportServer();
	m_pLocalRpt	 = new CLocalReport();

	CReportBox::Instance()->AddReportUnit(m_pRptServer);
	CReportBox::Instance()->AddReportUnit(m_pLocalRpt);

	if (!InitConfig())
	{
		AfxMessageBox("��ʼ�������ļ�ʧ��!");
		return FALSE;
	}
	WSAData wsaData;
	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		AfxMessageBox("Socket��ʼ��ʧ��!");
		return FALSE;
	}

	m_bStartDevMon = m_bUseDevMonitor;
	if (m_bStartDevMon)
		InitDevMonitor();

#ifdef SERVICE
	//��������
  	LPTSTR lpCmdLine = GetCommandLine();
	CString strCmdLine(lpCmdLine);
	if(strCmdLine.Find("-") == -1 && strCmdLine.Find("/") == -1)
	{
		m_KLineServ.StartServiceMain(SERVICE_NAME, &m_KLineServ);
	}
	else
	{
		int nPos = strCmdLine.ReverseFind('/');
		if (nPos == -1)
			nPos = strCmdLine.ReverseFind('-');
		int nLen = strCmdLine.GetLength();
		CString strCmd = strCmdLine.Right(nLen - nPos - 1);
		
		if(strCmd.CompareNoCase("i") == 0)
		{
			if(!m_KLineServ.Install(SERVICE_NAME))
				REPORT(MN, T("Install Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
			else
				REPORT(MN, T("Install Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
		}
		else if(strCmd.CompareNoCase("u") == 0)
		{
			if(!m_KLineServ.Uninstall(SERVICE_NAME))
				REPORT(MN, T("UnInstall Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
			else
				REPORT(MN, T("UnInstall Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
		}
		else if(strCmd.CompareNoCase("o") == 0)
		{
			if(!m_KLineServ.StartService(SERVICE_NAME))
				REPORT(MN, T("Start Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
			else
				REPORT(MN, T("Start Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
		}
		else if(strCmd.CompareNoCase("c") == 0)
		{
			if(!m_KLineServ.StopService(SERVICE_NAME))
				REPORT(MN, T("Stop Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
			else
				REPORT(MN, T("Stop Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
		}
	}
#else
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	//CKLine_ExeDlg dlg;
	CMainDlg dlg;
	m_pMainWnd = &dlg;
	m_pcheckThread = &dlg.m_checkThread;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˴����ô����ʱ�á�ȷ�������ر�
		//  �Ի���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ�á�ȡ�������ر�
		//  �Ի���Ĵ���
	}
#endif
	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

int CKLine_ExeApp::ExitInstance()
{
	m_bExit = FALSE;
	if (m_bStartDevMon)
		UninitDevMonitor();
	CReportBox::Instance()->Close();

	WSACleanup();

	CLogMap::iterator pos = m_mapLog.begin();
	while(pos != m_mapLog.end())
	{
		pos->second->Close();
		delete pos->second;
		pos++;
	}
	m_mapLog.clear();
	
	while(true)
	{
		tagLog* pLog = m_deqLog.PopFront();
		if (pLog != NULL)
			delete pLog;
		if (m_deqLog.Size() <= 0)
			break;
	}
	return CWinApp::ExitInstance();
}

BOOL CKLine_ExeApp::InitConfig()
{
	CString strCfgFilePath;
	CValidFunction::GetWorkPath(strCfgFilePath);
	strCfgFilePath += "\\QuoteConsole.ini";
	m_strCfgPath = strCfgFilePath;

	if (!CValidFunction::IsFileExist(strCfgFilePath)) //�����ļ������ھʹ���
	{
		CFile file;
		file.Open(strCfgFilePath, CFile::modeCreate);
		file.Close();
	}

	char chIP[50];
	GetPrivateProfileString("StockInfoServer", "IP", "127.0.0.1", chIP, 50, strCfgFilePath);
	m_strSerIP = chIP;
	m_nPort = GetPrivateProfileInt("StockInfoServer", "Port", 11000, strCfgFilePath);

	char chPath[MAX_PATH];
	CString strWorkPath;
	CValidFunction::GetWorkPath(strWorkPath);
	GetPrivateProfileString("KLineFile", "KFilePath", "D:\\QuoteData\\current", chPath, MAX_PATH, strCfgFilePath);
	m_strKFilePath = chPath;

	if (!CValidFunction::IsPathExist(m_strKFilePath))
	{
		if (!CValidFunction::CreatePath(m_strKFilePath))
			return FALSE;
	}

// 	GetPrivateProfileString("KLineFile", "HistoryKFilePath", "D:\\QuoteData\\history", chPath, MAX_PATH, strCfgFilePath);
// 	m_strHisKFilePath = chPath;
// 	if (!CValidFunction::IsPathExist(m_strHisKFilePath))
// 	{
// 		if (!CValidFunction::CreatePath(m_strHisKFilePath))
// 			return FALSE;
// 	}

	m_nReserveCount = GetPrivateProfileInt("KLineFile", "ReserveCount", 100, strCfgFilePath);
	m_nDeleteOtiose = GetPrivateProfileInt("KLineFile", "DeleteOtiose", 0, strCfgFilePath);
	m_bAutoCheck    = GetPrivateProfileInt("KLineFile", "AutoCheck", 0, strCfgFilePath);
	m_bUserHttpData	= GetPrivateProfileInt("KLineFile", "UserHttpData", 0, strCfgFilePath);
	m_bUpdateData   = GetPrivateProfileInt("KLineFile", "UpdateData", 0, strCfgFilePath);
	m_nCheckTime   	= GetPrivateProfileInt("KLineFile", "AutoCheckTime", 0, strCfgFilePath);
	m_bUseUpLoad	= GetPrivateProfileInt("KLineFile", "UseKFileUpLoad", 0, strCfgFilePath);
	m_bUseDownLoad	= GetPrivateProfileInt("KLineFile", "UseKFileDownLoad", 0, strCfgFilePath);
	m_nDownLoadTime	= GetPrivateProfileInt("KLineFile", "AutoDownLoadKFileTime", 0, strCfgFilePath);

	GetPrivateProfileString("KLineFile", "FTPServerAddr", "127.0.0.1", chPath, MAX_PATH, strCfgFilePath);
	m_strServerAddr = chPath;

	GetPrivateProfileString("KLineFile", "FTPUserName", "", chPath, MAX_PATH, strCfgFilePath);
	m_strUserName = chPath;

	GetPrivateProfileString("KLineFile", "FTPPassword", "", chPath, MAX_PATH, strCfgFilePath);
	m_strPassword = chPath;

	GetPrivateProfileString("KLineFile", "FTPPath", "", chPath, MAX_PATH, strCfgFilePath);
	m_strFTPPath = chPath;

	GetPrivateProfileString("KLineFile", "HQFTPPath", "", chPath, MAX_PATH, strCfgFilePath);
	m_strHQFTPPath = chPath;
	
	GetPrivateProfileString("KLineFile", "DatFilePath", "", chPath, MAX_PATH, strCfgFilePath);
	m_strDatFilePath = chPath;

	GetPrivateProfileString("KLineFile", "HQServIP", "", chPath, MAX_PATH, strCfgFilePath);
	m_strHQServIP = chPath;

	m_nHQServPort = GetPrivateProfileInt("KLineFile", "HQServPort", 0, strCfgFilePath);

	GetPrivateProfileString("KLineFile", "HQDayFilePath", "", chPath, MAX_PATH, strCfgFilePath);
	m_strHQDayFilePath = chPath;
	
	//DevMonitorInfo
	m_nDevMonPort = GetPrivateProfileInt("KLineFile", "DevMonPort", 32112, strCfgFilePath);
	m_nDevServTimeOut = GetPrivateProfileInt("KLineFile", "DevServTimeOut", 60, strCfgFilePath);
	m_nDevTimeOut = GetPrivateProfileInt("KLineFile", "DevTimeOut", 60, strCfgFilePath);
	m_bDevTORestart = GetPrivateProfileInt("KLineFile", "DevTimeOutRestart", 0, strCfgFilePath);

	m_bShowToolBar = GetPrivateProfileInt("KLineFile", "ShowToolBar", 1, strCfgFilePath);
	m_bShowStatusBar = GetPrivateProfileInt("KLineFile", "ShowStatusBar", 1, strCfgFilePath);

	CString strKey;
	for (int i=0; i<sizeof(m_nlstMsgClmWidth)/sizeof(m_nlstMsgClmWidth[0]); i++)
	{
		strKey.Format("lstMsgClmWidth%d", i);
		m_nlstMsgClmWidth[i] = GetPrivateProfileInt("KLineFile", strKey, 100, strCfgFilePath);
	}

	for (int i=0; i<sizeof(m_nlstDevClmW)/sizeof(m_nlstDevClmW[0]); i++)
	{
		strKey.Format("lstDevClmW%d", i);
		m_nlstDevClmW[i] = GetPrivateProfileInt("KLineFile", strKey, 200, strCfgFilePath);
	}

	for (int i=0; i<sizeof(m_nlstDevServClmW)/sizeof(m_nlstDevServClmW[0]); i++)
	{
		strKey.Format("lstDevServClmW%d", i);
		m_nlstDevServClmW[i] = GetPrivateProfileInt("KLineFile", strKey, 200, strCfgFilePath);
	}

	//��Ȩ�������ݿ�����
	GetPrivateProfileString("SQLServer", "SqlHQYZIP", "127.0.0.1", chPath, MAX_PATH, strCfgFilePath);
	m_strSqlIP = chPath;

	GetPrivateProfileString("SQLServer", "SqlDBN", "", chPath, MAX_PATH, strCfgFilePath);
	m_strSqlDBN = chPath;

	GetPrivateProfileString("SQLServer", "SqlUID", "", chPath, MAX_PATH, strCfgFilePath);
	m_strSqlUID = chPath;

	GetPrivateProfileString("SQLServer", "SqlPW", "", chPath, MAX_PATH, strCfgFilePath);
	m_strSqlPw = chPath;

	//
// 	GetPrivateProfileString("KLineFile", "KLBkPath", "D:\\QuoteData\\temp", chPath, MAX_PATH, strCfgFilePath);
// 	m_strKLBkPath = chPath;
// 	if (!CValidFunction::IsPathExist(m_strKLBkPath))
// 	{
// 		if (!CValidFunction::CreatePath(m_strKLBkPath))
// 			return FALSE;
// 	}
// 
// 	GetPrivateProfileString("KLineFile", "HQKLPath", "D:\\QuoteData\\weight", chPath, MAX_PATH, strCfgFilePath);
// 	m_strHQKLPath = chPath;
// 	
// 	if (!CValidFunction::IsPathExist(m_strHQKLPath))
// 	{
// 		if (!CValidFunction::CreatePath(m_strHQKLPath))
// 			return FALSE;
// 	}
	//
	m_bRealHQ = GetPrivateProfileInt("KLineFile", "RealHQ", 0, strCfgFilePath);
	m_bRWUpload = GetPrivateProfileInt("KLineFile", "RWUpload", 0, strCfgFilePath);
	m_bDownLoadHQ = GetPrivateProfileInt("KLineFile", "DownLoadHQ", 0, strCfgFilePath);

	m_nRealHQTime = GetPrivateProfileInt("KLineFile", "RealHQTime", 113000, strCfgFilePath);
	m_nDownHQTime = GetPrivateProfileInt("KLineFile", "DownHQTime", 113000, strCfgFilePath);

	//���ģ������

	GetPrivateProfileString("DevMonitor", "DevServIP", "127.0.0.1", chPath, MAX_PATH, strCfgFilePath);
	m_strDevServIP = chPath;

	m_nDevServPort = GetPrivateProfileInt("DevMonitor", "DevServPort", 7931, strCfgFilePath);

	GetPrivateProfileString("DevMonitor", "AppName", "QuoteConsole", chPath, MAX_PATH, strCfgFilePath);
	m_strAppName = chPath;

	m_bUseDevMonitor = GetPrivateProfileInt("DevMonitor", "UseDevMonitor", 0, strCfgFilePath);

	m_nMaxRecord = GetPrivateProfileInt("DevMonitor", "MaxRecord", 500, strCfgFilePath);
	return TRUE;
}

BOOL CKLine_ExeApp::SaveServerInfo()
{
	BOOL bRet = TRUE;
	bRet &= WritePrivateProfileString("StockInfoServer", "IP", m_strSerIP, m_strCfgPath);
	CString strPort;
	strPort.Format("%d", m_nPort);
	bRet &= WritePrivateProfileString("StockInfoServer", "Port", strPort, m_strCfgPath);
	return bRet;
}

BOOL CKLine_ExeApp::SaveKFileInfo()
{
	BOOL bRet = TRUE;
	bRet &= WritePrivateProfileString("KLineFile", "KFilePath", m_strKFilePath, m_strCfgPath);
// 	bRet &= WritePrivateProfileString("KLineFile", "HistoryKFilePath", m_strHisKFilePath, m_strCfgPath);

	CString strReserveCount;
	strReserveCount.Format("%d", m_nReserveCount);
	bRet &= WritePrivateProfileString("KLineFile", "ReserveCount", strReserveCount, m_strCfgPath);

	CString strDeleteOtiose;
	strDeleteOtiose.Format("%d", m_nDeleteOtiose);
	bRet &= WritePrivateProfileString("KLineFile", "DeleteOtiose", strDeleteOtiose, m_strCfgPath);

	CString strAutoCheck;
	strAutoCheck.Format("%d", m_bAutoCheck);
	bRet &= WritePrivateProfileString("KLineFile", "AutoCheck", strAutoCheck, m_strCfgPath);

	CString strAutoCheckTime;
	strAutoCheckTime.Format("%d", m_nCheckTime);
	bRet &= WritePrivateProfileString("KLineFile", "AutoCheckTime", strAutoCheckTime, m_strCfgPath);

	CString strUserHttpData;
	strUserHttpData.Format("%d", m_bUserHttpData);
	bRet &= WritePrivateProfileString("KLineFile", "UserHttpData", strUserHttpData, m_strCfgPath);

	CString strUpdateData;
	strUpdateData.Format("%d", m_bUpdateData);
	bRet &= WritePrivateProfileString("KLineFile", "UpdateData", strUpdateData, m_strCfgPath);
	CString strUseUpLoad;
	strUseUpLoad.Format("%d", m_bUseUpLoad);
	bRet &= WritePrivateProfileString("KLineFile", "UseKFileUpLoad", strUseUpLoad, m_strCfgPath);

	CString strUseDownLoad;
	strUseDownLoad.Format("%d", m_bUseDownLoad);
	bRet &= WritePrivateProfileString("KLineFile", "UseKFileDownLoad", strUseDownLoad, m_strCfgPath);

	CString strDownLoadTime;
	strDownLoadTime.Format("%d", m_nDownLoadTime);
	bRet &= WritePrivateProfileString("KLineFile", "AutoDownLoadKFileTime", strDownLoadTime, m_strCfgPath);
	
	bRet &= WritePrivateProfileString("KLineFile", "FTPServerAddr", m_strServerAddr, m_strCfgPath);
	bRet &= WritePrivateProfileString("KLineFile", "FTPUserName", m_strUserName, m_strCfgPath);
	bRet &= WritePrivateProfileString("KLineFile", "FTPPassword", m_strPassword, m_strCfgPath);
	bRet &= WritePrivateProfileString("KLineFile", "FTPPath", m_strFTPPath, m_strCfgPath);
	bRet &= WritePrivateProfileString("KLineFile", "HQFTPPath", m_strHQFTPPath, m_strCfgPath);
	bRet &= WritePrivateProfileString("KLineFile", "DatFilePath", m_strDatFilePath, m_strCfgPath);

	bRet &= WritePrivateProfileString("KLineFile", "HQServIP", m_strHQServIP, m_strCfgPath);
	
	CString strHQServPort;
	strHQServPort.Format("%d", m_nHQServPort);
	bRet &= WritePrivateProfileString("KLineFile", "HQServPort", strHQServPort, m_strCfgPath);

	bRet &= WritePrivateProfileString("KLineFile", "HQDayFilePath", m_strHQDayFilePath, m_strCfgPath);

	//��Ȩ�������ݿ�����
	bRet &= WritePrivateProfileString("SQLServer", "SqlHQYZIP", m_strSqlIP, m_strCfgPath);
	bRet &= WritePrivateProfileString("SQLServer", "SqlDBN", m_strSqlDBN, m_strCfgPath);
	bRet &= WritePrivateProfileString("SQLServer", "SqlUID", m_strSqlUID, m_strCfgPath);
	bRet &= WritePrivateProfileString("SQLServer", "SqlPW", m_strSqlPw, m_strCfgPath);

// 	bRet &= WritePrivateProfileString("KLineFile", "KLBkPath", m_strKLBkPath, m_strCfgPath);
// 	bRet &= WritePrivateProfileString("KLineFile", "HQKLPath", m_strHQKLPath, m_strCfgPath);

	CString strTemp;
	strTemp.Format("%d", m_bRealHQ);
	bRet &= WritePrivateProfileString("KLineFile", "RealHQ", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_bRWUpload);
	bRet &= WritePrivateProfileString("KLineFile", "RWUpload", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_bDownLoadHQ);
	bRet &= WritePrivateProfileString("KLineFile", "DownLoadHQ", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_nRealHQTime);
	bRet &= WritePrivateProfileString("KLineFile", "RealHQTime", strTemp, m_strCfgPath);
	
	strTemp.Format("%d", m_nDownHQTime);
	bRet &= WritePrivateProfileString("KLineFile", "DownHQTime", strTemp, m_strCfgPath);

	return bRet;
}

BOOL CKLine_ExeApp::SaveDevMonitorInfo()
{
	BOOL bRet = TRUE;

	CString strTemp;
	strTemp.Format("%d", m_nDevMonPort);
	bRet &= WritePrivateProfileString("KLineFile", "DevMonPort", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_nDevServTimeOut);
	bRet &= WritePrivateProfileString("KLineFile", "DevServTimeOut", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_nDevTimeOut);
	bRet &= WritePrivateProfileString("KLineFile", "DevTimeOut", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_bDevTORestart);
	bRet &= WritePrivateProfileString("KLineFile", "DevTimeOutRestart", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_bShowToolBar);
	bRet &= WritePrivateProfileString("KLineFile", "ShowToolBar", strTemp, m_strCfgPath);

	strTemp.Format("%d", m_bShowStatusBar);
	bRet &= WritePrivateProfileString("KLineFile", "ShowStatusBar", strTemp, m_strCfgPath);

	CString strKey;
	for (int i=0; i<sizeof(m_nlstMsgClmWidth)/sizeof(m_nlstMsgClmWidth[0]); i++)
	{
		strTemp.Format("%d", m_nlstMsgClmWidth[i]);
		strKey.Format("lstMsgClmWidth%d", i);
		bRet &= WritePrivateProfileString("KLineFile", strKey, strTemp, m_strCfgPath);
	}

	for (int i=0; i<sizeof(m_nlstDevClmW)/sizeof(m_nlstDevClmW[0]); i++)
	{
		strTemp.Format("%d", m_nlstDevClmW[i]);
		strKey.Format("lstDevClmW%d", i);
		bRet &= WritePrivateProfileString("KLineFile", strKey, strTemp, m_strCfgPath);
	}

	for (int i=0; i<sizeof(m_nlstDevServClmW)/sizeof(m_nlstDevServClmW[0]); i++)
	{
		strTemp.Format("%d", m_nlstDevServClmW[i]);
		strKey.Format("lstDevServClmW%d", i);
		bRet &= WritePrivateProfileString("KLineFile", strKey, strTemp, m_strCfgPath);
	}

	strTemp.Format("%d", m_nDevServPort);
	bRet &= WritePrivateProfileString("DevMonitor", "DevServPort", strTemp, m_strCfgPath);
	bRet &= WritePrivateProfileString("DevMonitor", "DevServIP", m_strDevServIP, m_strCfgPath);
	bRet &= WritePrivateProfileString("DevMonitor", "AppName", m_strAppName, m_strCfgPath);

	strTemp.Format("%d", m_bUseDevMonitor);
	bRet &= WritePrivateProfileString("DevMonitor", "UseDevMonitor", strTemp, m_strCfgPath);
	
	strTemp.Format("%d", m_nMaxRecord);
	bRet &= WritePrivateProfileString("DevMonitor", "MaxRecord", strTemp, m_strCfgPath);
	
	return bRet;
}

/*void CKLine_ExeApp::LogEvent(WORD nLogType, LPCTSTR pFormat, ...)
{
	char chMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(chMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);
}*/

void CKLine_ExeApp::AddLog(WORD wLogType, tagLog* pLog)
{		 
	BOOL bExit = m_bExit;
	if (!bExit) //�˳�ʱ���ٷ�����־
	{
		m_deqLog.PushBack(pLog);
	}
	else
	{
		delete pLog; 
	}
		
	/*m_csLog.Lock();	
	BOOL bExit = m_bExit;
	if (!bExit) //�˳�ʱ���ٷ�����־
	{
		if (m_pMainWnd != NULL && m_bInitCompleted)
		{
			CMainDlg* pMainDlg = (CMainDlg*)m_pMainWnd;
			pMainDlg->OnAddLog(wLogType, pLog);
		}

		if (m_pMainWnd != NULL && !m_bInitCompleted)
		{
			CMainDlg* pMainDlg = (CMainDlg*)m_pMainWnd;
			pMainDlg->m_deqLog.PushBack(pLog);
		}
	}  

	//д����־�ļ�
	CString strLogPath;
	CValidFunction::GetWorkPath(strLogPath);
	strLogPath += "\\Log";
	if (!pLog->bLocal) //������־
		strLogPath = strLogPath + "\\" + pLog->szDevName + pLog->szDevServIP;

	CLogMap::iterator find = m_mapLog.find(strLogPath);
	CLog* pLogObj = NULL;
	if (find == m_mapLog.end())
	{
		pLogObj = new CLog();
		if (pLog->bLocal) //������־
			pLogObj->Open(strLogPath, "QuoteConsole");
		else
			pLogObj->Open(strLogPath, pLog->szDevName);

		m_mapLog.insert(std::make_pair(strLogPath, pLogObj));
	}
	else
	{
		pLogObj = find->second;
	}

	char szMsg[MSG_BUF_LEN];
	sprintf_s(szMsg, MSG_BUF_LEN, "%s %s Msg:%s [%s] ThreadID:%d %s %s\n",
		pLog->szTime, pLog->szLogType, pLog->szMsg, pLog->szModule, pLog->dwThreadID,
		pLog->szLogPos, pLog->szIP);
	
	pLogObj->AddLog(szMsg, (int)strlen(szMsg));

	if (bExit ||
		(wLogType & 0xFF) == RPT_HEARTBEAT ||
		(wLogType & 0xFF) == RPT_ADDI_INFO || 
		(wLogType & 0xFF) == RPT_VERSION   ||
		(wLogType & 0xFF) == RPT_RESTART)
		delete pLog; 

	m_csLog.Unlock();*/
}

void CKLine_ExeApp::WriteToLogFile(tagLog* pLog)
{
	m_csLog.Lock();
	//д����־�ļ�
	CString strLogPath;
	CValidFunction::GetWorkPath(strLogPath);
	strLogPath += "\\Log";
	if (!pLog->bLocal) //������־
		strLogPath = strLogPath + "\\" + pLog->szDevName + pLog->szDevServIP;

	CLogMap::iterator find = m_mapLog.find(strLogPath);
	CLog* pLogObj = NULL;
	if (find == m_mapLog.end())
	{
		pLogObj = new CLog();
		if (pLog->bLocal) //������־
			pLogObj->Open(strLogPath, "QuoteConsole");
		else
			pLogObj->Open(strLogPath, pLog->szDevName);

		m_mapLog.insert(std::make_pair(strLogPath, pLogObj));
	}
	else
	{
		pLogObj = find->second;
	}

	char szMsg[MSG_BUF_LEN];
	sprintf_s(szMsg, MSG_BUF_LEN, "%s %s Msg:%s [%s] ThreadID:%d %s %s\n",
		pLog->szTime, pLog->szLogType, pLog->szMsg, pLog->szModule, pLog->dwThreadID,
		pLog->szLogPos, pLog->szIP);
	
	pLogObj->AddLog(szMsg, (int)strlen(szMsg));
	m_csLog.Unlock();
}

void CKLine_ExeApp::DownloadXml(const char *lpPath, const char *pDataBuf, int nDataSize)
{
	//д���ļ�
	HANDLE hFile = CreateFile(lpPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		REPORT(MN, T("Open file[%s]failed, ErrCode:%d\n", lpPath, GetLastError()), RPT_ERROR);
	}

	DWORD dwWirtten = 0;
	WriteFile(hFile, pDataBuf, nDataSize, &dwWirtten, NULL);
	if (dwWirtten != nDataSize)
	{
		REPORT(MN, T("WriteFile failed, ErrCode:%d\n", GetLastError()), RPT_ERROR);
	}
	CloseHandle(hFile);
	
	if (IDYES == ::MessageBox(m_pMainWnd->m_hWnd, "����xml�����ļ��ɹ����Ƿ���ļ�?", "��ʾ", MB_YESNO))
	{
		ShellExecute(NULL, "open", "notepad.exe", lpPath, NULL, SW_SHOW);
	}
}