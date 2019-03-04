// KLine_ExeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "KLine_ExeDlg.h"
#include "ValidFunction.h"
#include "FilePath.h"
#include "FolderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ET_NULL               0x00
#define ET_START_CHECK        0x01
#define ET_CHECK_STOPPED	  0x02
#define ET_ENABLE_SAVE     	  0x03
#define ET_STOP_CHECK     	  0x04
#define ET_CHECK_ALL     	  0x05
#define ET_CHECK_ALL_STOPPED  0x06

#define IDT_AUTOCHECK         0x01
#pragma warning(disable:4819)
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CKLine_ExeDlg 对话框




CKLine_ExeDlg::CKLine_ExeDlg(CWnd* pParent /*=NULL*/)
: CDialog(CKLine_ExeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bStopCheck   = FALSE;
	m_nEnableType  = ET_NULL;
	m_bDataIntegrality = FALSE;
	m_bDataChanged = FALSE;
	m_bIsCheckAll  = FALSE;
	m_bHideFileList = TRUE;
	m_bStartWeight = FALSE;
	m_bEnCheckBtn = TRUE;
}

void CKLine_ExeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_INFO, m_listInfo);

	DDX_Control(pDX, IDC_LIST_ERROR, m_listResult);
	DDX_Control(pDX, IDC_LIST_FILE, m_listFile);
}

BEGIN_MESSAGE_MAP(CKLine_ExeDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_COMMAND(ID_CHECK_SINGLE_FILE, &CKLine_ExeDlg::OnCheckSingleFile)

	ON_WM_DESTROY()
	ON_LBN_DBLCLK(IDC_LIST_ERROR, &CKLine_ExeDlg::OnLbnDblclkListError)
	//	ON_COMMAND(ID_STOPCHECK, &CKLine_ExeDlg::OnStopcheck)
	ON_BN_CLICKED(IDC_BTN_RECHECK, &CKLine_ExeDlg::OnBnClickedBtnRecheck)
	ON_BN_CLICKED(IDC_BTN_SAVETOFILE, &CKLine_ExeDlg::OnBnClickedBtnSavetofile)
	ON_BN_CLICKED(IDC_BTN_STOPCHECK, &CKLine_ExeDlg::OnBnClickedBtnStopcheck)
	ON_BN_CLICKED(IDC_BTN_INSERTRECORD, &CKLine_ExeDlg::OnBnClickedBtnInsertrecord)
	ON_BN_CLICKED(IDC_BTN_DELSELREC, &CKLine_ExeDlg::OnBnClickedBtnDelselrec)
	ON_BN_CLICKED(IDC_BTN_MOVEPRE, &CKLine_ExeDlg::OnBnClickedBtnMovepre)
	ON_BN_CLICKED(IDC_BTN_MOVENEXT, &CKLine_ExeDlg::OnBnClickedBtnMovenext)

	ON_MESSAGE(WM_THREADEND, OnThreadEnd)
	ON_MESSAGE(WM_VALUE_CHANGED, OnValueChanged)
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI(ID_CHECK_SINGLE_FILE, &CKLine_ExeDlg::OnUpdateCheckSingleFile)
	//	ON_UPDATE_COMMAND_UI(ID_STOPCHECK, &CKLine_ExeDlg::OnUpdateStopcheck)
	///
	ON_UPDATE_COMMAND_UI_RANGE(100,10000,OnUpdateControl)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_INFO, &CKLine_ExeDlg::OnLvnItemchangedListInfo)
	ON_MESSAGE(WM_SAVE_FAILED, OnSaveFileFailed)
	ON_COMMAND(ID_CHECKALLFILE, &CKLine_ExeDlg::OnCheckallfile)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILE, &CKLine_ExeDlg::OnNMDblclkListFile)
	ON_UPDATE_COMMAND_UI(ID_CHECKALLFILE, &CKLine_ExeDlg::OnUpdateCheckallfile)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_USEHTTPDATA, &CKLine_ExeDlg::OnBnClickedBtnUsehttpdata)
	ON_COMMAND(ID_DOWNLOADKFILE, &CKLine_ExeDlg::OnDownloadkfile)
	ON_COMMAND(ID_UPLOADKFILE, &CKLine_ExeDlg::OnUploadkfile)

	ON_COMMAND(ID_MI_DB_HQ, &CKLine_ExeDlg::OnMiDbHq)
	ON_UPDATE_COMMAND_UI(ID_MI_DB_HQ, &CKLine_ExeDlg::OnUpdateMiDbHq)

	ON_MESSAGE(WM_LIST_MSG, OnListMsg)
	//	ON_COMMAND(ID_MI_STOPWEIGHT, &CKLine_ExeDlg::OnMiStopweight)
	//	ON_UPDATE_COMMAND_UI(ID_MI_STOPWEIGHT, &CKLine_ExeDlg::OnUpdateMiStopweight)
	ON_COMMAND(ID_MI_UPLOAD_HQ, &CKLine_ExeDlg::OnMiUploadHq)
	ON_COMMAND(ID_MI_DOWNLOAD_HQ, &CKLine_ExeDlg::OnMiDownloadHq)
	ON_COMMAND(ID_MI_CUTFILE, &CKLine_ExeDlg::OnMiCutfile)
	ON_BN_CLICKED(IDC_BTN_ADDRECORD, &CKLine_ExeDlg::OnBnClickedBtnAddrecord)
	ON_COMMAND(ID_UPDATE_WEIGHT, &CKLine_ExeDlg::OnUpdateWeight)
END_MESSAGE_MAP()


// CKLine_ExeDlg 消息处理程序

BOOL CKLine_ExeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// TODO: 在此添加额外的初始化代码
	Init();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CKLine_ExeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CKLine_ExeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CKLine_ExeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKLine_ExeDlg::Init()
{	
	m_pWeightThread = new CWorkThread(_WeightThread, this);
	InitCtrlPos();

	m_listFile.InsertColumn(0, "错误文件", LVCFMT_RIGHT, 160);
	m_listFile.InsertColumn(1, "路    径", LVCFMT_RIGHT, 140);
	{//init listctrl
		m_listInfo.InsertColumn(0, "日    期", LVCFMT_RIGHT, 80);
		m_listInfo.InsertColumn(1, "开盘价格", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(2, "最高价格", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(3, "最低价格", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(4, "收盘价格", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(5, "成交数量", LVCFMT_RIGHT, 80);
		m_listInfo.InsertColumn(6, "成交金额", LVCFMT_RIGHT, 80);

		m_bmpNormal.LoadBitmap(IDB_NORMAL);
		m_bmpError.LoadBitmap(IDB_ERROR);
		m_imgList.Create(15, 15, ILC_COLORDDB, 2, 2);
		m_imgList.Add(&m_bmpNormal, RGB(255,0,255));
		m_imgList.Add(&m_bmpError, RGB(255,0,255));
		m_listInfo.SetImageList(&m_imgList, LVSIL_SMALL);
		m_listInfo.EnableEdit();
	}		  

	theApp.m_pcheckThread->SetCtrlWnd(m_listInfo.m_hWnd, m_listResult.m_hWnd, this->m_hWnd,
		m_listFile.m_hWnd);
	UpdateDialogControls(NULL, FALSE);

	SetTimer(IDT_AUTOCHECK, 1000, NULL); //一秒钟检测一次
}

void CKLine_ExeDlg::InitFileList(CString strFile)
{
	if (strFile.IsEmpty())
		return;

	CString strFolder = strFile;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			CString strFolderName = ffind.GetFileName();

			InitFileList(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFileName();
			CString strFolder = ffind.GetFilePath();
		}
	}
}

void CKLine_ExeDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	InitCtrlPos(m_bHideFileList);
}

void CKLine_ExeDlg::InitCtrlPos(BOOL bHideFileList)
{
	CRect rcClient;
	GetClientRect(rcClient);

	m_bHideFileList = bHideFileList;
	if (bHideFileList)
	{
		if (m_listFile.m_hWnd != NULL)
			m_listFile.MoveWindow(0, 0, 0, 0);

		if (m_listInfo.m_hWnd != NULL)
			m_listInfo.MoveWindow(0, 0, rcClient.Width() - 82, rcClient.Height() - 150);
	}
	else
	{
		if (m_listFile.m_hWnd != NULL)
			m_listFile.MoveWindow(0, 0, 160, rcClient.Height() - 150);

		if (m_listInfo.m_hWnd != NULL)
			m_listInfo.MoveWindow(163, 0, rcClient.Width() - 245, rcClient.Height() - 150);
	}
	if (m_listResult.m_hWnd != NULL)
		m_listResult.MoveWindow(0, rcClient.Height() - 147, rcClient.Width(), 147);

	CWnd *pWnd = NULL; 
	pWnd = GetDlgItem(IDC_BTN_STOPCHECK);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, 5, 67, 20);

	pWnd = GetDlgItem(IDC_BTN_RECHECK);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, 28, 67, 20);

	pWnd = GetDlgItem(IDC_BTN_SAVETOFILE);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, 83, 67, 20);

	pWnd = GetDlgItem(IDC_BTN_USEHTTPDATA);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, 51, 67, 20);

	///////////////////////
	pWnd = GetDlgItem(IDC_BTN_ADDRECORD);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, rcClient.Height() - 267, 67, 20);

	pWnd = GetDlgItem(IDC_BTN_INSERTRECORD);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, rcClient.Height() - 244, 67, 20);

	pWnd = GetDlgItem(IDC_BTN_DELSELREC);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, rcClient.Height() - 221, 67, 20);

	pWnd = GetDlgItem(IDC_BTN_MOVEPRE);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, rcClient.Height() - 198, 67, 20);

	pWnd = GetDlgItem(IDC_BTN_MOVENEXT);
	if (pWnd != NULL)
		pWnd->MoveWindow(rcClient.Width() - 75, rcClient.Height() - 175, 67, 20);
}

