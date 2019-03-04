#pragma once
#include "afxdtctl.h"
#include "afxcmn.h"


// CDlg_CfgKFile 对话框

class CDlg_CfgKFile : public CDialog
{
	DECLARE_DYNAMIC(CDlg_CfgKFile)

public:
	CDlg_CfgKFile(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_CfgKFile();

// 对话框数据
	enum { IDD = IDD_DLG_KLINECFG };

	BOOL SaveConfig();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnBrowsedatfile();
	afx_msg void OnBnClickedBtnOpendatafile();
	afx_msg void OnBnClickedBtnBrowsdayfile();
	afx_msg void OnBnClickedBtnOpendayfile();
	afx_msg void OnBnClickedBtnBrowseBackup();
	afx_msg void OnBnClickedBtnOpenBackup();
	afx_msg void OnBnClickedBtnBrowseHq();
	afx_msg void OnBnClickedBtnOpenHq();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	UINT           m_nReserveCount;
	int            m_nDeleteOtiose;
	BOOL           m_bAutoCheck;
	UINT           m_nCheckTime;

	CDateTimeCtrl  m_dtCheckTime;
	BOOL           m_bUserHttpData;
	BOOL           m_bUseUpLoad;
	BOOL           m_bUseDownLoad;
				   
	CDateTimeCtrl  m_dtcAutoDownTime;
	CString        m_strServerAddr;
	CString        m_strUserName;
	CString        m_strPassword;
	CString        m_strFTPPath;
	CString        m_strDatFilePath;
	CIPAddressCtrl m_ipHQServ;
	UINT           m_nHQSerPort;
	CString        m_strHQDayFilePath;
public:
	afx_msg void OnBnClickedCUsehttpdata();
public:
	int m_nCheck;
};
