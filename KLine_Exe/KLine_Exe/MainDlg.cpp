// MainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "MainDlg.h"
#include "Dlg_CfgKFile.h"
#include "Dlg_StockInfo.h"
#include "Dlg_NetHistory.h"
#include "Dlg_DevAttr.h"
#include "FilePath.h"
#include "Dlg_Setup.h"
#include "KLine_ExeDlg.h"
#include "Dlg_DevServInfo.h"
#include "Dlg_DevVer.h"
#include "Dlg_CombiFile.h"


#define WM_NOTIFYICON	      WM_USER+5
#define IDI_ICON		      0x0005

#define SERVER_PORT			  8428
#define ROOT_ID               0
// CMainDlg 对话框
#pragma warning (disable:4819)
#pragma warning(disable:936)
IMPLEMENT_DYNAMIC(CMainDlg, CDialog)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
: CDialog(CMainDlg::IDD, pParent)
{
	m_nCurSelID   = 0;
	m_nLogTimeout = 10;
	m_bDelLog     = FALSE;
	m_bShowDequeLog = TRUE;
	m_hToolBarBmp = NULL;
	m_dwPreLogTick = 0;
	m_bHasLog = FALSE;
	m_dwSecTick = 0;
}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STATE, m_listState);
	DDX_Control(pDX, IDC_TREE_DEVICE, m_treeDevice);
	//DDX_Control(pDX, IDC_LIST_INFO, m_lbInfo);
	DDX_Control(pDX, IDC_LIST_MSG, m_listMsg);
	DDX_Control(pDX, IDC_DT_START, m_dtStart);
	DDX_Control(pDX, IDC_DT_STOP, m_dtStop);
	DDX_Control(pDX, IDC_C_DEVSERV, m_cDevServ);
	DDX_Control(pDX, IDC_C_DEV, m_cDev);
	DDX_Control(pDX, IDC_C_LOGTYPE, m_cLogType);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_SIZE()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_DEVICE, &CMainDlg::OnNMRclickTreeDevice)
	ON_COMMAND(ID_MI_KLINE, &CMainDlg::OnMiKline)
	ON_WM_DESTROY()
	ON_COMMAND(ID_MI_SETUP, &CMainDlg::OnMiSetup)
	ON_COMMAND(ID_MI_STOCKINFO, &CMainDlg::OnMiStockinfo)
	ON_COMMAND(ID_MI_NETDOWNDATA, &CMainDlg::OnMiNetdowndata)

	ON_MESSAGE(WM_DESTROYDLG, OnDestroyDlg)
	ON_MESSAGE(WM_NOTIFYICON, OnNotifyIcon)
	ON_COMMAND(ID_MENU_EXIT, &CMainDlg::OnMenuExit)
	ON_COMMAND(ID_MENU_SHOW, &CMainDlg::OnMenuShow)
	ON_COMMAND(ID_TMI_ADDGROUP, &CMainDlg::OnTmiAddgroup)
	ON_UPDATE_COMMAND_UI(ID_TMI_ADDGROUP, &CMainDlg::OnUpdateTmiAddgroup)

	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_TMI_ADDDEVICE, &CMainDlg::OnTmiAdddevice)
	ON_UPDATE_COMMAND_UI(ID_TMI_ADDDEVICE, &CMainDlg::OnUpdateTmiAdddevice)
	ON_COMMAND(ID_TMI_STARTSELDEV, &CMainDlg::OnTmiStartseldev)
	ON_UPDATE_COMMAND_UI(ID_TMI_STARTSELDEV, &CMainDlg::OnUpdateTmiStartseldev)
	ON_COMMAND(ID_TMI_STOPSELDEV, &CMainDlg::OnTmiStopseldev)
	ON_UPDATE_COMMAND_UI(ID_TMI_STOPSELDEV, &CMainDlg::OnUpdateTmiStopseldev)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_DEVICE, &CMainDlg::OnTvnEndlabeleditTreeDevice)
	ON_COMMAND(ID_TMI_DEL, &CMainDlg::OnTmiDel)
	ON_UPDATE_COMMAND_UI(ID_TMI_DEL, &CMainDlg::OnUpdateTmiDel)
	ON_MESSAGE(WM_DELITEMDATA, OnDelItemData)
	ON_MESSAGE(WM_LOGEVENT, OnLogEvent)
	ON_MESSAGE(WM_GETITEMDATA, OnGetItemData)

	ON_COMMAND(ID_TMI_MODEFYDEV, &CMainDlg::OnTmiModefydev)
	ON_UPDATE_COMMAND_UI(ID_TMI_MODEFYDEV, &CMainDlg::OnUpdateTmiModefydev)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_DEVICE, &CMainDlg::OnTvnSelchangedTreeDevice)
	ON_COMMAND(ID_TMI_ADDDEVSERV, &CMainDlg::OnTmiAdddevserv)
	ON_UPDATE_COMMAND_UI(ID_TMI_ADDDEVSERV, &CMainDlg::OnUpdateTmiAdddevserv)

	ON_MESSAGE(WM_DEVSERVLOGIN, OnDevServLogin)
	ON_MESSAGE(WM_DEVSERVLOGOUT, OnDevServLogout)
	ON_MESSAGE(WM_UPDATE_DEV_STATUS, OnUpdateDevStatus)
	ON_MESSAGE(WM_DEVLOGIN, OnDevLogin)
	ON_MESSAGE(WM_DEVLOGOUT, OnDevLogout)
	ON_MESSAGE(WM_DEVSERV_TIMEOUT, OnDevServTimeOut)

	ON_BN_CLICKED(IDC_CHECK_USE, &CMainDlg::OnBnClickedCheckUse)
	ON_CBN_SELCHANGE(IDC_C_DEVSERV, &CMainDlg::OnCbnSelchangeCDevserv)
	ON_CBN_DROPDOWN(IDC_C_DEVSERV, &CMainDlg::OnCbnDropdownCDevserv)
	ON_COMMAND(ID_TMI_STARTALLDEV, &CMainDlg::OnTmiStartalldev)
	ON_COMMAND(ID_TMI_STOPALLDEV, &CMainDlg::OnTmiStopalldev)
	ON_COMMAND(ID_MI_OPENALLDEVICE, &CMainDlg::OnMiOpenalldevice)
	ON_COMMAND(ID_MI_CLOSEALLDEVICE, &CMainDlg::OnMiClosealldevice)
	ON_COMMAND(ID_APP_EXIT, &CMainDlg::OnAppExit)

	ON_COMMAND(ID_BTN_STARTDEV, OnStartDev)
	ON_COMMAND(ID_BTN_STOPDEV, OnStopDev)
	ON_COMMAND(ID_BTN_KLINE, OnKLine)
	ON_COMMAND(ID_BTN_STOCKQUERY, OnStockQuery)
	ON_COMMAND(ID_BTN_SETUP, OnSetup)
	ON_COMMAND(ID_MI_TOOLBAR, &CMainDlg::OnMiToolbar)
	ON_UPDATE_COMMAND_UI(ID_MI_TOOLBAR, &CMainDlg::OnUpdateMiToolbar)
	ON_COMMAND(ID_MI_STATUSBAR, &CMainDlg::OnMiStatusbar)
	ON_UPDATE_COMMAND_UI(ID_MI_STATUSBAR, &CMainDlg::OnUpdateMiStatusbar)
	ON_COMMAND(ID_TMI_DEVVERSION, &CMainDlg::OnTmiDevversion)
	ON_UPDATE_COMMAND_UI(ID_TMI_DEVVERSION, &CMainDlg::OnUpdateTmiDevversion)
	ON_BN_CLICKED(IDC_C_ADDSPEED, &CMainDlg::OnBnClickedCAddspeed)
	ON_COMMAND(ID_TMI_XMLCFG, &CMainDlg::OnTmiXmlcfg)
	ON_UPDATE_COMMAND_UI(ID_TMI_XMLCFG, &CMainDlg::OnUpdateTmiXmlcfg)

	ON_COMMAND(ID_BTN_DOWNKLINE, OnDownKLine)
	ON_COMMAND(ID_BTN_DOWNHQKLINE, OnDownHqKLine)
	ON_COMMAND(ID_MI_SAVEDEVICELIST, &CMainDlg::OnMiSavedevicelist)
	ON_COMMAND(ID_MI_COMBINATION_DAT_FILE, &CMainDlg::OnMiCombinationDatFile)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序

BOOL CMainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(AfxGetApp()-> LoadIcon(IDR_MAINFRAME), FALSE);
	// TODO:  在此添加额外的初始化
	m_wndSplitter1.Create(WS_CHILD | WS_VISIBLE, CRect(0,0,1,2), this, IDC_SPLITTER1);
	m_wndSplitter2.Create(WS_CHILD | WS_VISIBLE, CRect(0,0,2,1), this, IDC_SPLITTER2);
	m_bShowStatusBar = theApp.m_bShowStatusBar;
	m_bShowToolBar  = theApp.m_bShowToolBar;

	InitToolBar();
	InitStatusBar();
	InitCtrlPos();
	Init();
	InitDeviceInfo();
	InitFilterWnd();

	theApp.m_bInitCompleted = TRUE;

	m_pTimerThread = new CWorkThread(_OnTimer, this);
	if (!m_pTimerThread->Start())
		MessageBox("启动显示监控日志线程失败");

	ShowWindow(SW_SHOWMAXIMIZED);

	((CButton*)GetDlgItem(IDC_C_ADDSPEED))->SetCheck(1);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMainDlg::InitToolBar()
{
	if(!m_wndReBar.Create(this))
	{
		return;
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC))
	{
		return;
	}

	CImageList img;
		//下面的代码是设置按钮的宽度和长度
	m_wndToolBar.GetToolBarCtrl().SetButtonWidth(40, 150);
	//下面的代码是设置"热"的位图
	img.Create(22, 22, ILC_COLOR8|ILC_MASK,2,2);
	img.SetBkColor(::GetSysColor(COLOR_BTNFACE));
	
	img.Add(AfxGetApp()->LoadIcon(IDI_START));
	img.Add(AfxGetApp()->LoadIcon(IDI_STOP));
	img.Add(AfxGetApp()->LoadIcon(IDI_MAINTENANCE));
	img.Add(AfxGetApp()->LoadIcon(IDI_SEARCH));
	img.Add(AfxGetApp()->LoadIcon(IDI_SETUP));
	img.Add(AfxGetApp()->LoadIcon(IDI_DOWN1));
	img.Add(AfxGetApp()->LoadIcon(IDI_DOWN2));
	
	m_wndToolBar.GetToolBarCtrl().SetHotImageList(&img);
	img.Detach();	

	//下面的代码是设置"冷"的位图
	img.Create(19, 19, ILC_COLOR8|ILC_MASK, 2,2);
	img.SetBkColor(::GetSysColor(COLOR_BTNFACE));
	img.Add(AfxGetApp()->LoadIcon(IDI_START));
	img.Add(AfxGetApp()->LoadIcon(IDI_STOP));
	img.Add(AfxGetApp()->LoadIcon(IDI_MAINTENANCE));
	img.Add(AfxGetApp()->LoadIcon(IDI_SEARCH));
	img.Add(AfxGetApp()->LoadIcon(IDI_SETUP));
	img.Add(AfxGetApp()->LoadIcon(IDI_DOWN1));
	img.Add(AfxGetApp()->LoadIcon(IDI_DOWN2));
	m_wndToolBar.GetToolBarCtrl().SetImageList(&img);
	img.Detach();

	//改变属性
	m_wndToolBar.ModifyStyle(0, TBSTYLE_FLAT |CBRS_TOOLTIPS | TBSTYLE_TRANSPARENT|TBBS_CHECKBOX );
	m_wndToolBar.SetButtons(NULL, 10);	
	// set up each toolbar button设置文字  

	m_wndToolBar.SetButtonInfo(0, ID_BTN_STARTDEV, TBSTYLE_BUTTON, 0);
	m_wndToolBar.SetButtonText(0, "启动设备");	

	m_wndToolBar.SetButtonInfo(1, ID_BTN_STOPDEV, TBSTYLE_BUTTON, 1);
	m_wndToolBar.SetButtonText(1, "关闭设备");	

	m_wndToolBar.SetButtonInfo(2, IDC_MSG_BTNSPLI, TBBS_SEPARATOR, 0);

	m_wndToolBar.SetButtonInfo(3, ID_BTN_KLINE, TBSTYLE_BUTTON, 2);
	m_wndToolBar.SetButtonText(3, "K线维护");	

	m_wndToolBar.SetButtonInfo(4, ID_BTN_STOCKQUERY, TBSTYLE_BUTTON, 3);
	m_wndToolBar.SetButtonText(4, "股票查询");	

	m_wndToolBar.SetButtonInfo(5, IDC_MSG_BTNSPLI, TBBS_SEPARATOR, 0);

	m_wndToolBar.SetButtonInfo(6, ID_BTN_DOWNKLINE, TBSTYLE_BUTTON, 5);
	m_wndToolBar.SetButtonText(6, "下载原始k线");

	m_wndToolBar.SetButtonInfo(7, ID_BTN_DOWNHQKLINE, TBSTYLE_BUTTON, 6);
	m_wndToolBar.SetButtonText(7, "下载还权k线");

	m_wndToolBar.SetButtonInfo(8, IDC_MSG_BTNSPLI, TBBS_SEPARATOR, 0);

	m_wndToolBar.SetButtonInfo(9, ID_BTN_SETUP, TBSTYLE_BUTTON, 4);
	m_wndToolBar.SetButtonText(9, "设    置");	

	CRect rectToolBar;
	//得到按钮的大小
	m_wndToolBar.GetItemRect(0, &rectToolBar);
	//设置按钮的大小
	m_wndToolBar.SetSizes(CSize(75, 38), CSize(20, 20));
	
	//在Rebar中加入ToolBar
	m_wndReBar.AddBar(&m_wndToolBar);
	//改变一些属性
	REBARBANDINFO rbbi;
	rbbi.cbSize = sizeof(rbbi);		//这个是必须填
	rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_SIZE;// | RBBIM_BACKGROUND;
	//工具条的宽度
	rbbi.cxMinChild = rectToolBar.Width();
	//高度
	rbbi.cyMinChild = 80;//rectToolBar.Height();
	//下面这行代码是为工具条加入背景位图，请注意上rbbi.fMask中RBBIM_BACKGROUND标志
	m_hToolBarBmp = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_TOOLBARBACK));
	rbbi.hbmBack = m_hToolBarBmp;
	
	rbbi.cx = rbbi.cxIdeal = rectToolBar.Width() * 10;
	
	m_wndReBar.GetReBarCtrl().SetBandInfo(0, &rbbi);

	m_wndToolBar.ShowWindow(SW_SHOW);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STARTDEV, FALSE);
	(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STOPDEV, FALSE);

	m_wndToolBar.ShowWindow(m_bShowToolBar ? SW_SHOW : SW_HIDE);
}

