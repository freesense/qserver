// Dlg_Edit.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_Edit.h"


// CDlg_Edit �Ի���

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


// CDlg_Edit ��Ϣ�������

BOOL CDlg_Edit::OnInitDialog()
{
	CDialog::OnInitDialog();			 

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	GetDlgItem(IDC_E_PATH)->SetWindowText(m_strPath);
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CDlg_Edit::OnDestroy()
{
	GetDlgItem(IDC_E_PATH)->GetWindowText(m_strPath);

	CDialog::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
}
