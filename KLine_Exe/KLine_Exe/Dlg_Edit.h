#pragma once


// CDlg_Edit 对话框

class CDlg_Edit : public CDialog
{
	DECLARE_DYNAMIC(CDlg_Edit)

public:
	CDlg_Edit(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_Edit();

	CString    m_strPath;
// 对话框数据
	enum { IDD = IDD_DLG_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnDestroy();
};
