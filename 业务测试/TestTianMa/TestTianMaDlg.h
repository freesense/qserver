// TestTianMaDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "../../tianmadata/common/inc/TMDdef.h"
#include <vector>
#include <map>
#include <list>
#include "PressTest.h"
#include "resource.h"
#include "ClientConn.h"

typedef std::list<CPressTest*> CPressTestList;
#define WM_LISTRESULT WM_USER + 1213

struct tagTestState
{
	int nConTimes;
	int nConFailed;
	int nConSuccessed;
	double allTime;
	double maxTime;
	double minTime;
};

struct tagtestResult
{
	int nSendTimes;
	int nSendFailed;
	int nSuccessTimes;
	int nRecvFailed;
	int nTimeOut;

	double allTime;
	double maxTime;
	double minTime;
};

typedef std::map<int, tagtestResult*> CTestRetMap;
// CTestTianMaDlg 对话框
class CTestTianMaDlg : public CDialog
{
// 构造
public:
	CTestTianMaDlg(CWnd* pParent = NULL);	// 标准构造函数

	void OnConnect(double time);
	void OnDisconnect();
	void OnConnectFailed();
	void OnReconnect();
	void OnReadFailed(int nPackType);
	void OnWriteFailed(int nPackType);

	void OnWrite();
	void OnRead(const char* pBuf, int nBufLen, int nPackType, double time);
	void OnTimeOut(int nPackType);

	int GetSendBuf(const char* pBuf, int& nBufLen);
// 对话框数据
	enum { IDD = IDD_TESTTIANMA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	BOOL StartTest();
    void StopTest();
	
	void SendData();
	void InitCtrlPos();
	void HideAllCtrl();

	//wuqing add
	int ResetComboItem();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCheckBase();
	afx_msg void OnBnClickedCheckPlus();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedCheckPlus2();
	
	afx_msg void OnCbnSelchangeCombo();
	afx_msg LRESULT OnListResult(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	CIPAddressCtrl m_ServIP;
	UINT           m_nPort;
	CComboBox      m_combo;
	BOOL           m_bSingle;
	UINT           m_nConCount;
	
	CButton        m_cbBase;
	CButton        m_cbPlus;
	CButton        m_cbPlus2;
	CButton        m_cbInvalidData;

	CString        m_strFile;
	BOOL             m_bRun;
	
	BOOL             m_bIsShortCon;

	UINT             m_nPackType;
	CRITICAL_SECTION m_csListRet;	
	CPressTest *   m_pressTest;

	int              m_nCode;
	int              m_nName;
	int              m_nIndex;
	int              m_nRecCount;

	CClientConn      m_cltCon;
public:
	BOOL m_bHK;
public:
	CListBox m_lstBox;
public:
	CListBox m_lstStock;
public:
	afx_msg void OnBnClickedBtnAddstart();
};