void CKLine_ExeDlg::OnCheckSingleFile()
{
// 	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST);
// 	int StructSize = 0;
// 	DWORD dwVersion = GetVersion();
// 	if(dwVersion < 0x80000000)			//winnt/xp
// 		StructSize = 88;				//show new dialog
// 	else								//win9x
// 		StructSize = 76;				//show old dialog
// 
// 	CString strDir = theApp.m_strKFilePath;
// 	dlg.m_ofn.lStructSize		= StructSize;
// 	//dlg.m_ofn.lpstrInitialDir	= strDir;
// 
// 	if(dlg.DoModal() == IDOK)
// 	{
// 		strDir = dlg.GetPathName();
// 
// 		{//不能对还权k线网上校对
// 			CString strPath = strDir.Left(theApp.m_strHQKLPath.GetLength());
// 			if (theApp.m_strHQKLPath.CollateNoCase(strPath) == 0)
// 				m_bEnCheckBtn = FALSE;
// 			else
// 				m_bEnCheckBtn = TRUE;
// 		}
// 
// 		m_listInfo.DeleteAllItems();
// 		m_listResult.ResetContent();
// 
// 		InitCtrlPos();
// 
// 		m_bIsCheckAll = FALSE;
// 		m_bDataIntegrality = FALSE;
// 		CDlg_Info dlgInfo;
// 		theApp.m_pDlgInfoWnd = &dlgInfo;
// 		theApp.m_pcheckThread->CheckSingleFile(strDir);
// 
// 		m_nEnableType = ET_START_CHECK;
// 		UpdateDialogControls(NULL, FALSE);
// 
// 		dlgInfo.m_strInfo = "正在检测文件...";
// 		if (dlgInfo.DoModal() == IDCANCEL) //取消操作
// 			OnBnClickedBtnStopcheck();
// 	}
}

void CKLine_ExeDlg::OnDestroy()
{
	delete m_pWeightThread;
	KillTimer(IDT_AUTOCHECK);
	theApp.m_pcheckThread->Stop();

	CDialog::OnDestroy();
}

void CKLine_ExeDlg::OnLbnDblclkListError()
{
	int nIndex = m_listResult.GetCurSel();
	if (nIndex != LB_ERR)
	{
		DWORD dwData = (DWORD)m_listResult.GetItemData(nIndex);
		int nItem = dwData / 10;

		if (dwData % 10 == 1)
		{
			if (nItem >= 0 && nItem < m_listInfo.GetItemCount())
			{
				m_listInfo.SetItemState(m_listInfo.GetSelectionMark(), 0 ,LVIS_SELECTED); 
				m_listInfo.SetItemState(nItem, LVIS_SELECTED ,LVIS_SELECTED);
				m_listInfo.SetSelectionMark(nItem);
				m_listInfo.EnsureVisible(nItem,TRUE);
			}
		}
		else if (dwData % 10 == 0)
		{
			if (nItem >= 0 && nItem < m_listFile.GetItemCount())
			{
				m_listFile.SetItemState(m_listFile.GetSelectionMark(), 0 ,LVIS_SELECTED); 
				m_listFile.SetItemState(nItem, LVIS_SELECTED ,LVIS_SELECTED);
				m_listFile.SetSelectionMark(nItem);
				m_listFile.EnsureVisible(nItem,TRUE);
			}
		}
	}
}


//void CKLine_ExeDlg::OnStopcheck()
//{
//}

void CKLine_ExeDlg::OnBnClickedBtnStopcheck()
{
	m_bDataIntegrality = FALSE;
	m_nEnableType = ET_STOP_CHECK;
	UpdateDialogControls(NULL, FALSE);

	theApp.m_pcheckThread->Stop();
}

void CKLine_ExeDlg::OnBnClickedBtnRecheck()
{

	if (m_bDataChanged && !m_bIsCheckAll)
	{
		if (MessageBox("数据已修改，如果不保存数据，则修改无效，是否需要保存数据？",
			"警告", MB_YESNO) == IDYES)
			return;

		m_bDataChanged = FALSE;
	}

	m_bDataIntegrality = FALSE;

	m_nEnableType = ET_START_CHECK;
	UpdateDialogControls(NULL, FALSE);

	m_listInfo.DeleteAllItems();
	m_listResult.ResetContent();

	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo; 
	if (!m_bIsCheckAll)
	{
		theApp.m_pcheckThread->RecheckFile();
		dlgInfo.m_strInfo = "正在检测文件...";
	}
	else
	{
		m_listFile.DeleteAllItems();
		theApp.m_pcheckThread->StartCheckAllFile(m_strCheckAllPath);
		dlgInfo.m_strInfo = "正在检测选择目录下所有文件...";
	}

	if (dlgInfo.DoModal() == IDCANCEL) //取消操作
		OnBnClickedBtnStopcheck();
}

void CKLine_ExeDlg::OnBnClickedBtnSavetofile()
{
	m_bDataChanged = FALSE;
	GetDlgItem(IDC_BTN_SAVETOFILE)->EnableWindow(FALSE);
	theApp.m_pcheckThread->SaveDataToFile();
}

void CKLine_ExeDlg::OnBnClickedBtnInsertrecord()
{
	int nIndex = m_listInfo.InsetItemInCurSel();
	m_listInfo.SetItemState(m_listInfo.GetSelectionMark(), 0 ,LVIS_SELECTED); 
	m_listInfo.SetItemState(nIndex, LVIS_SELECTED ,LVIS_SELECTED);
	m_listInfo.SetSelectionMark(nIndex);
	m_listInfo.EnsureVisible(nIndex,TRUE);
}

void CKLine_ExeDlg::OnBnClickedBtnDelselrec()
{
	POSITION pt = m_listInfo.GetFirstSelectedItemPosition();
	if (pt == NULL)
		return;

	if (MessageBox("确定要删除选中的记录吗?", "警告", MB_YESNO) == IDYES)
		m_listInfo.DeleteSelectItem();
}

void CKLine_ExeDlg::OnBnClickedBtnMovepre()
{
	m_listInfo.MoveSelItemToPrev();
}

void CKLine_ExeDlg::OnBnClickedBtnMovenext()
{
	m_listInfo.MoveSelItemToNext();
}


BOOL CKLine_ExeDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

LRESULT CKLine_ExeDlg::OnThreadEnd(WPARAM wParam, LPARAM lParam)
{
	if (wParam == CMD_NULL)
	{
		HWND hDlgInfoWnd = theApp.m_pDlgInfoWnd->GetSafeHwnd();
		if (theApp.m_pDlgInfoWnd != NULL && hDlgInfoWnd != NULL)
			::PostMessage(hDlgInfoWnd, WM_COMMAND, IDOK, 0);
		EnableWindow(TRUE);
		return 0;
	}

	m_bDataIntegrality = TRUE;  //检测完成，没有被中途停止

	if (wParam == CMD_CHECK_ALL_FILE)
		m_nEnableType = ET_CHECK_ALL_STOPPED;
	else
		m_nEnableType = ET_CHECK_STOPPED;

	UpdateDialogControls(NULL, FALSE);

	HWND hDlgInfoWnd = theApp.m_pDlgInfoWnd->GetSafeHwnd();
	if (theApp.m_pDlgInfoWnd != NULL && hDlgInfoWnd != NULL)
		::PostMessage(hDlgInfoWnd, WM_COMMAND, IDOK, 0);
	return 0;
}

