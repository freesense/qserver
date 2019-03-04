// KLine_ExeDlg.cpp : ʵ���ļ�
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
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
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


// CKLine_ExeDlg �Ի���




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


// CKLine_ExeDlg ��Ϣ�������

BOOL CKLine_ExeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	Init();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CKLine_ExeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CKLine_ExeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKLine_ExeDlg::Init()
{	
	m_pWeightThread = new CWorkThread(_WeightThread, this);
	InitCtrlPos();

	m_listFile.InsertColumn(0, "�����ļ�", LVCFMT_RIGHT, 160);
	m_listFile.InsertColumn(1, "·    ��", LVCFMT_RIGHT, 140);
	{//init listctrl
		m_listInfo.InsertColumn(0, "��    ��", LVCFMT_RIGHT, 80);
		m_listInfo.InsertColumn(1, "���̼۸�", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(2, "��߼۸�", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(3, "��ͼ۸�", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(4, "���̼۸�", LVCFMT_RIGHT, 60);
		m_listInfo.InsertColumn(5, "�ɽ�����", LVCFMT_RIGHT, 80);
		m_listInfo.InsertColumn(6, "�ɽ����", LVCFMT_RIGHT, 80);

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

	SetTimer(IDT_AUTOCHECK, 1000, NULL); //һ���Ӽ��һ��
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
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
// 		{//���ܶԻ�Ȩk������У��
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
// 		dlgInfo.m_strInfo = "���ڼ���ļ�...";
// 		if (dlgInfo.DoModal() == IDCANCEL) //ȡ������
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
		if (MessageBox("�������޸ģ�������������ݣ����޸���Ч���Ƿ���Ҫ�������ݣ�",
			"����", MB_YESNO) == IDYES)
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
		dlgInfo.m_strInfo = "���ڼ���ļ�...";
	}
	else
	{
		m_listFile.DeleteAllItems();
		theApp.m_pcheckThread->StartCheckAllFile(m_strCheckAllPath);
		dlgInfo.m_strInfo = "���ڼ��ѡ��Ŀ¼�������ļ�...";
	}

	if (dlgInfo.DoModal() == IDCANCEL) //ȡ������
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

	if (MessageBox("ȷ��Ҫɾ��ѡ�еļ�¼��?", "����", MB_YESNO) == IDYES)
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

	m_bDataIntegrality = TRUE;  //�����ɣ�û�б���;ֹͣ

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
	if (m_bDataIntegrality) //���������ԣ����ļ�������������
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
	//����ļ���С�������ƴ�С
	//m_checkThread.StartCheckFileSize();

	dlgInfo.m_strInfo = "���ڼ��ѡ��Ŀ¼�������ļ�...";
	if (dlgInfo.DoModal() == IDCANCEL) //ȡ������
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
		dlgInfo.m_strInfo = "���ڼ���ļ�...";
		theApp.m_pcheckThread->CheckSingleFile(strPath);

		if (dlgInfo.DoModal() == IDCANCEL) //ȡ������
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
// 			dlgInfo.m_strInfo = "�������ص�ǰ���ԭʼk��...";
// 			dlgInfo.m_bEnableBtn = FALSE;
// 			dlgInfo.DoModal();
// 			dlgInfo.m_bEnableBtn = TRUE;
		}

		if (nTime == theApp.m_nRealHQTime && theApp.m_bRWUpload) //ʵʱ��Ȩ
		{
			m_bRealHQ = TRUE;
			StartHuanQuan();
		}

		if (nTime == theApp.m_nDownHQTime && theApp.m_bDownLoadHQ) //��ʱ���ػ�Ȩ�ļ�
		{
			CDlg_Info dlgInfo;
			theApp.m_pDlgInfoWnd = &dlgInfo;
			theApp.m_pcheckThread->StartDownloadHQKLineFile();
			dlgInfo.m_strInfo = "�������ص�ǰ��Ļ�Ȩk��...";
			dlgInfo.m_bEnableBtn = FALSE;
			dlgInfo.DoModal();
			dlgInfo.m_bEnableBtn = TRUE;
		}
	}

	CDialog::OnTimer(nIDEvent);		
}

void CKLine_ExeDlg::AutoCheck()
{
	//��������ļ�
// 	m_bIsCheckAll = TRUE;
// 	InitCtrlPos(FALSE);
// 	m_listFile.DeleteAllItems();
// 	m_listResult.ResetContent();
// 	m_nEnableType = ET_CHECK_ALL;
// 	UpdateDialogControls(NULL, FALSE);
// 
// 	CDlg_Info dlgInfo;
// 	theApp.m_pDlgInfoWnd = &dlgInfo;
// 	if (theApp.m_bUserHttpData)  //ʹ���Ż��ƽ���������У�ԣ������´�������
// 	{
// 		dlgInfo.m_strInfo = "����ʹ����������У�Ե�ǰ���ԭʼk��...";
// 		theApp.m_pcheckThread->StartCheckAllFileUseHttpData(theApp.m_strKFilePath);
// 	}
// 	else
// 	{
// 		dlgInfo.m_strInfo = "���ڼ�⵱ǰ���ԭʼk��...";
// 		theApp.m_pcheckThread->StartCheckAllFile(theApp.m_strKFilePath, TRUE);	
// 	}
// 
// 	dlgInfo.m_bEnableBtn = FALSE;
// 	dlgInfo.DoModal();
// 	dlgInfo.m_bEnableBtn = TRUE;
// 
// 	//����ļ���С�������ƴ�С
// 	CDlg_Info dlgInfo2;
// 	theApp.m_pDlgInfoWnd = &dlgInfo2;
// 	dlgInfo2.m_strInfo = "���ڼ�⵱ǰ��ԭʼk�ߵĴ�С...";
// 	theApp.m_pcheckThread->StartCheckFileSize(theApp.m_strKFilePath);
// 	dlgInfo2.m_bEnableBtn = FALSE;
// 	dlgInfo2.DoModal();
// 	dlgInfo2.m_bEnableBtn = TRUE;
// 
// 	//ѹ���ϴ�k���ļ�
// 	if (theApp.m_bUseUpLoad)
// 	{
// 		CDlg_Info dlgInfo3;
// 		theApp.m_pDlgInfoWnd = &dlgInfo3;
// 		dlgInfo3.m_strInfo = "�����ϴ���ǰ���ԭʼk��...";
// 		theApp.m_pcheckThread->StartRarUpLoadKFile();
// 		dlgInfo3.m_bEnableBtn = FALSE;
// 		dlgInfo3.DoModal();
// 		dlgInfo3.m_bEnableBtn = TRUE;
// 	}
}
void CKLine_ExeDlg::OnBnClickedBtnUsehttpdata()
{
	//if (MessageBox("����У�Ի��޸����ݣ�ȷ��Ҫʹ������У����?", "����", MB_YESNO) == IDNO)
	//	return;
	m_bDataIntegrality = FALSE;

	m_nEnableType = ET_START_CHECK;
	UpdateDialogControls(NULL, FALSE);

	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	if (m_bIsCheckAll)
	{
		theApp.m_pcheckThread->StartCheckAllFileUseHttpData(theApp.m_strKFilePath);
		dlgInfo.m_strInfo = "����ʹ����������У�������ļ�...";
	}
	else
	{
		theApp.m_pcheckThread->StartCheckSingeleFileUseHttpData();
		dlgInfo.m_strInfo = "����ʹ����������У���ļ�...";
	}

	if (dlgInfo.DoModal() == IDCANCEL) //ȡ������
		OnBnClickedBtnStopcheck();
}


void CKLine_ExeDlg::OnDownloadkfile()
{
// 	CDlg_Info dlgInfo;
// 	theApp.m_pDlgInfoWnd = &dlgInfo;
// 	theApp.m_pcheckThread->StartUnRarCurKLineFile();
// 	dlgInfo.m_strInfo = "�������ص�ǰ���ԭʼk��...";
// 	if (dlgInfo.DoModal() == IDCANCEL)
// 	{
// 		AddMsg(RPT_INFO, "�û�ȡ�����ص�ǰ���ԭʼk��\n");
// 		theApp.m_pcheckThread->StopUploadandDownload();
// 
// 		CDlg_Info dlgInfo;
// 		theApp.m_pDlgInfoWnd = &dlgInfo;
// 		dlgInfo.m_strInfo = "������ֹ����...";
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
// 	dlgInfo.m_strInfo = "�����ϴ���ǰ���ԭʼk��...";
// 	if (dlgInfo.DoModal() == IDCANCEL)
// 	{
// 		AddMsg(RPT_INFO, "�û�ȡ���ϴ���ǰ���ԭʼk��\n");
// 		theApp.m_pcheckThread->StopUploadandDownload();
// 
// 		CDlg_Info dlgInfo;
// 		theApp.m_pDlgInfoWnd = &dlgInfo;
// 		dlgInfo.m_strInfo = "������ֹ�ϴ�...";
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
	// TODO: �ڴ���������������
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
	//	AddMsg(RPT_ERROR, "����K����ʷ��Ȩʧ��");

	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartWeightThread(m_bRealHQ);

	if (m_bRealHQ)
	{
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.m_strInfo = "���ڶ�K��ʵʱ��Ȩ...";
	}
	else
		dlgInfo.m_strInfo = "���ڶ�K����ʷ��Ȩ...";

	if (dlgInfo.DoModal() == IDCANCEL) //ȡ������
	{
		theApp.m_pcheckThread->StopWeightThread();

		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		dlgInfo.m_strInfo = "������ֹ��Ȩ...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}

	dlgInfo.m_bEnableBtn = TRUE;
}

BOOL CKLine_ExeDlg::BackupKLine()
{
	return TRUE;
// 	AddMsg(RPT_INFO, "���ڱ���k�����ݵ���ʱĿ¼[%s]...", theApp.m_strKLBkPath);
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return FALSE;
// 	}
// 	if (CFilePath::IsFolderHasFile(theApp.m_strKLBkPath))
// 	{
// 		UINT nRetID = IDYES;
// 		if (!m_bRealHQ)
// 			nRetID = MessageBox("��ʱĿ¼["+theApp.m_strKLBkPath +"]���Ѵ����ļ�, �ڸ����ļ�ǰ�Ƿ���ɾ�����ļ�?", "����", MB_YESNO);
// 
// 		if (nRetID == IDYES)
// 		{//ɾ���ļ�
// 			AddMsg(RPT_INFO, "����ɾ����ʱĿ¼[%s]����ļ�...", theApp.m_strKLBkPath);
// 			CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
// 			AddMsg(RPT_INFO, "ɾ���ļ����");
// 		}
// 	}
// 
// 	m_strCurKLine = CFilePath::GetPathLastFolderName(theApp.m_strKFilePath);
// 	m_strHisKLine = CFilePath::GetPathLastFolderName(theApp.m_strHisKFilePath);
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return FALSE;
// 	}
// 
// 	BOOL bRet = TRUE;
// 	if (m_bRealHQ) //ʵʱ��Ȩ
// 	{
// 		AddMsg(RPT_INFO, "���ڸ���[%s]����ļ�����ʱĿ¼[%s]...", theApp.m_strHQKLPath, theApp.m_strKLBkPath);
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath, theApp.m_strHQKLPath);
// 	}
// 	else		   //��ʷ��Ȩ
// 	{
// 		AddMsg(RPT_INFO, "���ڸ���[%s]����ļ�����ʱĿ¼[%s]...", theApp.m_strKFilePath, theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str(), theApp.m_strKFilePath);
// 		AddMsg(RPT_INFO, "���ڸ���[%s]����ļ�����ʱĿ¼[%s]...", theApp.m_strHisKFilePath, theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str(), theApp.m_strHisKFilePath);
// 	}
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
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
	AddMsg(RPT_INFO, "���ڴ����ݿ�[%s:%s]��ȡ��Ȩ����...", theApp.m_strSqlIP, theApp.m_strSqlDBN);
	//�����ݿ��ȡ���ݵ��ڴ�
	std::string strIP, strDBN, strUID, strPw;
	strIP  = theApp.m_strSqlIP.GetBuffer();
	theApp.m_strSqlIP.ReleaseBuffer();
	strDBN = theApp.m_strSqlDBN.GetBuffer();
	theApp.m_strSqlDBN.ReleaseBuffer();
	strUID = theApp.m_strSqlUID.GetBuffer();
	theApp.m_strSqlUID.ReleaseBuffer();
	strPw  = theApp.m_strSqlPw.GetBuffer();
	theApp.m_strSqlPw.ReleaseBuffer();

	if (!m_bStartWeight)  //��ֹ��Ȩ
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return FALSE;
	}

	if (!m_sqlDB.Open(strIP, strDBN, strUID, strPw))
	{
		AddMsg(RPT_ERROR, "�򿪻�Ȩ�������ݿ�ʧ��");
		return FALSE;
	}

	CRecordSet reSet(&m_sqlDB);
	string strCon = "SELECT * FROM IND_S_RIGHT";
	if (m_bRealHQ) //ʵʱ��Ȩ
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
		AddMsg(RPT_ERROR, "�򿪻�Ȩ�������ݿ��IND_S_RIGHTʧ��");

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
		AddMsg(RPT_INFO, "��ȡ��Ȩ������ϣ���[%d]��¼", m_lstYinZi.size());
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

	if (pDlg->m_bRealHQ && theApp.m_bRWUpload)//���ʵʱ��Ȩ��Ҫ�ϴ���Ȩk���򲻹ر���Ϣ��
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
	if (m_bRealHQ) //ʵʱ��Ȩ
		AddMsg(RPT_INFO, "���ڿ�ʼK��ʵʱ��Ȩ...");
	else
		AddMsg(RPT_INFO, "���ڿ�ʼK����ʷ��Ȩ...");

	if (!BackupKLine())
	{
		AddMsg(RPT_ERROR, "����K���ļ�����ʱ�ļ���ʧ��");
		return;
	}
	AddMsg(RPT_INFO, "����K���ļ�����ʱ�ļ������");

	if (!m_bStartWeight)  //��ֹ��Ȩ
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return;
	}

	if (m_bRealHQ)
	{
		AddMsg(RPT_INFO, "������������ɵ������ļ�����ʱ�ļ����еĻ�Ȩ�����ļ���...");
		UpdateDayLineToWeight();
	}

	if (!m_bStartWeight)  //��ֹ��Ȩ
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return;
	}

	if (!QueryWeight())  //��û�Ȩ����
		return;

	//��ʼ��k�߻�Ȩ
	AddMsg(RPT_INFO, "�������߻�Ȩ...");
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
	AddMsg(RPT_INFO, "���߻�Ȩ���");
	//�������ļ����������ļ���������
	if (!m_bStartWeight)
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return;
	}

	CString strDay = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day";

	AddMsg(RPT_INFO, "���ڲ������ߺ������ļ�...");
	if (!GetWekMonLine(strDay))
	{
		AddMsg(RPT_ERROR, "�������ļ��������ߺ������ļ�ʧ��");
		return;
	}
	AddMsg(RPT_INFO, "�������ߺ��������");

	if (!m_bStartWeight)  //��ֹ��Ȩ
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return;
	}

	if (m_bRealHQ) //ʵʱ��Ȩ�����ļ���С�������100��
	{
		AddMsg(RPT_INFO, "���ڼ���ļ���С...");
		CString strFilePath = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str();
		CheckFileSize(strFilePath);
		AddMsg(RPT_INFO, "����ļ���С���");
	}

	if (!m_bStartWeight)  //��ֹ��Ȩ
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return;
	}
	//��Ȩ��Ϻ󣬴���ʱ�ļ��и��ƻ�Ȩ��K���ļ�����Ȩk���ļ���
	if (m_bStartWeight)
	{
		//����Ѿ����ڻ�Ȩk���ļ�����Ȩǰѯ���û��Ƿ�ɾ�����ļ�
		if (m_bStartWeight && CFilePath::IsFolderHasFile(theApp.m_strHQKLPath))
		{
			UINT nRetID = IDYES;
			if (!m_bRealHQ)
				nRetID = MessageBox("��Ȩk���ļ�Ŀ¼["+theApp.m_strHQKLPath +"]���Ѵ����ļ�, �ڻ�Ȩǰ�Ƿ���ɾ�����ļ�?", "����", MB_YESNO);

			if (IDYES == nRetID)
			{//ɾ���ļ�
				AddMsg(RPT_INFO, "����ɾ����Ȩk���ļ�Ŀ¼[%s]����ļ�...", theApp.m_strHQKLPath);
				CFilePath::DeleteFolderAllFile(theApp.m_strHQKLPath);
				AddMsg(RPT_INFO, "ɾ���ļ����");
			}
		}

		if (!m_bStartWeight)  //��ֹ��Ȩ
		{
			AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
			return ;
		}
		AddMsg(RPT_INFO, "���ڴ���ʱ�ļ����и��ƻ�Ȩ���k���ļ���[%s]...", theApp.m_strHQKLPath);
		if (!CFilePath::CopyFolderAllFile(theApp.m_strHQKLPath, theApp.m_strKLBkPath))
			AddMsg(RPT_ERROR, "����ʱ�ļ����и��ƻ�Ȩ���k���ļ���[%s]ʧ��", theApp.m_strHQKLPath);

		AddMsg(RPT_INFO, "���ƻ�Ȩ���k���ļ����");

		if (!m_bStartWeight)  //��ֹ��Ȩ
		{
			AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
			return;
		}
		AddMsg(RPT_INFO, "����ɾ����ʱ�ļ�Ŀ¼[%s]����ļ�...", theApp.m_strKLBkPath);
		CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
		AddMsg(RPT_INFO, "ɾ���ļ����");

		if (m_bRealHQ) //ʵʱ��Ȩ
			AddMsg(RPT_INFO, "K��ʵʱ��Ȩ���");
		else
			AddMsg(RPT_INFO, "K����ʷ��Ȩ���");
	}

	if (!m_bStartWeight)  //��ֹ��Ȩ
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return;
	}

	//ʵʱ��Ȩ���ϴ�
	if (m_bRealHQ && theApp.m_bRWUpload) //ʵʱ��Ȩ
	{
		AddMsg(RPT_INFO, "�����ϴ���Ȩk��...");
		m_checkThread.StartUploadHQKLineFile();
		theApp.m_dlgInfo.m_strInfo = "�����ϴ���ǰ��Ļ�ȨK��...";
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			GetWekMonLine(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFolder = ffind.GetFilePath(); //ָ��ҲҪ�������ߺ�����
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
// 		if (i == 1) //��ǰ��
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 		else
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\day\\" + strFileName;
// 
// 		CKLineFile kfile;
// 		if (!kfile.Open(strDayFile,  GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 		{
// 			AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strDayFile);
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
// 	//��������
// 	{
// 		tagKLineInfo*  pWekInfo = NULL;
// 		BOOL bWekFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("��Ч������:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
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
// 				REPORT(MN, T("��Ч������:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				continue;
// 			}
// 			UINT nNextDate = (*pos)->nDate;
// 			if (!IsSameWeek(nPreDate, nNextDate)) //��һ��	  
// 			{
// 				bWekFirstDay = TRUE;
// 				lstWekLine.push_back(pWekInfo);
// 				pWekInfo = NULL;
// 			}
// 		}
// 		if (pWekInfo != NULL)
// 			lstWekLine.push_back(pWekInfo); //���һ��
// 	}
// 	//��������
// 	{
// 		tagKLineInfo*  pMonInfo = NULL;
// 		BOOL bMonFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("��Ч������:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
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
// 			if ((*pos)->nDate % 10000 / 100 != pMonInfo->nDate % 10000 / 100) //��һ��
// 			{
// 				bMonFirstDay = TRUE;
// 				lstMonLine.push_back(pMonInfo);
// 				pMonInfo = NULL;
// 			}
// 		}
// 		if (pMonInfo != NULL)
// 			lstMonLine.push_back(pMonInfo); //���һ��
// 	}
// 
// 	{//��������
// 		CFile fHisWekL;
// 		CFile fCurWekL;
// 
// 		if (!fCurWekL.Open(strCurWekFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strCurWekFile);
// 
// 		CKLineList::iterator pos = lstWekLine.begin();
// 		int nHisCount = (int)lstWekLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0) //�ж���ļ�¼�Ŵ���ʷ�ļ�
// 		{
// 			if (!fHisWekL.Open(strHisWekFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strHisWekFile);
// 		}
// 
// 		while(pos != lstWekLine.end())
// 		{
// 			if (nHisCount <= 0) //�����ڵ�ǰ��
// 			{
// 				fCurWekL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//��������ʷ��
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
// 	{//��������
// 		CFile fHisMonL;
// 		CFile fCurMonL;
// 		CKLineList::iterator pos = lstMonLine.begin();
// 		int nHisCount = (int)lstMonLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0)
// 		{
// 			if (!fHisMonL.Open(strHisMonFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strHisMonFile);
// 		}
// 
// 		if (!fCurMonL.Open(strCurMonFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strCurMonFile);
// 
// 		while(pos != lstMonLine.end())
// 		{
// 			if (nHisCount <= 0) //�����ڵ�ǰ��
// 			{
// 				fCurMonL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//��������ʷ��
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
// 	if (nStockCode < 3000 || nStockCode >= 200000 && nStockCode < 300000)   //����, �����Ϻ�
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
// 		AddMsg(RPT_WARNING, "k���ļ�[%s]������", strFilePath);
// 	}
// 	else
// 	{
// 		if ((bCurKLine && m_strCurKFilePath.Compare(strFilePath) == 0 || 
// 			!bCurKLine && m_strHisKFilePath.Compare(strFilePath) == 0)
// 			&& pKLFile->IsOpen())
// 		{ //ͬһ���ļ�ֻ��һ��
// 			pKLFile->MoveFirst();
// 		}
// 		else
// 		{
// 			if (pKLFile->IsOpen()) 
// 				pKLFile->Close(); //�ر��ϴδ򿪵��ļ�
// 
// 			if (!pKLFile->Open(strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 			{
// 				AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strFilePath);
// 				if (bCurKLine)
// 					m_strCurKFilePath.Empty();
// 				else
// 					m_strHisKFilePath.Empty();
// 			}
// 			else
// 			{
// 				if (bCurKLine)
// 					m_strCurKFilePath = strFilePath; //������һ���ļ�·���������һ������һ����ͬһ���ļ�����ֻ��һ��
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
// 			if (klInfo.nDate >= pWeight->nDate)  //С�ڻ�Ȩ���ڵ�����k�߻�Ȩ
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
// 			AddMsg(RPT_ERROR, "�����ļ�[]ʧ��", strFilePath);
// 	}
}

//void CKLine_ExeDlg::OnMiStopweight()
//{
//	AddMsg(RPT_WARNING, "������ֹk�߻�Ȩ...");
//	m_bStartWeight = FALSE;
//	EnableWindow(FALSE);
//	m_pWeightThread->Stop();
//}

void CKLine_ExeDlg::StopWeight()
{
	AddMsg(RPT_WARNING, "������ֹk�߻�Ȩ...");
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
	AddMsg(RPT_INFO, "�����ϴ���Ȩk��...");
	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartUploadHQKLineFile();
	dlgInfo.m_strInfo = "�����ϴ���ǰ��Ļ�Ȩk��...";
	if (dlgInfo.DoModal() == IDCANCEL)
	{
		AddMsg(RPT_INFO, "�û�ȡ���ϴ���ǰ��Ļ�Ȩk��\n");
		theApp.m_pcheckThread->StopUploadandDownload();

		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		dlgInfo.m_strInfo = "������ֹ�ϴ�...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}
}

void CKLine_ExeDlg::OnMiDownloadHq()
{
	AddMsg(RPT_INFO, "�������ػ�Ȩk��...");
	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartDownloadHQKLineFile();
	dlgInfo.m_strInfo = "�������ص�ǰ��Ļ�Ȩk��...";
	if (dlgInfo.DoModal() == IDCANCEL)
	{
		AddMsg(RPT_INFO, "�û�ȡ�����ص�ǰ��Ļ�Ȩk��\n");
		theApp.m_pcheckThread->StopUploadandDownload();

		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		dlgInfo.m_strInfo = "������ֹ����...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}
}

void CKLine_ExeDlg::UpdateDayLineToWeight()  //����������ɵ��������ݵ���Ȩ���������У�Ȼ����ʵʱ��Ȩ
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
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
// 		AddMsg(RPT_ERROR, "����: �޷��������ļ�[%s]!", strFile);
// 		return;
// 	}
// 
// 	CString strHQFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 
// 	CKLineFile hqfile;     //��Ȩ����
// 	CKLineFile orifile;	   //ԭʼ����
// 	if (!hqfile.Open(strHQFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS))
// 	{
// 		AddMsg(RPT_ERROR, "����: ���ļ�[%s]ʧ��!", strHQFile);
// 		return;
// 	}
// 	hqfile.MoveLast();
// 	tagKLineInfo info = {0};
// 	hqfile.GetRecordToBuffer((char*)&info);
// 
// 	if (!orifile.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 	{
// 		AddMsg(RPT_ERROR, "����: ���ļ�[%s]ʧ��!", strHQFile);
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
// 		AddMsg(RPT_ERROR, "����: AllocateMemoryForSave() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.SetAllRecordToBuffer(pOldKLineBuf))
// 	{
// 		AddMsg(RPT_ERROR, "����: SetAllRecordToBuffer() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.WriteData())
// 	{
// 		AddMsg(RPT_ERROR, "����: WriteData() failed!");
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
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
// 		AddMsg(RPT_ERROR, "����: �޷��������ļ�[%s]!", strFile);
// 		return;
// 	}
// 	CKLineFile curfile; 
// 	if (!curfile.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 	{
// 		AddMsg(RPT_ERROR, "����: ���ļ�[%s]ʧ��!", strFile);
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
// 	//����Ҫɾ���ļ�¼
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
// 		AddMsg(RPT_ERROR, "����: ���ļ�[%s]ʧ��!", strHisName);
// 		curfile.Close();
// 		return;
// 	}
// 
// 	if (!kfileHis.AllocateMemoryForSave(nDelCount))
// 	{
// 		curfile.Close();
// 		kfileHis.Close();
// 		AddMsg(RPT_ERROR, "����: ���ļ�[%s]�����ڴ�ʧ��!", strHisName);
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
// 		AddMsg(RPT_ERROR, "����: �����ļ�[%s]ʧ��!", strHisName);
// 	}
// 
// 	kfileHis.Close();
// 
// 	{//ɾ������ļ�¼
// 
// 		//AddMsg(RPT_INFO, "[%s]ɾ������ļ�¼!", strFile);
// 		char* pRecBuf = new char[nRecSize*nReserveCount];
// 		if (pRecBuf == NULL)
// 		{
// 			AddMsg(RPT_ERROR, "����: �����ڴ�ʧ��!");
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
// 			AddMsg(RPT_ERROR, "����: ���ļ�[%s]�����ڴ�ʧ��!", strFile);
// 			curfile.Close();
// 			return;
// 		}
// 
// 		if (!curfile.SetAllRecordToBuffer(pRecBuf))
// 		{
// 			AddMsg(RPT_ERROR, "����: �����ļ�[%s]ʧ��!", strFile);
// 			curfile.Close();
// 			return;
// 		}
// 		if (!curfile.WriteData())
// 		{
// 			AddMsg(RPT_ERROR, "����: �����ļ�[%s]ʧ��!", strFile);
// 		}
// 
// 		delete []pRecBuf;
// 		pRecBuf = NULL;
// 	}
// 	curfile.Close();	
}

BOOL CKLine_ExeDlg::IsSameWeek(unsigned int uiNow, unsigned int uiPrev)
{//�ȽϽ��պ������������Ƿ���ͬһ����
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
// 	//����ļ���С�������ƴ�С
// 	CDlg_Info dlgInfo;
// 	theApp.m_pDlgInfoWnd = &dlgInfo;
// 	theApp.m_pcheckThread->StartCheckFileSize(m_strCheckSizeFilePath);
// 
// 	dlgInfo.m_strInfo = "���ڷָ��ļ�...";
// 	if (dlgInfo.DoModal() == IDCANCEL) //ȡ������
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
