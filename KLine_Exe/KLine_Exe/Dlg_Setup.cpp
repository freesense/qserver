// Dlg_Setup.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_Setup.h"


// CDlg_Setup 对话框

IMPLEMENT_DYNAMIC(CDlg_Setup, CDialog)

CDlg_Setup::CDlg_Setup(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_Setup::IDD, pParent)
{

}

CDlg_Setup::~CDlg_Setup()
{
}

void CDlg_Setup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_SETUP, m_tabSetup);
}


BEGIN_MESSAGE_MAP(CDlg_Setup, CDialog)
	ON_BN_CLICKED(IDOK, &CDlg_Setup::OnBnClickedOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SETUP, &CDlg_Setup::OnTcnSelchangeTabSetup)
END_MESSAGE_MAP()


// CDlg_Setup 消息处理程序

BOOL CDlg_Setup::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_tabSetup.InsertItem(0, "K线维护");
	m_tabSetup.InsertItem(1, "设备监控");

	CRect rcClient;
	m_tabSetup.GetClientRect(rcClient);
	rcClient.DeflateRect(1, 21, 2, 2);
	
	m_dlgCfgKFile.Create(IDD_DLG_KLINECFG, &m_tabSetup);
	m_dlgCfgKFile.MoveWindow(&rcClient);
	m_dlgCfgKFile.ShowWindow(SW_SHOW);
	
	m_dlgDevMonitor.Create(IDD_DLG_DEVMONITOR, &m_tabSetup);
	m_dlgDevMonitor.MoveWindow(&rcClient);
	m_dlgDevMonitor.ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlg_Setup::OnBnClickedOk()
{
	BOOL bRet = m_dlgCfgKFile.SaveConfig();
	bRet &= m_dlgDevMonitor.SaveConfig();

	if (bRet)
		OnOK();
}

void CDlg_Setup::OnTcnSelchangeTabSetup(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nIndex = m_tabSetup.GetCurSel();
	if (nIndex == 0)
	{
		m_dlgCfgKFile.ShowWindow(SW_SHOW);
		m_dlgDevMonitor.ShowWindow(SW_HIDE);
	}
	else
	{
		m_dlgCfgKFile.ShowWindow(SW_HIDE);
		m_dlgDevMonitor.ShowWindow(SW_SHOW);
	}

	*pResult = 0;
}

BOOL CDlg_Setup::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}
