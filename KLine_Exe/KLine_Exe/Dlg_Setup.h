#pragma once
#include "Dlg_CfgKFile.h"
#include "Dlg_DevMonitor.h"
#include "afxcmn.h"

// CDlg_Setup �Ի���

class CDlg_Setup : public CDialog
{
	DECLARE_DYNAMIC(CDlg_Setup)

public:
	CDlg_Setup(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_Setup();

// �Ի�������
	enum { IDD = IDD_DLG_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTcnSelchangeTabSetup(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

public:
	CTabCtrl          m_tabSetup;
	CDlg_CfgKFile  	  m_dlgCfgKFile;	   	
	CDlg_DevMonitor   m_dlgDevMonitor;
	
};
