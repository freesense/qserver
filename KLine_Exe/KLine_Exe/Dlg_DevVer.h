#pragma once
#include "afxcmn.h"


// CDlg_DevVer �Ի���


class CDlg_DevVer : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevVer)

public:
	CDlg_DevVer(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_DevVer();

	CString     m_strDevName;
	CString     m_strVer;
	CListCtrl   m_lstVer;
// �Ի�������
	enum { IDD = IDD_DLG_DEVVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
	
};