void CMainDlg::InitStatusBar()
{
	m_StatusBar.Create(WS_CHILD|WS_VISIBLE|SBT_OWNERDRAW, CRect(0,0,0,0), this, 0);

	int strPartDim[3]= {200, 500, -1}; //分割数量
	m_StatusBar.SetParts(3, strPartDim);

	//设置状态栏文本
	m_StatusBar.SetText("  推荐分辨率1024*768", 1, 0);
	m_StatusBar.SetText("  版权所有@深圳市今日投资财经资讯有限公司", 2, 0);
	m_StatusBar.ShowWindow(m_bShowStatusBar ? SW_SHOW : SW_HIDE);
}

void CMainDlg::InitCtrlPos()
{
	int nToolHeight, nStatusBarH;
	if (m_bShowToolBar)
		nToolHeight = 43;
	else
		nToolHeight = 0;

	if (m_bShowStatusBar)
		nStatusBarH = 19;
	else
		nStatusBarH = 0;

	CRect rcClient;
	GetClientRect(rcClient);

	int nWidth  = rcClient.Width() / 4;
	int nHeight	= (rcClient.Height() - nToolHeight - nStatusBarH) / 3;

	if (m_treeDevice.GetSafeHwnd() != NULL)
		m_treeDevice.MoveWindow(0, nToolHeight, nWidth, nHeight-nToolHeight);

	if (m_listState.GetSafeHwnd() != NULL)
		m_listState.MoveWindow(nWidth+5, nToolHeight, rcClient.Width() - (nWidth+5), nHeight-nToolHeight);

	if (m_listMsg.GetSafeHwnd() != NULL)
		m_listMsg.MoveWindow(0, nHeight+80, rcClient.Width(),
		rcClient.Height() - (nHeight+80) - nStatusBarH);

	CWnd* pWnd = GetDlgItem(IDC_CHECK_USE);
	if (pWnd != NULL)
		pWnd->MoveWindow(780, nHeight+20, 100, 22);
	 
	pWnd = GetDlgItem(IDC_C_ADDSPEED);
	if (pWnd != NULL)
		pWnd->MoveWindow(780, nHeight+50, 150, 22);

	pWnd = GetDlgItem(IDC_S_DEVSERV);
	if (pWnd != NULL)
		pWnd->MoveWindow(15, nHeight+20, 70, 22);

	pWnd = GetDlgItem(IDC_C_DEVSERV);
	if (pWnd != NULL)
		pWnd->MoveWindow(85, nHeight+20, 300, 250);

	pWnd = GetDlgItem(IDC_S_DEV);
	if (pWnd != NULL)
		pWnd->MoveWindow(395, nHeight+20, 55, 22);

	pWnd = GetDlgItem(IDC_C_DEV);
	if (pWnd != NULL)
		pWnd->MoveWindow(453, nHeight+20, 300, 150);

	pWnd = GetDlgItem(IDC_S_LOGTYPE);
	if (pWnd != NULL)
		pWnd->MoveWindow(15, nHeight+50, 70, 22);

	pWnd = GetDlgItem(IDC_C_LOGTYPE);
	if (pWnd != NULL)
		pWnd->MoveWindow(85, nHeight+50, 300, 150);

	pWnd = GetDlgItem(IDC_S_TIME);
	if (pWnd != NULL)
		pWnd->MoveWindow(395, nHeight+50, 58, 22);

	pWnd = GetDlgItem(IDC_DT_START);
	if (pWnd != NULL)
		pWnd->MoveWindow(453, nHeight+50, 140, 22);

	pWnd = GetDlgItem(IDC_S_TO);
	if (pWnd != NULL)
		pWnd->MoveWindow(598, nHeight+50, 15, 22);

	pWnd = GetDlgItem(IDC_DT_STOP);
	if (pWnd != NULL)
		pWnd->MoveWindow(614, nHeight+50, 140, 22);

	pWnd = GetDlgItem(IDC_SPLITTER1);
	if (pWnd != NULL)
		pWnd->MoveWindow(nWidth+2, nToolHeight, 1, nHeight);
	if (m_wndSplitter1.GetSafeHwnd() != NULL)
	{
		m_wndSplitter1.MoveWindow(nWidth, nToolHeight, 4, nHeight+1-nToolHeight);
		m_wndSplitter1.SetRange(nWidth, nWidth, -1);
	}

	pWnd = GetDlgItem(IDC_SPLITTER2);
	if (pWnd != NULL)
		pWnd->MoveWindow(0, nHeight, rcClient.Width(), 2);
	if (m_wndSplitter2.GetSafeHwnd() != NULL)
	{
		m_wndSplitter2.MoveWindow(0, nHeight, rcClient.Width(), 4);
		m_wndSplitter2.SetRange(nHeight-nToolHeight, nHeight, -1);
	}

	if (m_StatusBar.GetSafeHwnd() != NULL)
		m_StatusBar.MoveWindow(0,0,0,0);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	Invalidate();
}

void CMainDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPos();
	// TODO: 在此处添加消息处理程序代码
}

void CMainDlg::Init()
{
	m_imgTree.Create(IDB_TREE, 16, 2, RGB(255,255,255));

	m_listState.SetImageList(&m_imgTree, LVSIL_SMALL);
	m_listState.SetExtendedStyle(/*LVS_EX_GRIDLINES|*/LVS_EX_FULLROWSELECT);
	//Init tree
	m_treeDevice.SetImageList(&m_imgTree, TVSIL_NORMAL);
	m_treeDevice.SetMsgWnd(m_hWnd);

	int nIndex = 0;
	m_listMsg.InsertColumn(nIndex++, "设备名称", LVCFMT_RIGHT, theApp.m_nlstMsgClmWidth[0]);
	m_listMsg.InsertColumn(nIndex++, "时    间", LVCFMT_CENTER, theApp.m_nlstMsgClmWidth[1]);
	m_listMsg.InsertColumn(nIndex++, "日志类型", LVCFMT_LEFT, theApp.m_nlstMsgClmWidth[2]);
	m_listMsg.InsertColumn(nIndex++, "日志内容", LVCFMT_LEFT, theApp.m_nlstMsgClmWidth[3]);
	m_listMsg.InsertColumn(nIndex++, "模块名称", LVCFMT_LEFT, theApp.m_nlstMsgClmWidth[4]);
	m_listMsg.InsertColumn(nIndex++, "线 程 ID", LVCFMT_LEFT, theApp.m_nlstMsgClmWidth[5]);
	m_listMsg.InsertColumn(nIndex++, "位    置", LVCFMT_LEFT, theApp.m_nlstMsgClmWidth[6]);
	m_listMsg.InsertColumn(nIndex++, "IP 地 址", LVCFMT_LEFT, theApp.m_nlstMsgClmWidth[7]);

	//ClosePrice 服务器端
	/*if (!m_sockServ.StartServer(SERVER_PORT))
		REPORT(MN, T("启动收盘价格服务器失败\n"), RPT_ERROR);
	else
		REPORT(MN, T("启动收盘价格服务器成功\n"), RPT_INFO);
	*/
	m_iOcpServer.SetMsgWnd(m_hWnd);
	m_iOcpServer.SetTimeOut(10, theApp.m_nDevServTimeOut);
	m_nLogTimeout = theApp.m_nDevTimeOut;
	if (!m_iOcpServer.StartServer(theApp.m_nDevMonPort, PACKET_HEAD_LEN))
	{
		REPORT(MN, T("CCompletionPort::StartServer() failed\n"), RPT_ERROR);
		MessageBox("启动设备监控服务器失败!");
	}

	m_KLinedlg.Create(IDD_KLINE_EXE_DIALOG, this);
	m_KLinedlg.ShowWindow(SW_HIDE);

#ifdef SERVICE
	m_nd.cbSize	= sizeof (NOTIFYICONDATA);
	m_nd.hWnd   = m_hWnd;
	m_nd.uID    = IDI_ICON;
	m_nd.uFlags	= NIF_ICON|NIF_MESSAGE|NIF_TIP;
	m_nd.uCallbackMessage	= WM_NOTIFYICON;
	m_nd.hIcon	= AfxGetApp()-> LoadIcon(IDR_MAINFRAME);
	strcpy_s(m_nd.szTip, 128,  "今日投资行情综控台");
	Shell_NotifyIcon(NIM_ADD, &m_nd);
#endif
}
void CMainDlg::OnNMRclickTreeDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
	//右键单击时选择项
	CPoint   pt;  
	::GetCursorPos(&pt);  
	m_treeDevice.ScreenToClient(&pt);  
	UINT   uFlags;  
	HTREEITEM   hItem = m_treeDevice.HitTest(pt, &uFlags);  
	if (hItem != NULL)  
		m_treeDevice.SelectItem(hItem);  
	//
	HTREEITEM hSelTree = m_treeDevice.GetSelectedItem();
	m_bAddGroup = m_bAddDevServ = m_bAddDevice = m_bEnableXmlCfg = FALSE;
	m_bModifyDev = m_bStartSelDev = m_bStopSelDev = m_bEnableDevVer = FALSE;
	if (hSelTree != NULL)
	{
		DWORD dwData = (DWORD)m_treeDevice.GetItemData(hSelTree);
		tagData* pData = (tagData*)dwData;
		if (pData != NULL)
		{
			if (pData->nDataType == TDT_GROUP)
			{
				m_bAddGroup = m_bAddDevServ = TRUE;
			}
			else if (pData->nDataType == TDT_DEV_SERV)
			{
				m_bAddDevice = TRUE;
				m_bModifyDev = TRUE;
				m_bEnableDevVer = TRUE;
				m_bEnableXmlCfg = TRUE;

			}
			else if (pData->nDataType == TDT_DEVICE)
			{
				m_bModifyDev    = TRUE;
				m_bEnableDevVer = TRUE;
				m_bEnableXmlCfg = TRUE;
				tagDevData* pDevData = (tagDevData*)dwData;
				m_bStartSelDev = !pDevData->bDevStarted;
				m_bStopSelDev  = pDevData->bDevStarted;
			}

			m_nModifyType = pData->nDataType;
		}
	}

	//Show Menu
	CMenu menu;
	menu.LoadMenu(IDR_MENU_TREE);
	CMenu* pSubMenu = menu.GetSubMenu(0);
	ASSERT(pSubMenu != NULL);

	POINT point;
	GetCursorPos( &point );
	pSubMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, 
		point.y, this);

	*pResult = 0;
}

void CMainDlg::OnMiKline()
{
	m_KLinedlg.CenterWindow();
	m_KLinedlg.ShowWindow(SW_NORMAL);
	/*CKLine_ExeDlg dlg;
	ShowWindow(SW_HIDE);
	dlg.DoModal();
	ShowWindow(SW_SHOW);*/
}

void CMainDlg::OnDestroy()
{
	theApp.m_bExit = TRUE;	 //退出时不再发送日志
	m_pTimerThread->Stop();
	delete m_pTimerThread;

	m_iOcpServer.StopServer();
	m_KLinedlg.DestroyWindow();

	GetListHeadWidth();
	theApp.m_bShowStatusBar = m_bShowStatusBar;
	theApp.m_bShowToolBar   = m_bShowToolBar;
	theApp.SaveDevMonitorInfo();

	if (m_cDev.GetCount() > 0)
	{
		tagData* pData = (tagData*)m_cDev.GetItemData(0);
		if (pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}


	for (int i=0; i<m_cLogType.GetCount(); i++)
	{
		tagData* pData = (tagData*)m_cLogType.GetItemData(i);
		if (pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
	}

	CDialog::OnDestroy();

	SaveDevInfo(); //保存设备信息到数据库
	m_treeDevice.DelAllItemData(m_treeDevice.GetRootItem());

	//m_sockServ.StopServer();  //停止ClosePrice服务器

	m_csLog.Lock();
	deque<tagLog*>* pdeq = m_deqLog.GetDeque();
	deque<tagLog*>::iterator pos = pdeq->begin();
	while(pos != pdeq->end())
	{
		if (*pos != NULL)
		{
			delete(*pos);
			*pos = NULL;
		}
		pos++;
	}  
	m_csLog.Unlock();

	::DeleteObject(m_hToolBarBmp);
#ifdef SERVICE
	// 将图标从系统托盘中删除
	Shell_NotifyIcon(NIM_DELETE, &m_nd);
#endif
}

void CMainDlg::OnMiSetup()
{
	CDlg_Setup dlg;
	dlg.DoModal();
}

void CMainDlg::OnMiStockinfo()
{
	CDlg_StockInfo  dlg;
	dlg.DoModal();
}

void CMainDlg::OnMiNetdowndata()
{
	CDlg_NetHistory  dlg;
	dlg.DoModal();
}

LRESULT CMainDlg::OnDestroyDlg(WPARAM wParam, LPARAM lParam)
{
	CDialog::OnCancel();
	return 0;
}

BOOL CALLBACK EnumProc( HWND hWnd, LPARAM lParam)
{
	static const TCHAR szAfxOldWndProc[] = _T("AfxOldWndProc423");  // Visual C++ 6.0
	//check for property and unsubclass if necessary
	WNDPROC oldWndProc = (WNDPROC)::GetProp(hWnd, szAfxOldWndProc);
	if (oldWndProc!=NULL)
	{
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)oldWndProc);
		RemoveProp(hWnd, szAfxOldWndProc);
	}

	return TRUE;
}

