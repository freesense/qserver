#pragma once
#include "MulFuncList.h"
#include "SocketServer.h"
#include "resource.h"
#include "MulFuncTree.h"
#include <map>
#include "AdoDB.h"
#include "IOCPServer.h"
#include "../../public/devmonitor/auth_xml_protocol.h"
#include "afxdtctl.h"
#include "afxwin.h"
#include "Container.h"
#include "LockFile.h"
#include "SplitterControl.h"
#include "KLine_ExeDlg.h"
#include "Dlg_XmlCfg.h"
// CMainDlg 对话框

#define WM_DESTROYDLG  WM_USER+1212
#define WM_ADDLOG      WM_USER+1213

#define TDT_GROUP      0x01
#define TDT_DEV_SERV   0x02
#define TDT_DEVICE     0x03

struct tagData
{
	tagData()
	{
		nID       = 0;
		nParentID = 0;
		szName[0] = 0;
		bCheck    = FALSE;
	}

	UINT     nDataType;   //数据类型
	UINT     nID;         //分组或设备的ID
	int      nParentID;   //父项的ID
	char     szName[50];  //分组或设备的名称
	BOOL     bCheck;      //CheckComboBox使用
};

//分组数据
struct tagGroupData : tagData
{
	tagGroupData()
	{
		nDataType = TDT_GROUP;
		hItem     = NULL;
	}

	HTREEITEM     hItem;
};

//设备数据
struct tagDevData : tagData
{
	tagDevData()
	{
		nDataType   = TDT_DEVICE;
		bDevStarted = FALSE;
		szPath[0]	= 0;
		szRemark[0] = 0;
		szParam[0]	= 0;
		szInfo[0]	= 0;
		szTime[0]   = 0;
		szVersion[0]= 0;
		hItem       = NULL;
		dwProID     = 0;
		bRestart    = FALSE;
		bTODevHeartBeat = FALSE;
		szXmlPath[0] = 0;
	}

	BOOL       bDevStarted;      //设备是否开启
	char	   szPath[MAX_PATH]; //程序的路径
	char       szInfo[1024];     //附加信息
	char       szTime[50];		 //附加信息时间
	char       szParam[255];     //参数
	char       szRemark[255];    //备注
	char       szVersion[500];    //版本信息
	HTREEITEM  hItem;
	DWORD      dwProID;          //进程ID
	time_t     tmPreCheck;       //上次检测时间
	BOOL       bRestart;         //重启设备
	BOOL       bTODevHeartBeat;  //超时设备发送心跳日志
	char       szXmlPath[1024];  //配置文件的路径，多个路径用符号‘$’间隔
};

//设备服务器数据
struct tagDevServData : tagData
{
	tagDevServData()
	{
		nDataType   = TDT_DEV_SERV;
		bDevStarted = FALSE;
		szIP[0]		= 0;
		nPort       = 0;
		szRemark[0] = 0;
		szVersion[0]= 0;
		hItem       = NULL;
		hSock       = INVALID_SOCKET;
		szXmlPath[0] = 0;
	}

	BOOL       bDevStarted;    //设备是否开启
	char	   szIP[20];	   //设备所在主机IP
	UINT       nPort;
	char       szRemark[255];  //备注
	char       szVersion[100]; //版本信息
	HTREEITEM  hItem;
	SOCKET     hSock;
	char       szXmlPath[1024];  //配置文件的路径，多个路径用符号‘$’间隔
};

typedef std::map<UINT, tagGroupData*>   CGroupDataMap;
typedef std::map<UINT, tagDevServData*> CDevServDataMap;
typedef std::map<UINT, tagDevData*>     CDevDataMap;
typedef std::map<CString, CString>      CLoginDevMap;

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
class CMainDlg : public CDialog
{
    friend class CDlg_XmlCfg;
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainDlg();

	CDeque<tagLog*>	m_deqLog;
	CAuthProtoclXML m_xmlProtocol; //xml协议

	CCheckThread  m_checkThread;
// 对话框数据
	enum { IDD = IDD_DLG_MAIN };
	
	BOOL IsDevServExist(CString strDevServIP, UINT nCurDevServID);
	BOOL IsDevExist(CString strDevName, UINT nParentID, UINT nDevID);
	void OnAddLog(WORD wLogType, tagLog* pLog);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	afx_msg void OnStartDev();
	afx_msg void OnStopDev();
	afx_msg void OnKLine();
	afx_msg void OnStockQuery();
	afx_msg void OnSetup();
	afx_msg void OnDownKLine();
	afx_msg void OnDownHqKLine();

	DECLARE_MESSAGE_MAP()
private:
	void InitCtrlPos();
	void Init();
	void InitDeviceInfo();
	void SaveDevInfo();
	UINT GetValidID(UINT nTreeDataType);
	HTREEITEM CreateRootItem();
	void InitGroupData(HTREEITEM hItem, UINT nID);
	void InitDevServData();
	void InitDevData();
	void InsertListColumn(UINT nTreeDataType);
	void UpdateDevServState(tagConnect* pCon, BOOL bStart = TRUE);
	void UpdateDevState(UINT nDevID, DWORD dwProID, BOOL bStart = TRUE, BOOL bReport = TRUE);
	UINT IsDevValid(CString strDevServIP, CString* pDevName);
	void OnSelchangedTreeCtrl(HTREEITEM hSelItem);
    void StartAndStopApp(BOOL bStartApp = TRUE);
	void StartStopApp(SOCKET hSock, CString strFile, CString strParam, DWORD dwProID = 0, 
		BOOL bStart = TRUE);
	void UpdateDevAddiInfo(tagDevData* pDevData);
	void InitFilterWnd();
	void AddValidDataToLstMsg(tagLog* pLog, BOOL bInsertBegin = FALSE);
	BOOL IsTimeValid(CString strTime, CTime tmStart, CTime tmStop);
	void DevServAllDevLogout(CString strDevServIP);
	void StartStopAllDev(BOOL bStart = TRUE);