LRESULT CKLine_ExeDlg::OnValueChanged(WPARAM wParam, LPARAM lParam)
{
	if (m_bDataIntegrality) //数据完整性，从文件读完所有数据
	{
		m_bDataChanged = TRUE;
		m_nEnableType = ET_ENABLE_SAVE;
		UpdateDialogControls(NULL, FALSE);
	}
	else
	{
		m_bDataChanged = FALSE;
	}
	return 0;
}

void CKLine_ExeDlg::OnUpdateControl(CCmdUI *pCmdUI)
{
	UINT nID = pCmdUI->m_nID;
	if(IsCtrlEnabled(nID))
	{
		pCmdUI->Enable(TRUE);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}

	if (!m_bEnCheckBtn)
		GetDlgItem(IDC_BTN_USEHTTPDATA)->EnableWindow(FALSE);
}

BOOL CKLine_ExeDlg::IsCtrlEnabled(UINT nID)
{
	static UINT nEnable[] =
	{
		IDC_LIST_ERROR, IDC_LIST_INFO//, IDC_BTN_INSERTRECORD, IDC_BTN_DELSELREC, IDC_BTN_MOVEPRE, IDC_BTN_MOVENEXT 
	};
	if(Find(nEnable, sizeof(nEnable)/sizeof(nEnable[0]), nID))
		return TRUE;

	switch (m_nEnableType)
	{
	case ET_START_CHECK:
		{
			static UINT nExcludeArr[] = 
			{
				IDC_BTN_STOPCHECK
			};
			m_bStopCheck = TRUE;
			if(Find(nExcludeArr, sizeof(nExcludeArr)/sizeof(nExcludeArr[0]), nID))
				return TRUE;
			break;
		}
	case ET_CHECK_STOPPED:
		{
			static UINT nExcludeArr[] = 
			{
				IDC_BTN_RECHECK, IDC_BTN_INSERTRECORD, IDC_LIST_FILE, IDC_BTN_USEHTTPDATA
				, IDC_BTN_INSERTRECORD, IDC_BTN_DELSELREC, IDC_BTN_MOVEPRE, IDC_BTN_MOVENEXT,IDC_BTN_ADDRECORD 
			};
			m_bStopCheck = FALSE;
			if(Find(nExcludeArr, sizeof(nExcludeArr)/sizeof(nExcludeArr[0]), nID))
				return TRUE;
			break;
		}
	case ET_ENABLE_SAVE:
		{		
			static UINT nExcludeArr[] = 
			{
				IDC_BTN_SAVETOFILE, IDC_BTN_RECHECK, IDC_BTN_INSERTRECORD, IDC_BTN_USEHTTPDATA,
				IDC_LIST_FILE, IDC_BTN_DELSELREC, IDC_BTN_MOVEPRE, IDC_BTN_MOVENEXT,IDC_BTN_ADDRECORD
			};
			if(Find(nExcludeArr, sizeof(nExcludeArr)/sizeof(nExcludeArr[0]), nID))
				return TRUE;
			break;
		}
	case ET_STOP_CHECK:
		{
			static UINT nExcludeArr[] = 
			{
				IDC_BTN_RECHECK, IDC_LIST_FILE, IDC_BTN_USEHTTPDATA
			};
			m_bStopCheck = FALSE;
			if(Find(nExcludeArr, sizeof(nExcludeArr)/sizeof(nExcludeArr[0]), nID))
				return TRUE;
			break;
		}
	case ET_CHECK_ALL:
		{
			static UINT nExcludeArr[] = 
			{
				IDC_BTN_STOPCHECK
			};
			m_bStopCheck = TRUE;
			if(Find(nExcludeArr, sizeof(nExcludeArr)/sizeof(nExcludeArr[0]), nID))
				return TRUE;
			break;
		}
	case ET_CHECK_ALL_STOPPED:
		{
			static UINT nExcludeArr[] = 
			{
				IDC_LIST_FILE, IDC_BTN_RECHECK, IDC_BTN_USEHTTPDATA
			};
			m_bStopCheck = FALSE;
			if(Find(nExcludeArr, sizeof(nExcludeArr)/sizeof(nExcludeArr[0]), nID))
				return TRUE;
			break;
		}
	default:
		{
			break;
		}
	}

	return FALSE;
}

BOOL CKLine_ExeDlg::Find(UINT *pArr, int nItem, UINT dwFindData)
{
	for(int i=0; i<nItem; i++)
	{
		if(pArr[i] == dwFindData)
			return TRUE;
	}

	return FALSE;
}

void CKLine_ExeDlg::OnUpdateCheckSingleFile(CCmdUI *pCmdUI)
{
	if (!m_bStopCheck)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

//void CKLine_ExeDlg::OnUpdateStopcheck(CCmdUI *pCmdUI)
//{
//	if (m_bStopCheck)
//		pCmdUI->Enable(TRUE);
//	else
//		pCmdUI->Enable(FALSE);
//}

void CKLine_ExeDlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu)
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

void CKLine_ExeDlg::OnLvnItemchangedListInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	POSITION pt = m_listInfo.GetFirstSelectedItemPosition();
	if (pt != NULL && m_bDataIntegrality)
	{
		GetDlgItem(IDC_BTN_DELSELREC)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_MOVEPRE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_MOVENEXT)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BTN_DELSELREC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MOVEPRE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MOVENEXT)->EnableWindow(FALSE);
	}

	*pResult = 0;
}

LRESULT CKLine_ExeDlg::OnSaveFileFailed(WPARAM wParam, LPARAM lParam)
{
	m_bDataChanged = TRUE;
	GetDlgItem(IDC_BTN_SAVETOFILE)->EnableWindow(TRUE);
	return 0;
}

void CKLine_ExeDlg::OnCheckallfile()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;
	m_strCheckAllPath = dlg.GetPathName();

	m_bIsCheckAll = TRUE;
	InitCtrlPos(FALSE);
	m_listFile.DeleteAllItems();
	m_listResult.ResetContent();
	m_nEnableType = ET_CHECK_ALL;
	UpdateDialogControls(NULL, FALSE);

	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartCheckAllFile(m_strCheckAllPath);
	//如果文件大小超过限制大小
	//m_checkThread.StartCheckFileSize();

	dlgInfo.m_strInfo = "正在检测选择目录下所有文件...";
	if (dlgInfo.DoModal() == IDCANCEL) //取消操作
		OnBnClickedBtnStopcheck();
}

void CKLine_ExeDlg::OnNMDblclkListFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nIndex = m_listFile.GetSelectionMark();
	if (nIndex != -1)
	{
		CString strPath = m_listFile.GetItemText(nIndex, 1);
		m_nEnableType = ET_START_CHECK;
		UpdateDialogControls(NULL, FALSE);

		m_listFile.EnableWindow(FALSE);
		m_listInfo.DeleteAllItems();
		m_bIsCheckAll = FALSE;

		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		dlgInfo.m_strInfo = "正在检测文件...";
		theApp.m_pcheckThread->CheckSingleFile(strPath);

		if (dlgInfo.DoModal() == IDCANCEL) //取消操作
			OnBnClickedBtnStopcheck();
	}
	*pResult = 0;
}

void CKLine_ExeDlg::OnUpdateCheckallfile(CCmdUI *pCmdUI)
{
	if (!m_bStopCheck)
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
}

