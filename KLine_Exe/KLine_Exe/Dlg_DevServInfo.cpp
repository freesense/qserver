// Dlg_DevServInfo.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_DevServInfo.h"
#include "MainDlg.h"

// CDlg_DevServInfo �Ի���

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


// CDlg_DevServInfo ��Ϣ�������

BOOL CDlg_DevServInfo::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!m_bAddDevServ)
	{
		GetDlgItem(IDC_E_DEVSERVNAME)->SetWindowText(m_strDevServName);
		GetDlgItem(IDC_IP_DEVSERV)->SetWindowText(m_strDevServIP);
		GetDlgItem(IDC_E_REMARK)->SetWindowText(m_strRemark);
		
		SetWindowText("�޸��豸����������");
	}
	else
	{
		SetWindowText("����豸������");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

BOOL CDlg_DevServInfo::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���

	return CDialog::PreTranslateMessage(pMsg);
}

void CDlg_DevServInfo::OnBnClickedOk()
{
	GetDlgItem(IDC_E_DEVSERVNAME)->GetWindowText(m_strDevServName);
	GetDlgItem(IDC_IP_DEVSERV)->GetWindowText(m_strDevServIP);
	GetDlgItem(IDC_E_REMARK)->GetWindowText(m_strRemark);

	if (m_strDevServName.IsEmpty() || m_strDevServIP.IsEmpty())
	{
		MessageBox("��������������Ϣ!");
		return;
	}

	CMainDlg* pMainDlg = (CMainDlg*)theApp.m_pMainWnd;
	ASSERT(pMainDlg != NULL);
	if (pMainDlg->IsDevServExist(m_strDevServIP, m_nDDevServID))
	{
		MessageBox("��IP��ע��!");
		GetDlgItem(IDC_IP_DEVSERV)->SetWindowText("");
		return;
	}

	OnOK();
}
