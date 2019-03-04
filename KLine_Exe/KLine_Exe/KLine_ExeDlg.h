// KLine_ExeDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "MulFuncList.h"
#include "afxwin.h"
#include "CheckThread.h"
#include "resource.h"
#include "AdoDB.h"
#include <list>
#include "WorkThread.h"

typedef std::list<tagHQYinZi*> CYinZiList;   

typedef std::list<tagKLineInfo*> CKLineList;

// CKLine_ExeDlg 对话框
class CKLine_ExeDlg : public CDialog
{
// 构造
	friend class CMainDlg;
public:
	CKLine_ExeDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_KLINE_EXE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	void Init();
	void InitCtrlPos(BOOL bHideFileList = TRUE);
	BOOL Find(UINT *pArr, int nItem, UINT dwFindData);
	BOOL IsCtrlEnabled(UINT nID);
	void InitFileList(CString strFile);
	void AutoCheck();
	void StartHuanQuan();
	BOOL QueryWeight();
	BOOL BackupKLine();
	void AddMsg(UINT nPackType, LPCSTR pFormat, ...);

	static UINT WINAPI _WeightThread(LPVOID lpParam);
	void WeightThread();
	void StartWeight(tagHQYinZi* pWeight, BOOL bCurKLine = TRUE);
	BOOL GetWekMonLine(CString strDayLine);
	BOOL GetSingleWekMonLine(CString strDayFile);
	void UpdateDayLineToWeight();
	void UpdateAllFile(CString strFile);
	void UpdateSingleFile(CString strFile, CString strFileName);
	void CheckFileSize(CString strFile);
	void CutFile(CString strFile);
	BOOL IsSameWeek(unsigned int uiNow, unsigned int uiPrev);
	void StopWeight();

	HICON         m_hIcon;
	CImageList    m_imgList;
	CBitmap       m_bmpNormal;
	CBitmap       m_bmpError;

	BOOL          m_bStopCheck;
	UINT          m_nEnableType;       //按钮禁用类型
	BOOL          m_bDataIntegrality;  //数据的完整性，检测是否人工终止
	BOOL          m_bDataChanged;	   //数据更改过

	BOOL          m_bIsCheckAll;	   //最近一次检测是否为检测所有文件
	BOOL          m_bHideFileList;	   //是否隐藏文件列表
    CWorkThread*  m_pWeightThread;
	string        m_strCurKLine;  //文件夹名称
	string        m_strHisKLine;
	
	CKLineFile    m_KFileCur;
	CKLineFile    m_KFileHis;
	
	CString       m_strCurKFilePath;
	CString       m_strHisKFilePath;

	BOOL          m_bStartWeight;
	BOOL          m_bRealHQ;
    BOOL          m_bEnCheckBtn;

	CString       m_strCheckAllPath;
	CString       m_strCheckSizeFilePath;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
	afx_msg void OnDestroy();
	afx_msg void OnLbnDblclkListError();
//	afx_msg void OnStopcheck();
	afx_msg void OnBnClickedBtnRecheck();
	afx_msg void OnBnClickedBtnSavetofile();
	afx_msg void OnBnClickedBtnStopcheck();
	afx_msg void OnBnClickedBtnInsertrecord();
	afx_msg void OnBnClickedBtnDelselrec();
	afx_msg void OnBnClickedBtnMovepre();
	afx_msg void OnBnClickedBtnMovenext();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnThreadEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnValueChanged(WPARAM wParam, LPARAM lParam);
	
	void OnUpdateControl(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCheckSingleFile(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateStopcheck(CCmdUI *pCmdUI);
	afx_msg void OnLvnItemchangedListInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnCheckSingleFile();
	afx_msg LRESULT OnSaveFileFailed(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCheckallfile();
	afx_msg void OnNMDblclkListFile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateCheckallfile(CCmdUI *pCmdUI);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnUsehttpdata();
	afx_msg void OnDownloadkfile();
	afx_msg void OnUploadkfile();
	afx_msg void OnMiDbHq();
	afx_msg void OnUpdateMiDbHq(CCmdUI *pCmdUI);
//	afx_msg void OnMiStopweight();
//	afx_msg void OnUpdateMiStopweight(CCmdUI *pCmdUI);
	afx_msg void OnMiUploadHq();
	afx_msg void OnMiDownloadHq();

	DECLARE_MESSAGE_MAP()
    afx_msg LRESULT OnListMsg(WPARAM wParam, LPARAM lParam);
public:
	CMulFuncList   m_listInfo;
	CListBox       m_listResult;	
	CMulFuncList   m_listFile;
	CSQLServerDB   m_sqlDB;

	CYinZiList     m_lstYinZi;
public:
	afx_msg void OnMiCutfile();
public:
	afx_msg void OnBnClickedBtnAddrecord();
public:
	afx_msg void OnUpdateWeight();
};
