// KLine_Exe.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "Log.h"
#include "KLineServ.h"
#include "../../public/devmonitor/ReportServer.h"
#include "LocalReport.h"
#include "Dlg_Info.h"
#include "Container.h"
#include "CheckThread.h"
// CKLine_ExeApp:
// �йش����ʵ�֣������ KLine_Exe.cpp
//
typedef std::map<CString, CLog*> CLogMap;
class CKLine_ExeApp : public CWinApp
{
public:
	CKLine_ExeApp();

// ��д
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
// ʵ��
	BOOL InitConfig();
	BOOL SaveServerInfo();
	BOOL SaveKFileInfo();
	BOOL SaveDevMonitorInfo();
	void InitDevMonitor();
	void UninitDevMonitor();
	//void LogEvent(WORD nLogType, LPCTSTR pFormat, ...);
	void AddLog(WORD wLogType, tagLog* pLog);
	void DownloadXml(const char *lpPath, const char *pDataBuf, int nDataSize);
	void WriteToLogFile(tagLog* pLog);

public:
	CString  m_strSerIP;
	UINT     m_nPort;
	CString  m_strCfgPath;

	CString  m_strKFilePath;
// 	CString  m_strHisKFilePath;
	UINT     m_nReserveCount;      //�ļ��б������ļ�¼��
	int      m_nDeleteOtiose;      //ɾ������ļ�¼
	BOOL     m_bAutoCheck;         //�Ƿ������Զ���⹦��
	UINT     m_nCheckTime;
	BOOL     m_bUserHttpData;      //ʹ���������ݽ�����֤
	BOOL	 m_bUpdateData;		   //�Ƿ��滻��ƥ�������
	BOOL     m_bUseUpLoad;         //ʹ��k���ļ��ϴ�
	BOOL     m_bUseDownLoad;       //ʹ��k���ļ�����
	UINT     m_nDownLoadTime;	   //�Զ�����k���ļ�ʱ��

	CString  m_strServerAddr;	   //FTP��������ַ
	CString  m_strUserName;		   //�û���
	CString  m_strPassword;		   //����
	CString  m_strFTPPath;         //FTP�ϴ�·��
	CString  m_strHQFTPPath;         //��Ȩk��FTP�ϴ�·��
	CString  m_strDatFilePath;     //dat�ļ�·��������index.dat��mink.dat��quote.dat��tick.dat
	

	CString  m_strHQServIP;		   //���������IP, ���ڻ�ȡָ���������̼۸�
	UINT     m_nHQServPort;		   //����������˿�
	CString  m_strHQDayFilePath;   //������ȡָ���������̼۸񡱱��������ļ�
	
	UINT     m_nDevMonPort;        //�豸��ط������˿� 
	int      m_nDevServTimeOut;    //�豸��������ʱ
	int      m_nDevTimeOut;        //�豸��ʱ
	BOOL     m_bDevTORestart;      //�豸��ʱ�Ƿ������豸

	CKLineServ         m_KLineServ;	   
	CCriticalSection   m_lock;     //��־��

	CReportServer*     m_pRptServer;
	CLocalReport*      m_pLocalRpt;
	BOOL               m_bInitCompleted; //��ʼ�����

	BOOL      m_bShowToolBar;      //��ʾ������
	BOOL      m_bShowStatusBar;	   //��ʾ״̬��

	UINT      m_nlstMsgClmWidth[8];
	UINT      m_nlstDevClmW[4];
	UINT      m_nlstDevServClmW[3];

	//��Ȩ�������ݿ�����
	CString   m_strSqlIP;
	CString   m_strSqlDBN;
	CString   m_strSqlUID;
	CString   m_strSqlPw;

	//
// 	CString   m_strKLBkPath;  //ԭʼk�߱���Ŀ¼
// 	CString   m_strHQKLPath;  //��Ȩk��Ŀ¼

	//ʵʱ��Ȩ
	BOOL      m_bRealHQ;     //ʵʱ��Ȩ
	BOOL      m_bRWUpload;   //ʵʱ��Ȩ���ϴ�
	UINT      m_nRealHQTime; //ʵʱ��Ȩʱ��

	BOOL      m_bDownLoadHQ; //��ʱ���ػ�Ȩ�ļ�
	UINT      m_nDownHQTime; //��ʱ����ʱ��

	BOOL      m_bExit;

	//���ģ������
    CString   m_strDevServIP; 
	UINT      m_nDevServPort;
	CString   m_strAppName;

	BOOL      m_bUseDevMonitor;
	BOOL      m_bStartDevMon;

	UINT      m_nMaxRecord;  //�����ʾ��������¼
	CLogMap   m_mapLog;      //������־ָ��

	CDeque<tagLog*>	m_deqLog;
	CCriticalSection m_csLog;

	CWnd*     m_pDlgInfoWnd;

	CCheckThread* m_pcheckThread;
	DECLARE_MESSAGE_MAP()
};

extern CKLine_ExeApp theApp;