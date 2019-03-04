// KLine_Exe.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
#include "Log.h"
#include "KLineServ.h"
#include "../../public/devmonitor/ReportServer.h"
#include "LocalReport.h"
#include "Dlg_Info.h"
#include "Container.h"
#include "CheckThread.h"
// CKLine_ExeApp:
// 有关此类的实现，请参阅 KLine_Exe.cpp
//
typedef std::map<CString, CLog*> CLogMap;
class CKLine_ExeApp : public CWinApp
{
public:
	CKLine_ExeApp();

// 重写
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
// 实现
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
	UINT     m_nReserveCount;      //文件中保留最多的记录数
	int      m_nDeleteOtiose;      //删除多余的记录
	BOOL     m_bAutoCheck;         //是否启用自动检测功能
	UINT     m_nCheckTime;
	BOOL     m_bUserHttpData;      //使用网上数据进行验证
	BOOL	 m_bUpdateData;		   //是否替换不匹配的数据
	BOOL     m_bUseUpLoad;         //使用k线文件上传
	BOOL     m_bUseDownLoad;       //使用k线文件下载
	UINT     m_nDownLoadTime;	   //自动下载k线文件时间

	CString  m_strServerAddr;	   //FTP服务器地址
	CString  m_strUserName;		   //用户名
	CString  m_strPassword;		   //密码
	CString  m_strFTPPath;         //FTP上传路径
	CString  m_strHQFTPPath;         //还权k线FTP上传路径
	CString  m_strDatFilePath;     //dat文件路径，包括index.dat、mink.dat、quote.dat、tick.dat
	

	CString  m_strHQServIP;		   //行情服务器IP, 用于获取指定日期收盘价格
	UINT     m_nHQServPort;		   //行情服务器端口
	CString  m_strHQDayFilePath;   //供“获取指定日期收盘价格”本地日线文件
	
	UINT     m_nDevMonPort;        //设备监控服务器端口 
	int      m_nDevServTimeOut;    //设备服务器超时
	int      m_nDevTimeOut;        //设备超时
	BOOL     m_bDevTORestart;      //设备超时是否重启设备

	CKLineServ         m_KLineServ;	   
	CCriticalSection   m_lock;     //日志锁

	CReportServer*     m_pRptServer;
	CLocalReport*      m_pLocalRpt;
	BOOL               m_bInitCompleted; //初始化完成

	BOOL      m_bShowToolBar;      //显示工具栏
	BOOL      m_bShowStatusBar;	   //显示状态栏

	UINT      m_nlstMsgClmWidth[8];
	UINT      m_nlstDevClmW[4];
	UINT      m_nlstDevServClmW[3];

	//还权因子数据库配置
	CString   m_strSqlIP;
	CString   m_strSqlDBN;
	CString   m_strSqlUID;
	CString   m_strSqlPw;

	//
// 	CString   m_strKLBkPath;  //原始k线备份目录
// 	CString   m_strHQKLPath;  //还权k线目录

	//实时还权
	BOOL      m_bRealHQ;     //实时还权
	BOOL      m_bRWUpload;   //实时还权后上传
	UINT      m_nRealHQTime; //实时还权时间

	BOOL      m_bDownLoadHQ; //定时下载还权文件
	UINT      m_nDownHQTime; //定时下载时间

	BOOL      m_bExit;

	//监控模块设置
    CString   m_strDevServIP; 
	UINT      m_nDevServPort;
	CString   m_strAppName;

	BOOL      m_bUseDevMonitor;
	BOOL      m_bStartDevMon;

	UINT      m_nMaxRecord;  //最多显示多少条记录
	CLogMap   m_mapLog;      //保存日志指针

	CDeque<tagLog*>	m_deqLog;
	CCriticalSection m_csLog;

	CWnd*     m_pDlgInfoWnd;

	CCheckThread* m_pcheckThread;
	DECLARE_MESSAGE_MAP()
};

extern CKLine_ExeApp theApp;