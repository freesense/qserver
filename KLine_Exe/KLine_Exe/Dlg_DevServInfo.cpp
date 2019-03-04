// Dlg_DevServInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_DevServInfo.h"
#include "MainDlg.h"

// CDlg_DevServInfo 对话框

IMPLEMENT_DYNAMIC(CDlg_DevServInfo, CDialog)

CDlg_DevServInfo::CDlg_DevServInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_DevServInfo::IDD, pParent)
{
	m_bAddDevServ = TRUE;
	m_nDDevServID = -2;
}

CDlg_DevServInfo::~CDlg_DevServInfo()
{
}

void CDlg_DevServInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlg_DevServInfo, CDialog)
	ON_BN_CLICKED(IDOK, &CDlg_DevServInfo::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlg_DevServInfo 消息处理程序

BOOL CDlg_DevServInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!m_bAddDevServ)
	{
		GetDlgItem(IDC_E_DEVSERVNAME)->SetWindowText(m_strDevServName);
		GetDlgItem(IDC_IP_DEVSERV)->SetWindowText(m_strDevServIP);
		GetDlgItem(IDC_E_REMARK)->SetWindowText(m_strRemark);
		
		SetWindowText("修改设备服务器属性");
	}
	else
	{
		SetWindowText("添加设备服务器");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDlg_DevServInfo::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialog::PreTranslateMessage(pMsg);
}

void CDlg_DevServInfo::OnBnClickedOk()
{
	GetDlgItem(IDC_E_DEVSERVNAME)->GetWindowText(m_strDevServName);
	GetDlgItem(IDC_IP_DEVSERV)->GetWindowText(m_strDevServIP);
	GetDlgItem(IDC_E_REMARK)->GetWindowText(m_strRemark);

	if (m_strDevServName.IsEmpty() || m_strDevServIP.IsEmpty())
	{
		MessageBox("请输入完整的信息!");
		return;
	}

	CMainDlg* pMainDlg = (CMainDlg*)theApp.m_pMainWnd;
	ASSERT(pMainDlg != NULL);
	if (pMainDlg->IsDevServExist(m_strDevServIP, m_nDDevServID))
	{
		MessageBox("此IP已注册!");
		GetDlgItem(IDC_IP_DEVSERV)->SetWindowText("");
		return;
	}

	OnOK();
}
