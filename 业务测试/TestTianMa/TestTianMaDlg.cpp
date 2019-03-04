// TestTianMaDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestTianMa.h"
#include "TestTianMaDlg.h"
#include "..\..\public\commx\highperformancecounter.h"
#include "ConDef.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma warning(disable:4819)
#define IDT_UPDATE 1

tagTestState     g_TestState;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTestTianMaDlg 对话框




CTestTianMaDlg::CTestTianMaDlg(CWnd* pParent /*=NULL*/)
: CDialog(CTestTianMaDlg::IDD, pParent)
, m_nPort(11001)
, m_bSingle(TRUE)
, m_nConCount(1)
, m_bIsShortCon(FALSE)
, m_bHK(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nPackType = 0;
//	m_pPressTest = NULL;
	m_nCode = -1;
	m_nName = -1;
}

void CTestTianMaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVERIP, m_ServIP);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_COMBO, m_combo);
	DDX_Control(pDX, IDC_CHECK_BASE, m_cbBase);
	DDX_Control(pDX, IDC_CHECK_PLUS, m_cbPlus);
	DDX_Control(pDX, IDC_CHECK_PLUS2, m_cbPlus2);

	
	DDX_Radio(pDX, IDC_R_CN, m_bHK);
	DDX_Control(pDX, IDC_LIST3, m_lstBox);
	DDX_Control(pDX, IDC_L_STOCK, m_lstStock);
}

BEGIN_MESSAGE_MAP(CTestTianMaDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_START, &CTestTianMaDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CTestTianMaDlg::OnBnClickedBtnStop)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_BASE, &CTestTianMaDlg::OnBnClickedCheckBase)
	ON_BN_CLICKED(IDC_CHECK_PLUS, &CTestTianMaDlg::OnBnClickedCheckPlus)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_PLUS2, &CTestTianMaDlg::OnBnClickedCheckPlus2)

	ON_CBN_SELCHANGE(IDC_COMBO, &CTestTianMaDlg::OnCbnSelchangeCombo)
	ON_MESSAGE(WM_LISTRESULT, OnListResult)
	ON_BN_CLICKED(IDC_BTN_ADDSTART, &CTestTianMaDlg::OnBnClickedBtnAddstart)
END_MESSAGE_MAP()


// CTestTianMaDlg 消息处理程序

BOOL CTestTianMaDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	
	// TODO: 在此添加额外的初始化代码
	m_ServIP.SetWindowText("192.168.11.25");
	//m_ServIP.SetWindowText("192.168.48.15");

	InitCtrlPos();

	m_cbBase.SetCheck(1);
	ResetComboItem();
	m_combo.SetCurSel(0);
	OnCbnSelchangeCombo();

	::InitializeCriticalSection(&m_csListRet);

	m_pressTest = new CPressTest(this);
	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestTianMaDlg::InitCtrlPos()
{
	CRect rcWnd;
	GetClientRect(rcWnd);
	rcWnd.DeflateRect(10,150,10,10);
	m_lstBox.MoveWindow(rcWnd);
}

void CTestTianMaDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestTianMaDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CTestTianMaDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTestTianMaDlg::OnBnClickedBtnStart()
{
	if (!StartTest())
		MessageBox("StartTest() 失败!");
}

void CTestTianMaDlg::OnBnClickedBtnStop()
{
	StopTest();
}

BOOL CTestTianMaDlg::StartTest()
{
	m_lstBox.ResetContent();
	GetDlgItem(IDC_S_1)->SetWindowText("");

	StopTest();

	UpdateData();

	char szIP[50] = {0};
	m_ServIP.GetWindowText(szIP, 50);
	
	/*
	m_pressTest->StartThread();
	if (!m_pressTest->Connect(szIP, m_nPort))
	//*/
	if (!m_cltCon.Connect(szIP, m_nPort))
	{
		MessageBox("Connect() failed");
		return FALSE;
	}

	//*
	char szBuf[1024] = {0};
	int nLen = 0;
	GetSendBuf(szBuf, nLen);
    if (!m_cltCon.SendPacket(szBuf, nLen))
	{
		MessageBox("SendPacket() failed");
		return FALSE;
	}

	
	CommxHead  comxHead = {0};
	int nRet = m_cltCon.RecvPacket((char*)&comxHead, sizeof(comxHead));
	if (nRet != sizeof(CommxHead))
	{
		MessageBox("RecvPacket() failed");
		return FALSE;
	}

	int nRecvSize = comxHead.Length + sizeof(comxHead);
	char* pRecvBuf = new char[nRecvSize];
	memset(pRecvBuf, 0, nRecvSize);
	memcpy(pRecvBuf, &comxHead, sizeof(comxHead));

	nRet = m_cltCon.RecvPacket(pRecvBuf+sizeof(comxHead), comxHead.Length);
	if (nRet != comxHead.Length)
	{
		MessageBox("RecvPacket() failed");
		return FALSE;
	}

	OnRead(pRecvBuf, nRecvSize, m_nPackType, 0);
	delete pRecvBuf;
	//*/
	return TRUE;
}

void CTestTianMaDlg::OnConnect(double time)
{ 
}

