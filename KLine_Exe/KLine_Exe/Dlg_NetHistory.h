#pragma once
#include "afxcmn.h"
#include "HttpDataFile.h"

// CDlg_NetHistory �Ի���

class CDlg_NetHistory : public CDialog
{
	DECLARE_DYNAMIC(CDlg_NetHistory)

public:
	CDlg_NetHistory(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_NetHistory();

// �Ի�������
	enum { IDD = IDD_DLG_QUERYNETHISTORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnBrowsebtn();
	afx_msg void OnBnClickedBtnOpenfolder();
	DECLARE_MESSAGE_MAP()

	CHttpDataFile    m_file;
	CListCtrl        m_listInfo;
};
