// Dlg_Edit.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_Edit.h"


// CDlg_Edit 对话框

IMPLEMENT_DYNAMIC(CDlg_Edit, CDialog)

CDlg_Edit::CDlg_Edit(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_Edit::IDD, pParent)
{

}

CDlg_Edit::~CDlg_Edit()
{
}

void CDlg_Edit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlg_Edit, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlg_Edit 消息处理程序

BOOL CDlg_Edit::OnInitDialog()
{
	CDialog::OnInitDialog();			 

	// TODO:  在此添加额外的初始化
	GetDlgItem(IDC_E_PATH)->SetWindowText(m_strPath);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlg_Edit::OnDestroy()
{
	GetDlgItem(IDC_E_PATH)->GetWindowText(m_strPath);

	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}