int CTestTianMaDlg::GetSendBuf(const char* pBuf, int& nBufLen)
{
	int nPackType = 0;
	nPackType = m_nPackType;

	int nPackSize = 0;
	tagPackHead packHead = {0};
	tagComInfo comInfo = {0};
	packHead.comxHead.SerialNo = nPackType;
	packHead.nFuncNo = nPackType;
	packHead.nReqCount = 1;
	packHead.comxHead.Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);

	std::string strMktType = "CN";
	char szStockCode[8] = "000001";

	switch(nPackType)
	{
	case STOCK_INFO:
	case INDUSTRY_INFO:
	case INDEX_INFO:
	case IDU_REPRE_DATA:
	case MKT_FACTOR_IDX:
	case MKT_MIX_IDX:
		{
			nPackSize = sizeof(tagPackHead) + sizeof(tagComInfo);	
			comInfo.nPacktype = nPackType;

			memcpy(comInfo.MarketType, m_bHK ? "HK":"CN", 2);
			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);
			memcpy(pBufPos, &comInfo, sizeof(tagComInfo));
			break;
		}
	case PAYOFF_COLLECT:
	case PAYOFF_BILL:
	case COMMENTARY_BILL:
	case COMMENTARY_COLLECT:
	case HIS_FINANCE_IDX:
		{
			nPackSize = sizeof(tagPackHead) + sizeof(tagComInfo) + 8;	
			comInfo.nPacktype = nPackType;
			//memcpy(comInfo.MarketType, strMktType.c_str(), 2);
			memcpy(comInfo.MarketType, m_bHK ? "HK":"CN", 2);

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);
			memcpy(pBufPos, &comInfo, sizeof(tagComInfo));
			pBufPos += sizeof(tagComInfo);

			memset(szStockCode, 0, sizeof(szStockCode));
			GetDlgItem(IDC_E_2)->GetWindowText(szStockCode, 8);
			memcpy(pBufPos, szStockCode, 8);
			break;
		}
	case SUBTAB_INFO:
	case IDU_SORT_DATA:
		{
			nPackSize = sizeof(tagPackHead) + sizeof(tagComInfo) + sizeof(unsigned short);	
			comInfo.nPacktype = nPackType;
			//memcpy(comInfo.MarketType, strMktType.c_str(), 2);
			memcpy(comInfo.MarketType, m_bHK ? "HK":"CN", 2);

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);
			memcpy(pBufPos, &comInfo, sizeof(tagComInfo));
			pBufPos += sizeof(tagComInfo);

			CString strValue;
			GetDlgItem(IDC_E_2)->GetWindowText(strValue);

			unsigned short nSubTabNum = atoi(strValue);
			memcpy(pBufPos, &nSubTabNum, sizeof(unsigned short));

			break;
		}
	case HQ_INDEX:
	case IDX_INDEX:
	case IDU_INDEX:
		{
			int nCount = m_lstStock.GetCount();
			nPackSize = sizeof(tagPackHead) + sizeof(unsigned short) + 
				sizeof(unsigned short) + sizeof(tagMarketandStock)*nCount;

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			memcpy(pBufPos, &nCount, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strValue;
			CString strMarketType;
			CString strStockCode;
			for(int i=0; i<nCount; i++)
			{
				tagMarketandStock info = {0};
				
				m_lstStock.GetText(i, strValue);
				strMarketType = strValue.Left(2);
				memcpy(info.MarketType, strMarketType, 2);
				strStockCode = strValue.Right(strValue.GetLength() - strValue.Find("-") - 1);
				memcpy(info.Sec_cd, strStockCode, 8); 

				memcpy(pBufPos, &info, sizeof(tagMarketandStock));
				pBufPos += sizeof(tagMarketandStock);
			}
			break;
		}
	case ANALYSER_INFO:
	case INIT_INFO:
	case SELSTKMOD_INFO:
		{
			nPackSize = sizeof(tagPackHead) + sizeof(unsigned short);
			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);
			break;
		}
	case ANALYSER_REMARK:
		{
			nPackSize = sizeof(tagPackHead) + sizeof(unsigned short);
			nPackSize += 20;
			nPackSize += 8;
			nPackSize += 10;

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strValue;
			GetDlgItem(IDC_E_2)->GetWindowText(strValue);
			if (strValue.IsEmpty())
				memset(pBufPos, 0, 20);
			else
				memcpy(pBufPos, strValue, 20);
			pBufPos += 20;

			GetDlgItem(IDC_E_3)->GetWindowText(strValue);
			if (strValue.IsEmpty())
				memset(pBufPos, 0, 8);
			else
				memcpy(pBufPos, strValue, 8);
			pBufPos += 8;

			GetDlgItem(IDC_E_4)->GetWindowText(strValue);
			if (strValue.IsEmpty())
				memset(pBufPos, 0, 10);
			else
				memcpy(pBufPos, strValue, 10);
			pBufPos += 10;

			break;
		}
	case ADD_REMARK:
		{
			CTime tmCur = CTime::GetCurrentTime();
			CString strDate;
			strDate.Format("%04d%02d%02d", tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay());

			CString strValue;
			GetDlgItem(IDC_E_2)->GetWindowText(strValue);
			CString strAnaID = strValue;

			GetDlgItem(IDC_E_3)->GetWindowText(strValue);
			CString strStkCode = strValue;

			GetDlgItem(IDC_E_4)->GetWindowText(strValue);
			CString strIduCode = strValue;

			GetDlgItem(IDC_E_5)->GetWindowText(strValue);
			CString strRemark = strValue;

			nPackSize = sizeof(tagPackHead) + sizeof(unsigned short);
			nPackSize += 8;
			nPackSize += 20;
			nPackSize += 8;
			nPackSize += 10;
			nPackSize += sizeof(unsigned short);
			nPackSize += (int)strRemark.GetLength() + 1;

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			memcpy(pBufPos, strDate, 8);
			pBufPos += 8;
			memcpy(pBufPos, strAnaID, 20);
			pBufPos += 20;			
			memcpy(pBufPos, strStkCode, 8);
			pBufPos += 8; 			
			memcpy(pBufPos, strIduCode, 10);
			pBufPos += 10;

			int nLen = (int)strRemark.GetLength() + 1;
			memcpy(pBufPos, &nLen, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);
			memcpy(pBufPos, strRemark, strRemark.GetLength() + 1);

			break;
		}
	case VERIFY_USERINFO:
	case MODIFY_PASSWORD:
		{
			CString strValue;
			GetDlgItem(IDC_E_2)->GetWindowText(strValue);
			CString strUserID = strValue;

			GetDlgItem(IDC_E_3)->GetWindowText(strValue);
			CString strPw = strValue;

			GetDlgItem(IDC_E_4)->GetWindowText(strValue);
			CString strNewPw = strValue;

			nPackSize = sizeof(tagPackHead) + sizeof(unsigned short) + 16 + 32;
			if (nPackType == MODIFY_PASSWORD)
				nPackSize += 32;

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			strcpy_s(pBufPos, 16, strUserID);
			pBufPos += 16;

			strcpy_s(pBufPos, 32, strPw);
			pBufPos += 32;

			if (nPackType == MODIFY_PASSWORD)
			{
				strcpy_s(pBufPos, 32, strNewPw);
				pBufPos += 32;
			}

			break;
		}
	case INDEX_HIS_DATA:
		{
			nPackSize = sizeof(tagPackHead) + sizeof(tagComInfo) + 8 + sizeof(unsigned short);	
			comInfo.nPacktype = nPackType;
			memcpy(comInfo.MarketType, m_bHK ? "HK":"CN", 2);
			//memcpy(comInfo.MarketType, strMktType.c_str(), 2);

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);
			memcpy(pBufPos, &comInfo, sizeof(tagComInfo));
			pBufPos += sizeof(tagComInfo);
			
			CString strValue;
			GetDlgItem(IDC_E_2)->GetWindowText(strValue);
			strcpy_s(pBufPos, 8, strValue);
			pBufPos += 8;
			
			unsigned short nIdxMark = 0;
			GetDlgItem(IDC_E_3)->GetWindowText(strValue);
			nIdxMark = atoi(strValue);

			memcpy(pBufPos, &nIdxMark, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);
			break;
		}
	case QUOTE_BILL:
	case REAL_MIN:
	case BARGAIN_BILL:
	case DAY_KLINE:
	case WEK_KLINE:
	case MON_KLINE:
	case TWO_DAY_REAL_MIN:
		{
			int nCount = m_lstStock.GetCount();

			nPackSize = sizeof(tagPackHead) + sizeof(unsigned short) +
				sizeof(unsigned short) + SYMBOL_LEN * nCount;
			if (nPackType == BARGAIN_BILL)
			{
				nPackSize = sizeof(tagPackHead) + sizeof(unsigned short) +
					sizeof(unsigned short) + (SYMBOL_LEN + sizeof(unsigned short))* nCount;
			}
			else if (nPackType == TWO_DAY_REAL_MIN )
			{
				nPackSize = sizeof(tagPackHead) + sizeof(unsigned short) +
					sizeof(unsigned short) + (SYMBOL_LEN + sizeof(unsigned long)*2) * nCount;
			}
			else if (nPackType == DAY_KLINE ||
				nPackType == WEK_KLINE ||
				nPackType == MON_KLINE)
			{
				nPackSize = sizeof(tagPackHead) + sizeof(unsigned short) +
					sizeof(unsigned short) + (SYMBOL_LEN + sizeof(unsigned long)*2 + sizeof(unsigned short)) * nCount;
			}

			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			memcpy(pBufPos, &nCount, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strValue;
			CString strStockCode;
			CString strTime;
			CString strStartDate;
			CString strStopDate;
			for(int i=0; i<nCount; i++)
			{
				m_lstStock.GetText(i, strValue);
				if (nPackType == QUOTE_BILL || nPackType == REAL_MIN)
				{
					strStockCode = strValue;
				}
				else if (nPackType == BARGAIN_BILL)
				{
					strStockCode = strValue.Left(strValue.Find("-"));
					strTime = strValue.Right(strValue.GetLength() - strValue.Find("-") - 1);
				}
				else if (nPackType == DAY_KLINE ||
					    nPackType == WEK_KLINE ||
					    nPackType == MON_KLINE ||
					nPackType == TWO_DAY_REAL_MIN )
				{
					strStockCode = strValue.Left(strValue.Find("-"));
					strStartDate = strValue.Mid(strValue.Find("-")+1, strValue.ReverseFind('-') - strValue.Find("-") - 1);
					strStopDate = strValue.Right(strValue.GetLength() - strValue.ReverseFind('-') - 1);
				}

				strcpy_s(pBufPos, SYMBOL_LEN, strStockCode);
				pBufPos += SYMBOL_LEN;

				if (nPackType == BARGAIN_BILL)
				{
					unsigned short nTime = atoi(strTime);
					memcpy(pBufPos, &nTime, sizeof(unsigned short));
					pBufPos += sizeof(unsigned short);
				}
				else if (nPackType == DAY_KLINE ||
					nPackType == WEK_KLINE ||
					nPackType == MON_KLINE ||
					nPackType == TWO_DAY_REAL_MIN )
				{
					unsigned long nDate = atoi(strStartDate);
					memcpy(pBufPos, &nDate, sizeof(unsigned long));
					pBufPos += sizeof(unsigned long);

					nDate = atoi(strStopDate);
					memcpy(pBufPos, &nDate, sizeof(unsigned long));
					pBufPos += sizeof(unsigned long);

					if (nPackType != TWO_DAY_REAL_MIN)
					{
						unsigned short nHQMkt = 0;
						memcpy(pBufPos, &nHQMkt, sizeof(unsigned short));
						pBufPos += sizeof(unsigned short);
					}
				}
			}
			break;
		}
	case SORT_INFO:
		{
			m_nName = -1;
			m_nCode = -1;
			m_nRecCount = 0;
			
			CString strField;// = "jrkp|code|name|zrsp|zjjg|zjcj";
			GetDlgItem(IDC_E_5)->GetWindowText(strField);
			int nPos = strField.Find("|");
			CString strTemp;
			if (nPos == -1)
			{
				if (strField.CompareNoCase("code") == 0)
					m_nCode = 0;
				if (strField.CompareNoCase("name") == 0)
					m_nName = 0;
			}

			m_nIndex = 0;
			int nBegPos = 0;
			while(nPos != -1)
			{
				strTemp = strField.Mid(nBegPos, nPos - nBegPos);
				if (strTemp.CompareNoCase("code") == 0)
					m_nCode = m_nIndex;
				if (strTemp.CompareNoCase("name") == 0)
					m_nName = m_nIndex;

				m_nIndex++;
				nBegPos = nPos+1;
				nPos = strField.Find("|", nBegPos);
			}

			strTemp = strField.Mid(nBegPos, strField.GetLength() - nBegPos);
			if (strTemp.CompareNoCase("code") == 0)
				m_nCode = m_nIndex;
			if (strTemp.CompareNoCase("name") == 0)
				m_nName = m_nIndex;
			m_nIndex++;
			
			CString strSymbol;
			GetDlgItem(IDC_E_2)->GetWindowText(strSymbol);
			strSymbol += "@";
			strSymbol += strField;
			
			nPackSize = sizeof(CommxHead) + sizeof(unsigned short) + 1 + 
				2*sizeof(unsigned int) + strSymbol.GetLength()+1;
			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead.comxHead, sizeof(CommxHead));
			pBufPos += sizeof(CommxHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			unsigned char SortType = 1; 
			if (m_bHK)
			{
				SortType = 1; 
				memcpy(pBufPos, &SortType, 1);
			}
			else
			{
				SortType = 0; 
				memcpy(pBufPos, &SortType, 1);
			}
			pBufPos += 1;

			CString strValue;
			GetDlgItem(IDC_E_3)->GetWindowText(strValue);

			unsigned int nStartLine = atoi(strValue);
			memcpy(pBufPos, &nStartLine, sizeof(unsigned int));
			pBufPos += sizeof(unsigned int);

			GetDlgItem(IDC_E_4)->GetWindowText(strValue);
			unsigned int nCount = atoi(strValue);
			memcpy(pBufPos, &nCount, sizeof(unsigned int));
			pBufPos += sizeof(unsigned int);
			m_nRecCount = nCount;

			memcpy(pBufPos, strSymbol, strSymbol.GetLength());
			pBufPos += strSymbol.GetLength() + 1;
			break;
		}
	case IDX_NUM:
		{
			nPackSize = sizeof(CommxHead) + sizeof(unsigned short) + 32;
			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead.comxHead, sizeof(CommxHead));
			pBufPos += sizeof(CommxHead);

			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			const int nSymbolLen = 32;
			char szSymbol[nSymbolLen] = {0};
			GetDlgItem(IDC_E_2)->GetWindowText(szSymbol, nSymbolLen);
			memcpy(pBufPos, szSymbol, nSymbolLen);
			pBufPos += nSymbolLen;
			break;
		}
	case SELSTKMOD_DATA:
		{
			nPackSize = sizeof(tagPackHead) + sizeof(unsigned short) + 8;	
			packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

			char* pBufPos = (char*)pBuf;
			memcpy(pBufPos, &packHead, sizeof(tagPackHead));
			pBufPos += sizeof(tagPackHead);
			memcpy(pBufPos, &nPackType, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strValue;
			GetDlgItem(IDC_E_2)->GetWindowText(strValue);
			strcpy_s(pBufPos, 8, strValue);
			break;
		}
	case INVALID_DATA:   //乱包测试
		{
			//是否有包头
			BOOL bHead = TRUE;
			srand((UINT)time(NULL));
			bHead = rand()%2;

			FILE* pFile;
			pFile = _fsopen(m_strFile, "a+", _SH_DENYNO);
			srand((UINT)time(NULL));
			int nDataSize = rand()%300;

			if (bHead)
			{
				nPackSize = sizeof(tagPackHead) + nDataSize;
				packHead.comxHead.Length = nPackSize - sizeof(CommxHead);

				char* pBufPos = (char*)pBuf;
				memcpy(pBufPos, &packHead, sizeof(tagPackHead));
				pBufPos += sizeof(tagPackHead);

				fread(pBufPos, sizeof(char), nDataSize, pFile);
			}
			else
			{
				nPackSize = nDataSize;
				fread((char*)pBuf, sizeof(char), nDataSize, pFile);
			}

			fclose(pFile);
			break;
		}
	default:
		{
			REPORT(MN, T("没有处理的业务类型\n"), RPT_ERROR);
			return false;
		}
	}

	nBufLen = nPackSize;
	//发送
