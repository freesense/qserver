// Dlg_StockInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_StockInfo.h"
#include "config.h"
#include "FilePath.h"

#define  CODE_MARKET_BUFLEN  15
#define  SZ_INDEX_CODE_MIN   399000	  //深圳交易所 指数范围的最小值
#define  SH_INDEX_CODE_MAX	 2000	  //上海交易所 指数范围的最大值
// CDlg_StockInfo 对话框

IMPLEMENT_DYNAMIC(CDlg_StockInfo, CDialog)

CDlg_StockInfo::CDlg_StockInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_StockInfo::IDD, pParent)
{
	m_pRecvBuf     = NULL;
	m_nRecvBufSize = 0;

	m_chSendBuf[0] = 0;
	m_nPackSize    = 0;
	m_nNetQuery    = 0;
	m_pdataFarm    = NULL;
}

CDlg_StockInfo::~CDlg_StockInfo()
{
	if (m_pRecvBuf != NULL)
	{
		delete[]m_pRecvBuf;
		m_pRecvBuf = NULL;
	}
}

void CDlg_StockInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DATATYPE, m_combDataType);
	DDX_Control(pDX, IDC_EDIT_STOCKCODE, m_editStockCode);
	DDX_Control(pDX, IDC_LIST_INFO, m_listInfo);
	DDX_Radio(pDX, IDC_R_NETQUERY, m_nNetQuery);
}


BEGIN_MESSAGE_MAP(CDlg_StockInfo, CDialog)
//	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlg_StockInfo::OnCbnSelchangeCombo1)
ON_BN_CLICKED(IDC_BTN_QUERY, &CDlg_StockInfo::OnBnClickedBtnQuery)
ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlg_StockInfo 消息处理程序