LRESULT CMainDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef SERVICE
	if (message == WM_SYSCOMMAND)
	{
		if (wParam == SC_MINIMIZE || wParam == SC_CLOSE)
		{
			ShowWindow(SW_HIDE);
			return TRUE;
		}
	}
	else if (message == WM_ENDSESSION)
	{
		DWORD dwProcessId;
		DWORD dwThreadId= GetWindowThreadProcessId(m_hWnd,&dwProcessId);
		EnumThreadWindows(dwThreadId, EnumProc,(LPARAM) dwThreadId);
		return TRUE;
	}

	static UINT const WM_TASKBARCREATED = RegisterWindowMessage( _T("TaskbarCreated") );
	if( WM_TASKBARCREATED == message ) // Explorer.exe重新启动
	{
		Shell_NotifyIcon(NIM_ADD, &m_nd);
		return TRUE;
	}
#endif

	if (message == WM_NOTIFY)
	{
		if (wParam == IDC_SPLITTER1)
		{	
			SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
			DoResize1(pHdr->delta);
		}
		else if (wParam == IDC_SPLITTER2)
		{
			SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
			DoResize2(pHdr->delta);
		}
	}

	return CDialog::DefWindowProc(message, wParam, lParam);
}

void CMainDlg::DoResize1(int delta)
{
	CSplitterControl::ChangeWidth(&m_treeDevice, delta);
	CSplitterControl::ChangeWidth(&m_listState, -delta, CW_RIGHTALIGN);
	Invalidate();
	UpdateWindow();

	m_wndToolBar.Invalidate();
}

void CMainDlg::DoResize2(int delta)
{
	CSplitterControl::ChangeHeight(&m_treeDevice, delta);
	CSplitterControl::ChangeHeight(&m_listState, delta);
	CSplitterControl::ChangeHeight(&m_wndSplitter1, delta);
	CSplitterControl::ChangeHeight(&m_listMsg, -delta, CW_BOTTOMALIGN);

	//
	UINT nID[] =
	{
		IDC_CHECK_USE,
		IDC_S_DEVSERV,
		IDC_C_DEVSERV,
		IDC_S_DEV,
		IDC_C_DEV,
		IDC_S_LOGTYPE,
		IDC_C_LOGTYPE,
		IDC_S_TIME,
		IDC_DT_START,
		IDC_S_TO,
		IDC_DT_STOP,
		IDC_C_ADDSPEED
	};

	CRect rcWnd;
	CWnd* pWnd = NULL;
	for (int i=0; i<sizeof(nID)/sizeof(nID[0]); i++)
	{
		pWnd = GetDlgItem(nID[i]);
		pWnd->GetWindowRect(rcWnd);
		ScreenToClient(rcWnd);
		rcWnd.OffsetRect(0, delta);
		pWnd->MoveWindow(rcWnd);
	}
		
	Invalidate();
	UpdateWindow();

	m_wndToolBar.Invalidate();
}

LRESULT CMainDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	if (wParam != IDI_ICON)
		return 0;

	switch(lParam)
	{
	case WM_LBUTTONDOWN:
		{
			ShowWindow(SW_SHOW);
			::SetForegroundWindow(m_hWnd);
			break;
		}
	case WM_RBUTTONDOWN:
		{
			CMenu menu;
			menu.LoadMenu(IDR_MENU_TRAY); 
			CMenu* pMenu=menu.GetSubMenu(0);
			CPoint pos;
			GetCursorPos(&pos);
			::SetForegroundWindow(m_hWnd);
			SetMenuDefaultItem(pMenu->m_hMenu, ID_MENU_SHOW, FALSE);
			::TrackPopupMenu(pMenu->m_hMenu, TPM_BOTTOMALIGN, pos.x, pos.y, 0,
				m_hWnd, NULL);

			break;
		}
	default:
		{
		}
	}

	return 0;
}

void CMainDlg::OnMenuExit()
{
	CDialog::OnCancel();
}

void CMainDlg::OnMenuShow()
{
	ShowWindow(SW_SHOW);
}

void CMainDlg::OnTmiAddgroup()
{
	HTREEITEM hSelTree = m_treeDevice.GetSelectedItem();
	if (hSelTree != NULL)
	{
		HTREEITEM hNewTree = m_treeDevice.InsertItem("新建分组", 0, 1, hSelTree);
		m_treeDevice.Expand(hSelTree, TVE_EXPAND);
		m_treeDevice.SelectItem(hNewTree);

		tagGroupData* pGroupData = new tagGroupData;
		strcpy_s(pGroupData->szName, 50, "新建分组"); 
		pGroupData->nID = GetValidID(TDT_GROUP);

		tagData* pData = (tagData*)m_treeDevice.GetItemData(hSelTree);
		pGroupData->nParentID = pData->nID;
		m_mapGroupData.insert(std::make_pair(pGroupData->nID, pGroupData));
		m_treeDevice.SetItemData(hNewTree, (DWORD_PTR)pGroupData);
	}
}

void CMainDlg::OnUpdateTmiAddgroup(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAddGroup);
}

void CMainDlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu)
{
	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
} 		 					

void CMainDlg::OnTmiAdddevice()
{
	HTREEITEM hSelTree = m_treeDevice.GetSelectedItem();
	if (hSelTree != NULL)
	{
		tagDevServData* pDevServData = (tagDevServData*)m_treeDevice.GetItemData(hSelTree);
		ASSERT(pDevServData != NULL);

		CDlg_DevAttr dlg;
		dlg.m_nParentID = pDevServData->nID;
		dlg.m_bAddDev = TRUE;

		UINT nDevID  = GetValidID(TDT_DEVICE);
		dlg.m_nDevID = nDevID;
		if (dlg.DoModal() != IDOK)
			return;

		HTREEITEM hNewTree = m_treeDevice.InsertItem(dlg.m_strDevName, 2, 2, hSelTree);
		m_treeDevice.Expand(hSelTree, TVE_EXPAND);
		m_treeDevice.SelectItem(hNewTree);

		tagDevData* pDevData = new tagDevData;
		strcpy_s(pDevData->szName, 50, dlg.m_strDevName); 
		strcpy_s(pDevData->szPath, MAX_PATH, dlg.m_strPath); 
		strcpy_s(pDevData->szRemark, 255, dlg.m_strRemark);
		strcpy_s(pDevData->szParam, 255, dlg.m_strParam);
		pDevData->nID   = nDevID;
		pDevData->hItem = hNewTree;

		tagData* pData = (tagData*)m_treeDevice.GetItemData(hSelTree);
		pDevData->nParentID = pData->nID;
		m_mapDevData.insert(std::make_pair(pDevData->nID, pDevData));
		m_treeDevice.SetItemData(hNewTree, (DWORD_PTR)pDevData);

		UpdateDevCombo(pDevData);
	}
}

void CMainDlg::OnUpdateTmiAdddevice(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAddDevice);
}

void CMainDlg::OnTmiStartseldev()
{
	StartAndStopApp();
}

void CMainDlg::OnUpdateTmiStartseldev(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bStartSelDev);
}

void CMainDlg::OnTmiStopseldev()
{
	StartAndStopApp(FALSE);
}

void CMainDlg::OnUpdateTmiStopseldev(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bStopSelDev);
}

void CMainDlg::StartAndStopApp(BOOL bStartApp)
{
	HTREEITEM hSelItem = m_treeDevice.GetSelectedItem();
	HTREEITEM hParentItem = m_treeDevice.GetParentItem(hSelItem);
	if (hSelItem == NULL || hParentItem == NULL)
		return;

	tagDevData* pDevData = (tagDevData*)m_treeDevice.GetItemData(hSelItem);
	tagDevServData* pDevServData = (tagDevServData*)m_treeDevice.GetItemData(hParentItem);
	ASSERT(pDevData != NULL);
	ASSERT(pDevServData != NULL);
	if (pDevServData->hSock == INVALID_SOCKET)
	{
		REPORT(MN, T("此设备所在服务器未启动\n"), RPT_ERROR);
		return;
	}

	StartStopApp(pDevServData->hSock, pDevData->szPath, pDevData->szParam,
		pDevData->dwProID, bStartApp);
	/*std::string strPro;
	if (bStartApp)
	strPro = m_xmlProtocol.StartApp(pDevData->szPath, pDevData->szParam, "");
	else
	{	
	CString strPath = pDevData->szPath;
	CString strFileName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

	CString strProID;
	strProID.Format("%d", pDevData->dwProID);
	strPro = m_xmlProtocol.StopApp(strFileName, strProID, "");
	}

	int nSendSize = (int)strPro.length() + sizeof(CommxHead);
	char szSendBuf[BUF_SIZE] = {0};
	CommxHead *pHead = (CommxHead*)szSendBuf;
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = (int)strPro.length();

	memcpy(szSendBuf + sizeof(CommxHead), strPro.c_str(), strPro.length());

	m_iOcpServer.SendPack(pDevServData->hSock, szSendBuf, nSendSize);  */
}

void CMainDlg::StartStopApp(SOCKET hSock, CString strFile, CString strParam,
							DWORD dwProID, BOOL bStart)
{
	CString strWorkDir;
	strWorkDir = strFile.Left(strFile.ReverseFind('\\'));

	std::string strPro;
	if (bStart)
		strPro = m_xmlProtocol.StartApp(strFile, strParam, strWorkDir);
	else
	{	
		CString strPath = strFile;
		CString strFileName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

		CString strProID;
		strProID.Format("%d", dwProID);
		strPro = m_xmlProtocol.StopApp(strFileName, strProID, strWorkDir);
	}

	int nSendSize = (int)strPro.length() + sizeof(CommxHead);
	char szSendBuf[BUF_SIZE] = {0};
	CommxHead *pHead = (CommxHead*)szSendBuf;
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = (int)strPro.length();

	memcpy(szSendBuf + sizeof(CommxHead), strPro.c_str(), strPro.length());

	m_iOcpServer.SendPack(hSock, szSendBuf, nSendSize);
}

void CMainDlg::OnTvnEndlabeleditTreeDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO pTVDispInfo = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);

	DWORD dwData = (DWORD)m_treeDevice.GetItemData(pTVDispInfo->item.hItem);
	tagData* pData = (tagData*)dwData;

	//禁止在同一个设备服务器下出现同样名称的设备
	if (pData != NULL && pData->nDataType == TDT_DEVICE && 
		pTVDispInfo->item.pszText != NULL) 
	{
		if (IsDevExist(pTVDispInfo->item.pszText, pData->nParentID, pData->nID))
		{
			MessageBox("此设备名称已注册!");
			*pResult = 0;
			return;
		}
	}

	if (pTVDispInfo->item.pszText != NULL)
		strcpy_s(pData->szName, 50, pTVDispInfo->item.pszText);

	if (pData->nDataType == TDT_DEVICE)
	{
		UpdateDevCombo((tagDevData*)pData, FALSE);
		UpdateDevCombo((tagDevData*)pData);
	}

	if (pData != NULL && (pData->nDataType == TDT_DEV_SERV)) 
	{
		CString strText = pTVDispInfo->item.pszText;
		if (!strText.IsEmpty())
		{
			int nPos = strText.Find("[");
			if (nPos != -1)
				strText = strText.Left(nPos);

			strcpy_s(pData->szName, 50, strText); 
			strText += "[";

			if (pData->nDataType == TDT_DEV_SERV)
			{
				tagDevServData* pDevServData = (tagDevServData*)dwData;
				strText += pDevServData->szIP;
			}
			strText += "]";
			m_treeDevice.SetItemText(pTVDispInfo->item.hItem, strText);
		}

		UpdateDevServCombo((tagDevServData*)pData, FALSE);
		UpdateDevServCombo((tagDevServData*)pData);
		*pResult = 0;
		return;
	}

	// TODO: 在此添加控件通知处理程序代码
	*pResult = 1;
}


void CMainDlg::OnTmiDel()
{
	HTREEITEM hSelTree = m_treeDevice.GetSelectedItem();
	if (hSelTree != NULL)
	{
		HTREEITEM hChild = m_treeDevice.GetChildItem(hSelTree);
		CString strMsg = (hChild == NULL ? "确定要删除此项吗?" : "确定要删除此项和它的子项吗?");
		if (MessageBox(strMsg, "警告!", MB_YESNO) == IDNO)
			return;

		HTREEITEM hRoot = m_treeDevice.GetRootItem();

		m_treeDevice.DelAllItemData(hSelTree);
		m_treeDevice.DeleteItem(hSelTree);

		if (hRoot == hSelTree) //如果删除的是RootItem，则创建默认的RootItem。
			CreateRootItem();
	}
}