void CKLine_ExeDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_AUTOCHECK)
	{
		CTime tmCur = CTime::GetCurrentTime();
		UINT nTime = tmCur.GetHour()*10000 + tmCur.GetMinute()*100 + tmCur.GetSecond();

		if (nTime == theApp.m_nCheckTime  && theApp.m_bAutoCheck)
		{
// 			AutoCheck();
		}
		else if (nTime == theApp.m_nDownLoadTime && theApp.m_bUseDownLoad)
		{
// 			CDlg_Info dlgInfo;
// 			theApp.m_pDlgInfoWnd = &dlgInfo;
// 			theApp.m_pcheckThread->StartUnRarCurKLineFile();
// 			dlgInfo.m_strInfo = "正在下载当前库的原始k线...";
// 			dlgInfo.m_bEnableBtn = FALSE;
// 			dlgInfo.DoModal();
// 			dlgInfo.m_bEnableBtn = TRUE;
		}

		if (nTime == theApp.m_nRealHQTime && theApp.m_bRWUpload) //实时还权
		{
			m_bRealHQ = TRUE;
			StartHuanQuan();
		}

		if (nTime == theApp.m_nDownHQTime && theApp.m_bDownLoadHQ) //定时下载还权文件
		{
			CDlg_Info dlgInfo;
			theApp.m_pDlgInfoWnd = &dlgInfo;
			theApp.m_pcheckThread->StartDownloadHQKLineFile();
			dlgInfo.m_strInfo = "正在下载当前库的还权k线...";
			dlgInfo.m_bEnableBtn = FALSE;
			dlgInfo.DoModal();
			dlgInfo.m_bEnableBtn = TRUE;
		}
	}

	CDialog::OnTimer(nIDEvent);		
}

void CKLine_ExeDlg::AutoCheck()
{
	//检测所有文件
// 	m_bIsCheckAll = TRUE;
// 	InitCtrlPos(FALSE);
// 	m_listFile.DeleteAllItems();
// 	m_listResult.ResetContent();
// 	m_nEnableType = ET_CHECK_ALL;
// 	UpdateDialogControls(NULL, FALSE);
// 
// 	CDlg_Info dlgInfo;
// 	theApp.m_pDlgInfoWnd = &dlgInfo;
// 	if (theApp.m_bUserHttpData)  //使用雅虎财金网上数据校对，并更新错误数据
// 	{
// 		dlgInfo.m_strInfo = "正在使用网上数据校对当前库的原始k线...";
// 		theApp.m_pcheckThread->StartCheckAllFileUseHttpData(theApp.m_strKFilePath);
// 	}
// 	else
// 	{
// 		dlgInfo.m_strInfo = "正在检测当前库的原始k线...";
// 		theApp.m_pcheckThread->StartCheckAllFile(theApp.m_strKFilePath, TRUE);	
// 	}
// 
// 	dlgInfo.m_bEnableBtn = FALSE;
// 	dlgInfo.DoModal();
// 	dlgInfo.m_bEnableBtn = TRUE;
// 
// 	//如果文件大小超过限制大小
// 	CDlg_Info dlgInfo2;
// 	theApp.m_pDlgInfoWnd = &dlgInfo2;
// 	dlgInfo2.m_strInfo = "正在检测当前库原始k线的大小...";
// 	theApp.m_pcheckThread->StartCheckFileSize(theApp.m_strKFilePath);
// 	dlgInfo2.m_bEnableBtn = FALSE;
// 	dlgInfo2.DoModal();
// 	dlgInfo2.m_bEnableBtn = TRUE;
// 
// 	//压缩上传k线文件
// 	if (theApp.m_bUseUpLoad)
// 	{
// 		CDlg_Info dlgInfo3;
// 		theApp.m_pDlgInfoWnd = &dlgInfo3;
// 		dlgInfo3.m_strInfo = "正在上传当前库的原始k线...";
// 		theApp.m_pcheckThread->StartRarUpLoadKFile();
// 		dlgInfo3.m_bEnableBtn = FALSE;
// 		dlgInfo3.DoModal();
// 		dlgInfo3.m_bEnableBtn = TRUE;
// 	}
}
void CKLine_ExeDlg::OnBnClickedBtnUsehttpdata()
{
	//if (MessageBox("网上校对会修改数据，确定要使用网上校对吗?", "警告", MB_YESNO) == IDNO)
	//	return;
	m_bDataIntegrality = FALSE;

	m_nEnableType = ET_START_CHECK;
	UpdateDialogControls(NULL, FALSE);

	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	if (m_bIsCheckAll)
	{
		theApp.m_pcheckThread->StartCheckAllFileUseHttpData(theApp.m_strKFilePath);
		dlgInfo.m_strInfo = "正在使用网上数据校对所有文件...";
	}
	else
	{
		theApp.m_pcheckThread->StartCheckSingeleFileUseHttpData();
		dlgInfo.m_strInfo = "正在使用网上数据校对文件...";
	}

	if (dlgInfo.DoModal() == IDCANCEL) //取消操作
		OnBnClickedBtnStopcheck();
}


void CKLine_ExeDlg::OnDownloadkfile()
{
// 	CDlg_Info dlgInfo;
// 	theApp.m_pDlgInfoWnd = &dlgInfo;
// 	theApp.m_pcheckThread->StartUnRarCurKLineFile();
// 	dlgInfo.m_strInfo = "正在下载当前库的原始k线...";
// 	if (dlgInfo.DoModal() == IDCANCEL)
// 	{
// 		AddMsg(RPT_INFO, "用户取消下载当前库的原始k线\n");
// 		theApp.m_pcheckThread->StopUploadandDownload();
// 
// 		CDlg_Info dlgInfo;
// 		theApp.m_pDlgInfoWnd = &dlgInfo;
// 		dlgInfo.m_strInfo = "正在终止下载...";
// 		dlgInfo.m_bEnableBtn = FALSE;
// 		dlgInfo.DoModal();
// 		dlgInfo.m_bEnableBtn = TRUE;
// 	}
}

void CKLine_ExeDlg::OnUploadkfile()
{
// 	CDlg_Info dlgInfo;
// 	theApp.m_pDlgInfoWnd = &dlgInfo;
// 	theApp.m_pcheckThread->StartRarUpLoadKFile(FALSE);
// 	dlgInfo.m_strInfo = "正在上传当前库的原始k线...";
// 	if (dlgInfo.DoModal() == IDCANCEL)
// 	{
// 		AddMsg(RPT_INFO, "用户取消上传当前库的原始k线\n");
// 		theApp.m_pcheckThread->StopUploadandDownload();
// 
// 		CDlg_Info dlgInfo;
// 		theApp.m_pDlgInfoWnd = &dlgInfo;
// 		dlgInfo.m_strInfo = "正在终止上传...";
// 		dlgInfo.m_bEnableBtn = FALSE;
// 		dlgInfo.DoModal();
// 		dlgInfo.m_bEnableBtn = TRUE;
// 	}
}

void CKLine_ExeDlg::OnMiDbHq()
{
	m_bRealHQ = FALSE;
	StartHuanQuan();
}

void CKLine_ExeDlg::OnUpdateWeight()
{
	// TODO: 在此添加命令处理程序代码
//	theApp.m_pcheckThread->
}

void CKLine_ExeDlg::OnUpdateMiDbHq(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_bStartWeight);
}

void CKLine_ExeDlg::StartHuanQuan()
{
	//m_pWeightThread->Stop();

	//if (!m_pWeightThread->Start())
	//	AddMsg(RPT_ERROR, "启动K线历史还权失败");

	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartWeightThread(m_bRealHQ);

	if (m_bRealHQ)
	{
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.m_strInfo = "正在对K线实时还权...";
	}
	else
		dlgInfo.m_strInfo = "正在对K线历史还权...";

	if (dlgInfo.DoModal() == IDCANCEL) //取消操作
	{
		theApp.m_pcheckThread->StopWeightThread();

		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		dlgInfo.m_strInfo = "正在终止还权...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}

	dlgInfo.m_bEnableBtn = TRUE;
}

BOOL CKLine_ExeDlg::BackupKLine()
{
	return TRUE;
// 	AddMsg(RPT_INFO, "正在备份k线数据到临时目录[%s]...", theApp.m_strKLBkPath);
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return FALSE;
// 	}
// 	if (CFilePath::IsFolderHasFile(theApp.m_strKLBkPath))
// 	{
// 		UINT nRetID = IDYES;
// 		if (!m_bRealHQ)
// 			nRetID = MessageBox("临时目录["+theApp.m_strKLBkPath +"]中已存在文件, 在复制文件前是否先删除该文件?", "警告", MB_YESNO);
// 
// 		if (nRetID == IDYES)
// 		{//删除文件
// 			AddMsg(RPT_INFO, "正在删除临时目录[%s]里的文件...", theApp.m_strKLBkPath);
// 			CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
// 			AddMsg(RPT_INFO, "删除文件完毕");
// 		}
// 	}
// 
// 	m_strCurKLine = CFilePath::GetPathLastFolderName(theApp.m_strKFilePath);
// 	m_strHisKLine = CFilePath::GetPathLastFolderName(theApp.m_strHisKFilePath);
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return FALSE;
// 	}
// 
// 	BOOL bRet = TRUE;
// 	if (m_bRealHQ) //实时还权
// 	{
// 		AddMsg(RPT_INFO, "正在复制[%s]里的文件到临时目录[%s]...", theApp.m_strHQKLPath, theApp.m_strKLBkPath);
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath, theApp.m_strHQKLPath);
// 	}
// 	else		   //历史还权
// 	{
// 		AddMsg(RPT_INFO, "正在复制[%s]里的文件到临时目录[%s]...", theApp.m_strKFilePath, theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str(), theApp.m_strKFilePath);
// 		AddMsg(RPT_INFO, "正在复制[%s]里的文件到临时目录[%s]...", theApp.m_strHisKFilePath, theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str(), theApp.m_strHisKFilePath);
// 	}
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return FALSE;
// 	}
// 	return bRet;
}

