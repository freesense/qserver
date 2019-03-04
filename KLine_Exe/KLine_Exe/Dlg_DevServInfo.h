#pragma once


// CDlg_DevServInfo 对话框

class CDlg_DevServInfo : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevServInfo)

public:
	CDlg_DevServInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_DevServInfo();

// 对话框数据
	enum { IDD = IDD_DLG_DEVSERVINFO };

	BOOL      m_bAddDevServ;    //添加设备服务器还是修改信息
	CString   m_strDevServName;
	CString   m_strDevServIP;
	CString   m_strRemark;      //备注
	UINT      m_nDDevServID;    //设备服务器ID

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()
};