BOOL CDlg_StockInfo::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_editStockCode.SetFocus();

	m_combDataType.AddString("行情");
	m_combDataType.AddString("分时走势");
	m_combDataType.AddString("分笔成交");
	m_combDataType.SetCurSel(0);

	m_commxHead.Prop     = CommxHead::MakeProp(VERSION, CommxHead::ANSI, CommxHead::HOSTORDER);
	m_commxHead.SerialNo = 0;
	m_commxHead.Feed     = ROUTETARGET;

	m_listInfo.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	CRect rcClient;
	GetClientRect(rcClient);
	rcClient.DeflateRect(10,100,10,30);
	m_listInfo.MoveWindow(rcClient);
	m_listInfo.ShowWindow(FALSE);

	ShowStaticCtrl(FALSE);
	
	GetDlgItem(IDC_IPSTOCKSERVER)->SetWindowText(theApp.m_strSerIP);
	CString strPort;
	strPort.Format("%d", theApp.m_nPort);
	GetDlgItem(IDC_EDIT_PORT)->SetWindowText(strPort);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDlg_StockInfo::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_RETURN && pMsg->message != WM_KEYUP)
	{			
		OnBnClickedBtnQuery();	
	}

	if (pMsg->wParam == VK_RETURN)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CDlg_StockInfo::FillPack()
{
	UpdateData();
	char chCodeMarket[CODE_MARKET_BUFLEN] = {0};
	//unsigned char byDataType = 0; //dj 2008-12-6
	unsigned short nDataType = 0;

	CString strCodeMarket;
	m_editStockCode.GetWindowText(strCodeMarket);
	CString strMarket = strCodeMarket.Right(strCodeMarket.GetLength() - 
		strCodeMarket.ReverseFind('.') - 1);
	CString strCode =  strCodeMarket.Left(strCodeMarket.ReverseFind('.'));

	strcpy_s(chCodeMarket, CODE_MARKET_BUFLEN, strCode);
	strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, ".");
	
	{
		int nStockCode = atoi(strCode);
		if (strMarket.CompareNoCase("sz") == 0) //深圳 
		{
			if (nStockCode >= SZ_INDEX_CODE_MIN)
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "SZ");
			else
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "sz");
		}
		else if (strMarket.CompareNoCase("sh") == 0) //上海
		{
			if (nStockCode < SH_INDEX_CODE_MAX)
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "SH");
			else
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "sh");
		}
		else if (strMarket.CompareNoCase("hk") == 0) //港股
		{
			if (IsDigital(strCode))
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "hk");
			else
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "HK");
		}
		else
		{
			MessageBox("股票代码的格式不正确, 格式: xxxxxx.xx 如：000001.sz");
			return FALSE;
		}
	}

	{
		int nCurSel = m_combDataType.GetCurSel();
		if (nCurSel == 0)    //0 行情 
		{
			//byDataType = DT_HQ;   //dj 2008-12-6
			nDataType  = DT_HQ;
		}
		else if(nCurSel == 1) //1 K线
		{
			//byDataType = DT_KLINE;  //dj 2008-12-6
			nDataType  = DT_KLINE;
		}
		else if(nCurSel == 2) //2 分笔成交
		{
			//byDataType = DT_FBCJ;	  //dj 2008-12-6
			nDataType  = DT_FBCJ;
		}
	}

	//m_nPackSize = CODE_MARKET_BUFLEN + 1;  //1字节的数据类型  //dj 2008-12-6
	m_nPackSize = sizeof(unsigned short) + CODE_MARKET_BUFLEN;
	m_commxHead.Length = m_nPackSize;
	m_commxHead.SerialNo = 0xDEAD;

	//格式：CommxHead + 功能号 + 15字节stock code . Market
	//Fill pack header
	char* pSendBufPos = m_chSendBuf;
	memcpy(pSendBufPos, &m_commxHead, sizeof(CommxHead));
	pSendBufPos += sizeof(CommxHead);

	//功能号  //add dj 2008-12-6
	nDataType += 65000;
	//nDataType = 65535;
	memcpy(pSendBufPos, &nDataType, sizeof(unsigned short));
	pSendBufPos += sizeof(unsigned short);

	//Fill stock code . Market
	memcpy(pSendBufPos, chCodeMarket, CODE_MARKET_BUFLEN);
	pSendBufPos += CODE_MARKET_BUFLEN;
	//Fill Data type
	//memcpy(pSendBufPos, &byDataType, 1);	 //dj 2008-12-6

	return TRUE;
}

void CDlg_StockInfo::OnBnClickedBtnQuery()
{
	GetDlgItem(IDC_IPSTOCKSERVER)->GetWindowText(theApp.m_strSerIP);
	CString strPort;
	GetDlgItem(IDC_EDIT_PORT)->GetWindowText(strPort);
	theApp.m_nPort = atoi(strPort);

	UpdateData();
	if (m_nNetQuery == 1) //本地查询
	{
		LocalQuery();
		return;
	}
	//网络查询
	m_chSendBuf[0] = 0;
	m_nPackSize    = 0;

	if (!FillPack())
		return;
	if (!m_downData.Connect(theApp.m_strSerIP.GetBuffer(), theApp.m_nPort))
	{
		theApp.m_strSerIP.ReleaseBuffer();
		MessageBox("查询失败, 请检测服务器IP和端口!");
		return;
	}
	theApp.m_strSerIP.ReleaseBuffer();
	if (!m_downData.SendPacket(m_chSendBuf, m_nPackSize+sizeof(CommxHead)))
	{
		m_downData.DisConnect();
		MessageBox("查询失败!");
		return;
	}
	
	while(TRUE)	//过滤掉不需要的包
	{
		CommxHead  commxHead = {0};
		int nRecv = 0;
		nRecv = m_downData.RecvPacket((char*)&commxHead, sizeof(CommxHead));
		if (nRecv != sizeof(CommxHead))
		{
			m_downData.DisConnect();
			MessageBox("接收数据失败!");
			return;
		}

		if (commxHead.Length == 0)
		{
			m_downData.DisConnect();
			MessageBox("您查找的数据不存在!");
			return;
		}

		if (m_nRecvBufSize < (int)commxHead.Length)
		{
			if (m_pRecvBuf != NULL)
			{
				delete [] m_pRecvBuf;
				m_pRecvBuf = NULL;
			}

			m_pRecvBuf     = new char[commxHead.Length];
			m_nRecvBufSize = commxHead.Length;
		}

		int nRequestSize  = commxHead.Length;
		char* pRecvBufPos = m_pRecvBuf;
		while(nRequestSize > 0)
		{
			nRecv = m_downData.RecvPacket(pRecvBufPos , nRequestSize);
			if (nRecv <= 0)
				break;

			pRecvBufPos  += nRecv;
			nRequestSize -= nRecv;
		}

		if (commxHead.SerialNo != 0xDEAD)
			continue;
		int nCurSel = m_combDataType.GetCurSel();
		if (nCurSel == 0)    //0 行情 
		{
			ParseHQData(commxHead.Length);
		}
		else if(nCurSel == 1) //1 K线
		{
			ParseKLineData(commxHead.Length);
		}
		else if(nCurSel == 2) //2 分笔成交
		{
			ParseFbcjData(commxHead.Length);
		}
		if (commxHead.SerialNo == 0xDEAD)
			break;
	}
	
	m_downData.DisConnect();
}