void CKLine_ExeDlg::AddMsg(UINT nPackType, LPCSTR pFormat, ...)
{
	char szMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(szMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);

	REPORT(MN, T("%s\n", szMsg), nPackType);
	SendMessage(WM_LIST_MSG, (WPARAM)szMsg);
}

BOOL CKLine_ExeDlg::QueryWeight()
{
	AddMsg(RPT_INFO, "正在从数据库[%s:%s]获取还权因子...", theApp.m_strSqlIP, theApp.m_strSqlDBN);
	//从数据库读取数据到内存
	std::string strIP, strDBN, strUID, strPw;
	strIP  = theApp.m_strSqlIP.GetBuffer();
	theApp.m_strSqlIP.ReleaseBuffer();
	strDBN = theApp.m_strSqlDBN.GetBuffer();
	theApp.m_strSqlDBN.ReleaseBuffer();
	strUID = theApp.m_strSqlUID.GetBuffer();
	theApp.m_strSqlUID.ReleaseBuffer();
	strPw  = theApp.m_strSqlPw.GetBuffer();
	theApp.m_strSqlPw.ReleaseBuffer();

	if (!m_bStartWeight)  //终止还权
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return FALSE;
	}

	if (!m_sqlDB.Open(strIP, strDBN, strUID, strPw))
	{
		AddMsg(RPT_ERROR, "打开还权因子数据库失败");
		return FALSE;
	}

	CRecordSet reSet(&m_sqlDB);
	string strCon = "SELECT * FROM IND_S_RIGHT";
	if (m_bRealHQ) //实时还权
	{
		strCon += " WHERE F0010='";
		CTime tmCur = CTime::GetCurrentTime();
		CString strTime;
		strTime.Format("%d-%d-%d", tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay());
		strCon += strTime;
		strCon += "'";
	}
	else
	{
		strCon += " WHERE F0010<'";
		CTime tmCur = CTime::GetCurrentTime();
		CString strTime;
		strTime.Format("%d-%d-%d", tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay());
		strCon += strTime;
		strCon += "'";
	}

	strCon += " ORDER BY SEC_CD, F0010 DESC";

	if (!reSet.Open(strCon.c_str(), adOpenDynamic, adLockReadOnly))
	{
		AddMsg(RPT_ERROR, "打开还权因子数据库表IND_S_RIGHT失败");

		reSet.Close();
		m_sqlDB.Close();
		return FALSE;
	}

	while(!reSet.IsEOF() && m_bStartWeight)
	{
		tagHQYinZi* pYinZi = new tagHQYinZi;
		memset(pYinZi, 0, sizeof(tagHQYinZi));

		reSet.GetCollect("SEC_CD", pYinZi->szStockCode, 8);
		SYSTEMTIME systm = {0};
		reSet.GetCollect("F0010", &systm);
		pYinZi->nDate = systm.wYear*10000 + systm.wMonth*100 + systm.wDay;

		if (!CCheckThread::IsValidDate(pYinZi->nDate))
		{
			delete pYinZi;
			reSet.MoveNext();
			continue;
		}
		reSet.GetCollect("F0020", pYinZi->f0020);
		if (pYinZi->f0020 == 0)
			pYinZi->f0020 = 1;

		m_lstYinZi.push_back(pYinZi);
		reSet.MoveNext();
	}

	reSet.Close();
	m_sqlDB.Close();

	if (m_bStartWeight)
		AddMsg(RPT_INFO, "获取还权因子完毕，共[%d]记录", m_lstYinZi.size());
	return TRUE;
}

UINT CKLine_ExeDlg::_WeightThread(LPVOID lpParam)
{
	::CoInitialize(NULL);
	CKLine_ExeDlg* pDlg = (CKLine_ExeDlg*)lpParam;
	HWND hWnd = pDlg->m_hWnd;
	pDlg->WeightThread();
	::CoUninitialize();
	pDlg->m_bStartWeight = FALSE;

	if (pDlg->m_bRealHQ && theApp.m_bRWUpload)//如果实时还权后要上传还权k线则不关闭消息框
	{
	}
	else
	{
		::SendMessage(hWnd, WM_THREADEND, 0, 0);
	}
	return 0xDEAD0010;
}

LRESULT CKLine_ExeDlg::OnListMsg(WPARAM wParam, LPARAM lParam)
{
	CString strMsg = (char*)wParam;
	int nIndex = m_listResult.AddString(strMsg);
	m_listResult.SetCurSel(nIndex);

	return 0;
}

void CKLine_ExeDlg::WeightThread()
{
/*	m_bStartWeight = TRUE;
	if (m_bRealHQ) //实时还权
		AddMsg(RPT_INFO, "正在开始K线实时还权...");
	else
		AddMsg(RPT_INFO, "正在开始K线历史还权...");

	if (!BackupKLine())
	{
		AddMsg(RPT_ERROR, "复制K线文件到临时文件夹失败");
		return;
	}
	AddMsg(RPT_INFO, "复制K线文件到临时文件夹完毕");

	if (!m_bStartWeight)  //终止还权
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return;
	}

	if (m_bRealHQ)
	{
		AddMsg(RPT_INFO, "正在添加新生成的日线文件到临时文件夹中的还权日线文件中...");
		UpdateDayLineToWeight();
	}

	if (!m_bStartWeight)  //终止还权
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return;
	}

	if (!QueryWeight())  //获得还权因子
		return;

	//开始日k线还权
	AddMsg(RPT_INFO, "正在日线还权...");
	CTime tmCur = CTime::GetCurrentTime();
	UINT ntmCur = tmCur.GetYear()*10000 + tmCur.GetMonth()*100 + tmCur.GetDay();

	CYinZiList::iterator pos = m_lstYinZi.begin();
	CString strSymbol;
	UINT    nDate = 0;
	while(pos != m_lstYinZi.end())
	{
		if ( (ntmCur < (*pos)->nDate) || (strSymbol.Compare((*pos)->szStockCode) == 0 && nDate == (*pos)->nDate) )
		{
			delete (*pos);
			pos++;
			continue;
		}

		strSymbol = (*pos)->szStockCode;
		nDate     = (*pos)->nDate;
		if (m_bStartWeight)
		{
			StartWeight(*pos);
			StartWeight(*pos, FALSE);
		}
		delete (*pos);
		pos++;
	}
	m_lstYinZi.clear();
	m_KFileCur.Close();
	m_KFileHis.Close();
	AddMsg(RPT_INFO, "日线还权完毕");
	//用日线文件产生周线文件和月线文
	if (!m_bStartWeight)
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return;
	}

	CString strDay = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day";

	AddMsg(RPT_INFO, "正在产生周线和月线文件...");
	if (!GetWekMonLine(strDay))
	{
		AddMsg(RPT_ERROR, "用日线文件生产周线和月线文件失败");
		return;
	}
	AddMsg(RPT_INFO, "产生周线和月线完毕");

	if (!m_bStartWeight)  //终止还权
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return;
	}

	if (m_bRealHQ) //实时还权后检测文件大小保留最近100条
	{
		AddMsg(RPT_INFO, "正在检测文件大小...");
		CString strFilePath = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str();
		CheckFileSize(strFilePath);
		AddMsg(RPT_INFO, "检测文件大小完毕");
	}

	if (!m_bStartWeight)  //终止还权
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return;
	}
	//还权完毕后，从临时文件夹复制还权后K线文件到还权k线文件夹
	if (m_bStartWeight)
	{
		//如果已经存在还权k线文件，还权前询问用户是否删除该文件
		if (m_bStartWeight && CFilePath::IsFolderHasFile(theApp.m_strHQKLPath))
		{
			UINT nRetID = IDYES;
			if (!m_bRealHQ)
				nRetID = MessageBox("还权k线文件目录["+theApp.m_strHQKLPath +"]中已存在文件, 在还权前是否先删除该文件?", "警告", MB_YESNO);

			if (IDYES == nRetID)
			{//删除文件
				AddMsg(RPT_INFO, "正在删除还权k线文件目录[%s]里的文件...", theApp.m_strHQKLPath);
				CFilePath::DeleteFolderAllFile(theApp.m_strHQKLPath);
				AddMsg(RPT_INFO, "删除文件完毕");
			}
		}

		if (!m_bStartWeight)  //终止还权
		{
			AddMsg(RPT_WARNING, "k线还权已终止");
			return ;
		}
		AddMsg(RPT_INFO, "正在从临时文件夹中复制还权后的k线文件到[%s]...", theApp.m_strHQKLPath);
		if (!CFilePath::CopyFolderAllFile(theApp.m_strHQKLPath, theApp.m_strKLBkPath))
			AddMsg(RPT_ERROR, "从临时文件夹中复制还权后的k线文件到[%s]失败", theApp.m_strHQKLPath);

		AddMsg(RPT_INFO, "复制还权后的k线文件完毕");

		if (!m_bStartWeight)  //终止还权
		{
			AddMsg(RPT_WARNING, "k线还权已终止");
			return;
		}
		AddMsg(RPT_INFO, "正在删除临时文件目录[%s]里的文件...", theApp.m_strKLBkPath);
		CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
		AddMsg(RPT_INFO, "删除文件完毕");

		if (m_bRealHQ) //实时还权
			AddMsg(RPT_INFO, "K线实时还权完毕");
		else
			AddMsg(RPT_INFO, "K线历史还权完毕");
	}

	if (!m_bStartWeight)  //终止还权
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return;
	}

	//实时还权后上传
	if (m_bRealHQ && theApp.m_bRWUpload) //实时还权
	{
		AddMsg(RPT_INFO, "正在上传还权k线...");
		m_checkThread.StartUploadHQKLineFile();
		theApp.m_dlgInfo.m_strInfo = "正在上传当前库的还权K线...";
		theApp.m_dlgInfo.SendMessage(WM_CHANGEINFO);
	}*/
}

