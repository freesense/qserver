#pragma once
#include "afxcmn.h"


// CDlg_DevVer 对话框


class CDlg_DevVer : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevVer)

public:
	CDlg_DevVer(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_DevVer();

	CString     m_strDevName;
	CString     m_strVer;
	CListCtrl   m_lstVer;
// 对话框数据
	enum { IDD = IDD_DLG_DEVVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	
};
