#pragma once
#include "afxcmn.h"
#include "HttpDataFile.h"

// CDlg_NetHistory 对话框

class CDlg_NetHistory : public CDialog
{
	DECLARE_DYNAMIC(CDlg_NetHistory)

public:
	CDlg_NetHistory(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_NetHistory();

// 对话框数据
	enum { IDD = IDD_DLG_QUERYNETHISTORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnBrowsebtn();
	afx_msg void OnBnClickedBtnOpenfolder();
	DECLARE_MESSAGE_MAP()

	CHttpDataFile    m_file;
	CListCtrl        m_listInfo;
};
