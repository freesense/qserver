// Dlg_CfgKFile.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_CfgKFile.h"
#include "FolderDlg.h"

// CDlg_CfgKFile 对话框

IMPLEMENT_DYNAMIC(CDlg_CfgKFile, CDialog)

CDlg_CfgKFile::CDlg_CfgKFile(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_CfgKFile::IDD, pParent)
	, m_nReserveCount(0)
	, m_nDeleteOtiose(0)
	, m_bAutoCheck(FALSE)
	, m_bUserHttpData(FALSE)
	, m_bUseUpLoad(FALSE)
	, m_bUseDownLoad(FALSE)
	, m_strServerAddr(_T(""))
	, m_strUserName(_T(""))
	, m_strPassword(_T(""))
	, m_strFTPPath(_T(""))
	, m_strDatFilePath(_T(""))
	, m_nHQSerPort(0)
	, m_strHQDayFilePath(_T(""))
	, m_nCheck(0)
{

}

CDlg_CfgKFile::~CDlg_CfgKFile()
{
}

void CDlg_CfgKFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_RESERVECOUNT, m_nReserveCount);
	DDX_Radio(pDX, IDC_R_SAVE, m_nDeleteOtiose);
	DDX_Check(pDX, IDC_C_AUTOCHECK, m_bAutoCheck);
	DDX_Control(pDX, IDC_DT_AUTOCHECKTIME, m_dtCheckTime);
	DDX_Check(pDX, IDC_C_USEHTTPDATA, m_bUserHttpData);
	DDX_Check(pDX, IDC_C_AUTOUPLOAD, m_bUseUpLoad);
	DDX_Check(pDX, IDC_C_AUTODOWN, m_bUseDownLoad);
	DDX_Control(pDX, IDC_DT_AUTODOWNTIME, m_dtcAutoDownTime);
	DDX_Text(pDX, IDC_E_FTPSERVER, m_strServerAddr);
	DDX_Text(pDX, IDC_E_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_E_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDC_E_UPDOWNPATH, m_strFTPPath);
	DDX_Text(pDX, IDC_E_DATFILEPATH, m_strDatFilePath);
	DDX_Control(pDX, IDC_IP_HQSERVER, m_ipHQServ);
	DDX_Text(pDX, IDC_E_HQSERVPORT, m_nHQSerPort);
	DDX_Text(pDX, IDC_EDIT_DAYFILEPATH, m_strHQDayFilePath);
	DDX_Radio(pDX, IDC_R_CHECK, m_nCheck);
}


BEGIN_MESSAGE_MAP(CDlg_CfgKFile, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlg_CfgKFile::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlg_CfgKFile::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CDlg_CfgKFile::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CDlg_CfgKFile::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_BROWSEDATFILE, &CDlg_CfgKFile::OnBnClickedBtnBrowsedatfile)
	ON_BN_CLICKED(IDC_BTN_OPENDATAFILE, &CDlg_CfgKFile::OnBnClickedBtnOpendatafile)
	ON_BN_CLICKED(IDC_BTN_BROWSDAYFILE, &CDlg_CfgKFile::OnBnClickedBtnBrowsdayfile)
	ON_BN_CLICKED(IDC_BTN_OPENDAYFILE, &CDlg_CfgKFile::OnBnClickedBtnOpendayfile)
	ON_BN_CLICKED(IDC_BTN_BROWSE_BACKUP, &CDlg_CfgKFile::OnBnClickedBtnBrowseBackup)
	ON_BN_CLICKED(IDC_BTN_OPEN_BACKUP, &CDlg_CfgKFile::OnBnClickedBtnOpenBackup)
	ON_BN_CLICKED(IDC_BTN_BROWSE_HQ, &CDlg_CfgKFile::OnBnClickedBtnBrowseHq)
	ON_BN_CLICKED(IDC_BTN_OPEN_HQ, &CDlg_CfgKFile::OnBnClickedBtnOpenHq)
	ON_BN_CLICKED(IDC_C_USEHTTPDATA, &CDlg_CfgKFile::OnBnClickedCUsehttpdata)
END_MESSAGE_MAP()


// CDlg_CfgKFile 消息处理程序

void CDlg_CfgKFile::OnBnClickedButton1()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_EDIT_KFILEPATH)->SetWindowText(strPathName);
}

