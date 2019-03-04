// Dlg_DevAttr.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_DevAttr.h"
#include "MainDlg.h"

// CDlg_DevAttr 对话框

IMPLEMENT_DYNAMIC(CDlg_DevAttr, CDialog)

CDlg_DevAttr::CDlg_DevAttr(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_DevAttr::IDD, pParent)
{
	m_bAddDev   = TRUE;
	m_nParentID	= 0;
	m_nDevID    = 0;
}

CDlg_DevAttr::~CDlg_DevAttr()
{
}

void CDlg_DevAttr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlg_DevAttr, CDialog)
	ON_BN_CLICKED(IDOK, &CDlg_DevAttr::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlg_DevAttr 消息处理程序

void CDlg_DevAttr::OnBnClickedOk()
{
	GetDlgItem(IDC_E_DEVNAME)->GetWindowText(m_strDevName);
	GetDlgItem(IDC_E_PATH)->GetWindowText(m_strPath);
	GetDlgItem(IDC_E_REMARK)->GetWindowText(m_strRemark);
	GetDlgItem(IDC_E_PARAM)->GetWindowText(m_strParam);

	if (m_strDevName.IsEmpty() || m_strPath.IsEmpty())
	{
		MessageBox("请输入完整的信息!");
		return;
	}

	CMainDlg* pMainDlg = (CMainDlg*)theApp.m_pMainWnd;
	ASSERT(pMainDlg != NULL);
	if (pMainDlg->IsDevExist(m_strDevName, m_nParentID, m_nDevID))
	{
		MessageBox("此设备名称已注册!");
		GetDlgItem(IDC_E_DEVNAME)->SetWindowText("");
		return;
	}

	OnOK();
}

BOOL CDlg_DevAttr::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!m_bAddDev)
	{
		GetDlgItem(IDC_E_DEVNAME)->SetWindowText(m_strDevName);
		GetDlgItem(IDC_E_PATH)->SetWindowText(m_strPath);
		GetDlgItem(IDC_E_REMARK)->SetWindowText(m_strRemark);
		GetDlgItem(IDC_E_PARAM)->SetWindowText(m_strParam);
		SetWindowText("修改设备属性");
	}
	else
	{
		SetWindowText("添加设备");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
