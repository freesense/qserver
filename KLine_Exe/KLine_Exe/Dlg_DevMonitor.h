#pragma once


// CDlg_DevMonitor 对话框

class CDlg_DevMonitor : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevMonitor)

public:
	CDlg_DevMonitor(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_DevMonitor();

// 对话框数据
	enum { IDD = IDD_DLG_DEVMONITOR };

	BOOL SaveConfig();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
