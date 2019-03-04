#pragma once
#include "Dlg_CfgKFile.h"
#include "Dlg_DevMonitor.h"
#include "afxcmn.h"

// CDlg_Setup 对话框

class CDlg_Setup : public CDialog
{
	DECLARE_DYNAMIC(CDlg_Setup)

public:
	CDlg_Setup(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_Setup();

// 对话框数据
	enum { IDD = IDD_DLG_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
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