void CDlg_StockInfo::DeleteAllColumn()
{
	int nColumnCount = m_listInfo.GetHeaderCtrl()->GetItemCount();
	for (int i=0; i < nColumnCount; i++)
	{
		m_listInfo.DeleteColumn(0);
	}
}

void CDlg_StockInfo::ParseKLineData(int nDataLen)
{
	ShowStaticCtrl(FALSE);
	m_listInfo.ShowWindow(TRUE);
	m_listInfo.DeleteAllItems();
	DeleteAllColumn();
	int nColumnIndex = 0;
	m_listInfo.InsertColumn(nColumnIndex++, "编号", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Time", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "OpenPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "MaxPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "MinPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "NewPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Volume", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "均价", LVCFMT_RIGHT, 60);

	MinUnit minUnit;
	int nPackCount = nDataLen / sizeof(MinUnit);
	char* pRecvBufPos = m_pRecvBuf;
	for (int i=0; i<nPackCount; i++)
	{
		memcpy(&minUnit, pRecvBufPos, sizeof(MinUnit));

		CString strTemp;
		strTemp.Format("%d", i+1);
		m_listInfo.InsertItem(i, strTemp);

		strTemp.Format("%d", minUnit.Time);
		m_listInfo.SetItemText(i, 1, strTemp);

		strTemp.Format("%d", minUnit.OpenPrice);
		m_listInfo.SetItemText(i, 2, strTemp);

		strTemp.Format("%d", minUnit.MaxPrice);
		m_listInfo.SetItemText(i, 3, strTemp);

		strTemp.Format("%d", minUnit.MinPrice);
		m_listInfo.SetItemText(i, 4, strTemp);

		strTemp.Format("%d", minUnit.NewPrice);
		m_listInfo.SetItemText(i, 5, strTemp);

		strTemp.Format("%u", minUnit.Volume);
		m_listInfo.SetItemText(i, 6, strTemp);

		strTemp.Format("%d", minUnit.AvgPrice);
		m_listInfo.SetItemText(i, 7, strTemp);

		pRecvBufPos += sizeof(MinUnit);
	}
}
void CDlg_StockInfo::ParseKLineData(MinBlock *pMinBlock, int nCount)
{
	ShowStaticCtrl(FALSE);
	m_listInfo.ShowWindow(TRUE);
	m_listInfo.DeleteAllItems();
	DeleteAllColumn();
	int nColumnIndex = 0;
	m_listInfo.InsertColumn(nColumnIndex++, "编号", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Time", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "OpenPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "MaxPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "MinPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "NewPrice", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Volume", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "均价", LVCFMT_RIGHT, 60);

	int nIndex = 0;
	for (int i=0; i<nCount; i++)
	{
		//DW("COUNT:%d", nIndex);
		int nItemIndex = m_listInfo.GetItemCount();
		CString strTemp;

		strTemp.Format("%d", i+1);
		m_listInfo.InsertItem(i, strTemp);

		strTemp.Format("%d", pMinBlock->unit[nIndex].Time);
		m_listInfo.SetItemText(nItemIndex, 1, strTemp);

		strTemp.Format("%d", pMinBlock->unit[nIndex].OpenPrice);
		m_listInfo.SetItemText(nItemIndex, 2, strTemp);

		strTemp.Format("%d", pMinBlock->unit[nIndex].MaxPrice);
		m_listInfo.SetItemText(nItemIndex, 3, strTemp);

		strTemp.Format("%d", pMinBlock->unit[nIndex].MinPrice);
		m_listInfo.SetItemText(nItemIndex, 4, strTemp);

		strTemp.Format("%d", pMinBlock->unit[nIndex].NewPrice);
		m_listInfo.SetItemText(nItemIndex, 5, strTemp);

		strTemp.Format("%u", pMinBlock->unit[nIndex].Volume);
		m_listInfo.SetItemText(nItemIndex, 6, strTemp);

		strTemp.Format("%d", pMinBlock->unit[nIndex].AvgPrice);
		m_listInfo.SetItemText(nItemIndex, 7, strTemp);
		nIndex++;

		if (nIndex == MINK_PERBLOCK)
		{
			nIndex = 0;
			int nNextIndex = pMinBlock->next;
			if (nNextIndex == 0)
				break;
			pMinBlock = m_pdataFarm->GetMinK(nNextIndex); 
		}
	}
}

void CDlg_StockInfo::ParseFbcjData(int nDataLen)
{
	ShowStaticCtrl(FALSE);
	m_listInfo.ShowWindow(TRUE);
	m_listInfo.DeleteAllItems();
	DeleteAllColumn();
	int nColumnIndex = 0;
	m_listInfo.InsertColumn(nColumnIndex++, "编号", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Time", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Price", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Volume", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Way", LVCFMT_RIGHT, 60);
	
	TickUnit tickUnit;
	int nPackCount = nDataLen / sizeof(TickUnit);
	char* pRecvBufPos = m_pRecvBuf;
	for (int i=0; i<nPackCount; i++)
	{
		memcpy(&tickUnit, pRecvBufPos, sizeof(TickUnit));

		CString strTemp;
		strTemp.Format("%d", i+1);
		m_listInfo.InsertItem(i, strTemp);

		strTemp.Format("%d", tickUnit.Time);
		m_listInfo.SetItemText(i, 1, strTemp);

		strTemp.Format("%d", tickUnit.Price);
		m_listInfo.SetItemText(i, 2, strTemp);

		strTemp.Format("%u", tickUnit.Volume);
		m_listInfo.SetItemText(i, 3, strTemp);

		strTemp.Format("%d", tickUnit.Way);
		m_listInfo.SetItemText(i, 4, strTemp);

		pRecvBufPos += sizeof(TickUnit);
	}
}

void CDlg_StockInfo::ParseFbcjData(TickBlock *pTickBlock, int nCount)
{
	ShowStaticCtrl(FALSE);
	m_listInfo.ShowWindow(TRUE);
	m_listInfo.DeleteAllItems();
	DeleteAllColumn();
	int nColumnIndex = 0;
	m_listInfo.InsertColumn(nColumnIndex++, "编号", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Time", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Price", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Volume", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "Way", LVCFMT_RIGHT, 60);
	
	int nIndex = 0;
	for (int i=0; i<nCount; i++)
	{
		//DW("COUNT:%d", nIndex);
		int nItemIndex = m_listInfo.GetItemCount();
		
		CString strTemp;
		strTemp.Format("%d", i+1);
		m_listInfo.InsertItem(i, strTemp);

		strTemp.Format("%d", pTickBlock->unit[nIndex].Time);
		m_listInfo.SetItemText(nItemIndex, 1, strTemp);

		strTemp.Format("%d", pTickBlock->unit[nIndex].Price);
		m_listInfo.SetItemText(nItemIndex, 2, strTemp);

		strTemp.Format("%u", pTickBlock->unit[nIndex].Volume);
		m_listInfo.SetItemText(nItemIndex, 3, strTemp);

		strTemp.Format("%d", pTickBlock->unit[nIndex].Way);
		m_listInfo.SetItemText(nItemIndex, 4, strTemp);
		nIndex++;

		if (nIndex == TICK_PERBLOCK)
		{
			nIndex = 0;
			int nNextIndex = pTickBlock->next;
			if (nNextIndex == 0)
				break;
			pTickBlock = m_pdataFarm->GetTick(nNextIndex); 
		}
	}
}

void CDlg_StockInfo::ParseHQData(int nDataLen, Quote *pQuote)
{
	m_listInfo.ShowWindow(FALSE);
	if (pQuote == NULL)
	{
		if (nDataLen != sizeof(Quote))
		{
			MessageBox("数据的大小不正确，查询失败!");
			return;
		}
	}

	ShowStaticCtrl(TRUE);
	Quote hqInfo;
	Quote* pCurInfo = NULL;
	if (pQuote == NULL)
	{
		memcpy(&hqInfo, m_pRecvBuf, sizeof(Quote));
		pCurInfo = &hqInfo;
	}
	else
	{
		pCurInfo = pQuote;
	}
	
	CString strTemp;
	GetDlgItem(IDC_S_DM_V)->SetWindowText(pCurInfo->szStockCode);
	char szStockName[20] = {0};
	memcpy(szStockName, pCurInfo->szStockName, STOCK_NAME_LEN);
	GetDlgItem(IDC_S_MC_V)->SetWindowText(szStockName);

	strTemp.Format("%d", pCurInfo->zrsp);
	GetDlgItem(IDC_S_ZRSP_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->jrkp);
	GetDlgItem(IDC_S_JRKP_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->ztjg);
	GetDlgItem(IDC_S_ZTJG_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->dtjg);
	GetDlgItem(IDC_S_DTJG_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->syl1);
	GetDlgItem(IDC_S_SYL1_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->syl2);
	GetDlgItem(IDC_S_SYL2_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->zgjg);
	GetDlgItem(IDC_S_ZGJG_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->zdjg);
	GetDlgItem(IDC_S_ZDJG_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->zjjg);
	GetDlgItem(IDC_S_ZJJG_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->zjcj);
	GetDlgItem(IDC_S_ZJCJ_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->cjsl);
	GetDlgItem(IDC_S_CJSL_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->cjje);
	GetDlgItem(IDC_S_CJJE_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->cjbs);
	GetDlgItem(IDC_S_CJBS_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BP1);
	GetDlgItem(IDC_S_BJW1_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BM1);
	GetDlgItem(IDC_S_BSL1_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BP2);
	GetDlgItem(IDC_S_BJW2_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BM2);
	GetDlgItem(IDC_S_BSL2_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BP3);
	GetDlgItem(IDC_S_BJW3_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BM3);
	GetDlgItem(IDC_S_BSL3_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BP4);
	GetDlgItem(IDC_S_BJW4_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BM4);
	GetDlgItem(IDC_S_BSL4_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BP5);
	GetDlgItem(IDC_S_BJW5_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->BM5);
	GetDlgItem(IDC_S_BSL5_V)->SetWindowText(strTemp);


	strTemp.Format("%d", pCurInfo->SP1);
	GetDlgItem(IDC_S_SJW1_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SM1);
	GetDlgItem(IDC_S_SSL1_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SP2);
	GetDlgItem(IDC_S_SJW2_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SM2);
	GetDlgItem(IDC_S_SSL2_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SP3);
	GetDlgItem(IDC_S_SJW3_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SM3);
	GetDlgItem(IDC_S_SSL3_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SP4);
	GetDlgItem(IDC_S_SJW4_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SM4);
	GetDlgItem(IDC_S_SSL4_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SP5);
	GetDlgItem(IDC_S_SJW5_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->SM5);
	GetDlgItem(IDC_S_SSL5_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->day5pjzs);
	GetDlgItem(IDC_S_5RJS_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->pjjg);
	GetDlgItem(IDC_S_JJ_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->wb);
	GetDlgItem(IDC_S_WB_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->lb);
	GetDlgItem(IDC_S_LB_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->np);
	GetDlgItem(IDC_S_NP_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->wp);
	GetDlgItem(IDC_S_WP_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->zdf);
	GetDlgItem(IDC_S_ZDF_V)->SetWindowText(strTemp);

	strTemp.Format("%d", pCurInfo->zf);
	GetDlgItem(IDC_S_ZF_V)->SetWindowText(strTemp);

	GetDlgItem(IDC_S_TYPE_V)->SetWindowText(pCurInfo->szStockKind);
	
}

void CDlg_StockInfo::ShowStaticCtrl(BOOL bShow)
{
	UINT nCtrlID[] = 
	{
		IDC_S_DM,IDC_S_DM_V,      IDC_S_MC,IDC_S_MC_V,       IDC_S_ZRSP,IDC_S_ZRSP_V,    IDC_S_JRKP,IDC_S_JRKP_V,
		IDC_S_ZTJG,IDC_S_ZTJG_V,  IDC_S_DTJG,IDC_S_DTJG_V,	 IDC_S_SYL1, IDC_S_SYL1_V,	 IDC_S_SYL2, IDC_S_SYL2_V,
		IDC_S_ZGJG,IDC_S_ZGJG_V,  IDC_S_ZDJG,IDC_S_ZDJG_V,   IDC_S_ZJJG,IDC_S_ZJJG_V,	 IDC_S_ZJCJ,IDC_S_ZJCJ_V,
		IDC_S_CJSL,IDC_S_CJSL_V,  IDC_S_CJJE,IDC_S_CJJE_V,   IDC_S_CJBS,IDC_S_CJBS_V,    IDC_S_BJW1,IDC_S_BJW1_V,
		IDC_S_BSL1,IDC_S_BSL1_V,  IDC_S_BJW2,IDC_S_BJW2_V,	 IDC_S_BSL2,IDC_S_BSL2_V,	 IDC_S_BJW3,IDC_S_BJW3_V,
		IDC_S_BSL3,IDC_S_BSL3_V,  IDC_S_BJW4,IDC_S_BJW4_V,	 IDC_S_BSL4,IDC_S_BSL4_V,	 IDC_S_BJW5,IDC_S_BJW5_V,
		IDC_S_BSL5,IDC_S_BSL5_V,  IDC_S_SJW1,IDC_S_SJW1_V,	 IDC_S_SSL1,IDC_S_SSL1_V,	 IDC_S_SJW2,IDC_S_SJW2_V,	 
		IDC_S_SSL2,IDC_S_SSL2_V,  IDC_S_SJW3,IDC_S_SJW3_V,	 IDC_S_SSL3,IDC_S_SSL3_V,	 IDC_S_SJW4,IDC_S_SJW4_V,	
		IDC_S_SSL4,IDC_S_SSL4_V,  IDC_S_SJW5,IDC_S_SJW5_V,	 IDC_S_SSL5,IDC_S_SSL5_V,	 IDC_S_JJ,IDC_S_JJ_V,
		IDC_S_WB,IDC_S_WB_V,      IDC_S_LB,IDC_S_LB_V,		 IDC_S_NP,IDC_S_NP_V,        IDC_S_WP,IDC_S_WP_V,
		IDC_S_5RJS,IDC_S_5RJS_V,  IDC_S_ZDF,IDC_S_ZDF_V,     IDC_S_ZF,IDC_S_ZF_V,        IDC_S_TYPE,IDC_S_TYPE_V 

	};
	
	for(int i=0; i<sizeof(nCtrlID)/sizeof(nCtrlID[0]); i++)
	{
		GetDlgItem(nCtrlID[i])->ShowWindow(bShow);
	}
}


long CDlg_StockInfo::cast(std::string *pStr, RINDEX *pRindex, bool bQuitIfNoKey)
{
	if (m_pdataFarm == NULL)
		return -1;

	int nCurSel = m_combDataType.GetCurSel();
	if (nCurSel == 0)    //0 行情 
	{
		Quote *pQuote = m_pdataFarm->GetQuote(pRindex->idxQuote);
		ParseHQData(sizeof(Quote), pQuote);
	}
	else if(nCurSel == 1) //1 K线
	{
		MinBlock *pMinBlock = m_pdataFarm->GetMinK(pRindex->idxMinK);
		if (pRindex->cntMinK > 0)
			ParseKLineData(pMinBlock, pRindex->cntMinK);
		else
		{
			MessageBox("没有您查询的股票");
			ZeroCtrl();
		}
	}
	else if(nCurSel == 2) //2 分笔成交
	{
		TickBlock *pTickBlock = m_pdataFarm->GetTick(pRindex->idxTick);
		if (pRindex->cntTick > 0)
			ParseFbcjData(pTickBlock, pRindex->cntTick);
		else
		{
			MessageBox("没有您查询的股票");
			ZeroCtrl();
		}
	}
	return 0;
}

void CDlg_StockInfo::LocalQuery()
{
	std::string strExecutePath = GetExecutePath();
	strExecutePath = strExecutePath.substr(0, strExecutePath.rfind('\\') + 1);

	if (!CFilePath::IsFileExist((strExecutePath + "QuoteFarm.xml").c_str()))
	{
		MessageBox("文件[QuoteFarm.xml]不存在，无法获得市场个数", "错误");
		return;
	}
	CConfig	cfg;
    cfg.Load((strExecutePath + "QuoteFarm.xml").c_str());

	UpdateData();				 
	char chCodeMarket[CODE_MARKET_BUFLEN] = {0};
	CString strCodeMarket;
	m_editStockCode.GetWindowText(strCodeMarket);
	CString strMarket = strCodeMarket.Right(strCodeMarket.GetLength() - 
		strCodeMarket.ReverseFind('.') - 1);
	CString strCode =  strCodeMarket.Left(strCodeMarket.ReverseFind('.'));

	strcpy_s(chCodeMarket, CODE_MARKET_BUFLEN, strCode);
	strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, ".");
	
	{
		int nStockCode = atoi(strCode);
		if (strMarket.CompareNoCase("sz") == 0) //深圳 
		{
			if (nStockCode >= SZ_INDEX_CODE_MIN)
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "SZ");
			else
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "sz");
		}
		else if (strMarket.CompareNoCase("sh") == 0) //上海
		{
			if (nStockCode < SH_INDEX_CODE_MAX)
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "SH");
			else
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "sh");
		}
		else if (strMarket.CompareNoCase("hk") == 0) //港股
		{
			if (IsDigital(strCode))
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "hk");
			else
				strcat_s(chCodeMarket, CODE_MARKET_BUFLEN, "HK");
		}
		else
		{
			MessageBox("股票代码的格式不正确, 格式: xxxxxx.xx 如：000001.sz");
			return;
		}
	}

	//读取index.dat文件头
	CFile file;
	CString strFile = theApp.m_strDatFilePath;
	strFile += "\\index.dat";
	if (!CValidFunction::IsFileExist(strFile))
	{
		MessageBox("index.dat文件不存在，请检测dat文件路径");
		return;
	}
	if (!file.Open(strFile, CFile::modeRead|CFile::shareDenyNone))
	{
		MessageBox("查询失败，打开index.dat文件失败");
		return;
	}

	m_hLockFile.SetFileHandle(file.m_hFile);
	BOOL bRet = m_hLockFile.ReadLockFile();
	if(!bRet)
	{
		file.Close();
		MessageBox("查询失败");
		return;
	}
	IndexHead indexHead = {0};
	UINT nReaded = file.Read(&indexHead, sizeof(IndexHead));
	bRet = m_hLockFile.UnlockFile();
	if(!bRet)
	{
		file.Close();
		MessageBox("查询失败");
		return;
	}
	if (nReaded != sizeof(IndexHead))
	{
		file.Close();
		MessageBox("查询失败");
		return;
	}
	file.Close();

	CDataFarm   dataFarm;
	m_pdataFarm = &dataFarm;
	if (!dataFarm.mapfile(strFile, cfg.m_nMaketCount, indexHead.nBucketCount, indexHead.nSymbolCount,
		indexHead.nTickCount, indexHead.nMinkCount))
	{
		MessageBox("查询失败");
		return;
	}

	if (dataFarm.m_hashIdx.cast(chCodeMarket, *this) == 0)
	{
		MessageBox("没有您查询的股票");
		ZeroCtrl();
	}
}

