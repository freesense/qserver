#pragma once
#include "farm.h"

// CDlg_CombiFile 对话框

class CDlg_CombiFile : public CDialog
{
	DECLARE_DYNAMIC(CDlg_CombiFile)

public:
	CDlg_CombiFile(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlg_CombiFile();

// 对话框数据
	enum { IDD = IDD_DLG_COMBITIONFILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedCombination();
	afx_msg void OnBnClickedBtnBrowse1();
	afx_msg void OnBnClickedBtnBrowse2();
	afx_msg void OnBnClickedBtnBrowse3();
	afx_msg void OnBnClickedBtnOpen1();
	afx_msg void OnBnClickedBtnOpen2();
	afx_msg void OnBnClickedBtnOpen3();

	BOOL ValidCheck();
	BOOL InitIndexHead(CString strIndexFile, IndexHead* pIndex);
private:
	int         m_nQuoteFile1;
	int         m_nMinFile1;
	int         m_nTickFile1;
	
	UINT        m_nMinK;
	UINT        m_nTick;
	UINT        m_nSymbol;

	CString     m_strPath1;
	CString     m_strPath2;
	CString     m_strPath3;
	
	UINT        m_nMarketCount1;
	UINT        m_nMarketCount2;
	UINT        m_nMarketCount3;
};