void CMainDlg::OnUpdateTmiDel(CCmdUI *pCmdUI)
{
	HTREEITEM hSelTree = m_treeDevice.GetSelectedItem();
	BOOL bEnable = FALSE;
	if (hSelTree != NULL)
	{
		bEnable = TRUE;
		if (m_treeDevice.GetRootItem() == hSelTree && 
			m_treeDevice.GetChildItem(hSelTree) == NULL)
			bEnable = FALSE;
	}
	else
	{
		bEnable = FALSE;
	}

	pCmdUI->Enable(bEnable);
}

void CMainDlg::InitDeviceInfo()
{
	//读取DBF
	CAccessDB accessDB(m_hWnd);
	std::string strDBPath = CFilePath::GetExeFilePath();
	strDBPath += "\\QuoteConsole.mdb";
	if (!accessDB.Open(strDBPath.c_str()))
	{
		REPORT_RUN(MN, T("打开数据库[%s]失败!\n", strDBPath.c_str()), RPT_ERROR, return);
	}
	CRecordSet Rset(&accessDB);
	//Read GroupInfo
	CString strSql = "SELECT* FROM GroupInfo";
	if (!Rset.Open(strSql))
	{
		REPORT_RUN(MN, T("打开数据表[GroupInfo]失败!\n"), RPT_ERROR, return);
	}
	while(!Rset.IsEOF())
	{
		tagGroupData* pGroupData = new tagGroupData;
		Rset.GetCollect("Name", pGroupData->szName, 50);
		Rset.GetCollect("GroupID", pGroupData->nID);
		Rset.GetCollect("ParentID", pGroupData->nParentID);
		m_mapGroupData.insert(std::make_pair(pGroupData->nID, pGroupData));

		Rset.MoveNext();
	}
	Rset.Close();

	//Read DeviceInfo
	strSql = "SELECT* FROM DeviceInfo";
	if (!Rset.Open(strSql))
	{
		REPORT_RUN(MN, T("打开数据表[DeviceInfo]失败!\n"), RPT_ERROR, return);
	}
	while(!Rset.IsEOF())
	{
		tagDevData* pDevData = new tagDevData;
		Rset.GetCollect("Name", pDevData->szName, 50);
		Rset.GetCollect("Path", pDevData->szPath, MAX_PATH);
		Rset.GetCollect("Param", pDevData->szParam, 255);
		Rset.GetCollect("Remark", pDevData->szRemark, 255);
		Rset.GetCollect("DeviceID", pDevData->nID);
		Rset.GetCollect("ParentID", pDevData->nParentID);
		Rset.GetCollect("xmlPath", pDevData->szXmlPath, 1024);
		pDevData->tmPreCheck = 0;
		m_mapDevData.insert(std::make_pair(pDevData->nID, pDevData));

		Rset.MoveNext();
	}
	Rset.Close();

	//Read DevServerInfo
	strSql = "SELECT* FROM DevServerInfo";
	if (!Rset.Open(strSql))
	{
		REPORT_RUN(MN, T("打开数据表[DevServerInfo]失败!\n"), RPT_ERROR, return);
	}
	while(!Rset.IsEOF())
	{
		tagDevServData* pDevServData = new tagDevServData;
		Rset.GetCollect("DevServerID", pDevServData->nID);
		Rset.GetCollect("Name", pDevServData->szName, 50);
		Rset.GetCollect("IP", pDevServData->szIP, 20);
		Rset.GetCollect("ParentID", pDevServData->nParentID);
		Rset.GetCollect("Remark", pDevServData->szRemark, 255);
		Rset.GetCollect("xmlPath", pDevServData->szXmlPath, 1024);
		m_mapDevServData.insert(std::make_pair(pDevServData->nID, pDevServData));

		Rset.MoveNext();
	}
	Rset.Close();

	accessDB.Close();
	//初始化设备
	HTREEITEM hRoot = NULL;
	if (m_mapGroupData.size() == 0) //如果没有RootItem，则创建默认的RootItem
		hRoot = CreateRootItem();
	else
	{
		CGroupDataMap::iterator find = m_mapGroupData.find(ROOT_ID);
		tagGroupData* pGroupData = find->second;
		hRoot = m_treeDevice.InsertItem(pGroupData->szName, 0, 1);
		pGroupData->hItem = hRoot;
		m_treeDevice.SetItemData(hRoot, (DWORD_PTR)pGroupData);
		m_treeDevice.SelectItem(hRoot);
	}

	//InitGroup
	InitGroupData(hRoot, ROOT_ID);
	InitDevServData();
	InitDevData();
	m_treeDevice.GetAllItemData(hRoot);
}

void CMainDlg::InitGroupData(HTREEITEM hItem, UINT nID)
{
	CGroupDataMap::iterator pos = m_mapGroupData.begin();
	while(pos != m_mapGroupData.end())
	{
		tagGroupData* pGroupData = pos->second;
		if (pGroupData->nParentID == nID)
		{
			HTREEITEM hChild = m_treeDevice.InsertItem(pGroupData->szName, 0, 1, hItem);
			pGroupData->hItem = hChild;
			m_treeDevice.SetItemData(hChild, (DWORD_PTR)pGroupData);

			InitGroupData(hChild, pGroupData->nID);
		}
		pos++;
	}
}

void CMainDlg::InitDevServData()
{
	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	while(pos != m_mapDevServData.end())
	{
		tagDevServData* pDevServData = pos->second;
		CGroupDataMap::iterator find = m_mapGroupData.find(pDevServData->nParentID);
		ASSERT(find != m_mapGroupData.end());

		if (find != m_mapGroupData.end())
		{
			CString strText;
			strText.Format("%s[%s]", pDevServData->szName, pDevServData->szIP);
			HTREEITEM hNewTree = m_treeDevice.InsertItem(strText, 5, 5, find->second->hItem);
			pDevServData->hItem = hNewTree;
			m_treeDevice.SetItemData(hNewTree, (DWORD_PTR)pDevServData);
		}
		pos++;
	}
}

void CMainDlg::InitDevData()
{
	CDevDataMap::iterator pos = m_mapDevData.begin();
	while(pos != m_mapDevData.end())
	{
		tagDevData* pDevData = pos->second;
		CDevServDataMap::iterator find = m_mapDevServData.find(pDevData->nParentID);
		ASSERT(find != m_mapDevServData.end());

		if (find != m_mapDevServData.end())
		{
			CString strText;
			strText.Format("%s", pDevData->szName);
			HTREEITEM hNewTree = m_treeDevice.InsertItem(strText, 2, 2, find->second->hItem);
			pDevData->hItem = hNewTree;
			m_treeDevice.SetItemData(hNewTree, (DWORD_PTR)pDevData);
		}
		pos++;
	}
}

void CMainDlg::SaveDevInfo()
{
	//保存数据到dbf
	CAccessDB accessDB(m_hWnd);
	std::string strDBPath = CFilePath::GetExeFilePath();
	strDBPath += "\\QuoteConsole.mdb";
	if (!accessDB.Open(strDBPath.c_str()))
	{
		REPORT_RUN(MN, T("打开数据库[%s]失败!\n", strDBPath.c_str()), RPT_ERROR, return);
	}
	CString strSql = "DELETE FROM GroupInfo";
	accessDB.Execute(strSql);
	strSql = "DELETE FROM DeviceInfo";
	accessDB.Execute(strSql);
	strSql = "DELETE FROM DevServerInfo";
	accessDB.Execute(strSql);

	CRecordSet Rset(&accessDB);
	//Save GroupInfo
	{
		strSql = "SELECT* FROM GroupInfo";
		if (!Rset.Open(strSql))
		{
			REPORT_RUN(MN, T("打开数据表[GroupInfo]失败!\n"), RPT_ERROR, return);
		}

		CGroupDataMap::iterator pos = m_mapGroupData.begin();
		while(pos != m_mapGroupData.end())
		{
			tagGroupData* pGroupData = pos->second;
			Rset.AddNew();
			Rset.PutCollect("GroupID", pGroupData->nID);
			Rset.PutCollect("Name", pGroupData->szName);
			Rset.PutCollect("ParentID", pGroupData->nParentID);
			Rset.Update();
			pos++;
		}
		Rset.Close();
	}
	//Save DeviceInfo
	{
		strSql = "SELECT* FROM DeviceInfo";
		if (!Rset.Open(strSql))
		{
			REPORT_RUN(MN, T("打开数据表[DeviceInfo]失败!\n"), RPT_ERROR, return);
		}
		CDevDataMap::iterator pos = m_mapDevData.begin();
		while(pos != m_mapDevData.end())
		{
			tagDevData* pDevData = pos->second;
			Rset.AddNew();
			Rset.PutCollect("DeviceID", pDevData->nID);
			Rset.PutCollect("Name", pDevData->szName);
			Rset.PutCollect("Path", pDevData->szPath);
			Rset.PutCollect("Param", pDevData->szParam);
			Rset.PutCollect("ParentID", pDevData->nParentID);
			Rset.PutCollect("Remark", pDevData->szRemark);
			Rset.PutCollect("xmlPath", pDevData->szXmlPath);
			Rset.Update();
			pos++;
		}
		Rset.Close();
	}

	//Save DevServerInfo
	{
		strSql = "SELECT* FROM DevServerInfo";
		if (!Rset.Open(strSql))
		{
			REPORT_RUN(MN, T("打开数据表[DevServerInfo]失败!\n"), RPT_ERROR, return);
		}
		CDevServDataMap::iterator pos = m_mapDevServData.begin();
		while(pos != m_mapDevServData.end())
		{
			tagDevServData* pDevServData = pos->second;
			Rset.AddNew();

			Rset.PutCollect("DevServerID", pDevServData->nID);
			Rset.PutCollect("Name", pDevServData->szName);
			Rset.PutCollect("IP", pDevServData->szIP);
			Rset.PutCollect("ParentID", pDevServData->nParentID);
			Rset.PutCollect("Remark", pDevServData->szRemark);
			Rset.PutCollect("xmlPath", pDevServData->szXmlPath);
			Rset.Update();
			pos++;
		}
		Rset.Close();
	}
	accessDB.Close();
}

HTREEITEM CMainDlg::CreateRootItem()
{
	HTREEITEM hRoot = m_treeDevice.InsertItem("设备管理", 0, 1);
	tagGroupData* pGroupData = new tagGroupData;
	strcpy_s(pGroupData->szName, 50, "设备管理"); 
	pGroupData->nID = ROOT_ID;
	pGroupData->nParentID = -1;
	pGroupData->hItem = hRoot;
	m_mapGroupData.insert(std::make_pair(pGroupData->nID, pGroupData));
	m_treeDevice.SetItemData(hRoot, (DWORD_PTR)pGroupData);
	m_treeDevice.SelectItem(hRoot);
	return hRoot;
}

UINT CMainDlg::GetValidID(UINT nTreeDataType)
{
	while(true)
	{
		srand((int)time(0));
		UINT nID = rand() % 10000 + 1;

		if (nTreeDataType == TDT_GROUP)
		{
			CGroupDataMap::iterator find = m_mapGroupData.find(nID);
			if (find == m_mapGroupData.end())
				return nID;
		}
		else if (nTreeDataType == TDT_DEV_SERV)
		{
			CDevServDataMap::iterator find = m_mapDevServData.find(nID);
			if (find == m_mapDevServData.end())
				return nID;
		}
		else if (nTreeDataType == TDT_DEVICE)
		{
			CDevDataMap::iterator find = m_mapDevData.find(nID);
			if (find == m_mapDevData.end())
				return nID;
		}
	}
}

LRESULT	CMainDlg::OnDelItemData(WPARAM wParam, LPARAM lParam)
{
	tagData* pData = (tagData*)wParam;
	ASSERT(pData != NULL);
	if (pData->nDataType == TDT_GROUP)
	{
		CGroupDataMap::iterator find = m_mapGroupData.find(pData->nID);
		if (find != m_mapGroupData.end())
			m_mapGroupData.erase(find);
	}
	else if (pData->nDataType == TDT_DEV_SERV)
	{
		CDevServDataMap::iterator find = m_mapDevServData.find(pData->nID);
		if (find != m_mapDevServData.end())
			m_mapDevServData.erase(find);
		
		UpdateDevServCombo((tagDevServData*)wParam, FALSE);
	}
	else if (pData->nDataType == TDT_DEVICE)
	{
		CDevDataMap::iterator find = m_mapDevData.find(pData->nID);
		if (find != m_mapDevData.end())
			m_mapDevData.erase(find);
		
		UpdateDevCombo((tagDevData*)wParam, FALSE);
	}

	return 0;
}

LRESULT CMainDlg::OnLogEvent(WPARAM wParam, LPARAM lParam)
{				
	REPORT_RUN(MN, T("(LPCTSTR)lParam\n"), wParam, return 0);
}

