#pragma once
#include "afxwin.h"
#include "../../public/protocol.h"
#include "farm.h"
#include "ClientConn.h"

// CDlg_StockInfo �Ի���

class CDlg_StockInfo : public CDialog
{
	DECLARE_DYNAMIC(CDlg_StockInfo)

public:
	CDlg_StockInfo(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_StockInfo();
	
	long cast(std::string *pStr, RINDEX *pRindex, bool bQuitIfNoKey = true);
// �Ի�������
	enum { IDD = IDD_DLG_STOCK_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnQuery();
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()

	
private:
	BOOL FillPack();
	void ParseHQData(int nDataLen, Quote *pQuote = NULL);
	void ParseKLineData(int nDataLen);
	void ParseFbcjData(int nDataLen);
	void DeleteAllColumn();
	void ShowStaticCtrl(BOOL bShow);
	void LocalQuery();
	void ParseKLineData(MinBlock *pMinBlock, int nCount);
	void ParseFbcjData(TickBlock *pTickBlock, int nCount);
	void ZeroCtrl();
	BOOL IsDigital(CString strCode);

    //Attributes
	CDataFarm*   m_pdataFarm;
	CLockFile    m_hLockFile;           //���������ļ�

	CComboBox    m_combDataType;
	CEdit        m_editStockCode;
	CListCtrl    m_listInfo;
	   
	CClientConn    m_downData;
	CommxHead    m_commxHead;

	char*        m_pRecvBuf;
	int          m_nRecvBufSize;

	char         m_chSendBuf[SEND_BUF_LEN];
	int          m_nPackSize;
	int          m_nNetQuery;
};