	void InitToolBar();
	void InitStatusBar();
	void DoResize1(int delta);
	void DoResize2(int delta);
	void GetListHeadWidth();
	void EnableToolBar();

	void UpdateDevServCombo(tagDevServData* pDevServData, BOOL bAdd = TRUE);
	void UpdateDevCombo(tagDevData* pDevData, BOOL bAdd = TRUE);

	static UINT WINAPI _OnTimer(LPVOID lpParam);
	void OnTimer();

private:
	CListCtrl      m_listState;
	CMulFuncTree   m_treeDevice;
	CMulFuncList   m_listMsg;

	CImageList     m_imgList;
	CImageList     m_imgTree;
	CBitmap        m_bmpNormal;
	CBitmap        m_bmpError;
	CSocketServer  m_sockServ;   //查询指定日期股票收盘价格

	//Disable Enable MenuItem
	BOOL           m_bAddGroup;
	BOOL           m_bAddDevServ;
	BOOL           m_bAddDevice;
	BOOL           m_bStartSelDev;
	BOOL           m_bStopSelDev;
	BOOL           m_bModifyDev;
	UINT           m_nModifyType;

	CGroupDataMap   m_mapGroupData;
	CDevServDataMap	m_mapDevServData;
	CDevDataMap     m_mapDevData;
	CLoginDevMap    m_mapLoginDev;

	UINT            m_nCurSelID;
	UINT            m_nCurSelType;

	int	    		m_nLogTimeout;   //recv超时时间

	CIOCPServer     m_iOcpServer;  //完成端口服务器

	CDateTimeCtrl   m_dtStart;
	CDateTimeCtrl   m_dtStop;
	CCheckComboBox  m_cDevServ;
	CCheckComboBox  m_cDev;
	CCheckComboBox  m_cLogType;

	CCriticalSection m_csLog;
	BOOL            m_bDelLog;
	BOOL            m_bShowDequeLog;

	CSplitterControl m_wndSplitter1;
	CSplitterControl m_wndSplitter2;

	CReBar		     m_wndReBar;        //Rebar条
	CToolBar         m_wndToolBar;
	CStatusBarCtrl   m_StatusBar;

	BOOL             m_bShowToolBar;
	BOOL             m_bShowStatusBar;
	BOOL             m_bEnableDevVer;

	CKLine_ExeDlg    m_KLinedlg;
	HBITMAP			 m_hToolBarBmp;

	CWorkThread*	 m_pTimerThread;
	DWORD            m_dwPreLogTick;
	BOOL             m_bHasLog;
	DWORD            m_dwSecTick;
	BOOL             m_bAddSpeed;

	BOOL             m_bEnableXmlCfg;

	CCriticalSection m_csMap;
	//CCriticalSection m_csDevServMap;
	//CCriticalSection m_csGroupMap;
#ifdef SERVICE
	NOTIFYICONDATA m_nd;  //托盘图标
#endif

public:
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnDelItemData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetItemData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLogEvent(WPARAM wParam, LPARAM lParam);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMRclickTreeDevice(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMiKline();
	afx_msg void OnDestroy();
	afx_msg void OnMiSetup();
	afx_msg void OnMiStockinfo();
	afx_msg void OnMiNetdowndata();
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnDestroyDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMenuExit();
	afx_msg void OnMenuShow();
	afx_msg void OnTmiAddgroup();
	afx_msg void OnUpdateTmiAddgroup(CCmdUI *pCmdUI);

	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
	afx_msg void OnTmiAdddevice();
	afx_msg void OnUpdateTmiAdddevice(CCmdUI *pCmdUI);
	afx_msg void OnTmiStartseldev();
	afx_msg void OnUpdateTmiStartseldev(CCmdUI *pCmdUI);
	afx_msg void OnTmiStopseldev();
	afx_msg void OnUpdateTmiStopseldev(CCmdUI *pCmdUI);
	afx_msg void OnTvnEndlabeleditTreeDevice(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTmiDel();
	afx_msg void OnUpdateTmiDel(CCmdUI *pCmdUI);
	afx_msg void OnTmiModefydev();
	afx_msg void OnUpdateTmiModefydev(CCmdUI *pCmdUI);
	afx_msg void OnTvnSelchangedTreeDevice(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTmiAdddevserv();
	afx_msg void OnUpdateTmiAdddevserv(CCmdUI *pCmdUI);
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCheckUse();
	afx_msg void OnCbnSelchangeCDevserv();
	afx_msg void OnCbnDropdownCDevserv();
	afx_msg void OnTmiStartalldev();
	afx_msg void OnTmiStopalldev();

	afx_msg LRESULT OnDevServLogin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDevServLogout(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDevLogin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateDevStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDevLogout(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDevServTimeOut(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMiOpenalldevice();
	afx_msg void OnMiClosealldevice();
	afx_msg void OnAppExit();
	afx_msg void OnMiToolbar();
	afx_msg void OnUpdateMiToolbar(CCmdUI *pCmdUI);
	afx_msg void OnMiStatusbar();
	afx_msg void OnUpdateMiStatusbar(CCmdUI *pCmdUI);
	afx_msg void OnTmiDevversion();
	afx_msg void OnUpdateTmiDevversion(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedCAddspeed();
	afx_msg void OnTmiXmlcfg();
	afx_msg void OnUpdateTmiXmlcfg(CCmdUI *pCmdUI);
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg void OnMiSavedevicelist();
public:
	afx_msg void OnMiCombinationDatFile();
};