LRESULT CMainDlg::OnGetItemData(WPARAM wParam, LPARAM lParam)
{
	tagData* pData = (tagData*)wParam;
	ASSERT(pData != NULL);
	int nIndex = m_listState.GetItemCount();
	if (pData->nDataType == TDT_GROUP)
	{
	}
	else if (pData->nDataType == TDT_DEV_SERV) //添加设备服务器到列表
	{
		if (m_nCurSelType == TDT_GROUP || m_nCurSelType == TDT_DEV_SERV) //如果选择的是分组
		{
			tagDevServData* pDevServData = (tagDevServData*)wParam;
			m_listState.InsertItem(nIndex, pData->szName, pDevServData->bDevStarted ? 4:5);
			m_listState.SetItemData(nIndex, (DWORD_PTR)pDevServData);
			
			m_listState.SetItemText(nIndex, 1, pDevServData->szIP);
			m_listState.SetItemText(nIndex, 2, pDevServData->bDevStarted ? "已启动":"已停止");
			//m_listState.SetItemText(nIndex, 3, pDevServData->szRemark);
		}
	}
	else if (pData->nDataType == TDT_DEVICE) //添加设备到列表
	{
		if (m_nCurSelType == TDT_DEVICE) 
		{
			tagDevData* pDevData = (tagDevData*)wParam;
			m_listState.InsertItem(nIndex, pData->szName, pDevData->bDevStarted ? 3:2);
			m_listState.SetItemData(nIndex, (DWORD_PTR)pDevData);

			m_listState.SetItemText(nIndex, 1, pDevData->bDevStarted ? "已启动":"已停止");

			m_listState.SetItemText(nIndex, 2, pDevData->szInfo);
			m_listState.SetItemText(nIndex, 3, pDevData->szTime);
		}
	}

	if (m_nCurSelID == pData->nID)
	{
		int nCurSel = m_listState.GetSelectionMark();
		m_listState.SetItemState(nIndex, LVIS_SELECTED ,LVIS_SELECTED|LVIS_FOCUSED);
		m_listState.SetSelectionMark(nIndex);
		m_listState.EnsureVisible(nIndex, TRUE);
	}
	return 0;
}

void CMainDlg::OnTmiModefydev()
{
	HTREEITEM hSelItem = m_treeDevice.GetSelectedItem();
	if (m_nModifyType == TDT_DEV_SERV)
	{
		CDlg_DevServInfo dlg;
		dlg.m_bAddDevServ = FALSE;

		tagDevServData* pDevServData = (tagDevServData*)m_treeDevice.GetItemData(hSelItem);
		dlg.m_strDevServName = pDevServData->szName;
		dlg.m_strDevServIP   = pDevServData->szIP;
		dlg.m_strRemark      = pDevServData->szRemark;
		dlg.m_nDDevServID    = pDevServData->nID; 

		if (dlg.DoModal() != IDOK)
			return;

		strcpy_s(pDevServData->szName, 50, dlg.m_strDevServName); 
		strcpy_s(pDevServData->szIP, 20, dlg.m_strDevServIP);
		strcpy_s(pDevServData->szRemark, 255, dlg.m_strRemark);

		CString strText;
		strText.Format("%s[%s]", dlg.m_strDevServName, dlg.m_strDevServIP);
		m_treeDevice.SetItemText(hSelItem, strText);
	}
	else if (m_nModifyType == TDT_DEVICE)
	{
		CDlg_DevAttr dlg;
		dlg.m_bAddDev = FALSE;

		tagDevData* pDevData = (tagDevData*)m_treeDevice.GetItemData(hSelItem);
		dlg.m_strDevName = pDevData->szName;
		dlg.m_strPath    = pDevData->szPath;
		dlg.m_strRemark  = pDevData->szRemark;
		dlg.m_strParam   = pDevData->szParam;
		dlg.m_nParentID  = pDevData->nParentID;
		dlg.m_nDevID     = pDevData->nID;

		if (dlg.DoModal() != IDOK)
			return;

		strcpy_s(pDevData->szName, 50, dlg.m_strDevName); 
		strcpy_s(pDevData->szPath, MAX_PATH, dlg.m_strPath);
		strcpy_s(pDevData->szRemark, 255, dlg.m_strRemark);
		strcpy_s(pDevData->szParam, 255, dlg.m_strParam);

		m_treeDevice.SetItemText(hSelItem, dlg.m_strDevName);
	}

	OnSelchangedTreeCtrl(m_treeDevice.GetSelectedItem());  //更新列表中的数据
}

void CMainDlg::OnUpdateTmiModefydev(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bModifyDev);
}

void CMainDlg::OnTvnSelchangedTreeDevice(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	HTREEITEM hSelItem = pNMTreeView->itemNew.hItem;
	OnSelchangedTreeCtrl(hSelItem);

	EnableToolBar();
	*pResult = 0;
}

void CMainDlg::OnSelchangedTreeCtrl(HTREEITEM hSelItem)
{
	DWORD dwData = m_treeDevice.GetItemData(hSelItem);
	if (dwData == 0)
		return;

	tagData* pData = (tagData*)dwData;
	m_nCurSelID = -1;
	m_nCurSelType  = pData->nDataType;
	InsertListColumn(pData->nDataType);
	if (pData->nDataType == TDT_GROUP)  //获得选择项下的所有设备
	{
		m_treeDevice.GetAllItemData(hSelItem);
	}
	else if (pData->nDataType == TDT_DEV_SERV)
	{
		m_nCurSelID = pData->nID;
		HTREEITEM hParentItem = m_treeDevice.GetParentItem(hSelItem);
		m_treeDevice.GetAllItemData(hParentItem);
	}
	else if (pData->nDataType == TDT_DEVICE)
	{
		m_nCurSelID = pData->nID;
		HTREEITEM hParentItem = m_treeDevice.GetParentItem(hSelItem);
		m_treeDevice.GetAllItemData(hParentItem);
	}
}

void CMainDlg::EnableToolBar()
{
	(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_DOWNKLINE, FALSE);
	(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STARTDEV, FALSE);
	(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STOPDEV, FALSE);

	HTREEITEM hSelItem = m_treeDevice.GetSelectedItem();
	if (hSelItem == NULL)
		return;

	DWORD dwData = m_treeDevice.GetItemData(hSelItem);
	if (dwData == 0)
		return;

	tagDevData* pDevData = (tagDevData*)dwData;
	if (pDevData->nDataType != TDT_DEVICE)
		return;

	if (pDevData->bDevStarted)
	{
		(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STARTDEV, FALSE);
		(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STOPDEV, TRUE);
	}
	else
	{
		(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STARTDEV, TRUE);
		(m_wndToolBar.GetToolBarCtrl()).EnableButton(ID_BTN_STOPDEV, FALSE);
	}
}

void CMainDlg::OnTmiAdddevserv()
{
	HTREEITEM hSelTree = m_treeDevice.GetSelectedItem();
	if (hSelTree != NULL)
	{
		CDlg_DevServInfo dlg;
		dlg.m_bAddDevServ = TRUE;

		UINT nDevServID = GetValidID(TDT_DEV_SERV);
		dlg.m_nDDevServID = nDevServID;

		if (dlg.DoModal() != IDOK)
			return;

		CString strText;
		strText.Format("%s[%s]", dlg.m_strDevServName, dlg.m_strDevServIP);
		HTREEITEM hNewTree = m_treeDevice.InsertItem(strText, 5, 5, hSelTree);
		m_treeDevice.Expand(hSelTree, TVE_EXPAND);
		m_treeDevice.SelectItem(hNewTree);

		tagDevServData* pDevServData = new tagDevServData;
		strcpy_s(pDevServData->szName, 50, dlg.m_strDevServName); 
		strcpy_s(pDevServData->szIP, 20, dlg.m_strDevServIP);
		strcpy_s(pDevServData->szRemark, 255, dlg.m_strRemark);
		pDevServData->nID = nDevServID;
		pDevServData->hItem = hNewTree;

		tagData* pData = (tagData*)m_treeDevice.GetItemData(hSelTree);
		pDevServData->nParentID = pData->nID;
		m_mapDevServData.insert(std::make_pair(pDevServData->nID, pDevServData));
		m_treeDevice.SetItemData(hNewTree, (DWORD_PTR)pDevServData);

		UpdateDevServCombo(pDevServData);
	}
}

void CMainDlg::OnUpdateTmiAdddevserv(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAddDevServ);
}

void CMainDlg::InsertListColumn(UINT nTreeDataType)
{
	m_listState.DeleteAllItems();
	CHeaderCtrl* pHdrCtrl = m_listState.GetHeaderCtrl();
	ASSERT(pHdrCtrl != NULL);
	//保存宽度
	int nColumnCount = pHdrCtrl->GetItemCount();
	if (nColumnCount == 4)
	{
		for (int i=0; i<nColumnCount; i++)
		{
			theApp.m_nlstDevClmW[i] = m_listState.GetColumnWidth(i);
		}
	}
	else if (nColumnCount == 3)
	{
		for (int i=0; i<nColumnCount; i++)
		{
			theApp.m_nlstDevServClmW[i] = m_listState.GetColumnWidth(i);
		}
	}
	//
	while(pHdrCtrl->GetItemCount())
	{
		m_listState.DeleteColumn(0);
	}

	int nIndex = 0;
	if (nTreeDataType == TDT_GROUP || nTreeDataType == TDT_DEV_SERV)
	{
		m_listState.InsertColumn(nIndex++, "设备服务器名称", LVCFMT_RIGHT, theApp.m_nlstDevServClmW[0]);
		m_listState.InsertColumn(nIndex++, "IP", LVCFMT_CENTER, theApp.m_nlstDevServClmW[1]);
		m_listState.InsertColumn(nIndex++, "状  态", LVCFMT_CENTER, theApp.m_nlstDevServClmW[2]);
	}
	else if (nTreeDataType == TDT_DEVICE)
	{
		m_listState.InsertColumn(nIndex++, "设备名称", LVCFMT_RIGHT, theApp.m_nlstDevClmW[0]);
		m_listState.InsertColumn(nIndex++, "状  态", LVCFMT_CENTER, theApp.m_nlstDevClmW[1]);
		m_listState.InsertColumn(nIndex++, "信  息", LVCFMT_LEFT, theApp.m_nlstDevClmW[2]);
		m_listState.InsertColumn(nIndex++, "时  间", LVCFMT_LEFT, theApp.m_nlstDevClmW[3]);
	}
}

LRESULT CMainDlg::OnDevServLogin(WPARAM wParam, LPARAM lParam)
{
	tagConnect* pCon = (tagConnect*)wParam;
	UpdateDevServState(pCon);
	return 0;
}

LRESULT CMainDlg::OnDevServLogout(WPARAM wParam, LPARAM lParam)
{
	tagConnect* pCon = (tagConnect*)wParam;
	UpdateDevServState(pCon, FALSE);
	DevServAllDevLogout(pCon->szIP);
	return 0;
}

void CMainDlg::DevServAllDevLogout(CString strDevServIP)
{
	int nDevServID = -1;
	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	while(pos != m_mapDevServData.end())
	{		
		if (strDevServIP.CompareNoCase(pos->second->szIP) == 0)
		{
			nDevServID = pos->second->nID;
			break;
		}
		pos++;
	}

	if (nDevServID != -1)
	{
		CDevDataMap::iterator posDev = m_mapDevData.begin();
		while(posDev != m_mapDevData.end())
		{		
			if (nDevServID == posDev->second->nParentID && posDev->second->bDevStarted)
			{
				CString strDevName = posDev->second->szName;
				SendMessage(WM_DEVLOGOUT, (WPARAM)strDevServIP.GetBuffer(), 
					(LPARAM)&strDevName);
				strDevServIP.ReleaseBuffer();
			}
			posDev++;
		}
	}
}

void CMainDlg::UpdateDevServState(tagConnect* pCon, BOOL bStart)
{
	CString strIP = pCon->szIP;
	UINT nPort	  = pCon->nPort;

	BOOL bFind = FALSE;
	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	char szName[256] = {0};
	struct hostent*  pHost;
	if (gethostname(szName, 256) == 0)
	{
		pHost = gethostbyname(szName);
	}
	
	CString strIP1, strIP2;
	while(pos != m_mapDevServData.end())
	{
		tagDevServData* pDevServData = pos->second;
		CString strDevServIP = pDevServData->szIP;
		BOOL bFindDevServ = FALSE;
		if (strDevServIP.CompareNoCase(strIP) == 0)
		{
			bFindDevServ = TRUE;
			strIP1 = strDevServIP;
		}
		else if (strIP.Compare("127.0.0.1") == 0)
		{
			for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
			{
				LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
				if (strDevServIP.CompareNoCase(lpIPAddr) == 0)
				{
					bFindDevServ = TRUE;
					strIP1 = "127.0.0.1";
					strIP2 = strDevServIP;
					break;
				}
			}
		}
		else if (strDevServIP.Compare("127.0.0.1") == 0)
		{
			for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
			{
				LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
				if (strIP.CompareNoCase(lpIPAddr) == 0)
				{
					bFindDevServ = TRUE;
					strIP1 = "127.0.0.1";
					strIP2 = strIP;
					break;
				}
			}
		}

		if (bFindDevServ)
		{
			if (bStart)	//登录
			{
				/*if (pDevServData->bDevStarted) //已经登录
				{
					REPORT(MN, T("设备服务器[%s:%s]已经登录，不能再次登录\n", 
						pDevServData->szName, pDevServData->szIP), RPT_WARNING);
					pDevServData->nPort = nPort;
					pDevServData->hSock = pCon->hSock;
					return;
				}
				else*/
				{
					REPORT(MN, T("设备服务器[%s:%s]登录\n", pDevServData->szName, 
						pDevServData->szIP), RPT_INFO);
					pDevServData->nPort = nPort;
					pDevServData->hSock = pCon->hSock;
				}
			} 
			else	   //退出
			{
				if (nPort != pDevServData->nPort)
					return;

				pDevServData->hSock = INVALID_SOCKET;
				REPORT(MN, T("设备服务器[%s:%s]退出\n", pDevServData->szName, 
					pDevServData->szIP), RPT_INFO);
				pDevServData->szVersion[0] = 0;
			}

			pDevServData->bDevStarted = bStart;
			TVITEM item = {0};
			item.mask   = TVIF_IMAGE|TVIF_HANDLE|TVIF_SELECTEDIMAGE;
			item.hItem  = pDevServData->hItem;
			item.iImage = item.iSelectedImage = bStart ? 4:5;
			m_treeDevice.SetItem(&item);
			bFind = TRUE;
			break;
		}
		pos++;
	}

	if (!bFind)
	{
		if (bStart)
			REPORT(MN, T("没有注册的设备服务器[%s:%d]已连接\n", strIP, nPort), RPT_WARNING);
		else
			REPORT(MN, T("没有注册的设备服务器[%s:%d]已退出\n", strIP, nPort), RPT_WARNING);
		return;
	}

	for (int i=0; i<m_listState.GetItemCount(); i++)
	{
		tagDevServData* pDevServData = (tagDevServData*)m_listState.GetItemData(i);
		ASSERT(pDevServData != NULL);
		if (pDevServData->nDataType != TDT_DEV_SERV)
			continue;

		CString strDevServIP = pDevServData->szIP;
		if (strDevServIP.CompareNoCase(strIP1) == 0 || strDevServIP.CompareNoCase(strIP2) == 0)
		{
			LVITEM item = {0};
			item.mask = LVIF_IMAGE;
			item.iItem = i;
			item.iImage = bStart ? 4:5; 
			m_listState.SetItem(&item);
			m_listState.SetItemText(i, 2, pDevServData->bDevStarted ? "已启动":"已停止");
		}
	}
}