#ifdef PRESS_TEST
	::EnterCriticalSection(&m_csMap);
	tagtestResult* pRet = GetTestRet(nPackType);
	pRet->nSendTimes++;
	::LeaveCriticalSection(&m_csMap);
#endif

	return nPackType;
}


void CTestTianMaDlg::OnConnectFailed()
{
}

void CTestTianMaDlg::OnReconnect()
{

}

void CTestTianMaDlg::OnDisconnect()
{
}

void CTestTianMaDlg::OnReadFailed(int nPackType)
{
}

void CTestTianMaDlg::OnWriteFailed(int nPackType)
{
}

void CTestTianMaDlg::OnWrite()
{
}

void CTestTianMaDlg::OnRead(const char* pBuf, int nBufLen, int nPackType, double time)
{
	//继续发送
	char szTemp[512];
	char* pBufPos = (char*)pBuf;
	int nLeaveSize = nBufLen;
	if (m_nPackType == SORT_INFO)
	{
		CommxHead  comxHead = {0};
		memcpy(&comxHead, pBufPos, sizeof(CommxHead));
		pBufPos += sizeof(CommxHead);
		nLeaveSize -= sizeof(CommxHead);
		unsigned int nFuc = 0;
		memcpy(&nFuc, pBufPos, sizeof(unsigned short));
		pBufPos += sizeof(unsigned short);
		nLeaveSize -= sizeof(unsigned short);
		if (nFuc == 0xffff)
		{
			CString strMsg = pBufPos;
			GetDlgItem(IDC_S_1)->SetWindowText(strMsg);
			return;
		}

		CString strRet;
		strRet.Format("列数:%d", m_nIndex);
		GetDlgItem(IDC_S_1)->SetWindowText(strRet);

		unsigned int nValue = 0;

		if (nLeaveSize <= 0)
		{
			GetDlgItem(IDC_S_1)->SetWindowText("没有您查询的数据!");
			return;
		}
		for (int j=0; j<m_nRecCount; j++)
		{
			strRet.Empty();
			for (int i=0; i<m_nIndex; i++)
			{
				strRet += "  ";
				if (m_nCode == i)
				{
					if (nLeaveSize <= 0)
						return;
					memcpy(szTemp, pBufPos, 32);
					pBufPos += 32;
					nLeaveSize -= 32;
				}
				else if (m_nName == i)
				{
					if (nLeaveSize <= 0)
						return;
					memcpy(szTemp, pBufPos, 32);
					pBufPos += 32;
					nLeaveSize -= 32;
				}
				else
				{
					if (nLeaveSize <= 0)
						return;
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					nLeaveSize -= sizeof(unsigned int);
					sprintf_s(szTemp, 512, "%d", nValue);
				}
				strRet += szTemp;

				if ( (i+1) % 10 == 0)
				{
					SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
					strRet.Empty();
				}
			}
			if (!strRet.IsEmpty())
				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
		}

		return;
	}
	else if (m_nPackType == IDX_NUM)
	{
		CommxHead  comxHead = {0};
		memcpy(&comxHead, pBufPos, sizeof(CommxHead));
		pBufPos += sizeof(CommxHead);

		unsigned int nFuc = 0;
		memcpy(&nFuc, pBufPos, sizeof(unsigned short));
		pBufPos += sizeof(unsigned short);

		CString strRet;
		unsigned int nValue = 0;

		strRet += "上涨:";
		memcpy(&nValue, pBufPos, sizeof(unsigned int));
		pBufPos += sizeof(unsigned int);
		sprintf_s(szTemp, 512, "%d", nValue);
		strRet += szTemp;

		strRet += "  下跌:";
		memcpy(&nValue, pBufPos, sizeof(unsigned int));
		pBufPos += sizeof(unsigned int);
		sprintf_s(szTemp, 512, "%d", nValue);
		strRet += szTemp;

		strRet += "  平盘家数:";
		memcpy(&nValue, pBufPos, sizeof(unsigned int));
		pBufPos += sizeof(unsigned int);
		sprintf_s(szTemp, 512, "%d", nValue);
		strRet += szTemp;

		SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
	}

	tagPackHead packHead = {0};
	memcpy(&packHead, pBufPos, sizeof(tagPackHead));
	pBufPos += sizeof(tagPackHead);

	tagComInfo comInfo = {0};
	memcpy(&comInfo, pBufPos, sizeof(tagComInfo));

	int nMinSize = sizeof(tagPackHead) + sizeof(unsigned short);
	if (nBufLen <= nMinSize)
	{
		GetDlgItem(IDC_S_1)->SetWindowText("记录个数:0");
		return;
	}

	switch(nPackType)
	{
	case STOCK_INFO:
		{
			pBufPos += sizeof(tagComInfo);
			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			CString strRet;
			strRet.Format("市场类别:%s 证券个数:%d", szMaketType, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagStockInfo info;
			for(int i=0; i<nCount; i++)
			{
				memset(&info, 0, sizeof(tagStockInfo));
				memcpy(&info, pBufPos, sizeof(tagStockInfo));
				pBufPos += sizeof(tagStockInfo);

				strRet.Empty();
				strRet += "Sec_cd:";
				strRet += (char*)info.Sec_cd;
				strRet += "  Sec_id:";
				strRet += (char*)info.Sec_id;
				strRet += "  Org_id:";
				strRet += (char*)info.Org_id;

				strRet += "  Idu_cd:";
				strRet += (char*)info.Idu_cd;
				strRet += "  Area_cd:";
				strRet += (char*)info.Area_cd;
				strRet += "  Var_cl:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Var_cl, 2);
				strRet += szTemp;

				strRet += "  Mkt_cl:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Mkt_cl, 1);
				strRet += szTemp;

				strRet += "  Sec_snm:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Sec_snm, 16);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				strRet += "Sec_esnm:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Sec_esnm, 16);
				strRet += szTemp;

				strRet += "  Sec_ssnm:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Sec_ssnm, 4);
				strRet += szTemp;

				strRet += "  nIss_dt:";
				sprintf_s(szTemp, 100, "%d", info.nIss_dt);
				strRet += szTemp;

				strRet += "  nMkt_dt:";
				sprintf_s(szTemp, 100, "%d", info.nMkt_dt);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case INDUSTRY_INFO:	//行业分类
		{
			pBufPos += sizeof(tagComInfo);
			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			CString strRet;
			strRet.Format("市场类别:%s 行业个数:%d", szMaketType, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagIndustryInfo info;
			for(int i=0; i<nCount; i++)
			{
				memset(&info, 0, sizeof(tagIndustryInfo));
				memcpy(&info, pBufPos, sizeof(tagIndustryInfo));
				pBufPos += sizeof(tagIndustryInfo);

				strRet.Empty();
				strRet += "Idu_cd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Idu_cd, 10);
				strRet += szTemp;

				strRet += "  Idu_nm:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Idu_nm, 40);
				strRet += szTemp;
				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case INDEX_INFO:
		{
			pBufPos += sizeof(tagComInfo);
			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			CString strRet;
			strRet.Format("市场类别:%s 指数个数:%d", szMaketType, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagIndexInfo info;
			char szStockCode[8];
			for(int i=0; i<nCount; i++)
			{
				memset(&info, 0, sizeof(tagIndexInfo));
				memcpy(&info, pBufPos, sizeof(tagIndexInfo));
				pBufPos += sizeof(tagIndexInfo);

				strRet.Empty();
				strRet += "Idx_cd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Idx_cd, 10);
				strRet += szTemp;

				strRet += "  Var_cl:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Var_cl, 2);
				strRet += szTemp;

				strRet += "  Mkt_cl:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Mkt_cl, 1);
				strRet += szTemp;

				strRet += "  Idx_snm:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Idx_snm, 16);
				strRet += szTemp;

				strRet += "  Idx_esnm:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Idx_esnm, 16);
				strRet += szTemp;

				strRet += "  Idx_ssnm:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Idx_ssnm, 4);
				strRet += szTemp;

				strRet += "  股票总数:";
				sprintf_s(szTemp, 100, "%d", info.nRecordCount);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);

				for (int j=0; j<info.nRecordCount; j++)
				{
					memcpy(szStockCode, pBufPos, 8);
					pBufPos += 8;

					CString strStockCode = szStockCode;
					SendMessage(WM_LISTRESULT, (WPARAM)&strStockCode);
				}
			}
			break;
		}
	case IDU_REPRE_DATA:
		{
			pBufPos += sizeof(tagComInfo);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);

			CString strRet;
			strRet.Format("市场类别:%s 记录个数:%d", szMaketType, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				tagIduRepresentData* pInfo = (tagIduRepresentData*)pBufPos;
				pBufPos += sizeof(tagIduRepresentData);

				strRet.Empty();
				strRet += "因素标识ID:";
				sprintf_s(szTemp, 100, "%d", pInfo->nFactorMark);
				strRet += szTemp;

				strRet += "  field1:";
				sprintf_s(szTemp, 100, "%f", pInfo->field1);
				strRet += szTemp;

				strRet += "  field2:";
				sprintf_s(szTemp, 100, "%f", pInfo->field2);
				strRet += szTemp;

				strRet += "  field3:";
				sprintf_s(szTemp, 100, "%f", pInfo->field3);
				strRet += szTemp;

				strRet += "  field4:";
				sprintf_s(szTemp, 100, "%f", pInfo->field4);
				strRet += szTemp;

				strRet += "  field5:";
				sprintf_s(szTemp, 100, "%f", pInfo->field5);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case PAYOFF_COLLECT:
		{
			pBufPos += sizeof(tagComInfo);
			char szStockCode[8];
			memcpy(szStockCode, pBufPos, 8);
			pBufPos += 8;

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);

			CString strRet;
			strRet.Format("市场类别:%s 股票代码:%s 指标个数:%d", szMaketType, szStockCode, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagPayOffCollect info;
			for(int i=0; i<nCount; i++)
			{
				memset(&info, 0, sizeof(tagPayOffCollect));
				memcpy(&info, pBufPos, sizeof(tagPayOffCollect));
				pBufPos += sizeof(tagPayOffCollect);

				strRet.Empty();
				strRet += "IND_ID:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.IND_ID, 6);
				strRet += szTemp;

				strRet += "  Rpt_Yr:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Rpt_Yr, 4);
				strRet += szTemp;

				strRet += "  Rpt_Prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Rpt_Prd, 4);
				strRet += szTemp;

				strRet += "  Sta_Prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Sta_Prd, 4);
				strRet += szTemp;

				strRet += "  Avg_Value:";
				sprintf_s(szTemp, 100, "%f", info.Avg_Value);
				strRet += szTemp;

				strRet += "  Hig_Value:";
				sprintf_s(szTemp, 100, "%f", info.Hig_Value);
				strRet += szTemp;

				strRet += "  Low_Value:";
				sprintf_s(szTemp, 100, "%f", info.Low_Value);
				strRet += szTemp;

				strRet += "  Ana_Num:";
				sprintf_s(szTemp, 100, "%d", info.Ana_Num);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case PAYOFF_BILL:
		{
			pBufPos += sizeof(tagComInfo);
			char szStockCode[8];
			memcpy(szStockCode, pBufPos, 8);
			pBufPos += 8;

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);

			CString strRet;
			strRet.Format("市场类别:%s 股票代码:%s 记录个数:%d", szMaketType, szStockCode, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagPayOffBill info;
			for(int i=0; i<nCount; i++)
			{
				memset(&info, 0, sizeof(tagPayOffBill));
				memcpy(&info, pBufPos, sizeof(tagPayOffBill));
				pBufPos += sizeof(tagPayOffBill);

				strRet.Empty();
				strRet += "ORG_CL:";
				sprintf_s(szTemp, 100, "%d",info.ORG_CL);
				strRet += szTemp;

				strRet += "  IND_ID:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.IND_ID, 6);
				strRet += szTemp;

				strRet += "  For_Date:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.For_Date, 8);
				strRet += szTemp;

				strRet += "  Rpt_Yr:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Rpt_Yr, 4);
				strRet += szTemp;

				strRet += "  Rpt_Prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Rpt_Prd, 4);
				strRet += szTemp;

				strRet += "  For_Value:";
				sprintf_s(szTemp, 100, "%f", info.For_Value);
				strRet += szTemp;

				strRet += "  Author:";
				strRet += (char*)info.Author;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				strRet += " Dil_Value:";
				sprintf_s(szTemp, 100, "%f", info.Dil_Value);
				strRet += szTemp;

				strRet += "  Dil_Date:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Dil_Date, 8);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case COMMENTARY_BILL:
		{
			pBufPos += sizeof(tagComInfo);
			char szStockCode[8];
			memcpy(szStockCode, pBufPos, 8);
			pBufPos += 8;

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			CString strRet;
			strRet.Format("市场类别:%s 股票代码:%s 记录个数:%d", szMaketType, szStockCode, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagCommentaryBill info;
			for(int i=0; i<nCount; i++)
			{
				memset(&info, 0, sizeof(tagCommentaryBill));
				memcpy(&info, pBufPos, sizeof(tagCommentaryBill));
				pBufPos += sizeof(tagCommentaryBill);

				strRet.Empty();
				strRet += "ORG_CL:";
				sprintf_s(szTemp, 100, "%d",info.ORG_CL);
				strRet += szTemp;

				strRet += "  Gra_Date:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Gra_Date, 8);
				strRet += szTemp;

				strRet += "  Gra_Prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, (void*)&info.Gra_Prd, 1);
				strRet += szTemp;

				strRet += " Gra_Value:";
				sprintf_s(szTemp, 100, "%f", info.Gra_Value);
				strRet += szTemp;

				strRet += " Tar_Price:";
				sprintf_s(szTemp, 100, "%f", info.Tar_Price);
				strRet += szTemp;

				strRet += "  Tar_Prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Tar_Prd, 3);
				strRet += szTemp;

				strRet += "  Author:";
				strRet += (char*)info.Author;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				strRet += " Exr_Price:";
				sprintf_s(szTemp, 100, "%f", info.Exr_Price);
				strRet += szTemp;

				strRet += "  Exr_Date:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Exr_Date, 8);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case COMMENTARY_COLLECT:
		{
			pBufPos += sizeof(tagComInfo);

			char szStockCode[8] = {0};
			memcpy(szStockCode, pBufPos, 8);
			pBufPos += 8;

			unsigned short nStockCount = 0;
			memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			CString strRet;
			strRet.Format("市场类别:%s 股票代码:%s 记录个数:%d", szMaketType, szStockCode, nStockCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagCommentaryCollect info;
			for(int i=0; i<nStockCount; i++)
			{
				memset(&info, 0, sizeof(tagCommentaryCollect));
				memcpy(&info, pBufPos, sizeof(tagCommentaryCollect));
				pBufPos += sizeof(tagCommentaryCollect);

				strRet.Empty();
				strRet += "Sta_Prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Sta_Prd, 4);
				strRet += szTemp;

				strRet += " Avg_Value:";
				sprintf_s(szTemp, 100, "%f", info.Avg_Value);
				strRet += szTemp;

				strRet += " Tbu_Num:";
				sprintf_s(szTemp, 100, "%f", info.Tbu_Num);
				strRet += szTemp;

				strRet += " Buy_Num:";
				sprintf_s(szTemp, 100, "%f", info.Buy_Num);
				strRet += szTemp;

				strRet += " Neu_Num:";
				sprintf_s(szTemp, 100, "%f", info.Neu_Num);
				strRet += szTemp;

				strRet += " Red_Num:";
				sprintf_s(szTemp, 100, "%f", info.Red_Num);
				strRet += szTemp;

				strRet += " Sel_Num:";
				sprintf_s(szTemp, 100, "%f", info.Sel_Num);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				strRet += " Avg_Price:";
				sprintf_s(szTemp, 100, "%f", info.Avg_Price);
				strRet += szTemp;

				strRet += " Hig_Price:";
				sprintf_s(szTemp, 100, "%f", info.Hig_Price);
				strRet += szTemp;

				strRet += " Low_Price:";
				sprintf_s(szTemp, 100, "%f", info.Low_Price);
				strRet += szTemp;
				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case HIS_FINANCE_IDX:
		{
			pBufPos += sizeof(tagComInfo);

			char szStockCode[8] = {0};
			memcpy(szStockCode, pBufPos, 8);
			pBufPos += 8;

			unsigned short nStockCount = 0;
			memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);

			CString strRet;
			strRet.Format("市场类别:%s 股票代码:%s 记录个数:%d", szMaketType, szStockCode, nStockCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nStockCount; i++)
			{
				tagHisFinanceIdx* pInfo = (tagHisFinanceIdx*)pBufPos;
				pBufPos += sizeof(tagHisFinanceIdx);

				strRet.Empty();
				strRet += "IND_ID:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->IND_ID, 10);
				strRet += szTemp;

				strRet += "Rpt_Yr:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->Rpt_Yr, 4);
				strRet += szTemp;

				strRet += "Rpt_Prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->Rpt_Prd, 4);
				strRet += szTemp;

				strRet += " Ind_Value:";
				sprintf_s(szTemp, 100, "%f", pInfo->Ind_Value);
				strRet += szTemp;
				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case MKT_FACTOR_IDX:
		{
			pBufPos += sizeof(tagComInfo);

			unsigned short nStockCount = 0;
			memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);

			CString strRet;
			strRet.Format("市场类别:%s 记录个数:%d", szMaketType, nStockCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nStockCount; i++)
			{
				tagMktFactorIdx* pInfo = (tagMktFactorIdx*)pBufPos;
				pBufPos += sizeof(tagMktFactorIdx);

				strRet.Empty();
				strRet += "IDX_CD:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->IDX_CD, 8);
				strRet += szTemp;

				strRet += "Dt_prd:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->Dt_prd, 5);
				strRet += szTemp;

				strRet += " cm0601:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0601);
				strRet += szTemp;

				strRet += " cm0602:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0602);
				strRet += szTemp;

				strRet += " cm0603:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0603);
				strRet += szTemp;

				strRet += " cm0604:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0604);
				strRet += szTemp;

				strRet += " cm0605:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0605);
				strRet += szTemp;

				strRet += " cm0606:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0606);
				strRet += szTemp;

				strRet += " cm0607:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0607);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				strRet += " cm0608:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0608);
				strRet += szTemp;

				strRet += " cm0609:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0609);
				strRet += szTemp;

				strRet += " cm0610:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0610);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case MKT_MIX_IDX:
		{
			pBufPos += sizeof(tagComInfo);

			unsigned short nStockCount = 0;
			memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);

			CString strRet;
			strRet.Format("市场类别:%s 记录个数:%d", szMaketType, nStockCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nStockCount; i++)
			{
				tagMktMixIdx* pInfo = (tagMktMixIdx*)pBufPos;
				pBufPos += sizeof(tagMktMixIdx);

				strRet.Empty();
				strRet += "IDX_CD:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->IDX_CD, 8);
				strRet += szTemp;

				strRet += " cm0701:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0701);
				strRet += szTemp;

				strRet += " cm0702:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0702);
				strRet += szTemp;

				strRet += " cm0703:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0703);
				strRet += szTemp;

				strRet += " cm0704:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0704);
				strRet += szTemp;

				strRet += " cm0705:";
				sprintf_s(szTemp, 100, "%f", pInfo->cm0705);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				strRet += " cm0706:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->cm0706, 50);
				strRet += szTemp;

				strRet += " cm0707:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->cm0707, 50);
				strRet += szTemp;

				strRet += " cm0708:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->cm0708, 50);
				strRet += szTemp;

				strRet += " cm0709:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->cm0709, 50);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case SUBTAB_INFO:
		{
			pBufPos += sizeof(tagComInfo);

			unsigned short nSubTabNum = 0;
			memcpy(&nSubTabNum, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			
			CString strRet;
			strRet.Format("市场类别:%s 科目表编码:%d 记录个数:%d", szMaketType, nSubTabNum, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			tagSubjectTable info;
			for(int i=0; i<nCount; i++)
			{
				memset(&info, 0, sizeof(tagSubjectTable));
				memcpy(&info, pBufPos, sizeof(tagSubjectTable));
				pBufPos += sizeof(tagSubjectTable);

				strRet.Empty();
				strRet += "IND_ID:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.IND_ID, 10);
				strRet += szTemp;

				strRet += "  IND_NM:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.IND_NM, 40);
				strRet += szTemp;

				strRet += "  IND_ENM:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.IND_ENM, 40);
				strRet += szTemp;

				strRet += "  Mea_Unit:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, info.Mea_Unit, 10);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case IDU_SORT_DATA:
		{
			pBufPos += sizeof(tagComInfo);

			unsigned short nFactorMark = 0;
			memcpy(&nFactorMark, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			
			CString strRet;
			strRet.Format("市场类别:%s 因素标识:%d 记录个数:%d", szMaketType, nFactorMark, nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				tagIduSortData* pInfo = (tagIduSortData*)pBufPos;
				pBufPos += sizeof(tagIduSortData);

				strRet.Empty();
				strRet += "Idu_CD:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->Idu_CD, 10);
				strRet += szTemp;

				strRet += " field1:";
				sprintf_s(szTemp, 100, "%f", pInfo->field1);
				strRet += szTemp;

				strRet += " field2:";
				sprintf_s(szTemp, 100, "%f", pInfo->field2);
				strRet += szTemp;

				strRet += " field3:";
				sprintf_s(szTemp, 100, "%f", pInfo->field3);
				strRet += szTemp;

				strRet += " field4:";
				sprintf_s(szTemp, 100, "%f", pInfo->field4);
				strRet += szTemp;

				strRet += " field5:";
				sprintf_s(szTemp, 100, "%f", pInfo->field5);
				strRet += szTemp;

				strRet += " field6:";
				sprintf_s(szTemp, 100, "%f", pInfo->field6);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				strRet += " field7:";
				sprintf_s(szTemp, 100, "%f", pInfo->field7);
				strRet += szTemp;

				strRet += " field8:";
				sprintf_s(szTemp, 100, "%f", pInfo->field8);
				strRet += szTemp;

				strRet += " field9:";
				sprintf_s(szTemp, 100, "%f", pInfo->field9);
				strRet += szTemp;

				strRet += " field10:";
				sprintf_s(szTemp, 100, "%f", pInfo->field10);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case HQ_INDEX:
		{
			pBufPos += sizeof(unsigned short);
			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("股票个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "市场类别:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 2);
				pBufPos += 2;
				strRet += szTemp;

				strRet += "  股票代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 8);
				pBufPos += 8;
				strRet += szTemp;

				double value = 0;
				for (int i=1; i<=123; i++)
				{
					strRet += "  ";
					memcpy(&value, pBufPos, sizeof(double));
					pBufPos += sizeof(double);
					sprintf_s(szTemp, 100, "%f", value);
					strRet += szTemp;

					if (i % 10 == 0)
					{
						SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
						strRet.Empty();
					}
				}

				if (!strRet.IsEmpty())
					SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case IDX_INDEX:
		{
			pBufPos += sizeof(unsigned short);
			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("指数个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "市场类别:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 2);
				pBufPos += 2;
				strRet += szTemp;

				strRet += "  指数代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 8);
				pBufPos += 8;
				strRet += szTemp;

				strRet += "  短期趋势:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 10);
				pBufPos += 10;
				strRet += szTemp;

				strRet += "  中期趋势:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 10);
				pBufPos += 10;
				strRet += szTemp;

				strRet += "  主要趋势:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 10);
				pBufPos += 10;
				strRet += szTemp;
				
				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
				strRet.Empty();

				double value = 0;
				for (int i=1; i<=19; i++)
				{
					strRet += "  ";
					memcpy(&value, pBufPos, sizeof(double));
					pBufPos += sizeof(double);
					sprintf_s(szTemp, 100, "%f", value);
					strRet += szTemp;

					if (i % 10 == 0)
					{
						SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
						strRet.Empty();
					}
				}

				if (!strRet.IsEmpty())
					SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}   
			break;
		}
	case IDU_INDEX:
		{
			pBufPos += sizeof(unsigned short);
			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("行业个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);
			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "市场类别:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 2);
				pBufPos += 2;
				strRet += szTemp;

				strRet += "  行业代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 8);
				pBufPos += 8;
				strRet += szTemp;

				double value = 0;
				for (int i=1; i<=99; i++)
				{
					strRet += "  ";
					memcpy(&value, pBufPos, sizeof(double));
					pBufPos += sizeof(double);
					sprintf_s(szTemp, 100, "%f", value);
					strRet += szTemp;

					if (i % 10 == 0)
					{
						SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
						strRet.Empty();
					}
				}
			
				if (!strRet.IsEmpty())
					SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
			}
			break;
		}
	case ANALYSER_INFO:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);
			
			CString strRet;
			strRet.Format("记录个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				tagAnalyserInfo* pInfo = (tagAnalyserInfo*)pBufPos;
				pBufPos += sizeof(tagAnalyserInfo);

				strRet.Empty();
				strRet += "ANA_Account:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->ANA_Account, 20);
				strRet += szTemp;

				strRet += "  ANA_NM:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->ANA_NM, 20);
				strRet += szTemp;

				strRet += "  Company:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->Company, 40);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case INIT_INFO:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);
			
			CString strRet;
			strRet.Format("证券个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			int nSzCount = 0;
			int nShCount = 0;
			int nHkCount = 0;
			CString strStockCode;
			CString strMkt;
			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "证券代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				{
					strStockCode = szTemp;
					strMkt = strStockCode.Right(2);
					if (strMkt.CompareNoCase("sz") == 0)
						nSzCount++;
					else if (strMkt.CompareNoCase("sh") == 0)
						nShCount++;
					else if (strMkt.CompareNoCase("hk") == 0)
						nHkCount++;
				}
				pBufPos += 16;
				strRet += szTemp;

				strRet += "  简称:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				pBufPos += 16;
				strRet += szTemp;

				strRet += "  拼音:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 4);
				pBufPos += 4;
				strRet += szTemp;

				strRet += "  昨收盘:";
				unsigned int nValue = 0;
				memcpy(&nValue, pBufPos, sizeof(unsigned int));
				pBufPos += sizeof(unsigned int);
				sprintf_s(szTemp, 100, "%d", nValue);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}

			strRet.Format("证券个数:%d SZ:%d SH:%d HK:%d", nCount, nSzCount, nShCount, nHkCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);
			break;
		}
	case SELSTKMOD_INFO:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);
			
			CString strRet;
			strRet.Format("记录个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				tagSelStkModInfo* pInfo = (tagSelStkModInfo*)pBufPos;
				pBufPos += sizeof(tagSelStkModInfo);

				strRet.Empty();
				strRet += "MOD_CD:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->MOD_CD, 8);
				strRet += szTemp;

				strRet += "  MOD_NM:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->MOD_NM, 100);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case ANALYSER_REMARK:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("记录个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				tagAnalyserRemark* pInfo = (tagAnalyserRemark*)pBufPos;
				pBufPos += sizeof(tagAnalyserRemark);

				strRet.Empty();
				strRet += "ANA_Account:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->ANA_Account, 20);
				strRet += szTemp;

				strRet += "  SEC_CD:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->SEC_CD, 8);
				strRet += szTemp;

				strRet += "  IDU_CD:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->IDU_CD, 10);
				strRet += szTemp;

				strRet += "  UPDT_TM:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->UPDT_TM, 8);
				strRet += szTemp;
					
				strRet += "  CommentLen:";
				memcpy(szTemp, pBufPos, pInfo->CommentLen);
				strRet += szTemp;
				pBufPos += pInfo->CommentLen;
				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case ADD_REMARK:
		{
			pBufPos += sizeof(unsigned short);
			char szRet[2] = {0};
			memcpy(szRet, pBufPos, 1);
			int nRet = atoi(szRet);
	
			GetDlgItem(IDC_S_1)->SetWindowText(nRet == 1 ? "操作成功" : "操作失败");
			break;
		}
	case VERIFY_USERINFO:
		{
			pBufPos += sizeof(unsigned short);
			tagVerifyUserInfo info = {0};
			memcpy(&info, pBufPos, sizeof(tagVerifyUserInfo));
			pBufPos += sizeof(tagVerifyUserInfo);

			char szRet[2] = {0};
			memcpy(szRet, info.Result, 1);

			CString strRet;
			strRet.Format("UserID:%s UserName:%s szRet:%s Role%s", info.UserID, info.UserName,
				szRet, info.Role);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			break;
		}
	case MODIFY_PASSWORD:
		{
			pBufPos += sizeof(unsigned short);
			tagModifyPassword info = {0};
			memcpy(&info, pBufPos, sizeof(tagModifyPassword));
			pBufPos += sizeof(tagModifyPassword);

			char szRet[2] = {0};
			memcpy(szRet, info.Result, 1);
			
			CString strRet;
			strRet.Format("UserID:%s szRet:%s ", info.UserID, szRet);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);
			break;
		}
	case INDEX_HIS_DATA:
		{
			pBufPos += sizeof(tagComInfo);

			char szStockCode[8] = {0};
			memcpy(szStockCode, pBufPos, 8);
			pBufPos += 8;

			unsigned short nIdxMark = 0;
			memcpy(&nIdxMark, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			unsigned short nStockCount = 0;
			memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			char szMaketType[3] = {0};
			memcpy(szMaketType, comInfo.MarketType, 2);
			
			CString strRet;
			strRet.Format("市场类别:%s 代码:%s 记录个数:%d", szMaketType, szStockCode, nStockCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nStockCount; i++)
			{
				tagIdxHisData* pInfo = (tagIdxHisData*)pBufPos;
				pBufPos += sizeof(tagIdxHisData);

				strRet.Empty();
				strRet += "Date:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->Date, 8);
				strRet += szTemp;

				strRet += "  Value:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->Value, 20);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
			}
			break;
		}
	case QUOTE_BILL:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("证券个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "证券代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				pBufPos += 16;
				strRet += szTemp;

				strRet += "  证券名称:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				pBufPos += 16;
				strRet += szTemp;

				unsigned int nVaule = 0;
				for (int i=1; i<=42; i++)
				{
					strRet += "  ";
					memcpy(&nVaule, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nVaule);
					strRet += szTemp;

					if (i % 10 == 0)
					{
						SendMessage(WM_LISTRESULT, (WPARAM)&strRet);
						strRet.Empty();
					}
				}

				if (!strRet.IsEmpty())
					SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
			}
			break;
		}
	case REAL_MIN:
		{
		   pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("证券个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			unsigned short nMin = 0;
			unsigned int nValue = 0;
			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "证券代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				pBufPos += 16;
				strRet += szTemp;
								
				strRet += "  分钟数";
				memcpy(&nMin, pBufPos, sizeof(unsigned short));
				pBufPos += sizeof(unsigned short);
				sprintf_s(szTemp, 100, "%d", nMin);
				strRet += szTemp;
				
				SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
				strRet.Empty();

				for (int i=0; i<nMin; i++)
				{
					strRet += "  时间";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  开盘价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最高价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最低价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最新价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  成交量";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  均价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
					strRet.Empty();
				}
			}
			break;
		}
	case BARGAIN_BILL:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("证券个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			unsigned short ncjbs = 0;
			unsigned int nValue = 0;
			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "证券代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				pBufPos += 16;
				strRet += szTemp;

				strRet += "  成交笔数";
				memcpy(&ncjbs, pBufPos, sizeof(unsigned short));
				pBufPos += sizeof(unsigned short);
				sprintf_s(szTemp, 100, "%d", ncjbs);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
				strRet.Empty();

				for (int i=0; i<ncjbs; i++)
				{
					strRet += "  时间";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  成交价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  成交量";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  方向:";
					unsigned char way = 0;
					memcpy(&way, pBufPos, sizeof(unsigned char));
					pBufPos++;
					sprintf_s(szTemp, 100, "%d", way);
					strRet += szTemp;

					SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
					strRet.Empty();
				}
			}
			break;
		}
	case TWO_DAY_REAL_MIN:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("证券个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			unsigned long nDateA = 0;
			unsigned long nDateB = 0;
			unsigned int nValue = 0;
			unsigned short nMinCount = 0;
			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "证券代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				pBufPos += 16;
				strRet += szTemp;

				strRet += "  日期A:";
				memcpy(&nDateA, pBufPos, sizeof(unsigned long));
				pBufPos += sizeof(unsigned long);
				sprintf_s(szTemp, 100, "%d", nDateA);
				strRet += szTemp;

				strRet += "  分钟数量";
				memcpy(&nMinCount, pBufPos, sizeof(unsigned short));
				pBufPos += sizeof(unsigned short);
				sprintf_s(szTemp, 100, "%d", nMinCount);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
				strRet.Empty();

				for (int i=0; i<nMinCount; i++)
				{
					strRet += "  时间";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  开盘价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最高价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最低价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最新价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  成交量";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  均价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
					strRet.Empty();
				}

				strRet += "  日期B:";
				memcpy(&nDateB, pBufPos, sizeof(unsigned long));
				pBufPos += sizeof(unsigned long);
				sprintf_s(szTemp, 100, "%d", nDateB);
				strRet += szTemp;

				strRet += "  分钟数量";
				memcpy(&nMinCount, pBufPos, sizeof(unsigned short));
				pBufPos += sizeof(unsigned short);
				sprintf_s(szTemp, 100, "%d", nMinCount);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
				strRet.Empty();

				for (int i=0; i<nMinCount; i++)
				{
					strRet += "  时间";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  开盘价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最高价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最低价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最新价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  成交量";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  均价";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
					strRet.Empty();
				}
			}
			break;
		}
	case DAY_KLINE:
	case WEK_KLINE:
	case MON_KLINE:
		{
			pBufPos += sizeof(unsigned short);

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);

			CString strRet;
			strRet.Format("证券个数:%d", nCount);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);

			unsigned int nValue = 0;
			unsigned int nKLineCount = 0;
			unsigned short nHQMkt = 0;
			for(int i=0; i<nCount; i++)
			{
				strRet.Empty();
				strRet += "证券代码:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pBufPos, 16);
				pBufPos += 16;
				strRet += szTemp;

				strRet += "  还权标志";
				memcpy(&nHQMkt, pBufPos, sizeof(unsigned short));
				pBufPos += sizeof(unsigned short);
				sprintf_s(szTemp, 100, "%d", nHQMkt);
				strRet += szTemp;

				strRet += "  K线个数";
				memcpy(&nKLineCount, pBufPos, sizeof(unsigned int));
				pBufPos += sizeof(unsigned int);
				sprintf_s(szTemp, 100, "%d", nKLineCount);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
				strRet.Empty();

				for (int i=0; i<(int)nKLineCount; i++)
				{
					strRet += "  日期/周/月";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  开盘价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最高价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  最低价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  收盘价格";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  成交量";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;

					strRet += "  成交金额";
					memcpy(&nValue, pBufPos, sizeof(unsigned int));
					pBufPos += sizeof(unsigned int);
					sprintf_s(szTemp, 100, "%d", nValue);
					strRet += szTemp;
					
					SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
					strRet.Empty();
				}
			}
			break;
		}
	case SELSTKMOD_DATA:
		{
			pBufPos += sizeof(unsigned short);

			char szCode[8] = {0};
			memcpy(szCode, pBufPos, 8);
			pBufPos += 8;

			unsigned short nCount = 0;
			memcpy(&nCount, pBufPos, sizeof(unsigned short));
			pBufPos += sizeof(unsigned short);
			
			CString strRet;
			strRet.Format("记录个数:%d 代码:%s", nCount, szCode);
			GetDlgItem(IDC_S_1)->SetWindowText(strRet);
			
			for(int i=0; i<nCount; i++)
			{
				tagSelStkModData* pInfo = (tagSelStkModData*)pBufPos;
				pBufPos += sizeof(tagSelStkModData);

				strRet.Empty();
				strRet += "SEC_CD:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->SEC_CD, 8);
				strRet += szTemp;

				strRet += "  MKT_CL:";
				memset(szTemp, 0, sizeof(szTemp));
				memcpy(szTemp, pInfo->MKT_CL, 2);
				strRet += szTemp;

				SendMessage(WM_LISTRESULT, (WPARAM)&strRet); 
				strRet.Empty();
			}
			break;
		}
	}
}

void CTestTianMaDlg::OnTimeOut(int nPackType)
{  	
}

void CTestTianMaDlg::StopTest()
{
	m_cltCon.DisConnect();
	/*m_pressTest->Disconnect();
	m_pressTest->StopThread();*/
}

void CTestTianMaDlg::OnDestroy()
{
	CDialog::OnDestroy();

	
	delete m_pressTest;
	
	::DeleteCriticalSection(&m_csListRet);
}

int CTestTianMaDlg::ResetComboItem()
{
	int nRet;
	while(m_combo.GetCount() > 0)
		m_combo.DeleteString(0);
	if (m_cbPlus.GetCheck())
	{
		nRet = m_combo.AddString("证券信息");
		m_combo.SetItemData(nRet, STOCK_INFO);

		nRet = m_combo.AddString("行业分类");
		m_combo.SetItemData(nRet, INDUSTRY_INFO);

		nRet = m_combo.AddString("指数分类");
		m_combo.SetItemData(nRet, INDEX_INFO);

		nRet = m_combo.AddString("盈利预测汇总");
		m_combo.SetItemData(nRet, PAYOFF_COLLECT);

		nRet = m_combo.AddString("盈利预测明细");
		m_combo.SetItemData(nRet, PAYOFF_BILL);

		nRet = m_combo.AddString("投资评级明细");
		m_combo.SetItemData(nRet, COMMENTARY_BILL);

		nRet = m_combo.AddString("股票行情类指标");
		m_combo.SetItemData(nRet, HQ_INDEX);

		nRet = m_combo.AddString("指数类指标");
		m_combo.SetItemData(nRet, IDX_INDEX);

		nRet = m_combo.AddString("行业类指标");
		m_combo.SetItemData(nRet, IDU_INDEX);

		nRet = m_combo.AddString("科目表");
		m_combo.SetItemData(nRet, SUBTAB_INFO);

		nRet = m_combo.AddString("评级汇总");
		m_combo.SetItemData(nRet, COMMENTARY_COLLECT);

		nRet = m_combo.AddString("验证用户信息");
		m_combo.SetItemData(nRet, VERIFY_USERINFO);

		nRet = m_combo.AddString("修改密码");
		m_combo.SetItemData(nRet, MODIFY_PASSWORD);

		nRet = m_combo.AddString("分析师信息");
		m_combo.SetItemData(nRet, ANALYSER_INFO);

		nRet = m_combo.AddString("分析师评论");
		m_combo.SetItemData(nRet, ANALYSER_REMARK);

		nRet = m_combo.AddString("增加分析师评论");
		m_combo.SetItemData(nRet, ADD_REMARK);

		nRet = m_combo.AddString("历史财务类指标");
		m_combo.SetItemData(nRet, HIS_FINANCE_IDX);

		nRet = m_combo.AddString("指标历史数据");
		m_combo.SetItemData(nRet, INDEX_HIS_DATA);

		nRet = m_combo.AddString("行业表现数据");
		m_combo.SetItemData(nRet, IDU_REPRE_DATA);

		nRet = m_combo.AddString("行业按给定因素排名数据");
		m_combo.SetItemData(nRet, IDU_SORT_DATA);

		nRet = m_combo.AddString("选股模型信息");
		m_combo.SetItemData(nRet, SELSTKMOD_INFO);

		nRet = m_combo.AddString("选股模型数据");
		m_combo.SetItemData(nRet, SELSTKMOD_DATA);

		nRet = m_combo.AddString("市场因素指标");
		m_combo.SetItemData(nRet, MKT_FACTOR_IDX);

		nRet = m_combo.AddString("市场混合指标");
		m_combo.SetItemData(nRet, MKT_MIX_IDX);

	}
	/////////////
	if (m_cbBase.GetCheck())
	{
		nRet = m_combo.AddString("初始化");
		m_combo.SetItemData(nRet, INIT_INFO);

		nRet = m_combo.AddString("成交明细");
		m_combo.SetItemData(nRet, BARGAIN_BILL);

		nRet = m_combo.AddString("报价表");
		m_combo.SetItemData(nRet, QUOTE_BILL);

		nRet = m_combo.AddString("分时走势");
		m_combo.SetItemData(nRet, REAL_MIN);

		nRet = m_combo.AddString("日线");
		m_combo.SetItemData(nRet, DAY_KLINE);

		nRet = m_combo.AddString("周线");
		m_combo.SetItemData(nRet, WEK_KLINE);

		nRet = m_combo.AddString("月线");
		m_combo.SetItemData(nRet, MON_KLINE);

		nRet = m_combo.AddString("两日分时走势");
		m_combo.SetItemData(nRet, TWO_DAY_REAL_MIN);
	}

	//业务插件2（排行榜）
	if (m_cbPlus2.GetCheck())
	{
		nRet = m_combo.AddString("排行榜数据");
		m_combo.SetItemData(nRet, SORT_INFO);

		nRet = m_combo.AddString("取指数的上涨，下跌和平盘家数");
		m_combo.SetItemData(nRet, IDX_NUM);
	}

	//乱包数据测试
	/*if (m_cbInvalidData.GetCheck())
	{
	nRet = m_combo.AddString("乱包数据测试");
	m_combo.SetItemData(nRet, INVALID_DATA);
	}*/

	if (m_combo.GetCount() > 0)
		m_combo.SetCurSel(0);
	OnCbnSelchangeCombo();
	return m_combo.GetCount();
}
void CTestTianMaDlg::OnBnClickedCheckBase()
{
	// TODO: 在此添加控件通知处理程序代码
	ResetComboItem();
}

void CTestTianMaDlg::OnBnClickedCheckPlus()
{
	// TODO: 在此添加控件通知处理程序代码
	ResetComboItem();
}

void CTestTianMaDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialog::OnTimer(nIDEvent);
}

void CTestTianMaDlg::OnBnClickedCheckPlus2()
{
	// TODO: 在此添加控件通知处理程序代码
	ResetComboItem();
}

void CTestTianMaDlg::OnCbnSelchangeCombo()
{
	m_lstStock.ResetContent();
	HideAllCtrl();

	int nIndex = m_combo.GetCurSel();
	m_nPackType = (int)m_combo.GetItemData(nIndex);

	switch(m_nPackType)
	{
	case STOCK_INFO:
	case INDUSTRY_INFO:	//行业分类
	case INDEX_INFO:
	case IDU_REPRE_DATA:
	case MKT_FACTOR_IDX:
	case MKT_MIX_IDX:
		{
			break;
		}
	case PAYOFF_COLLECT:
	case PAYOFF_BILL:
	case COMMENTARY_BILL:
	case COMMENTARY_COLLECT:
	case HIS_FINANCE_IDX:
		{
			GetDlgItem(IDC_S_2)->SetWindowText("股票代码:");
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			break;
		}
	case SUBTAB_INFO:
		{
			GetDlgItem(IDC_S_2)->SetWindowText("科目表编码:");
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			break;
		}
	case IDU_SORT_DATA:
		{
			GetDlgItem(IDC_S_2)->SetWindowText("因素标识:");
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			break;
		}
	case HQ_INDEX:
	case IDX_INDEX:
	case IDU_INDEX:
		{
			m_lstStock.ResetContent();
			if (m_nPackType == IDU_INDEX)
				GetDlgItem(IDC_S_2)->SetWindowText("行业代码:");
			else if (m_nPackType == IDX_INDEX)
				GetDlgItem(IDC_S_2)->SetWindowText("指数代码:");
			else
				GetDlgItem(IDC_S_2)->SetWindowText("股票代码:");

			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_ADDSTART)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_L_STOCK)->ShowWindow(SW_SHOW);
			break;
		}
	case ANALYSER_INFO:
	case INIT_INFO:
	case SELSTKMOD_INFO:
		{
			GetDlgItem(IDC_R_CN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_R_HK)->ShowWindow(SW_HIDE);
			break;
		}
	case ANALYSER_REMARK:
	case ADD_REMARK:
		{
			GetDlgItem(IDC_R_CN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_R_HK)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_S_2)->SetWindowText("分析师帐号:");
			GetDlgItem(IDC_S_3)->SetWindowText("股票代码:");
			GetDlgItem(IDC_S_4)->SetWindowText("行业代码:");

			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_S_3)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_3)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_S_4)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_4)->ShowWindow(SW_SHOW);

			if (m_nPackType == ADD_REMARK)
			{
				GetDlgItem(IDC_E_5)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_S_5)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_S_5)->SetWindowText("评论内容:");
			}
			break;
		}
	case VERIFY_USERINFO:
	case MODIFY_PASSWORD:
		{
			GetDlgItem(IDC_R_CN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_R_HK)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_S_2)->SetWindowText("用户编码:");
			GetDlgItem(IDC_S_3)->SetWindowText("用户密码:");
			if (m_nPackType == MODIFY_PASSWORD)
			{
				GetDlgItem(IDC_S_3)->SetWindowText("用户旧密码:");
				GetDlgItem(IDC_S_4)->SetWindowText("用户新密码:");

				GetDlgItem(IDC_S_4)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_E_4)->ShowWindow(SW_SHOW);
			}
			
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_S_3)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_3)->ShowWindow(SW_SHOW);
			break;
		}			
	case INDEX_HIS_DATA:
		{
			GetDlgItem(IDC_S_2)->SetWindowText("代码:");
			GetDlgItem(IDC_S_3)->SetWindowText("指标标识:");
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_S_3)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_3)->ShowWindow(SW_SHOW);
			
			break;
		}
	case QUOTE_BILL:
	case REAL_MIN:
	case BARGAIN_BILL:
	case DAY_KLINE:
	case WEK_KLINE:
	case MON_KLINE:
	case TWO_DAY_REAL_MIN:
		{
			GetDlgItem(IDC_S_2)->SetWindowText("股票代码:");
			GetDlgItem(IDC_R_CN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_R_HK)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_ADDSTART)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_L_STOCK)->ShowWindow(SW_SHOW);

			if (m_nPackType == BARGAIN_BILL)
			{
				GetDlgItem(IDC_S_3)->SetWindowText("开始时间:");
				GetDlgItem(IDC_S_3)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_E_3)->ShowWindow(SW_SHOW);
			}
			else if (m_nPackType == TWO_DAY_REAL_MIN)
			{
				GetDlgItem(IDC_S_3)->SetWindowText("日期A:");
				GetDlgItem(IDC_S_4)->SetWindowText("日期B:");
				GetDlgItem(IDC_S_3)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_E_3)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_S_4)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_E_4)->ShowWindow(SW_SHOW);
			}
			else if (m_nPackType == DAY_KLINE ||
				m_nPackType == WEK_KLINE ||
				m_nPackType == MON_KLINE )
			{
				GetDlgItem(IDC_S_3)->SetWindowText("起始日期:");
				GetDlgItem(IDC_S_4)->SetWindowText("终止日期:");
				GetDlgItem(IDC_S_3)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_E_3)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_S_4)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_E_4)->ShowWindow(SW_SHOW);
			}
			break;
		}
	case SELSTKMOD_DATA:
		{
			GetDlgItem(IDC_R_CN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_R_HK)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_S_2)->SetWindowText("模型代码:");
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			break;
		}
	case SORT_INFO:
		{
			GetDlgItem(IDC_R_CN)->SetWindowText("正序");
			GetDlgItem(IDC_R_HK)->SetWindowText("逆序");

			GetDlgItem(IDC_S_2)->SetWindowText("symbol代码:");
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);

			GetDlgItem(IDC_S_3)->SetWindowText("排序起始行:");
			GetDlgItem(IDC_S_3)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_3)->ShowWindow(SW_SHOW);

			GetDlgItem(IDC_S_4)->SetWindowText("返回条数:");
			GetDlgItem(IDC_S_4)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_4)->ShowWindow(SW_SHOW);

			GetDlgItem(IDC_S_5)->SetWindowText("返回的列:");
			GetDlgItem(IDC_S_5)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_5)->ShowWindow(SW_SHOW);
			break;
		}
	case IDX_NUM:
		{
			GetDlgItem(IDC_R_CN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_R_HK)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_S_2)->SetWindowText("指数代码:");
			GetDlgItem(IDC_S_2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_E_2)->ShowWindow(SW_SHOW);
			break;
		}
	default:
		{
			//MessageBox("数据包没有此类型");
		}
	}
}