BOOL CDlg_CfgKFile::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_EDIT_KFILEPATH)->SetWindowText(theApp.m_strKFilePath);
// 	GetDlgItem(IDC_EDIT_HISTORYKFILEPATH)->SetWindowText(theApp.m_strHisKFilePath);
	m_nReserveCount = theApp.m_nReserveCount;
	m_nDeleteOtiose = theApp.m_nDeleteOtiose;
	m_bAutoCheck    = theApp.m_bAutoCheck;
	m_nCheckTime    = theApp.m_nCheckTime;
	{
		int nHour = m_nCheckTime / 10000;
		int nMin  = (m_nCheckTime % 10000) / 100;
		int nSec  = m_nCheckTime % 100;
		CTime tmCur = CTime::GetCurrentTime();
		CTime tmCheck(tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay(),
			nHour, nMin, nSec);
		m_dtCheckTime.SetTime(&tmCheck);
	}
	m_bUserHttpData = theApp.m_bUserHttpData;
	GetDlgItem(IDC_R_CHECK)->EnableWindow(m_bUserHttpData ? TRUE : FALSE);
	GetDlgItem(IDC_R_CHECKUPDATE)->EnableWindow(m_bUserHttpData ? TRUE : FALSE);
	m_nCheck = theApp.m_bUpdateData ? 1 : 0;

	m_bUseUpLoad    = theApp.m_bUseUpLoad;
	m_bUseDownLoad  = theApp.m_bUseDownLoad;
	{
		int nHour = theApp.m_nDownLoadTime / 10000;
		int nMin  = (theApp.m_nDownLoadTime % 10000) / 100;
		int nSec  = theApp.m_nDownLoadTime % 100;
		CTime tmCur = CTime::GetCurrentTime();
		CTime tmCheck(tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay(),
			nHour, nMin, nSec);
		m_dtcAutoDownTime.SetTime(&tmCheck);
	}
	m_strServerAddr = theApp.m_strServerAddr;
	m_strUserName = theApp.m_strUserName;
	m_strPassword = theApp.m_strPassword;
	m_strFTPPath  = theApp.m_strFTPPath;
	m_strDatFilePath = theApp.m_strDatFilePath;

	m_ipHQServ.SetWindowText(theApp.m_strHQServIP);
	m_nHQSerPort = theApp.m_nHQServPort;
	m_strHQDayFilePath = theApp.m_strHQDayFilePath;

	GetDlgItem(IDC_IP_SQLHQYZ)->SetWindowText(theApp.m_strSqlIP);
	GetDlgItem(IDC_E_SQLDBNAME)->SetWindowText(theApp.m_strSqlDBN);
	GetDlgItem(IDC_E_SQLUID)->SetWindowText(theApp.m_strSqlUID);
	GetDlgItem(IDC_E_SQLPW)->SetWindowText(theApp.m_strSqlPw);

	//