UINT CMainDlg::IsDevValid(CString strDevServIP, CString* pDevName)
{
	int nDevServID = -1;		
	{
		char szName[256] = {0};
		struct hostent*  pHost;
		if (gethostname(szName, 256) == 0)
		{
			pHost = gethostbyname(szName);
		}

		CDevServDataMap::iterator pos = m_mapDevServData.begin();
		while(pos != m_mapDevServData.end())
		{
			CString strIP = pos->second->szIP;
			if (strDevServIP.CompareNoCase(strIP) == 0)
			{
				nDevServID = pos->second->nID;
				break;
			}
			else if (strIP.Compare("127.0.0.1") == 0)
			{
				for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
				{
					LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
					if (strDevServIP.CompareNoCase(lpIPAddr) == 0)
					{
						nDevServID = pos->second->nID;
						break;
					}
				}
			}
			else if (strDevServIP.Compare("127.0.0.1") == 0)
			{
				for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
				{
					LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
					if (strIP.CompareNoCase(lpIPAddr) == 0)
					{
						nDevServID = pos->second->nID;
						break;
					}
				}
			}
			pos++;
		}
	}

	if (nDevServID == -1)
		return -1;

	{
		CDevDataMap::iterator pos = m_mapDevData.begin();
		while(pos != m_mapDevData.end())
		{
			CString strName = pos->second->szName;
			if (pDevName->CompareNoCase(strName) == 0 && pos->second->nParentID == nDevServID)
			{
				return pos->second->nID;
			}
			pos++;
		}
	}

	return -1;
}