void CTestTianMaDlg::HideAllCtrl()
{
	GetDlgItem(IDC_S_2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_E_2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_ADDSTART)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_L_STOCK)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_R_CN)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_R_HK)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_S_3)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_E_3)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_S_4)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_E_4)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_S_5)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_E_5)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_R_CN)->SetWindowText("大陆");
	GetDlgItem(IDC_R_HK)->SetWindowText("港股");
}

LRESULT CTestTianMaDlg::OnListResult(WPARAM wParam, LPARAM lParam)
{
	::EnterCriticalSection(&m_csListRet);
	int nIndex = m_lstBox.GetCount();
	m_lstBox.InsertString(nIndex, *((CString*)wParam));
	::LeaveCriticalSection(&m_csListRet);
	return 0;
}
void CTestTianMaDlg::OnBnClickedBtnAddstart()
{
	UpdateData();
	CString strValue;
	GetDlgItem(IDC_E_2)->GetWindowText(strValue);
	if (strValue.IsEmpty())
		return;

	CString strText;
	if (m_nPackType == QUOTE_BILL || m_nPackType == REAL_MIN)
	{
		strText = strValue;
	}
	else if (m_nPackType == BARGAIN_BILL)
	{
		strText = strValue;
		strText += "-";
		GetDlgItem(IDC_E_3)->GetWindowText(strValue);
		strText += strValue;
	}
	else if (m_nPackType == TWO_DAY_REAL_MIN || 
		     m_nPackType == DAY_KLINE ||
		     m_nPackType == WEK_KLINE ||
		     m_nPackType == MON_KLINE )
	{
		strText = strValue;
		strText += "-";
		GetDlgItem(IDC_E_3)->GetWindowText(strValue);
		strText += strValue;
		strText += "-";
		GetDlgItem(IDC_E_4)->GetWindowText(strValue);
		strText += strValue;
	}
	else
	{	
		strText += (m_bHK ? "HK":"CN");
		strText += "-";
		strText += strValue;
	}

	m_lstStock.AddString(strText);
}
	