// 	GetDlgItem(IDC_E_KLINEBACKUP)->SetWindowText(theApp.m_strKLBkPath);
// 	GetDlgItem(IDC_E_HQKLINEPATH)->SetWindowText(theApp.m_strHQKLPath);

	//
	((CButton*)GetDlgItem(IDC_C_AUTOHUANQUAN))->SetCheck(theApp.m_bRealHQ ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_C_AUTOUPLOADHQ))->SetCheck(theApp.m_bRWUpload ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_C_AUTODOWN_HQ))->SetCheck(theApp.m_bDownLoadHQ ? BST_CHECKED : BST_UNCHECKED);

	CTime tmCur = CTime::GetCurrentTime();
	CTime tmTemp = CTime(tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay(),
		theApp.m_nRealHQTime/10000, theApp.m_nRealHQTime%10000/100, theApp.m_nRealHQTime%100);
	((CDateTimeCtrl*)GetDlgItem(IDC_DT_AUTOHUANQUAN))->SetTime(&tmTemp);

	tmTemp = CTime(tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay(),
		theApp.m_nDownHQTime/10000, theApp.m_nDownHQTime%10000/100, theApp.m_nDownHQTime%100);

	((CDateTimeCtrl*)GetDlgItem(IDC_DT_AUTODOWN_HQ))->SetTime(&tmTemp);

	GetDlgItem(IDC_E_UPDOWNHQPATH)->SetWindowText(theApp.m_strHQFTPPath);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlg_CfgKFile::OnBnClickedButton2()
{
	CString strPathName;
	GetDlgItem(IDC_EDIT_KFILEPATH)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

BOOL CDlg_CfgKFile::SaveConfig()
{
	GetDlgItem(IDC_EDIT_KFILEPATH)->GetWindowText(theApp.m_strKFilePath);
// 	GetDlgItem(IDC_EDIT_HISTORYKFILEPATH)->GetWindowText(theApp.m_strHisKFilePath);

	UpdateData();
	theApp.m_nReserveCount = m_nReserveCount;
	theApp.m_nDeleteOtiose = m_nDeleteOtiose;
	theApp.m_bAutoCheck    = m_bAutoCheck;
	{
		CTime tmCheck;
		m_dtCheckTime.GetTime(tmCheck);
		m_nCheckTime = tmCheck.GetHour()*10000 + tmCheck.GetMinute()*100 + tmCheck.GetSecond();
	}
	theApp.m_nCheckTime = m_nCheckTime;
	theApp.m_bUserHttpData = m_bUserHttpData;
	theApp.m_bUseUpLoad = m_bUseUpLoad;
	theApp.m_bUseDownLoad = m_bUseDownLoad;

	theApp.m_bUpdateData = m_nCheck == 0 ? FALSE : TRUE;
	{
		CTime tmDownLoad;
		m_dtcAutoDownTime.GetTime(tmDownLoad);
		theApp.m_nDownLoadTime = tmDownLoad.GetHour()*10000 + tmDownLoad.GetMinute()*100 +
			tmDownLoad.GetSecond();
	}

	theApp.m_strServerAddr = m_strServerAddr;
	theApp.m_strUserName   = m_strUserName;
	theApp.m_strPassword   = m_strPassword;
	theApp.m_strFTPPath    = m_strFTPPath;
	theApp.m_strDatFilePath = m_strDatFilePath;

	m_ipHQServ.GetWindowText(theApp.m_strHQServIP);
	theApp.m_nHQServPort = m_nHQSerPort;
	theApp.m_strHQDayFilePath = m_strHQDayFilePath;

	{//还权因子数据库配置
		GetDlgItem(IDC_IP_SQLHQYZ)->GetWindowText(theApp.m_strSqlIP);
		GetDlgItem(IDC_E_SQLDBNAME)->GetWindowText(theApp.m_strSqlDBN);
		GetDlgItem(IDC_E_SQLUID)->GetWindowText(theApp.m_strSqlUID);
		GetDlgItem(IDC_E_SQLPW)->GetWindowText(theApp.m_strSqlPw);
	}
	//
// 	GetDlgItem(IDC_E_KLINEBACKUP)->GetWindowText(theApp.m_strKLBkPath);
// 	GetDlgItem(IDC_E_HQKLINEPATH)->GetWindowText(theApp.m_strHQKLPath);

	//
	theApp.m_bRealHQ = ((CButton*)GetDlgItem(IDC_C_AUTOHUANQUAN))->GetCheck();
	theApp.m_bRWUpload = ((CButton*)GetDlgItem(IDC_C_AUTOUPLOADHQ))->GetCheck();
	theApp.m_bDownLoadHQ = ((CButton*)GetDlgItem(IDC_C_AUTODOWN_HQ))->GetCheck();

	
	CTime tmTemp;
	((CDateTimeCtrl*)GetDlgItem(IDC_DT_AUTOHUANQUAN))->GetTime(tmTemp);
	theApp.m_nRealHQTime = tmTemp.GetHour()*10000 + tmTemp.GetMinute()*100 + tmTemp.GetSecond();

	((CDateTimeCtrl*)GetDlgItem(IDC_DT_AUTODOWN_HQ))->GetTime(tmTemp);
	theApp.m_nDownHQTime = tmTemp.GetHour()*10000 + tmTemp.GetMinute()*100 + tmTemp.GetSecond();

	GetDlgItem(IDC_E_UPDOWNHQPATH)->GetWindowText(theApp.m_strHQFTPPath);

	if (!theApp.SaveKFileInfo())
	{
		MessageBox("保存K线维护数据失败!");
		return FALSE;
	}

	return TRUE;
}

void CDlg_CfgKFile::OnBnClickedBtnBrowse()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_EDIT_HISTORYKFILEPATH)->SetWindowText(strPathName);
}

void CDlg_CfgKFile::OnBnClickedBtnOpen()
{
	CString strPathName;
	GetDlgItem(IDC_EDIT_HISTORYKFILEPATH)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

void CDlg_CfgKFile::OnBnClickedBtnBrowsedatfile()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_E_DATFILEPATH)->SetWindowText(strPathName);
}

void CDlg_CfgKFile::OnBnClickedBtnOpendatafile()
{
	CString strPathName;
	GetDlgItem(IDC_E_DATFILEPATH)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

void CDlg_CfgKFile::OnBnClickedBtnBrowsdayfile()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_EDIT_DAYFILEPATH)->SetWindowText(strPathName);
}

void CDlg_CfgKFile::OnBnClickedBtnOpendayfile()
{
	CString strPathName;
	GetDlgItem(IDC_EDIT_DAYFILEPATH)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

BOOL CDlg_CfgKFile::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}

void CDlg_CfgKFile::OnBnClickedBtnBrowseBackup()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_E_KLINEBACKUP)->SetWindowText(strPathName);
}

void CDlg_CfgKFile::OnBnClickedBtnOpenBackup()
{
	CString strPathName;
	GetDlgItem(IDC_E_KLINEBACKUP)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

void CDlg_CfgKFile::OnBnClickedBtnBrowseHq()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_E_HQKLINEPATH)->SetWindowText(strPathName);
}

void CDlg_CfgKFile::OnBnClickedBtnOpenHq()
{
	CString strPathName;
	GetDlgItem(IDC_E_HQKLINEPATH)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

void CDlg_CfgKFile::OnBnClickedCUsehttpdata()
{
	int nRet = ( (CButton*)GetDlgItem(IDC_C_USEHTTPDATA) )->GetCheck();
	GetDlgItem(IDC_R_CHECK)->EnableWindow(nRet == BST_CHECKED ? TRUE : FALSE);
	GetDlgItem(IDC_R_CHECKUPDATE)->EnableWindow(nRet == BST_CHECKED ? TRUE : FALSE);
}
