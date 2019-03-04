#pragma once


#define WM_CHANGEINFO WM_USER + 1324
// CDlg_Info 对话框

class CDlg_Info : public CDialog
{
	DECLARE_DYNAMIC(CDlg_Info)

public:
	CDlg_Info(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_Info();

// 对话框数据
	enum { IDD = IDD_DLG_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

protected:
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnChangeInfo(WPARAM wParam, LPARAM lParam);
public:
	CString m_strInfo;
	BOOL    m_bEnableBtn;
	afx_msg void OnBnClickedOk();
};
