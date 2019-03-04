#pragma once
#include "afxcmn.h"
#include "MulFuncList.h "


// CDlg_XmlCfg 对话框

class CDlg_XmlCfg : public CDialog
{
	DECLARE_DYNAMIC(CDlg_XmlCfg)

public:
	CDlg_XmlCfg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_XmlCfg();

	CString  m_strXmlPath;
	CString  m_strDevName;
	CString  m_strIP;
	HWND     m_hMainWnd;
	SOCKET   m_hSock;

// 对话框数据
	enum { IDD = IDD_DLG_XMLCFG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedBtnDownload();
	afx_msg void OnBnClickedBtnUpload();
	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnAdd();
	afx_msg void OnBnClickedBtnDel();
	afx_msg void OnBnClickedBtnUp();
	afx_msg void OnBnClickedBtnDown();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnModify();
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);

	CMulFuncList m_lstXml;

	void ChangeStringToBinString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize);

	DECLARE_MESSAGE_MAP()	
};
