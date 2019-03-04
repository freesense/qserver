// Dlg_DevMonitor.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_DevMonitor.h"


// CDlg_DevMonitor 对话框

IMPLEMENT_DYNAMIC(CDlg_DevMonitor, CDialog)

CDlg_DevMonitor::CDlg_DevMonitor(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_DevMonitor::IDD, pParent)
{

}

CDlg_DevMonitor::~CDlg_DevMonitor()
{
}

void CDlg_DevMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlg_DevMonitor, CDialog)
	
END_MESSAGE_MAP()


// CDlg_DevMonitor 消息处理程序

BOOL CDlg_DevMonitor::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDlg_DevMonitor::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CString strTemp;
	strTemp.Format("%d", theApp.m_nDevMonPort);
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(strTemp);

	strTemp.Format("%d", theApp.m_nDevServTimeOut);
	GetDlgItem(IDC_E_DEVSERVTIMEOUT)->SetWindowText(strTemp);

	strTemp.Format("%d", theApp.m_nDevTimeOut);
	GetDlgItem(IDC_E_DEVTIMEOUT)->SetWindowText(strTemp);
	
	((CButton*)GetDlgItem(IDC_C_TIMEOUTRESTART))->SetCheck(
		theApp.m_bDevTORestart ? BST_CHECKED : BST_UNCHECKED);
														 
	strTemp.Format("%d", theApp.m_nMaxRecord);
	GetDlgItem(IDC_E_MAXRECORD)->SetWindowText(strTemp);	

	//监控模块设置
	strTemp.Format("%d", theApp.m_nDevServPort);
	GetDlgItem(IDC_E_DEVSERVPORT)->SetWindowText(strTemp);

	GetDlgItem(IDC_IP_DEVSERV)->SetWindowText(theApp.m_strDevServIP);
	GetDlgItem(IDC_E_APPNAME)->SetWindowText(theApp.m_strAppName);

	((CButton*)GetDlgItem(IDC_C_USEDEVMONITOR))->SetCheck(
		theApp.m_bUseDevMonitor ? BST_CHECKED : BST_UNCHECKED);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDlg_DevMonitor::SaveConfig()
{
	CString strTemp;
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(strTemp);
	theApp.m_nDevMonPort = atoi(strTemp);

	GetDlgItem(IDC_E_DEVSERVTIMEOUT)->GetWindowText(strTemp);
	theApp.m_nDevServTimeOut = atoi(strTemp);

	GetDlgItem(IDC_E_DEVTIMEOUT)->GetWindowText(strTemp);
	theApp.m_nDevTimeOut = atoi(strTemp);

	int nRet = ((CButton*)GetDlgItem(IDC_C_TIMEOUTRESTART))->GetCheck();
	if (nRet == BST_CHECKED)
		theApp.m_bDevTORestart = TRUE;
	else
		theApp.m_bDevTORestart = FALSE;

	GetDlgItem(IDC_E_MAXRECORD)->GetWindowText(strTemp);	
	theApp.m_nMaxRecord = atoi(strTemp);

	//监控模块设置
	GetDlgItem(IDC_E_DEVSERVPORT)->GetWindowText(strTemp);
	theApp.m_nDevServPort = atoi(strTemp);

	GetDlgItem(IDC_IP_DEVSERV)->GetWindowText(theApp.m_strDevServIP);
	GetDlgItem(IDC_E_APPNAME)->GetWindowText(theApp.m_strAppName);

	nRet = ((CButton*)GetDlgItem(IDC_C_USEDEVMONITOR))->GetCheck();
	theApp.m_bUseDevMonitor = (nRet == BST_CHECKED) ? TRUE : FALSE;
	
	if (!theApp.SaveDevMonitorInfo())
	{
		MessageBox("保存设备监控数据失败!");
		return FALSE;
	}

	return TRUE;
}