void CDlg_StockInfo::ZeroCtrl()
{
	UINT nCtrlID[] = 
	{
		IDC_S_DM_V,	 IDC_S_MC_V, IDC_S_ZRSP_V, IDC_S_JRKP_V, IDC_S_ZTJG_V,
		IDC_S_DTJG_V, IDC_S_SYL1_V, IDC_S_SYL2_V, IDC_S_ZGJG_V, IDC_S_ZDJG_V,
		IDC_S_ZJJG_V, IDC_S_CJSL_V, IDC_S_CJJE_V, IDC_S_CJBS_V, IDC_S_BJW1_V,
		IDC_S_BSL1_V, IDC_S_BJW2_V, IDC_S_BSL2_V,  IDC_S_BJW3_V, IDC_S_BSL3_V,
		IDC_S_BJW4_V, IDC_S_BSL4_V, IDC_S_BJW5_V, IDC_S_BSL5_V,	 IDC_S_SJW1_V,
		IDC_S_SSL1_V,  IDC_S_SJW2_V, IDC_S_SSL2_V, IDC_S_SJW3_V, IDC_S_SSL3_V,
		IDC_S_SJW4_V, IDC_S_SSL4_V,	IDC_S_SJW5_V, IDC_S_SSL5_V, IDC_S_5RJS_V,
		IDC_S_JJ_V, IDC_S_WB_V, IDC_S_LB_V, IDC_S_NP_V, IDC_S_WP_V, IDC_S_ZDF_V
	};
	
	for(int i=0; i<sizeof(nCtrlID)/sizeof(nCtrlID[0]); i++)
	{
		GetDlgItem(nCtrlID[i])->SetWindowText("");
	}

	m_listInfo.DeleteAllItems();
	DeleteAllColumn();

	ShowStaticCtrl(FALSE);
	m_listInfo.ShowWindow(FALSE);
}

BOOL CDlg_StockInfo::IsDigital(CString strCode)
{
	char szCode[50];
	strcpy_s(szCode, 50, strCode);
	for(unsigned int i=0; i<strlen(szCode); i++)
	{
		if(szCode[i] >= '0' && szCode[i] <= '9')
			continue;
		else
			return FALSE;
	}

	return TRUE;
}
void CDlg_StockInfo::OnDestroy()
{
	CDialog::OnDestroy();

	theApp.SaveServerInfo(); //保存IP地址和端口到配置文件
}
