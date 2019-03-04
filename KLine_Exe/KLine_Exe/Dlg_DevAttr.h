#pragma once


// CDlg_DevAttr 对话框

class CDlg_DevAttr : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevAttr)

public:
	CDlg_DevAttr(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_DevAttr();

// 对话框数据
	enum { IDD = IDD_DLG_DEVATTRIBUTE };

	BOOL      m_bAddDev;      //添加设备还是修改设备
	CString   m_strDevName;
	CString   m_strPath;
	CString   m_strRemark;    //备注
	CString   m_strParam;     //参数
	UINT      m_nParentID;    //父设备服务器的ID
	UINT      m_nDevID;       

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