LRESULT CMainDlg::OnUpdateDevStatus(WPARAM wParam, LPARAM lParam)
{			   
	CString strDevList = *((CString*)wParam);
	CString strIP = (char*)lParam;
	CString strDevName;

	CLoginDevMap::iterator oldLoginDev = m_mapLoginDev.find(strIP);
	if (oldLoginDev == m_mapLoginDev.end())
	{
		m_mapLoginDev.insert(std::make_pair(strIP, strDevList));
	}
	else
	{
		CString strOldDevList = oldLoginDev->second;

		int nPos = strOldDevList.Find('$');
		CString strProID;
		int nBeginPos  = 0;
		int nMidPos = 0;
		int nEndPos	= 0;
		CString strCurDev;
		while(nPos != -1)
		{
			nBeginPos = nPos;
			nMidPos = strOldDevList.Find('#', nBeginPos);
			strDevName = strOldDevList.Mid(nBeginPos+1, nMidPos - nBeginPos - 1);
			nEndPos = strOldDevList.Find('$', nMidPos);
			strProID= strOldDevList.Mid(nMidPos+1, nEndPos - nMidPos - 1);
			nPos = strOldDevList.Find('$', nEndPos+1);

			strCurDev = "$"+strDevName+"#"+strProID+"$";
			if (strDevList.Find(strCurDev) >= 0)	//找到设备
				continue;
				
			tagDevLogin devLogin = {0};
			strcpy_s(devLogin.szIP, 20, (char*)lParam);
			devLogin.dwProID = atoi(strProID);

			if (0 == SendMessage(WM_DEVLOGOUT, lParam, (LPARAM)&strDevName))
			{
				REPORT(MN, T("没有注册的设备[%s:%s]退出\n", (char*)lParam, strDevName), RPT_WARNING);
			}  
		}

		m_mapLoginDev[strIP] = strDevList;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	int nPos = strDevList.Find('$');
	CString strProID;
	int nBeginPos  = 0;
	int nMidPos = 0;
	int nEndPos	= 0;
	while(nPos != -1)
	{
		nBeginPos = nPos;
		nMidPos = strDevList.Find('#', nBeginPos);
		strDevName = strDevList.Mid(nBeginPos+1, nMidPos - nBeginPos - 1);
		nEndPos = strDevList.Find('$', nMidPos);
		strProID= strDevList.Mid(nMidPos+1, nEndPos - nMidPos - 1);
		nPos = strDevList.Find('$', nEndPos+1);

		tagDevLogin devLogin = {0};
		strcpy_s(devLogin.szIP, 20, (char*)lParam);
		devLogin.dwProID = atoi(strProID);

		if (0 == SendMessage(WM_DEVLOGIN, (WPARAM)&devLogin,
			(LPARAM)&strDevName))
		{
			REPORT(MN, T("没有注册的设备[%s:%s]登录\n", (char*)lParam, strDevName), RPT_WARNING);
		} 
	}

	return 0;
}

LRESULT CMainDlg::OnDevLogin(WPARAM wParam, LPARAM lParam)
{						
	tagDevLogin* pDevLogin = (tagDevLogin*)wParam;
	ASSERT(pDevLogin != NULL);

	UINT nDevID = IsDevValid(pDevLogin->szIP, (CString*)lParam);
	if (nDevID != -1)
	{
		UpdateDevState(nDevID, pDevLogin->dwProID);
		EnableToolBar();
		return 1;
	}

	return 0;
}

LRESULT CMainDlg::OnDevLogout(WPARAM wParam, LPARAM lParam)
{
	UINT nDevID = IsDevValid((char*)wParam, (CString*)lParam);
	if (nDevID != -1)
	{
		UpdateDevState(nDevID, 0, FALSE);
		EnableToolBar();
		return 1;
	}

	return 0;
}

void CMainDlg::UpdateDevState(UINT nDevID, DWORD dwProID, BOOL bStart, BOOL bReport)
{
	CDevDataMap::iterator find = m_mapDevData.find(nDevID);
	if (find == m_mapDevData.end())
		return;

	tagDevData* pDevData = find->second;
	ASSERT(pDevData);

	if (bStart)							  //可能出现同一个设备多次登录的现象
	{
		if (pDevData->bDevStarted && pDevData->dwProID == dwProID)
			return;
		if (pDevData->bDevStarted && pDevData->dwProID != dwProID)
		{
			if (bReport)
				REPORT(MN, T("设备[%s:0x%x]已经登录，同一个设备服务器下不能有相同名称的设备登录\n", pDevData->szName, dwProID), RPT_WARNING);
			return;
		}
	
		pDevData->dwProID = dwProID;
		if (bReport)  //在OnAddLog中调用UpdateDevState时不能使用REPORT, 会锁住   m_csLog.Lock();
			REPORT(MN, T("设备[%s:0x%x]登录\n", pDevData->szName, dwProID), RPT_INFO);

		pDevData->bDevStarted = TRUE;
		pDevData->tmPreCheck = time(NULL);
	}
	else
	{
		if (pDevData->bDevStarted)
		{
			if (bReport)
				REPORT(MN, T("设备[%s:0x%x]退出\n", pDevData->szName, pDevData->dwProID), RPT_INFO);
		}

		pDevData->bDevStarted = FALSE;
		pDevData->tmPreCheck = 0;
	}

	TVITEM item = {0};
	item.mask   = TVIF_IMAGE|TVIF_HANDLE|TVIF_SELECTEDIMAGE;
	item.hItem  = pDevData->hItem;
	item.iImage = item.iSelectedImage = pDevData->bDevStarted ? 3:2;
	m_treeDevice.SetItem(&item);

	for (int i=0; i<m_listState.GetItemCount(); i++)
	{
		tagDevData* pItemDevData = (tagDevData*)m_listState.GetItemData(i);
		ASSERT(pItemDevData != NULL);
		if (pItemDevData->nDataType != TDT_DEVICE)
			continue;

		if (pItemDevData->nID == nDevID)
		{
			LVITEM item = {0};
			item.mask = LVIF_IMAGE;
			item.iItem = i;
			item.iImage = pItemDevData->bDevStarted ? 3:2; 
			m_listState.SetItem(&item);
			m_listState.SetItemText(i, 1, pItemDevData->bDevStarted ? "已启动":"已停止");
		}
	}

	//判断是否重启设备
	if (pDevData->bRestart)
	{
		pDevData->bRestart = FALSE;
		CDevServDataMap::iterator find = m_mapDevServData.find(pDevData->nParentID);
		if (find != m_mapDevServData.end())
		{
			REPORT(MN, T("重启设备[%s]\n", pDevData->szName), RPT_INFO);
			StartStopApp(find->second->hSock, pDevData->szPath, pDevData->szParam, 
				pDevData->dwProID, TRUE);
		}
	}
}

BOOL CMainDlg::IsDevServExist(CString strDevServIP, UINT nCurDevServID)
{
	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	while(pos != m_mapDevServData.end())
	{
		if (pos->first == nCurDevServID)
		{
			pos++;
			continue;
		}

		CString strIP = pos->second->szIP;
		if (strDevServIP.CompareNoCase(strIP) == 0)
			return TRUE;
		pos++;
	}

	return FALSE;
}

BOOL CMainDlg::IsDevExist(CString strDevName, UINT nParentID, UINT nDevID)
{
	CDevDataMap::iterator pos = m_mapDevData.begin();
	while(pos != m_mapDevData.end())
	{
		if (nParentID != pos->second->nParentID || nDevID == pos->first)
		{
			pos++;
			continue;
		}

		CString strName = pos->second->szName;
		if (strDevName.CompareNoCase(strName) == 0)
			return TRUE;

		pos++;
	}

	return FALSE;
}

void CMainDlg::OnAddLog(WORD wLogType, tagLog* pLog)
{
	/*if (m_bShowDequeLog)		 //显示队列中未显示到界面的日志
	{
		m_bShowDequeLog = FALSE;
		OnBnClickedCheckUse();
	}

	if ((wLogType & 0xFF) != RPT_HEARTBEAT &&  //过滤日志
		(wLogType & 0xFF) != RPT_ADDI_INFO &&
		(wLogType & 0xFF) != RPT_VERSION   &&
		(wLogType & 0xFF) != RPT_RESTART)
	{
		m_csLog.Lock();
		m_deqLog.PushBack(pLog);
		m_csLog.Unlock();
	}*/
	
	AddValidDataToLstMsg(pLog);
	//添加附加信息
	if ((wLogType & 0xFF) != RPT_HEARTBEAT &&
		(wLogType & 0xFF) != RPT_ADDI_INFO &&
		(wLogType & 0xFF) != RPT_VERSION   &&
		(wLogType & 0xFF) != RPT_RESTART)
		return;

	time_t tmCur = time(NULL);
	int nDevServID = -1;
	SOCKET hServSock = INVALID_SOCKET;
	{
		char szName[256] = {0};
		struct hostent*  pHost;
		if (gethostname(szName, 256) == 0)
		{
			pHost = gethostbyname(szName);
		}
		CDevServDataMap::iterator pos = m_mapDevServData.begin();
		while(pos != m_mapDevServData.end())
		{
			tagDevServData* pDevServData = pos->second;
			CString strDevServIP = pDevServData->szIP;
			CString strIP = pLog->szDevServIP;
			if (strDevServIP.CompareNoCase(strIP) == 0)
			{
				nDevServID = pos->first;
				hServSock = pos->second->hSock;
				break;
			}
			else if (strIP.Compare("127.0.0.1") == 0)
			{
				for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
				{
					LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
					if (strDevServIP.CompareNoCase(lpIPAddr) == 0)
					{
						nDevServID = pos->first;
						hServSock = pos->second->hSock;
						break;
					}
				}
			}
			else if (strDevServIP.Compare("127.0.0.1") == 0)
			{
				for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
				{
					LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
					if (strIP.CompareNoCase(lpIPAddr) == 0)
					{
						nDevServID = pos->first;
						hServSock = pos->second->hSock;
						break;
					}
				}
			}
			pos++;
		}
	}

	if (nDevServID == -1)
		return;

	{
		CDevDataMap::iterator pos = m_mapDevData.begin();
		CString strDevName;
		while(pos != m_mapDevData.end())
		{
			tagDevData* pDevData = pos->second;
			strDevName = pDevData->szName;
			if (strDevName.CompareNoCase(pLog->szDevName) == 0 &&
				nDevServID == pDevData->nParentID)
			{
				if ((wLogType & 0xFF) == RPT_HEARTBEAT)
				{
					pDevData->tmPreCheck = tmCur;
					if (!pDevData->bDevStarted && !pDevData->bRestart) //设备超时状态改为关闭，当有心跳日志时，应重新设为启动状态
					{
						pDevData->bTODevHeartBeat = TRUE;
						UpdateDevState(pDevData->nID, pDevData->dwProID, TRUE, FALSE);
					}
				}
				else if ((wLogType & 0xFF) == RPT_ADDI_INFO)
				{
					strcpy_s(pDevData->szInfo, 1024, pLog->szMsg);
					strcpy_s(pDevData->szTime, 50, pLog->szTime);
					
					UpdateDevAddiInfo(pDevData); //更新信息
				}
				else if ((wLogType & 0xFF) == RPT_VERSION)
				{
					strcpy_s(pDevData->szVersion, 500, pLog->szMsg);
				}
				else if ((wLogType & 0xFF) == RPT_RESTART)
				{
					CString strDevPath = pLog->szMsg;
					StartStopApp(hServSock, strDevPath + "\\QuoteFarm.exe", "", 0, FALSE);
					StartStopApp(hServSock, strDevPath + "\\QuoteFeed.exe", "", 0, FALSE);
					StartStopApp(hServSock, strDevPath + "\\QuoteServer.exe", "", 0, FALSE);
					Sleep(5000);
					StartStopApp(hServSock, strDevPath + "\\QuoteFarm.exe", "", 0xFFFFFFFF, FALSE);  //删除dat文件
					Sleep(3000);
					StartStopApp(hServSock, strDevPath + "\\QuoteFarm.exe", "-o", 0, TRUE);
					Sleep(3000);
					StartStopApp(hServSock, strDevPath + "\\QuoteFeed.exe", "-o", 0, TRUE);
					Sleep(3000);
					StartStopApp(hServSock, strDevPath + "\\QuoteServer.exe", "-o", 0, TRUE);
				}

				break;
			}
			pos++;
		}
	}

	{
		if ((wLogType & 0xFF) == RPT_VERSION && _stricmp(pLog->szModule, "WatchDog") == 0)	 //设备服务器的版本信息
		{
			CDevServDataMap::iterator pos = m_mapDevServData.begin();
			while(pos != m_mapDevServData.end())
			{
				if (_stricmp(pLog->szDevServIP, pos->second->szIP) == 0)
				{
					strcpy_s(pos->second->szVersion, 100, pLog->szMsg);
					break;
				}
				pos++;
			}
		}
	}

	return;
}


void CMainDlg::AddValidDataToLstMsg(tagLog* pLog, BOOL bInsertBegin)
{
	if (_stricmp(pLog->szLogType, "RPT_HEARTBEAT") == 0)
		return;

	int nItemCount = 0;
	//*
	//设备服务器
	if (!pLog->bLocal)
	{
		nItemCount = m_cDevServ.GetCount();
		std::map<CString,char*> mapDevServ;      

		CString strIp;
		for (int i=0; i<nItemCount; i++)
		{
			if (!m_cDevServ.GetCheck(i))
				continue;

			tagDevServData* pDevServData = (tagDevServData*)m_cDevServ.GetItemData(i);

			strIp = pDevServData->szIP;
			mapDevServ.insert(std::make_pair(strIp, pDevServData->szIP));
		}

		strIp = pLog->szDevServIP;
		std::map<CString,char*>::iterator find = mapDevServ.find(strIp);
		BOOL bFindServ = FALSE;
		char szName[256] = {0};
		struct hostent*  pHost;
		if (gethostname(szName, 256) == 0)
		{
			pHost = gethostbyname(szName);
		}
		if (find != mapDevServ.end())
			bFindServ = TRUE;
		else if (strIp.Compare("127.0.0.1") == 0)
		{
			for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
			{
				LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
				find = mapDevServ.find(lpIPAddr);
				if (find == mapDevServ.end())
				{
					bFindServ = TRUE;
					break;
				}
			}
		}
		else
		{
			find = mapDevServ.find("127.0.0.1");
			if (find != mapDevServ.end())
			{
				for(int i=0; pHost!=NULL && pHost->h_addr_list[i] !=NULL; i++)
				{
					LPCSTR lpIPAddr = inet_ntoa (*(struct in_addr *)pHost->h_addr_list[i]);
					if (strIp.Compare(lpIPAddr) == 0)
					{
						bFindServ = TRUE;
						break;
					}
				}
			}
		}

		if (!bFindServ)
		{
			mapDevServ.clear();
			return;
		}
		mapDevServ.clear();
	}
	//设备
	BOOL bDevValid = FALSE;
	CString strText;
	for (int i=0; i<m_cDev.GetCount(); i++)
	{
		m_cDev.GetLBText(i, strText);
		if (m_cDev.GetCheck(i) && strText.Compare(pLog->szDevName) == 0)
		{
			bDevValid = TRUE;
			break;
		}
	}
	if (_stricmp(pLog->szDevName, "WatchDog") != 0)
	{
		if (!bDevValid)
			return;
	}

	// 日志类型
	bDevValid = FALSE;
	for (int i=0; i<m_cLogType.GetCount(); i++)
	{
		m_cLogType.GetLBText(i, strText);
		if (m_cLogType.GetCheck(i) && strText.Compare(pLog->szLogType) == 0)
		{
			bDevValid = TRUE;
			break;
		}
	}
	if (!bDevValid)
		return;

	CTime tmStart;
	m_dtStart.GetTime(tmStart);
	CTime tmStop;
	m_dtStop.GetTime(tmStop);
	if (!IsTimeValid(pLog->szTime, tmStart, tmStop))
		return;
	//*/
	
	//return;
	//TRACE("%s\n", pLog->szMsg);
	m_bAddSpeed = ((CButton*)GetDlgItem(IDC_C_ADDSPEED))->GetCheck();
	if (m_bAddSpeed)
		m_listMsg.SetRedraw(FALSE);   

	nItemCount = m_listMsg.GetItemCount();
	if (nItemCount > (int)theApp.m_nMaxRecord)
		m_listMsg.DeleteItem(0);

	if (bInsertBegin)
		nItemCount = 0;
	else
		nItemCount = m_listMsg.GetItemCount();
		
    m_listMsg.InsertItem(nItemCount, pLog->szDevName);
	
	m_listMsg.SetItemText(nItemCount, 1, pLog->szTime);
	m_listMsg.SetItemText(nItemCount, 2, pLog->szLogType);
	m_listMsg.SetItemText(nItemCount, 3, pLog->szMsg);
	m_listMsg.SetItemText(nItemCount, 4, pLog->szModule);
	CString strTemp;
	strTemp.Format("0x%x", pLog->dwThreadID);
	m_listMsg.SetItemText(nItemCount, 5, strTemp);
	m_listMsg.SetItemText(nItemCount, 6, pLog->szLogPos);
	m_listMsg.SetItemText(nItemCount, 7, pLog->szIP);
	
	if (_stricmp(pLog->szLogType, "RPT_ERROR") == 0)
	{
		m_listMsg.SetItemBackColor(RGB(248,198,198), nItemCount);
	}
	else if (_stricmp(pLog->szLogType, "RPT_WARNING") == 0)
	{
		m_listMsg.SetItemBackColor(RGB(253,250,207), nItemCount);
		m_listMsg.SetItemTextColor(RGB(0,0,255), nItemCount);
	}
	else if (_stricmp(pLog->szLogType, "RPT_CRITICAL") == 0)
	{
		m_listMsg.SetItemBackColor(RGB(255,0,0), nItemCount);
		m_listMsg.SetItemTextColor(RGB(255,255,255), nItemCount);
	}

	if(m_listMsg.GetNextItem(-1, LVNI_SELECTED) == nItemCount -1)
	{
		m_listMsg.SetItemState(nItemCount, LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
		m_listMsg.EnsureVisible(nItemCount, FALSE);
	}
	m_dwPreLogTick = ::GetTickCount();
	m_bHasLog = TRUE;
}

BOOL CMainDlg::IsTimeValid(CString strTime, CTime tmStart, CTime tmStop)
{
	UINT nStart = tmStart.GetHour()*10000 + tmStart.GetMinute()*100 + tmStart.GetSecond();
	UINT nStop  = tmStop.GetHour()*10000 + tmStop.GetMinute()*100 + tmStop.GetSecond();

	CString strHour = strTime.Left(strTime.Find(':'));
	strHour = strHour.Right(2);
	CString strMin = strTime.Mid(strTime.Find(':')+1, 2);
	CString strSec = strTime.Mid(strTime.ReverseFind(':')+1, 2);
	UINT nTime = atoi(strHour)*10000 + atoi(strMin)*100 + atoi(strSec);

	if (nTime >= nStart && nTime <= nStop)
		return TRUE;
	else 
		return FALSE;
}

void CMainDlg::UpdateDevAddiInfo(tagDevData* pDevData)
{
	for (int i=0; i<m_listState.GetItemCount(); i++)
	{
		tagDevData* pItemDevData = (tagDevData*)m_listState.GetItemData(i);
		if (pItemDevData->nDataType != TDT_DEVICE)
			continue;

		if (pItemDevData != pDevData)
			continue;

		m_listState.SetItemText(i, 2, pDevData->szInfo);
		m_listState.SetItemText(i, 3, pDevData->szTime);
	}
}


void CMainDlg::OnBnClickedCheckUse()
{
	/*m_csLog.Lock();
	m_listMsg.DeleteAllItems();
	deque<tagLog*>* pdeq = m_deqLog.GetDeque();
	deque<tagLog*>::reverse_iterator pos = pdeq->rbegin();
	m_listMsg.SetItemCount(theApp.m_nMaxRecord);
	while(pos != pdeq->rend())
	{
		AddValidDataToLstMsg(*pos, TRUE);
		
		int nItemCount = m_listMsg.GetItemCount();
		if (nItemCount > (int)theApp.m_nMaxRecord)
			break;
		pos++;
	}
	m_csLog.Unlock();   */
}

void CMainDlg::InitFilterWnd()
{
	int nIndex = m_cLogType.AddString("RPT_DEBUG");
	tagData* pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	nIndex = m_cLogType.AddString("RPT_INFO");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	nIndex = m_cLogType.AddString("RPT_WARNING");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	nIndex = m_cLogType.AddString("RPT_ERROR");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	nIndex = m_cLogType.AddString("RPT_CRITICAL");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	/*nIndex = m_cLogType.AddString("RPT_ADDI_INFO");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	nIndex = m_cLogType.AddString("RPT_IGNORE");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	nIndex = m_cLogType.AddString("RPT_NOPREFIX");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);

	nIndex = m_cLogType.AddString("RPT_IMPORTANT");
	pData = new tagData;
	m_cLogType.SetItemData(nIndex, (DWORD_PTR)pData);*/
	m_cLogType.SelectAll();

	///////////////////////////////////////////////////////////////

	CTime tmStart = CTime::GetCurrentTime();
	tmStart = CTime(tmStart.GetYear(), tmStart.GetMonth(), tmStart.GetDay(),
		0, 0, 0);
	m_dtStart.SetTime(&tmStart);
	CTime tmStop = CTime(tmStart.GetYear(), tmStart.GetMonth(), tmStart.GetDay(),
		23, 59, 59);
	m_dtStop.SetTime(&tmStop);

	//显示设备服务器
	m_cDevServ.ResetContent();

	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	while(pos != m_mapDevServData.end())
	{
		nIndex = m_cDevServ.AddString(pos->second->szName);
		m_cDevServ.SetItemData(nIndex, (DWORD_PTR)pos->second);
		pos++;
	}
	m_cDevServ.SelectAll();
	OnCbnSelchangeCDevserv();
}

void CMainDlg::OnCbnSelchangeCDevserv()
{				
	if (m_cDev.GetCount() > 0)
	{
		tagData* pData = (tagData*)m_cDev.GetItemData(0);
		if (pData != NULL)
		{
			delete pData;
			pData = NULL;
		}
		m_cDev.ResetContent();
	}

	int nIndex = m_cDev.AddString("本设备");
	tagData* pData = new tagData;
	strcpy_s(pData->szName, 50,  "本设备");
	m_cDev.SetItemData(nIndex, (DWORD_PTR)pData);

	int nItemCount = m_cDevServ.GetCount();
	int* pDevServID = new int[nItemCount];
	int nIDCount = 0;
	for (int i=0; i<nItemCount; i++)
	{
		if (!m_cDevServ.GetCheck(i))
			continue;

		tagDevServData* pDevServData = (tagDevServData*)m_cDevServ.GetItemData(i);
		pDevServID[nIDCount] = pDevServData->nID;
		nIDCount++;
	}

	CDevDataMap::iterator pos = m_mapDevData.begin();
	while(pos != m_mapDevData.end())
	{
		BOOL bValid = FALSE;
		for (int i=0; i<nIDCount; i++)
		{
			if (pos->second->nParentID == pDevServID[i])
			{
				bValid = TRUE;
				break;
			}
		}

		if (bValid)
		{
			nIndex = m_cDev.AddString(pos->second->szName);
			m_cDev.SetItemData(nIndex, (DWORD_PTR)pos->second);
		}
		pos++;
	}

	m_cDev.SelectAll();
	delete []pDevServID;
}

void CMainDlg::OnCbnDropdownCDevserv()
{
	//显示设备服务器
	m_cDevServ.ResetContent();
	int nIndex = 0;

	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	while(pos != m_mapDevServData.end())
	{
		nIndex = m_cDevServ.AddString(pos->second->szName);
		m_cDevServ.SetItemData(nIndex, (DWORD_PTR)pos->second);
		pos++;
	}
}

void CMainDlg::OnTmiStartalldev()
{
	StartStopAllDev();
}

void CMainDlg::OnTmiStopalldev()
{
	StartStopAllDev(FALSE);
}

void CMainDlg::StartStopAllDev(BOOL bStart)
{
	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	while(pos != m_mapDevServData.end())
	{		
		if (!pos->second->bDevStarted) //设备服务器没有开启
		{
			pos++;
			continue;
		}

		CDevDataMap::iterator posDev = m_mapDevData.begin();
		while(posDev != m_mapDevData.end())
		{		
			if (bStart)
			{
				if (!posDev->second->bDevStarted)
				{
					StartStopApp(pos->second->hSock, posDev->second->szPath,
						posDev->second->szParam);
				}
			}
			else //Stop
			{
				//if (posDev->second->bDevStarted)
				//{
				if (pos->second->bDevStarted) //如果设备服务器在运行
				{
					StartStopApp(pos->second->hSock, posDev->second->szPath,
						posDev->second->szParam, posDev->second->dwProID, FALSE);
				}
				else
				{
					UpdateDevState(posDev->second->nID, posDev->second->dwProID, FALSE);
				}
				//}
			}
			posDev++;
		}
		pos++;
	}
}

LRESULT CMainDlg::OnDevServTimeOut(WPARAM wParam, LPARAM lParam)
{
	CDevServDataMap::iterator pos = m_mapDevServData.begin();
	while(pos != m_mapDevServData.end())
	{
		if (pos->second->hSock == wParam)
		{
			REPORT(MN, T("设备服务器[%s:%s]超时，已断开连接\n", pos->second->szName, pos->second->szIP), RPT_WARNING);
			break;
		}
		pos++;
	}

	return 0;
}


void CMainDlg::OnMiOpenalldevice()
{
	StartStopAllDev();
}

void CMainDlg::OnMiClosealldevice()
{
	StartStopAllDev(FALSE);
}

void CMainDlg::OnAppExit()
{
	CDialog::OnCancel();
}

void CMainDlg::OnStartDev()
{
	OnTmiStartseldev();
}

void CMainDlg::OnStopDev()
{
	OnTmiStopseldev();
}

void CMainDlg::OnKLine()
{
	OnMiKline();
}

void CMainDlg::OnStockQuery()
{
	OnMiStockinfo();
}

void CMainDlg::OnSetup()
{
	OnMiSetup();
}
void CMainDlg::OnMiToolbar()
{
	m_wndToolBar.ShowWindow(m_bShowToolBar ? SW_HIDE : SW_SHOW);
	m_bShowToolBar = !m_bShowToolBar;

	InitCtrlPos();
}

void CMainDlg::OnUpdateMiToolbar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowToolBar);
}

