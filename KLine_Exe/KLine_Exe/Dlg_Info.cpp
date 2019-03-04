// Dlg_Info.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_Info.h"


// CDlg_Info �Ի���

IMPLEMENT_DYNAMIC(CDlg_Info, CDialog)

CDlg_Info::CDlg_Info(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_Info::IDD, pParent)
	, m_strInfo(_T(""))
{
	m_bEnableBtn = TRUE;
}

CDlg_Info::~CDlg_Info()
{
}

void CDlg_Info::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ST_INFO, m_strInfo);
}


BEGIN_MESSAGE_MAP(CDlg_Info, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CDlg_Info::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CDlg_Info::OnBnClickedOk)
	ON_MESSAGE(WM_CHANGEINFO, OnChangeInfo)
END_MESSAGE_MAP()


// CDlg_Info ��Ϣ�������

void CDlg_Info::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialog::OnCancel();
}

BOOL CDlg_Info::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	GetDlgItem(IDCANCEL)->ShowWindow(m_bEnableBtn ? SW_SHOW : SW_HIDE);
	UpdateData();
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

BOOL CDlg_Info::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

void CDlg_Info::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OnOK();
}

LRESULT CDlg_Info::OnChangeInfo(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDC_ST_INFO)->SetWindowText(m_strInfo);
	return 0;
}