BOOL CKLine_ExeDlg::GetWekMonLine(CString strDayLine)
{
	if (strDayLine.IsEmpty())
		return FALSE;

	CString strFolder = strDayLine;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			GetWekMonLine(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFolder = ffind.GetFilePath(); //指数也要生成日线和月线
			GetSingleWekMonLine(strFolder);
		}
	}
	return TRUE;
}

BOOL CKLine_ExeDlg::GetSingleWekMonLine(CString strDayFile)
{
	//day
// 	CKLineList lstDayLine;
// 	CKLineList lstWekLine;
// 	CKLineList lstMonLine;
// 
// 	CString strFileName = strDayFile.Right(strDayFile.GetLength() - strDayFile.ReverseFind('\\') - 1);
// 	CString strHisWekFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\wek\\" + strFileName;
// 	CString strHisMonFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\mon\\" + strFileName;
// 
// 	CString strCurWekFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\wek\\" + strFileName;
// 	CString strCurMonFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\mon\\" + strFileName;
// 
// 	for (int i=0; i<2; i++)
// 	{
// 		if (i == 1) //当前库
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 		else
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\day\\" + strFileName;
// 
// 		CKLineFile kfile;
// 		if (!kfile.Open(strDayFile,  GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 		{
// 			AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strDayFile);
// 			continue;
// 		}
// 		while(!kfile.IsEOF())
// 		{
// 			tagKLineInfo* pInfo = new tagKLineInfo;
// 			memset(pInfo, 0, sizeof(tagKLineInfo));
// 
// 			kfile.GetRecordToBuffer((char*)pInfo);
// 			lstDayLine.push_back(pInfo);
// 
// 			kfile.MoveNext();
// 		}
// 		kfile.Close();
// 	}
// 	//计算周线
// 	{
// 		tagKLineInfo*  pWekInfo = NULL;
// 		BOOL bWekFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("无效的日期:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				pos++;
// 				continue;
// 			}
// 			//UINT nWek = CCheckThread::GetDayofWeek((*pos)->nDate);
// 			UINT nPreDate =	(*pos)->nDate;
// 			if (bWekFirstDay)
// 			{
// 				bWekFirstDay = FALSE;
// 				pWekInfo = new tagKLineInfo;
// 				memcpy(pWekInfo, (*pos), sizeof(tagKLineInfo));
// 			}
// 			else
// 			{
// 				pWekInfo->nClosePrice = (*pos)->nClosePrice;
// 				if ((*pos)->nMaxPrice > pWekInfo->nMaxPrice)
// 					pWekInfo->nMaxPrice  = (*pos)->nMaxPrice;
// 				if ((*pos)->nMinPrice < pWekInfo->nMinPrice)
// 					pWekInfo->nMinPrice  = (*pos)->nMinPrice;
// 				pWekInfo->nSum += (*pos)->nSum;
// 				pWekInfo->nVolume += (*pos)->nVolume;
// 			}
// 			pos++;
// 			if (pos == lstDayLine.end())
// 				break;
// 
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("无效的日期:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				continue;
// 			}
// 			UINT nNextDate = (*pos)->nDate;
// 			if (!IsSameWeek(nPreDate, nNextDate)) //另一周	  
// 			{
// 				bWekFirstDay = TRUE;
// 				lstWekLine.push_back(pWekInfo);
// 				pWekInfo = NULL;
// 			}
// 		}
// 		if (pWekInfo != NULL)
// 			lstWekLine.push_back(pWekInfo); //最后一个
// 	}
// 	//计算月线
// 	{
// 		tagKLineInfo*  pMonInfo = NULL;
// 		BOOL bMonFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("无效的日期:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				delete (*pos);
// 				pos++;
// 				continue;
// 			}
// 			if (bMonFirstDay)
// 			{
// 				bMonFirstDay = FALSE;
// 				pMonInfo = new tagKLineInfo;
// 				memcpy(pMonInfo, (*pos), sizeof(tagKLineInfo));
// 			}
// 			else
// 			{
// 				pMonInfo->nClosePrice = (*pos)->nClosePrice;
// 				if ((*pos)->nMaxPrice > pMonInfo->nMaxPrice)
// 					pMonInfo->nMaxPrice  = (*pos)->nMaxPrice;
// 				if ((*pos)->nMinPrice < pMonInfo->nMinPrice)
// 					pMonInfo->nMinPrice  = (*pos)->nMinPrice;
// 				pMonInfo->nSum += (*pos)->nSum;
// 				pMonInfo->nVolume += (*pos)->nVolume;
// 			}
// 			delete (*pos);
// 			pos++;
// 			if (pos == lstDayLine.end())
// 				break;
// 
// 			if ((*pos)->nDate % 10000 / 100 != pMonInfo->nDate % 10000 / 100) //另一月
// 			{
// 				bMonFirstDay = TRUE;
// 				lstMonLine.push_back(pMonInfo);
// 				pMonInfo = NULL;
// 			}
// 		}
// 		if (pMonInfo != NULL)
// 			lstMonLine.push_back(pMonInfo); //最后一个
// 	}
// 
// 	{//保存周线
// 		CFile fHisWekL;
// 		CFile fCurWekL;
// 
// 		if (!fCurWekL.Open(strCurWekFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strCurWekFile);
// 
// 		CKLineList::iterator pos = lstWekLine.begin();
// 		int nHisCount = (int)lstWekLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0) //有多余的记录才打开历史文件
// 		{
// 			if (!fHisWekL.Open(strHisWekFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strHisWekFile);
// 		}
// 
// 		while(pos != lstWekLine.end())
// 		{
// 			if (nHisCount <= 0) //保存在当前库
// 			{
// 				fCurWekL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//保存在历史库
// 			{
// 				fHisWekL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			delete (*pos);
// 			pos++;
// 			nHisCount--;
// 		}
// 
// 		if (nHisCount > 0)
// 			fHisWekL.Close();
// 		fCurWekL.Close();
// 	}
// 
// 	{//保存月线
// 		CFile fHisMonL;
// 		CFile fCurMonL;
// 		CKLineList::iterator pos = lstMonLine.begin();
// 		int nHisCount = (int)lstMonLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0)
// 		{
// 			if (!fHisMonL.Open(strHisMonFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strHisMonFile);
// 		}
// 
// 		if (!fCurMonL.Open(strCurMonFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strCurMonFile);
// 
// 		while(pos != lstMonLine.end())
// 		{
// 			if (nHisCount <= 0) //保存在当前库
// 			{
// 				fCurMonL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//保存在历史库
// 			{
// 				fHisMonL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			delete (*pos);
// 			pos++;
// 			nHisCount--;
// 		}
// 
// 		if (nHisCount > 0)
// 			fHisMonL.Close();
// 		fCurMonL.Close();
// 	}

	return TRUE;
}

void CKLine_ExeDlg::StartWeight(tagHQYinZi* pWeight, BOOL bCurKLine)
{
// 	CString strStockCode = pWeight->szStockCode;
// 	int nStockCode = atoi(pWeight->szStockCode);
// 	if (nStockCode < 3000 || nStockCode >= 200000 && nStockCode < 300000)   //深圳, 其余上海
// 		strStockCode += ".sz";
// 	else
// 		strStockCode += ".sh";
// 
// 	CString strFilePath = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str();
// 	if (!bCurKLine)
// 		strFilePath = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str();
// 
// 	strFilePath += "\\day\\" + strStockCode;
// 
// 	CKLineFile* pKLFile = NULL;
// 	if (bCurKLine)
// 		pKLFile = &m_KFileCur;
// 	else
// 		pKLFile = &m_KFileHis;
// 
// 	if (!CFilePath::IsFileExist(strFilePath))
// 	{
// 		AddMsg(RPT_WARNING, "k线文件[%s]不存在", strFilePath);
// 	}
// 	else
// 	{
// 		if ((bCurKLine && m_strCurKFilePath.Compare(strFilePath) == 0 || 
// 			!bCurKLine && m_strHisKFilePath.Compare(strFilePath) == 0)
// 			&& pKLFile->IsOpen())
// 		{ //同一个文件只打开一次
// 			pKLFile->MoveFirst();
// 		}
// 		else
// 		{
// 			if (pKLFile->IsOpen()) 
// 				pKLFile->Close(); //关闭上次打开的文件
// 
// 			if (!pKLFile->Open(strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 			{
// 				AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strFilePath);
// 				if (bCurKLine)
// 					m_strCurKFilePath.Empty();
// 				else
// 					m_strHisKFilePath.Empty();
// 			}
// 			else
// 			{
// 				if (bCurKLine)
// 					m_strCurKFilePath = strFilePath; //保存上一个文件路径，如果下一个和上一个是同一个文件，则只打开一次
// 				else
// 					m_strHisKFilePath = strFilePath;
// 			}
// 		}
// 
// 		while(!pKLFile->IsEOF() && m_bStartWeight)
// 		{
// 			tagKLineInfo klInfo = {0};
// 			pKLFile->GetRecordToBuffer((char*)&klInfo);
// 
// 			if (klInfo.nDate >= pWeight->nDate)  //小于还权日期的所有k线还权
// 				break;
// 
// 			klInfo.nOpenPrice  *= pWeight->f0020;
// 			klInfo.nMaxPrice   *= pWeight->f0020;
// 			klInfo.nMinPrice   *= pWeight->f0020;
// 			klInfo.nClosePrice *= pWeight->f0020;
// 			klInfo.nVolume     /= pWeight->f0020;
// 
// 			pKLFile->SetRecordBuffer((char*)&klInfo);
// 			pKLFile->MoveNext();
// 		}
// 		if (!pKLFile->WriteData())
// 			AddMsg(RPT_ERROR, "保存文件[]失败", strFilePath);
// 	}
}

//void CKLine_ExeDlg::OnMiStopweight()
//{
//	AddMsg(RPT_WARNING, "正在终止k线还权...");
//	m_bStartWeight = FALSE;
//	EnableWindow(FALSE);
//	m_pWeightThread->Stop();
//}

void CKLine_ExeDlg::StopWeight()
{
	AddMsg(RPT_WARNING, "正在终止k线还权...");
	m_bStartWeight = FALSE;
	EnableWindow(FALSE);

	m_pWeightThread->Stop();
}
//void CKLine_ExeDlg::OnUpdateMiStopweight(CCmdUI *pCmdUI)
//{
//	pCmdUI->Enable(m_bStartWeight);
//}

void CKLine_ExeDlg::OnMiUploadHq()
{
	AddMsg(RPT_INFO, "正在上传还权k线...");
	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartUploadHQKLineFile();
	dlgInfo.m_strInfo = "正在上传当前库的还权k线...";
	if (dlgInfo.DoModal() == IDCANCEL)
	{
		AddMsg(RPT_INFO, "用户取消上传当前库的还权k线\n");
		theApp.m_pcheckThread->StopUploadandDownload();

		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		dlgInfo.m_strInfo = "正在终止上传...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}
}

void CKLine_ExeDlg::OnMiDownloadHq()
{
	AddMsg(RPT_INFO, "正在下载还权k线...");
	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartDownloadHQKLineFile();
	dlgInfo.m_strInfo = "正在下载当前库的还权k线...";
	if (dlgInfo.DoModal() == IDCANCEL)
	{
		AddMsg(RPT_INFO, "用户取消下载当前库的还权k线\n");
		theApp.m_pcheckThread->StopUploadandDownload();

		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		dlgInfo.m_strInfo = "正在终止下载...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}
}

void CKLine_ExeDlg::UpdateDayLineToWeight()  //更新最近生成的日线数据到还权日线数据中，然后再实时还权
{
	UpdateAllFile(theApp.m_strKFilePath);
}

void CKLine_ExeDlg::UpdateAllFile(CString strFile)
{
	if (strFile.IsEmpty())
		return;

	CString strFolder = strFile;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			CString strFolder = CValidFunction::GetFilePathPreFolder(strPath);
			if (strFolder.CompareNoCase("day") == 0)
				UpdateAllFile(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFolder = ffind.GetFilePath();
			CString strFileName = ffind.GetFileName();
			UpdateSingleFile(strFolder, strFileName);
		}
	}
}

void CKLine_ExeDlg::UpdateSingleFile(CString strFile, CString strFileName)
{
// 	CString strExe = CValidFunction::GetExeName(strFile);
// 	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
// 		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
// 		&& strExe.CompareNoCase(".hk") != 0 && strExe.CompareNoCase(".hkidx") != 0)
// 	{
// 		AddMsg(RPT_ERROR, "错误: 无法解析的文件[%s]!", strFile);
// 		return;
// 	}
// 
// 	CString strHQFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 
// 	CKLineFile hqfile;     //还权日线
// 	CKLineFile orifile;	   //原始日线
// 	if (!hqfile.Open(strHQFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS))
// 	{
// 		AddMsg(RPT_ERROR, "错误: 打开文件[%s]失败!", strHQFile);
// 		return;
// 	}
// 	hqfile.MoveLast();
// 	tagKLineInfo info = {0};
// 	hqfile.GetRecordToBuffer((char*)&info);
// 
// 	if (!orifile.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 	{
// 		AddMsg(RPT_ERROR, "错误: 打开文件[%s]失败!", strHQFile);
// 		return;
// 	}
// 
// 	char* pNewKLineBuf = new char[orifile.GetAllRecordSize()];
// 	char* pBufPos = pNewKLineBuf;
// 	tagKLineInfo tempinfo = {0};
// 	int nNeedAddCount = 0;
// 	while(!orifile.IsEOF())
// 	{
// 		orifile.GetRecordToBuffer((char*)&tempinfo);
// 		if (tempinfo.nDate > info.nDate)
// 		{
// 			//AddMsg(RPT_INFO, "Date:[%d] Date:[%d] FilePath:[%s]", tempinfo.nDate, info.nDate, strHQFile);
// 			memcpy(pBufPos, &tempinfo, sizeof(tagKLineInfo));
// 			pBufPos += sizeof(tagKLineInfo);
// 			nNeedAddCount++;
// 		}
// 		orifile.MoveNext();
// 	}
// 	orifile.Close();
// 
// 	if (nNeedAddCount == 0)
// 	{
// 		delete []pNewKLineBuf;
// 		return;
// 	}
// 
// 	int nHqKLineCount = hqfile.GetRecordCount();
// 	nHqKLineCount += nNeedAddCount;
// 
// 	char* pOldKLineBuf = new char[nHqKLineCount*sizeof(tagKLineInfo)];
// 	hqfile.GetAllRecordToBuffer(pOldKLineBuf);
// 	memcpy(pOldKLineBuf+(nHqKLineCount-nNeedAddCount)*sizeof(tagKLineInfo), pNewKLineBuf, nNeedAddCount*sizeof(tagKLineInfo));
// 	delete []pNewKLineBuf;
// 
// 	if (!hqfile.AllocateMemoryForSave(nHqKLineCount))
// 	{
// 		AddMsg(RPT_ERROR, "错误: AllocateMemoryForSave() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.SetAllRecordToBuffer(pOldKLineBuf))
// 	{
// 		AddMsg(RPT_ERROR, "错误: SetAllRecordToBuffer() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.WriteData())
// 	{
// 		AddMsg(RPT_ERROR, "错误: WriteData() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	hqfile.Close();
// 	delete []pOldKLineBuf;
}

void CKLine_ExeDlg::CheckFileSize(CString strFile)
{
	if (strFile.IsEmpty())
		return;

	CString strFolder = strFile;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			CheckFileSize(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFileName();
			CString strFolder = ffind.GetFilePath();
			CutFile(strFolder);
		}
	}
}

void CKLine_ExeDlg::CutFile(CString strFile)
{
// 	CString strExe = CValidFunction::GetExeName(strFile);
// 	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
// 		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
// 		&& strExe.CompareNoCase(".hk") != 0 && strExe.CompareNoCase(".hkidx") != 0)
// 	{
// 		AddMsg(RPT_ERROR, "错误: 无法解析的文件[%s]!", strFile);
// 		return;
// 	}
// 	CKLineFile curfile; 
// 	if (!curfile.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 	{
// 		AddMsg(RPT_ERROR, "错误: 打开文件[%s]失败!", strFile);
// 		return;
// 	}
// 
// 	int nRecCount  = curfile.GetRecordCount();
// 	int nReserveCount = theApp.m_nReserveCount < (UINT)nRecCount ? theApp.m_nReserveCount:nRecCount;
// 	int nDelCount  = nRecCount - nReserveCount;
// 	int nRecSize   = curfile.GetRecordSize();
// 	if (nDelCount == 0)
// 	{
// 		curfile.Close();
// 		return;
// 	}
// 
// 	//保存要删除的记录
// 	CString strName = CValidFunction::GetFileNamePrePath(strFile);
// 	CString strHisName = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\" + strName;
// 	CString strHisPath = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\";
// 	strHisPath += CValidFunction::GetFileNamePreFolder(strFile);	
// 	if (!CValidFunction::IsPathExist(strHisPath))
// 	{
// 		CValidFunction::CreatePath(strHisPath);
// 	}
// 
// 	CKLineFile kfileHis;
// 	if (!kfileHis.Open(strHisName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS))
// 	{
// 		AddMsg(RPT_ERROR, "错误: 打开文件[%s]失败!", strHisName);
// 		curfile.Close();
// 		return;
// 	}
// 
// 	if (!kfileHis.AllocateMemoryForSave(nDelCount))
// 	{
// 		curfile.Close();
// 		kfileHis.Close();
// 		AddMsg(RPT_ERROR, "错误: 给文件[%s]分配内存失败!", strHisName);
// 		return;
// 	}
// 
// 	kfileHis.MoveFirst();
// 	curfile.MoveFirst();
// 	char *pRecBuf = new char[nRecSize];
// 	while(!kfileHis.IsEOF())
// 	{
// 		curfile.GetRecordToBuffer(pRecBuf);
// 		kfileHis.SetRecordBuffer(pRecBuf);
// 		kfileHis.MoveNext();
// 		curfile.MoveNext();
// 	}
// 	delete []pRecBuf;
// 	pRecBuf = NULL;
// 
// 	if (!kfileHis.WriteDataToEnd())
// 	{
// 		AddMsg(RPT_ERROR, "错误: 保存文件[%s]失败!", strHisName);
// 	}
// 
// 	kfileHis.Close();
// 
// 	{//删除多余的记录
// 
// 		//AddMsg(RPT_INFO, "[%s]删除多余的记录!", strFile);
// 		char* pRecBuf = new char[nRecSize*nReserveCount];
// 		if (pRecBuf == NULL)
// 		{
// 			AddMsg(RPT_ERROR, "错误: 分配内存失败!");
// 			curfile.Close();
// 			return;
// 		}
// 
// 		char* pBufPos = pRecBuf + nRecSize*(nReserveCount-1);
// 		curfile.MoveLast();
// 		for(int i=0; i<nReserveCount; i++)
// 		{
// 			curfile.GetRecordToBuffer(pBufPos);
// 			curfile.MovePrev();
// 			pBufPos -= nRecSize;
// 		}
// 
// 		if (!curfile.AllocateMemoryForSave(nReserveCount))
// 		{
// 			AddMsg(RPT_ERROR, "错误: 给文件[%s]分配内存失败!", strFile);
// 			curfile.Close();
// 			return;
// 		}
// 
// 		if (!curfile.SetAllRecordToBuffer(pRecBuf))
// 		{
// 			AddMsg(RPT_ERROR, "错误: 保存文件[%s]失败!", strFile);
// 			curfile.Close();
// 			return;
// 		}
// 		if (!curfile.WriteData())
// 		{
// 			AddMsg(RPT_ERROR, "错误: 保存文件[%s]失败!", strFile);
// 		}
// 
// 		delete []pRecBuf;
// 		pRecBuf = NULL;
// 	}
// 	curfile.Close();	
}

BOOL CKLine_ExeDlg::IsSameWeek(unsigned int uiNow, unsigned int uiPrev)
{//比较今日和所给定日期是否在同一周内
	tm    stPrevTime,stNowTime;
	memset(&stPrevTime,0,sizeof(tm));
	stPrevTime.tm_year=uiPrev/10000-1900;
	stPrevTime.tm_mon=uiPrev%10000/100-1;
	stPrevTime.tm_mday=uiPrev%100;
	time_t stPrev=mktime(&stPrevTime);

	memset(&stNowTime,0,sizeof(tm));
	stNowTime.tm_year=uiNow/10000-1900;
	stNowTime.tm_mon=uiNow%10000/100-1;
	stNowTime.tm_mday=uiNow%100;
	time_t stNow=mktime(&stNowTime);

	if(stNow>stPrev && difftime(stNow,stPrev)>=7*24*3600)
		return FALSE;
	else if(stPrev>stNow && difftime(stPrev,stNow)>=7*24*3600)
		return FALSE;
	if(stNowTime.tm_wday<stPrevTime.tm_wday && stNow>stPrev)
		return FALSE;
	else if(stNowTime.tm_wday>stPrevTime.tm_wday && stNow<stPrev)
		return FALSE;

	return TRUE;
}

void CKLine_ExeDlg::OnMiCutfile()
{
// 	CFolderDlg dlg;
// 	if (dlg.DoModal() == IDCANCEL)
// 		return;
// 	m_strCheckSizeFilePath = dlg.GetPathName();
// 
// 	//如果文件大小超过限制大小
// 	CDlg_Info dlgInfo;
// 	theApp.m_pDlgInfoWnd = &dlgInfo;
// 	theApp.m_pcheckThread->StartCheckFileSize(m_strCheckSizeFilePath);
// 
// 	dlgInfo.m_strInfo = "正在分割文件...";
// 	if (dlgInfo.DoModal() == IDCANCEL) //取消操作
// 		theApp.m_pcheckThread->Stop();
}

void CKLine_ExeDlg::OnBnClickedBtnAddrecord()
{
// 	int nIndex = m_listInfo.AddItem();
// 	m_listInfo.SetItemState(m_listInfo.GetSelectionMark(), 0 ,LVIS_SELECTED); 
// 	m_listInfo.SetItemState(nIndex, LVIS_SELECTED ,LVIS_SELECTED);
// 	m_listInfo.SetSelectionMark(nIndex);
// 	m_listInfo.EnsureVisible(nIndex,TRUE);
}