void CMainDlg::OnMiStatusbar()
{
	m_StatusBar.ShowWindow(m_bShowStatusBar ? SW_HIDE : SW_SHOW);
	m_bShowStatusBar = !m_bShowStatusBar;
	m_wndToolBar.Invalidate();

	InitCtrlPos();
}

void CMainDlg::OnUpdateMiStatusbar(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bShowStatusBar);
}

void CMainDlg::GetListHeadWidth()
{
	for (int i=0; i<m_listMsg.GetColumCount(); i++)
	{
		theApp.m_nlstMsgClmWidth[i] = m_listMsg.GetColumnWidth(i);
	}
}

void CMainDlg::UpdateDevServCombo(tagDevServData* pDevServData, BOOL bAdd)
{
	if (bAdd)
	{
		int nIndex = m_cDevServ.AddString(pDevServData->szName);
		m_cDevServ.SetItemData(nIndex, (DWORD_PTR)pDevServData);
		m_cDevServ.SetCheck(nIndex, TRUE);
	}
	else
	{
		for (int i=0; i<m_cDevServ.GetCount(); i++)
		{
			tagDevServData* pData = (tagDevServData*)m_cDevServ.GetItemData(i);
			if (pData == pDevServData)
			{
				m_cDevServ.DeleteString(i);
				return;
			}
		}
	}
}

void CMainDlg::UpdateDevCombo(tagDevData* pDevData, BOOL bAdd)
{
	if (bAdd)
	{
		int nIndex = m_cDev.AddString(pDevData->szName);
		m_cDev.SetItemData(nIndex, (DWORD_PTR)pDevData);
		m_cDev.SetCheck(nIndex, TRUE);
	}
	else
	{
		for (int i=0; i<m_cDev.GetCount(); i++)
		{
			tagDevData* pData = (tagDevData*)m_cDev.GetItemData(i);
			if (pData == pDevData)
			{
				m_cDev.DeleteString(i);
				return;
			}
		}
	}
}

void CMainDlg::OnTmiDevversion()
{
	HTREEITEM hCurSel = m_treeDevice.GetSelectedItem();
	tagDevData* pDevData = (tagDevData*)m_treeDevice.GetItemData(hCurSel);

	CDlg_DevVer dlg;
	if (pDevData->nDataType != TDT_DEVICE)
	{
		tagDevServData* pDevServData = (tagDevServData*)m_treeDevice.GetItemData(hCurSel);
		dlg.m_strDevName = pDevServData->szName;
		dlg.m_strVer = pDevServData->szVersion;
	}
	else
	{
		dlg.m_strDevName = pDevData->szName;
		dlg.m_strVer = pDevData->szVersion;
	}
	dlg.DoModal();
}

void CMainDlg::OnUpdateTmiDevversion(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableDevVer);
}

UINT CMainDlg::_OnTimer(LPVOID lpParam)
{
	CMainDlg* pDlg = (CMainDlg*)lpParam;
	pDlg->OnTimer();

	return 0xDEAD0011;
}

void CMainDlg::OnTimer()
{
	DWORD dwCurTick = 0;
	while(!theApp.m_bExit)
	{
		dwCurTick = ::GetTickCount();
		if (dwCurTick - m_dwPreLogTick > 100 && m_dwPreLogTick != 0 && m_bHasLog)
		{
			if (m_bAddSpeed)
			{
				m_listMsg.SetRedraw(TRUE);
				m_listMsg.Invalidate();
			}
			m_bHasLog = FALSE;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		if (dwCurTick - m_dwSecTick > 30 * 1000)
		{
			if (m_dwSecTick == 0)
			{
				m_dwSecTick = dwCurTick;
				continue;
			}

			{
				//如果添加设备监控模块，也要发送心跳日志
				if (theApp.m_bStartDevMon)
					REPORT(MN, T("HeartBeat\n"), RPT_HEARTBEAT|RPT_IGNORE);
				//
				time_t tmCur = time(NULL);
				CDevDataMap::iterator pos = m_mapDevData.begin();
				while (pos != m_mapDevData.end())
				{
					if (!pos->second->bDevStarted)
					{
						pos++;
						continue;
					}

					if (pos->second->bTODevHeartBeat)
					{
						pos->second->bTODevHeartBeat = FALSE;
						REPORT(MN, T("超时设备[%s:0x%x]重新发送心跳日志，更新设备的状态\n", pos->second->szName, pos->second->dwProID), RPT_INFO);
					}

					if (pos->second->tmPreCheck != 0 && 
						tmCur - pos->second->tmPreCheck > m_nLogTimeout*2)  //发送日志的线程超时
					{
						REPORT(MN, T("设备[%s:0x%x]超时\n", pos->second->szName, pos->second->dwProID), RPT_WARNING);
						if (theApp.m_bDevTORestart)//发送日志的线程出现故障，是否关闭设备重新启动
						{
							CDevServDataMap::iterator find = m_mapDevServData.find(pos->second->nParentID);
							if (find != m_mapDevServData.end())
							{
								REPORT(MN, T("正在关闭超时设备[%s:0x%x]...\n", pos->second->szName, pos->second->dwProID), RPT_INFO);
								StartStopApp(find->second->hSock, pos->second->szPath,
									pos->second->szParam, pos->second->dwProID, FALSE);
								pos->second->bRestart = TRUE;
							}
						}
						else
						{
							REPORT(MN, T("更新超时设备[%s:0x%x]的状态\n", pos->second->szName, pos->second->dwProID), RPT_INFO);
							UpdateDevState(pos->second->nID, pos->second->dwProID, FALSE);
						}
					}
					pos++;
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////
			{//删除前天的日志，只保留当天的日志
				time_t  timeCur;
				time(&timeCur);
				tm tmCur;
				localtime_s(&tmCur, &timeCur);
				if (!m_bDelLog && tmCur.tm_hour == 23 && tmCur.tm_min == 59)
				{
					REPORT(MN, T("正在删当天的日志...\n"), RPT_INFO);
					m_bDelLog = TRUE; 

					int nItemCount = 0;
					m_csLog.Lock();
					deque<tagLog*>* pdeq = m_deqLog.GetDeque();
					nItemCount = m_deqLog.Size();
					deque<tagLog*>::iterator pos = pdeq->begin();
					while(pos != pdeq->end())
					{
						delete (*pos);
						pos++;
					}
					pdeq->clear();
					m_listMsg.DeleteAllItems();
					m_csLog.Unlock();

					REPORT(MN, T("删除当天的日志完毕，共[%d]条记录\n", nItemCount), RPT_INFO);
				}
				else
				{
					m_bDelLog = FALSE; 
				}

				m_dwSecTick = dwCurTick;
			}
		}
		
		DWORD dwTick1 = GetTickCount();
		DWORD dwTick2 = dwTick1;
		while(!theApp.m_bExit)
		{
			if (dwTick2 - dwTick1 > 1000*30)
				break;
			tagLog* pLog = theApp.m_deqLog.PopFront();
			if (pLog == NULL)
				break;

			//TRACE("队列大小:%d\n", theApp.m_deqLog.Size());
			if (theApp.m_deqLog.Size() > 1024) //积累太多日志，RPT_INFO类型日志只写入文件，不显示在列表
			{	
				if ( (pLog->nLevel & 0xFF) != RPT_INFO)
					OnAddLog(pLog->nLevel, pLog);
				theApp.WriteToLogFile(pLog);
			}
			else
			{
				theApp.WriteToLogFile(pLog);
				OnAddLog(pLog->nLevel, pLog);
			} 
			delete pLog;
			dwTick2 = GetTickCount();
		}

		if (dwTick2 - dwTick1 < 100)
			Sleep(100);
	}
}
void CMainDlg::OnBnClickedCAddspeed()
{
	m_listMsg.SetRedraw(TRUE);
}

void CMainDlg::OnTmiXmlcfg()
{
	HTREEITEM hCurSel = m_treeDevice.GetSelectedItem();
	tagDevData* pDevData = (tagDevData*)m_treeDevice.GetItemData(hCurSel);
	CDlg_XmlCfg dlg;
	dlg.m_hMainWnd = m_hWnd;

	if (pDevData->nDataType != TDT_DEVICE)
	{
		tagDevServData* pDevServData = (tagDevServData*)m_treeDevice.GetItemData(hCurSel);
		dlg.m_strXmlPath = pDevServData->szXmlPath;
		dlg.m_strDevName = pDevServData->szName;
		dlg.m_strIP = pDevServData->szIP;
		dlg.m_hSock = pDevServData->hSock;
		dlg.DoModal();

		memset(pDevServData->szXmlPath, 0, 1024);
		strcpy_s(pDevServData->szXmlPath, 1024, dlg.m_strXmlPath);
	}
	else
	{
		dlg.m_strXmlPath = pDevData->szXmlPath;
		dlg.m_strDevName = pDevData->szName;

		CDevServDataMap::iterator find = m_mapDevServData.find(pDevData->nParentID);
		ASSERT(find != m_mapDevServData.end());
		dlg.m_hSock = find->second->hSock;
		dlg.m_strIP = find->second->szIP;
		dlg.DoModal();

		memset(pDevData->szXmlPath, 0, 1024);
		strcpy_s(pDevData->szXmlPath, 1024, dlg.m_strXmlPath);
	}
}

void CMainDlg::OnUpdateTmiXmlcfg(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bEnableXmlCfg);
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
	{
		m_treeDevice.SetFocus();
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CMainDlg::OnDownKLine()
{
	REPORT(MN, T("正在下载原始k线...\n"), RPT_INFO);
	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartUnRarCurKLineFile();
	dlgInfo.m_strInfo = "正在下载当前库的原始k线...";
	UINT nRet = dlgInfo.DoModal();
	if (nRet == IDCANCEL)
	{
		REPORT(MN, T("用户取消下载当前库的原始k线\n"), RPT_INFO);
		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		theApp.m_pcheckThread->StopUploadandDownload();

		dlgInfo.m_strInfo = "正在终止下载...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}
}

void CMainDlg::OnDownHqKLine()
{
	REPORT(MN, T("正在下载还权k线...\n"), RPT_INFO);
	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartDownloadHQKLineFile();
	dlgInfo.m_strInfo = "正在下载当前库的还权k线...";
	if (dlgInfo.DoModal() == IDCANCEL)
	{
		REPORT(MN, T("用户取消下载当前库的还权k线\n"), RPT_INFO);
		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		theApp.m_pcheckThread->StopUploadandDownload();

		dlgInfo.m_strInfo = "正在终止下载...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}
}

void CMainDlg::OnMiSavedevicelist()
{
	SaveDevInfo();
}

void CMainDlg::OnMiCombinationDatFile()
{
	CDlg_CombiFile dlg;
	dlg.DoModal();
}
