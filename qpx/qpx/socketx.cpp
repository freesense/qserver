
#include "stdafx.h"
#include "socketx.h"
#include <algorithm>
#include <functional>
#include <WinSock2.h>
#include "../../public/commx/code_convert.h"
#include "StockInfo.h"
#include "config.h"

#pragma comment(lib, "ws2_32.lib")

C2cs *g_pcs = NULL;
C2q *g_pcq = NULL;

LockSingle     g_lockStat;
unsigned int   g_nRecvCount = 0;
unsigned int   g_nCurRecvCount = 0;
unsigned int   g_nSendCount = 0;
unsigned int   g_nCurSendCount = 0;
unsigned int   g_nPeakCount = 0;
//////////////////////////////////////////////////////////////////////////////////
C2cs::C2cs()
{
	m_nClients = (long)0;
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

C2cs::~C2cs()
{
	::CloseHandle(m_hEvent);
	m_hEvent = NULL;
}

ICommX* C2cs::handle_accept(const char *lpLocalAddr, const char *lpPeerAddr)
{
	C2cc *pClient = new C2cc(this);
	CHECK_RUN(!pClient, MN, T("分配内存[%d]失败\n", sizeof(C2cc)), RPT_CRITICAL, return NULL);

	WGUARD(LockRW, m_lock, grd);
	m_clients.insert(pClient);
	m_nClients = (long)m_clients.size();
	return pClient;
}

int C2cs::handle_close(int iError)
{
	class close_handle : std::unary_function<C2cc*, void>
	{
	public:
		result_type operator()(argument_type c)
		{
			c->Close();
		}
	};

	{//关闭所有已经打开的客户端连接对象
		REPORT(MN, T("关闭[%d]个客户连接\n", (long)m_nClients), RPT_INFO);
		RGUARD(LockRW, m_lock, grd);
		std::for_each(m_clients.begin(), m_clients.end(), close_handle());
	}

	//等候所有客户端连接对象完全关闭
	while (m_nClients != (long)0)
		WaitForSingleObject(m_hEvent, 100);

	return -1;
}

void C2cs::handle_release()
{
	REPORT(MN, T("监听[%s]关闭\n", GetLocalAddress()), RPT_INFO);
	delete this;
	g_pcs = NULL;
}

void C2cs::OnClientDisconnect(C2cc *pClient)
{
	WGUARD(LockRW, m_lock, grd);
	m_clients.erase(pClient);
	m_nClients = (long)m_clients.size();
}

//////////////////////////////////////////////////////////////////////////////////
C2cc::C2cc(C2cs *pcs)
: m_pcs(pcs)
{
	m_pucMRequestData = new	unsigned char[MAX_STRUCTSIZE];
}

C2cc::~C2cc()
{
	if (m_pucMRequestData != NULL)
		delete []m_pucMRequestData;
}

const char* C2cc::getMarket(unsigned short mcode)
{
	static char *szzs = "SZ";
	static char *szgp = "sz";
	static char *shzs = "SH";
	static char *shgp = "sh";

	if (mcode == 4097)
		return shzs;
	else if (mcode == 4113)
		return szzs;
	else if (mcode < 4110)
		return shgp;
	else
		return szgp;
}

char* C2cc::GetRecvBuffer(int readID)
{
	if (0 == readID)
		return m_szHttpHead;
	else if (1 == readID)
		return (char*)(&m_oldHead);
	else
	{
		m_lpBuffer = (char*)mpnew(m_oldHead.nSize);
		CHECK_RUN(!m_lpBuffer, MN, T("分配内存[%d]失败\n", m_oldHead.nSize), RPT_CRITICAL, return NULL);
		return m_lpBuffer;
	}
}

unsigned int C2cc::GetRecvBufSize(int readID)
{
	if (0 == readID)
		return sizeof(m_szHttpHead);
	else if (1 == readID)
		return sizeof(_old_head);
	else
		return m_oldHead.nSize;
}

int C2cc::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	REPORT(MN, T("客户[%s]连接已建立\n", GetPeerAddress()), RPT_INFO);
	// 收http包头
	m_nReadID = 0;
	CHECK_RUN(-1 == Read(0, 4, 1, "\r\n\r\n"), MN, T("读取客户HTTP包头失败\n"), RPT_ERROR, return -1);
	return HTTP_MAX_LENGTH;
}

int C2cc::handle_read(char *lpBuffer, unsigned int nLength)
{
	if (0 == m_nReadID)
	{
		m_nReadID = 1;
		CHECK_RUN(-1 == Read(1, sizeof(m_oldHead)), MN, T("读取客户请求包头失败\n"), RPT_ERROR, return -1);
	}
	else if (1 == m_nReadID)
	{
		m_oldHead.nSize = ntohl(m_oldHead.nSize);
		m_nReadID = 2;
		CHECK_RUN(-1 == Read(2, m_oldHead.nSize), MN, T("读取客户请求数据失败\n"), RPT_ERROR, return -1);
	}
	else
	{// 收到完整的请求包
		bool bOK = false;

		g_lockStat.lock();
		g_nRecvCount++;
		g_nCurRecvCount++;
		g_lockStat.unlock();

		SMRequestData* pstMRequestData = (SMRequestData*)lpBuffer;
		pstMRequestData->NetToHost(m_pucMRequestData);
		unsigned char* pucData = (unsigned char*)pstMRequestData;
		pstMRequestData = (SMRequestData*)m_pucMRequestData;
		m_usType = pstMRequestData->m_usType & 0xff00;
		
		REPORT(MN, T("请求类型:%x\n", m_usType), RPT_INFO);
		if (m_usType != SORTDATA)  //排行榜例外
		{
			pucData = (unsigned char*)mpnew(sizeof(SRequestData) + 
				sizeof(SCodeInfo) * pstMRequestData->m_usSize);
		}
		
		switch (m_usType)
		{
		case INITMARKET: //市场初始化
			{
				bOK = GetInitInfo(pstMRequestData);
				mpdel(pucData);
				break;
			}
		case STATDATA:   ////个股统计信息
		case REPORTDATA:  //自选股报价
			{
				bOK = OnReportData(pstMRequestData, pucData);
				break;
			}
		case SORTDATA:	 //排行榜
			{
				bOK = OnSortData(pstMRequestData, pucData);
				break;	
			}
		case NEWREALMIN:  //新分时协议
		case REALMINSDATA:
			{
				bOK = OnRealMinsData(pstMRequestData, pucData);
				break;
			}
		case HISKDATA:   //历史K线数据
			{
				bOK = OnHisKData(pstMRequestData, pucData);
				break;
			}
		case REALMINSTAT: //新分时统计协议
			{
				bOK = OnStatData(pstMRequestData, pucData);
				break;	
			}
		case REALMINOVERLAP: //叠加分时协议
			{
				bOK = OnRealMinsOverlapData(pstMRequestData, pucData);
				break;
			}
		default:
			mpdel(pucData);
			REPORT(MN, T("无效的请求类型[0x%04x]\n", pstMRequestData->m_usType), RPT_WARNING);
			//todo: 增加一个返回错误的饭汇报，协议采用交易的错误返回协议
			break;
		}
		
		if (m_usType != SORTDATA)  //排行榜例外
			mpdel(m_lpBuffer); 

		if (!bOK)  //返回错误代码
		{
			REPORT(MN, T("请求类型[0x%04x]数据有误\n", pstMRequestData->m_usType), RPT_WARNING);
			Answer(NULL, 0, 0);
		}

		m_nReadID = 0;
		CHECK_RUN(-1 == Read(0, 4, 1, "\r\n\r\n"), MN, T("读取客户HTTP包头失败\n"), RPT_ERROR, return -1);
	}
	return 0;
}

bool C2cc::IsSpell(char* pStock)
{
	for (int i = 0; i < CODE_LEN; i++)
	{
		if (pStock[i] == 0x00)
			break;
		if (isalpha(pStock[i]))
			return true;
	}

	return false;
}

bool C2cc::GetInitInfo(SMRequestData* pstMRequestData)
{
#pragma pack(1)
	struct STextData
	{
		char  m_structsize;
		short	m_usType;//响应类型
		short	m_usIndex;//请求索引
		int		m_uiSize;//信息长度
	};
#pragma pack()

	std::string suid = pstMRequestData->m_acRight;
	std::string mobile_type = suid.substr(0, suid.find('_'));
	std::string mobile_ver = suid.substr(suid.find('_')+1);
	std::string strVer = mobile_ver;
	std::string strUpgradeUrl = "";

	int nforce = 0;
	if (mobile_type == "kjava")
	{
		int n = g_cfg.m_strKjavaVer.rfind('.');
		strVer = g_cfg.m_strKjavaVer.substr(0, n);
		if (mobile_ver < strVer)
		{
			strUpgradeUrl = "<upgradeurl>";
			strUpgradeUrl += g_cfg.m_strKjavaUpgradeXml;
			strUpgradeUrl += "</upgradeurl>";
			strUpgradeUrl += "<kjava_upgradecontent>";
			strUpgradeUrl += g_cfg.m_strKjavaContent;
			strUpgradeUrl += "</kjava_upgradecontent>";
			nforce = atoi(g_cfg.m_strKjavaVer.substr(n+1).c_str());
		}
	}
	if (mobile_type == "ppc" && mobile_ver < g_cfg.m_strPpcVer)
	{
		int n = g_cfg.m_strPpcVer.rfind('.');
		strVer = g_cfg.m_strPpcVer.substr(0, n);
		if (mobile_ver < strVer)
		{
			strUpgradeUrl = "<upgradeurl>";
			strUpgradeUrl += g_cfg.m_strPpcUpgradeXml;
			strUpgradeUrl += "</upgradeurl>";
			strUpgradeUrl += "<ppc_upgradecontent>";
			strUpgradeUrl += g_cfg.m_strPpcContent;
			strUpgradeUrl += "</ppc_upgradecontent>";
			nforce = atoi(g_cfg.m_strPpcVer.substr(n+1).c_str());
		}
	}

	{
		g_cfg.m_strOther = "<hkurl>";
		g_cfg.m_strOther += g_cfg.m_strhkurl;
		g_cfg.m_strOther += "</hkurl>";

		g_cfg.m_strOther += "<hsurl>";
		g_cfg.m_strOther += g_cfg.m_strhsurl;
		g_cfg.m_strOther += "</hsurl>";

		g_cfg.m_strOther += "<infourl>";
		g_cfg.m_strOther += g_cfg.m_strinfourl;
		g_cfg.m_strOther += "</infourl>";

		g_cfg.m_strOther += "<tmdb>";
		g_cfg.m_strOther += g_cfg.m_strtmdb;
		g_cfg.m_strOther += "</tmdb>";

		g_cfg.m_strOther += "<tmdbuser>";
		g_cfg.m_strOther += g_cfg.m_strtmdbuser;
		g_cfg.m_strOther += "</tmdbuser>";

		g_cfg.m_strOther += "<tmdbpwd>";
		g_cfg.m_strOther += g_cfg.m_strtmdbpwd;
		g_cfg.m_strOther += "</tmdbpwd>";

		g_cfg.m_strOther += "<tmtable>";
		g_cfg.m_strOther += g_cfg.m_strtmtable;
		g_cfg.m_strOther += "</tmtable>";
	}

	char szVersion[10240];
	sprintf(szVersion, "<version>%s_%s</version><forceupdate>%d</forceupdate>%s%s",
		mobile_type.c_str(), strVer.c_str(), nforce, strUpgradeUrl.c_str(), g_cfg.m_strOther.c_str());

	std::string s = szVersion;
	unsigned int size = sizeof(STextData) + 2 * s.length();

	char* lpInitInfo = (char*)mpnew(size);
	STextData *ptd = (STextData*)lpInitInfo;
	ptd->m_structsize = sizeof(STextData);
	ptd->m_usType = reverse_s(pstMRequestData->m_usType);
	ptd->m_usIndex = reverse_s(pstMRequestData->m_usIndex);
	ptd->m_uiSize = reverse_i(s.length()*2);

	size = MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,s.c_str(),s.length(),(LPWSTR)(lpInitInfo+sizeof(STextData)),s.length()*2);

	unsigned int nLength = size*2+sizeof(STextData);
	m_lpBuffer = lpInitInfo;
	//发送http包头和协议包头
	char szHead[256];
	int nPos = sprintf(szHead, "HTTP/1.1 200 OK, Success\r\nContent-length: %d\r\ncontent-type: audio/mp3;charset=UTF-8\r\ncontent-source : caishentong\r\n\r\n", nLength);
	unsigned int *lpSize = (unsigned int*)(&szHead[nPos]);
	*lpSize = htonl(nLength);
	if (-1 == Write(szHead, nPos + 8))
		return false;

	//发送应答数据
	Write(m_lpBuffer, nLength);
	return true;
}

bool C2cc::OnRealMinsData(SMRequestData* pstMRequestData, unsigned char* pucData)
{
	SRequestData*	pstRequestData = (SRequestData*)pucData;
	pstRequestData->m_usType = pstMRequestData->m_usType;
	pstRequestData->m_usIndex = pstMRequestData->m_usIndex;
	pstRequestData->m_usMarketType = pstMRequestData->m_usMarketType;
	pstRequestData->m_usSize = pstMRequestData->m_usSize;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	SMCodeInfo*		pstMCodeInfo = (SMCodeInfo*)(pstMRequestData + 1);
		
	pstCodeInfo->m_usMarketType = pstMCodeInfo->m_usMarketType;
	memcpy(pstCodeInfo->m_acCode, pstMCodeInfo->m_acCode, CODE_LEN);
	pstCodeInfo->m_uiCode = atol(pstMCodeInfo->m_acCode);
	memcpy(pstCodeInfo->m_acName, pstMCodeInfo->m_acName, NAME_LEN);
	////////////////////////////////////////////////////////////////////////////////////////////
	int nCount = 0;
	char* pStockCode = NULL;
	bool bIsSpell = false;
	if (IsSpell(pstCodeInfo->m_acCode))  //如果是拼音简称
	{
		bIsSpell = true;
		pStockCode = g_StockInfo.GetStockCode(pstCodeInfo->m_acCode, nCount);
		if (pStockCode == NULL)
		{
			mpdel(pucData);
			return false;
		}
		if (nCount > 1) //发送代码列表
		{
			bool bRet = SendCodeList(pStockCode, nCount, pucData);
			mpdel(pStockCode);
			mpdel(pucData);
			return bRet;
		}
		else if (nCount <= 0)
		{
			mpdel(pStockCode);
			mpdel(pucData);
			return false;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	if (bIsSpell)
	{
		if (pstCodeInfo->m_usMarketType == 0)
		{
			char szSymbol[16] = {0};
			memcpy(szSymbol, ((tagStockInfo*)pStockCode)->szCode, 16);
			GetMktType(szSymbol, pstCodeInfo->m_usMarketType);
			memcpy(pstCodeInfo->m_acCode, szSymbol, 6);
		}
	}
	else
	{
		if (pstCodeInfo->m_usMarketType == 0)
			pstCodeInfo->m_usMarketType = g_StockInfo.GetMktType((char*)pstCodeInfo->m_acCode);
	}

	struct _new_req
	{
		CommxHead head;
		unsigned short funcno1;
		unsigned short reqnum;
		
		unsigned short funcno2;		  //分时走势请求
		unsigned short symbol_num1;
		char symbol1[16];

		unsigned short funcno3;		  //报价表请求
		unsigned short symbol_num2;
		char symbol2[16];
	} *preq;

	preq = (_new_req*)mpnew(sizeof(_new_req));
	memset(preq, 0x00, sizeof(_new_req));
	preq->head.Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	preq->head.SerialNo = onRequest(this);
	preq->head.Length = sizeof(_new_req) - sizeof(CommxHead);
	preq->funcno1 = preq->funcno2 = PT_REAL_MINS;
	preq->reqnum = 2;
	preq->symbol_num1 = preq->symbol_num2 = 1;
	preq->funcno3 = PT_QUOTEPRICE;
	
	if (bIsSpell)
	{
		memcpy(preq->symbol1, ((tagStockInfo*)pStockCode)->szCode, 16);
		mpdel(pStockCode);
	}
	else
		GetSymbol(pstCodeInfo, preq->symbol1);
	
	memcpy(preq->symbol2, preq->symbol1, 16);

	InsertCltData(preq->head.SerialNo, (char*)pucData);

	CHECK_RUN(-1 == g_pcq->Write(preq, sizeof(_new_req)), MN, T("向行情服务器发送请求失败\n"), RPT_ERROR, return false);
	return true;
}

bool C2cc::SendCodeList(char* pCode, int nCount, unsigned char* pucData)
{
	unsigned int usSize = ((SCodeList*)m_pucMRequestData)->GetSize(nCount);
	char* pDataBuf = m_szCodeLstBuf;  //必须使用固定缓冲区 否则write后改了缓冲区前四个字节

	SRequestData*	pstRequestData = (SRequestData*)pucData;

	SCodeList* pstCodeList = (SCodeList*)m_pucMRequestData;
	pstCodeList->m_ucSize = nCount;
	pstCodeList->m_usIndex = pstRequestData->m_usIndex;
	pstCodeList->m_usType = LISTCODE;
	
	SMCodeInfo* pstMCodeInfo = (SMCodeInfo*)(pstCodeList + 1);
	pstCodeList->m_pstCodeInfo = pstMCodeInfo;

	tagStockInfo* pStockInfo = (tagStockInfo*)pCode;
	for ( int i = 0; i < nCount; i++)
	{
		memcpy(pstMCodeInfo->m_acName, pStockInfo[i].szName, 16);
		GetMktType(pStockInfo[i].szCode, pstMCodeInfo->m_usMarketType);
		memcpy(pstMCodeInfo->m_acCode, pStockInfo[i].szCode, CODE_LEN);
		pstMCodeInfo++;
	}

	usSize = pstCodeList->HostToNet((unsigned char*)pDataBuf);
	Answer(pDataBuf, usSize, 0);
	return true;
}

void C2cc::GetMktType(char* pSymbol, unsigned short& nMktType)
{
	char* pMty = strstr(pSymbol, ".");
	pMty[0] = 0;
	pMty += 1;

	unsigned int nCode = atoi(pSymbol);
	if (strcmp(pMty, "sz") == 0 || strcmp(pMty, "SZ") == 0)
	{
		if(nCode < 3000)
			nMktType = SZ_AStock;
		else if (nCode < 100000)
			nMktType = SZ_Other;
		else if (nCode < 160000)
			nMktType = SZ_Bond;
		else if (nCode < 200000)
			nMktType = SZ_Fund;
		else if (nCode < 300000)
			nMktType = SZ_BStock;
		else
			nMktType = SZ_Index;
	}
	else if (strcmp(pMty, "sh") == 0 || strcmp(pMty, "SH") == 0)
	{
		if(nCode < 696)
			nMktType = SH_Index;
		else if (nCode < 500001)
			nMktType = SH_Bond;
		else if (nCode < 600000)
			nMktType = SH_Fund;
		else if (nCode < 700000)
			nMktType = SH_AStock;
		else if (nCode < 900000)
			nMktType = SH_Other;
		else
			nMktType = SH_BStock;
	}
	else if (strcmp(pMty, "hk") == 0 || strcmp(pMty, "HK") == 0)
	{
		nMktType = 8196;
	}
}

void C2cc::GetSymbol(SCodeInfo* pstCodeInfo, char* pSymbol)
{
	if (pstCodeInfo->m_usMarketType == 8196) //港股
	{
		int nCode = atoi(pstCodeInfo->m_acCode);
		switch(nCode)
		{
		case 999800: {strcpy_s(pSymbol, 16, "HSI.HK"); break;}  //恒生指数
		case 999801: {strcpy_s(pSymbol, 16, "HSIFIN.HK"); break;} //金融分类
		case 999802: {strcpy_s(pSymbol, 16, "HSIUTL.HK"); break;}  //公用分类
		case 999803: {strcpy_s(pSymbol, 16, "HSIPRO.HK"); break;}  //地产分类
		case 999804: {strcpy_s(pSymbol, 16, "HSICI.HK"); break;}  //工商分类
		case 999805: {strcpy_s(pSymbol, 16, "HSCFI.HK"); break;}  //H股指数
		case 999806: {break;}  //恒50股指
		case 999807: {strcpy_s(pSymbol, 16, "HSCCI.HK"); break;}  //红筹股指
		case 999808: {break;}  //恒生指数
		case 999809: {break;}  //恒生指数
		case 999810: {break;}  //恒生指数
		case 999811: {strcpy_s(pSymbol, 16, "HKL.HK"); break;}  //大型股指
		case 999812: {break;}  //恒生指数
		case 999813: {break;}  //恒生指数
		case 999814: {strcpy_s(pSymbol, 16, "HSHKCI.HK"); break;}  //香港综合
		case 999815: {strcpy_s(pSymbol, 16, "GEM.HK"); break;}  //创业指数
		case 100002: {strcpy_s(pSymbol, 16, "HSCI.HK"); break;}  //恒生综合
		case 100010: {strcpy_s(pSymbol, 16, "HSMLCI.HK"); break;}  //内地综合
		default:
			{
				sprintf_s(pSymbol, 16, "%05d.hk", nCode);
			}
		}
	}
	else									 //深沪
	{
		sprintf_s(pSymbol, 16, "%s.%s", pstCodeInfo->m_acCode, getMarket(pstCodeInfo->m_usMarketType));
	}
}

char* C2cc::AtRealMinsData(char *lpData, unsigned int &nLen, unsigned short serialno)
{
	char* pCltData = RemoveCltData(serialno);
	if (pCltData == NULL)
		return NULL;

	char* pBufPos  = lpData + 8;
	unsigned short* pStockCount = (unsigned short*)(lpData+6);
	if (*pStockCount <= 0)
	{
		mpdel(pCltData);
		return NULL;
	}

	char *lpSymbol = pBufPos;		 //证券代码
	pBufPos += 16;
	unsigned short nMinsCount = 0;   //分钟数
	memcpy(&nMinsCount, pBufPos, sizeof(unsigned short));
	pBufPos += sizeof(unsigned short);

	SRequestData*	pstRequestData = (SRequestData*)pCltData;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	pstRequestData->m_pstCode = pstCodeInfo;

	int nMinsBufSize = nMinsCount * sizeof(SRealMinsUnit);
	char* pMinsBuf = NULL;
	if (nMinsCount > 0)
	{
		pMinsBuf = (char*)mpnew(nMinsBufSize);
		memset(pMinsBuf, 0, nMinsBufSize);
		char* pMinsBufPos = pMinsBuf;

		for (unsigned int i=0; i<nMinsCount; i++)
		{
			tagRealMins* pRealMinsNew = (tagRealMins*)pBufPos;
			pBufPos += sizeof(tagRealMins);

			SRealMinsUnit* pMinsUnit = (SRealMinsUnit*)pMinsBufPos;
			pMinsBufPos += sizeof(SRealMinsUnit);

			pMinsUnit->m_uiNewPrice = pRealMinsNew->nNewPrice;
			pMinsUnit->m_uiVolume   = pRealMinsNew->nVolume;
			pMinsUnit->m_usTime     = (pRealMinsNew->nTime/60)*100 + (pRealMinsNew->nTime%60);
			pMinsUnit->m_uiAverage  = pRealMinsNew->AvgPrice;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	unsigned short* pPackType = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pPackType != PT_QUOTEPRICE)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}

	pStockCount = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pStockCount != 1)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}

	tagQuotedPrice* pQuotePrice = (tagQuotedPrice*)pBufPos;
	//发送数据到旧的客户端
	SRealMinsData realMins = {0};
	realMins.m_usType = pstRequestData->m_usType;
	realMins.m_usIndex=	pstRequestData->m_usIndex;
	realMins.m_usMarketType = pstCodeInfo->m_usMarketType;
	memcpy(realMins.m_acName, pQuotePrice->szName, NAME_LEN);
	memcpy(realMins.m_acCode, pstCodeInfo->m_acCode, CODE_LEN);
	memcpy(realMins.m_ggpj, pstRequestData->m_pstCode->m_ggpj, sizeof(pstRequestData->m_pstCode->m_ggpj));
	
	realMins.m_uiPrevClose = pQuotePrice->nZrsp;
	realMins.m_usMinsUnits = nMinsCount;
	realMins.m_pstMinsUnit = (SRealMinsUnit *)pMinsBuf;
	
	unsigned short	usTime;
	if (pstRequestData->m_pstCode->m_usMarketType == HK_Stock)
	{
		usTime = 600;//开市时间 第一次
		realMins.m_usFirstOpen = usTime/60*100 + usTime%60;

		usTime = 750;//闭市时间 第一次
		realMins.m_usFirstClose = usTime/60*100 + usTime%60;

		usTime =  870;//开市时间 第二次
		realMins.m_usSecondOpen = usTime/60*100 + usTime%60;

		usTime =  960;//闭市时间 第二次
		realMins.m_usSecondClose = usTime/60*100 + usTime%60;
	}
	else
	{
		// add by ljz 2004/06/22  ???
		usTime = 570;//开市时间 第一次
		realMins.m_usFirstOpen = usTime/60*100 + usTime%60;

		usTime = 690;//闭市时间 第一次
		realMins.m_usFirstClose = usTime/60*100 + usTime%60;

		usTime =  780;//开市时间 第二次
		realMins.m_usSecondOpen = usTime/60*100 + usTime%60;

		usTime =  900;//闭市时间 第二次
		realMins.m_usSecondClose = usTime/60*100 + usTime%60;
		// add by ljz 2004/06/22 end
	}

	unsigned short nBufNeedSize = realMins.GetBufNeedSize(nMinsCount, NEWREALMIN);

	char* pSendBuf = (char*)mpnew(nBufNeedSize);
	realMins.HostToNet((unsigned char*)pSendBuf, NEWREALMIN);

	if (pMinsBuf != NULL)
		mpdel(pMinsBuf);
	nLen = nBufNeedSize;
	mpdel(pCltData);
	return pSendBuf;
}

bool C2cc::OnReportData(SMRequestData* pstMRequestData, unsigned char* pucData)
{
	SRequestData*	pstRequestData = (SRequestData*)pucData;
	pstRequestData->m_usType = pstMRequestData->m_usType;
	pstRequestData->m_usIndex = pstMRequestData->m_usIndex;
	pstRequestData->m_usMarketType = pstMRequestData->m_usMarketType;
	pstRequestData->m_usSize = pstMRequestData->m_usSize;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	SMCodeInfo*		pstMCodeInfo = (SMCodeInfo*)(pstMRequestData + 1);
	int i;
	for ( i = 0; i < pstRequestData->m_usSize; i++)
	{
		pstCodeInfo->m_usMarketType = pstMCodeInfo->m_usMarketType;
		memcpy(pstCodeInfo->m_acCode, pstMCodeInfo->m_acCode, CODE_LEN);
		pstCodeInfo->m_uiCode = atol(pstMCodeInfo->m_acCode);
		memcpy(pstCodeInfo->m_acName, pstMCodeInfo->m_acName, NAME_LEN);

		if (pstCodeInfo->m_usMarketType == 0)
			pstCodeInfo->m_usMarketType = g_StockInfo.GetMktType((char*)pstCodeInfo->m_acCode);

		pstCodeInfo++;
		pstMCodeInfo++;
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	int nCount = 0;
	char* pStockCode = NULL;
	bool bIsSpell = false;
	if (IsSpell(pstCodeInfo->m_acCode))  //如果是拼音简称
	{
		bIsSpell = true;
		pStockCode = g_StockInfo.GetStockCode(pstCodeInfo->m_acCode, nCount);
		if (pStockCode == NULL)
		{
			mpdel(pucData);
			return false;
		}
		if (nCount > 1) //发送代码列表
		{
			bool bRet = SendCodeList(pStockCode, nCount, pucData);
			mpdel(pStockCode);
			mpdel(pucData);
			return bRet;
		}
		else if (nCount <= 0)
		{
			mpdel(pStockCode);
			mpdel(pucData);
			return false;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	if (bIsSpell)
	{
		if (pstCodeInfo->m_usMarketType == 0)
		{
			char szSymbol[16] = {0};
			memcpy(szSymbol, ((tagStockInfo*)pStockCode)->szCode, 16);
			GetMktType(szSymbol, pstCodeInfo->m_usMarketType);
			memcpy(pstCodeInfo->m_acCode, szSymbol, 6);
		}
	}

	struct _new_req
	{
		CommxHead head;
		unsigned short funcno1;
		unsigned short reqnum;
		unsigned short funcno2;
		unsigned short symbol_num;
	} *preq;
	int nStockCount  = pstMRequestData->m_usSize;
	int nSendBufSize = sizeof(_new_req) + nStockCount*16;
	char* pSendBuf = (char*)mpnew(nSendBufSize);
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nSendBufSize), RPT_CRITICAL);
		return false;
	}

	preq = (_new_req*)pSendBuf;
	memset(preq, 0x00, sizeof(_new_req));
	preq->head.Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	preq->head.SerialNo = onRequest(this);
	preq->head.Length = nSendBufSize - sizeof(CommxHead);
	preq->funcno1 = preq->funcno2 = PT_QUOTEPRICE;
	preq->reqnum = 1;
	preq->symbol_num = nStockCount;

	InsertCltData(preq->head.SerialNo, (char*)pucData);

	char* pSendBufPos = pSendBuf + sizeof(_new_req);
	pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	for(int i=0; i<nStockCount; i++)
	{
		if (bIsSpell)
			memcpy(pSendBufPos, ((tagStockInfo*)pStockCode)->szCode, 16);
		else
			GetSymbol(&pstCodeInfo[i], pSendBufPos);
		//sprintf_s(pSendBufPos, 16, "%s.%s", pstCodeInfo[i].m_acCode, getMarket(pstCodeInfo[i].m_usMarketType));
		pSendBufPos += 16;
	}

	if (bIsSpell)
		mpdel(pStockCode);

	CHECK_RUN(-1 == g_pcq->Write(pSendBuf, nSendBufSize), MN, T("向行情服务器发送请求失败\n"), RPT_ERROR, return false);
	return true;
}

char* C2cc::AtReportData(char *lpData, unsigned int &nLen, unsigned short serialno)
{
	char* pDataPos = lpData + 6;
	unsigned short nStockCount = 0;
	memcpy(&nStockCount, pDataPos, sizeof(unsigned short));
	pDataPos += sizeof(unsigned short);

	char* pCltData = RemoveCltData(serialno);
	if (pCltData == NULL)
		return NULL;

	if (nStockCount <= 0)
	{
		mpdel(pCltData);
		return NULL;
	}

	SRequestData*	pstRequestData = (SRequestData*)pCltData;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	pstRequestData->m_pstCode = pstCodeInfo;

	//组包并发送到旧的客户端
	int nSortUnitSize = sizeof(SSortUnit)*nStockCount;
	char* pSortUnitBuf = (char*)mpnew(nSortUnitSize);
	if (pSortUnitBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nSortUnitSize), RPT_CRITICAL);
		mpdel(pCltData);
		return NULL;
	}
	memset(pSortUnitBuf, 0, nSortUnitSize);

	SSortUnit* pSortUint = (SSortUnit*)pSortUnitBuf;
	
	for(int i=0; i<nStockCount; i++)
	{
		tagQuotedPrice* pQuotePrice = (tagQuotedPrice*)pDataPos;  //行情数据
		pSortUint[i].m_usMarketCode = GetCodeMarketType(pQuotePrice->szCode, pstRequestData);
		memcpy(pSortUint[i].m_acCode, pQuotePrice->szCode, 6);
		memcpy(pSortUint[i].m_acName, pQuotePrice->szName, 12);
		pSortUint[i].m_uiPrevClose = pQuotePrice->nZrsp;
		pSortUint[i].m_uiNewPrice = pQuotePrice->nZjjg;
		pSortUint[i].m_uiUpMargin = pQuotePrice->nZdf;
		pSortUint[i].m_uiVolume = pQuotePrice->nCjsl;
		pSortUint[i].m_uiCurVol = pQuotePrice->nZjcj;
		pSortUint[i].m_uiSum  = pQuotePrice->nCjje;
		pSortUint[i].m_uiBuyPrice = pQuotePrice->nNp;
		pSortUint[i].m_uiSellPrice = pQuotePrice->nWp;
		pSortUint[i].m_uiOpenPrice = pQuotePrice->nJrkp;
		pSortUint[i].m_uiMaxPrice = pQuotePrice->nZgjg;
		pSortUint[i].m_uiMinPrice = pQuotePrice->nZdjg;
		pSortUint[i].m_uiVolRatio = pQuotePrice->nLb;
		pSortUint[i].m_iEntrustRatio = pQuotePrice->nWb;

		pDataPos += sizeof(tagQuotedPrice);
	}
	  
	unsigned short nBufNeedSize = SSortData::GetBufNeedSize((unsigned char)nStockCount, CODE_SORT);
	int nSendBufSize = nBufNeedSize;
	char* pNewSendBuf = (char*)mpnew(nSendBufSize);
	if (pNewSendBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nSendBufSize), RPT_CRITICAL);
		mpdel(pSortUnitBuf);
		mpdel(pCltData);
		return NULL;
	}
	char* pSendBufPos = pNewSendBuf;

	SSortData sortData = {0};
	sortData.m_usType  = pstRequestData->m_usType;
	sortData.m_usIndex = pstRequestData->m_usIndex;
	sortData.m_usSize  = nStockCount;
	sortData.m_pstSortUnit = (SSortUnit*)pSortUnitBuf;
	
	sortData.HostToNet((unsigned char*)pSendBufPos, CODE_SORT);
	
	mpdel(pSortUnitBuf);
	nLen = nSendBufSize;
	mpdel(pCltData);
	return pNewSendBuf;
}

char* C2cc::AtReportStatData(char *lpData, unsigned int &nLen, unsigned short serialno)
{
	char* pDataPos = lpData + 6;
	unsigned short nStockCount = 0;
	memcpy(&nStockCount, pDataPos, sizeof(unsigned short));
	pDataPos += sizeof(unsigned short);

	char* pCltData = RemoveCltData(serialno);
	if (pCltData == NULL)
		return NULL;

	//////////////////////////////////////////////////////////////////////////////
	if (nStockCount <= 0)
	{
		mpdel(pCltData);
		return NULL;
	}

	SRequestData*	pstRequestData = (SRequestData*)pCltData;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	pstRequestData->m_pstCode = pstCodeInfo;

	tagQuotedPrice* pQuotePrice = (tagQuotedPrice*)pDataPos;  //行情数据
	//组包并发送到旧的客户端
	unsigned short nBufNeedSize = ((SStatData*)pDataPos)->GetSize(pstRequestData->m_usMarketType & 0x0f);
	int nSendBufSize = nBufNeedSize;
	char* pNewSendBuf = (char*)mpnew(nSendBufSize);
	if (pNewSendBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nSendBufSize), RPT_CRITICAL);
		mpdel(pCltData);
		return NULL;
	}

	char* pucUserDataBuf = (char*)mpnew(nBufNeedSize);
	if (pucUserDataBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nBufNeedSize), RPT_CRITICAL);
		mpdel(pCltData);
		return NULL;
	}

	SStatData*	pstStatData = (SStatData*)pucUserDataBuf;
	pstStatData->m_usIndex = pstRequestData->m_usIndex;
	pstStatData->m_usMarketType = pstCodeInfo->m_usMarketType;
	pstStatData->m_usType = pstRequestData->m_usType;
	memcpy(pstStatData->m_acName, pQuotePrice->szName, NAME_LEN);
	memcpy(pstStatData->m_acCode, pstRequestData->m_pstCode->m_acCode, CODE_LEN);
	memcpy(pstStatData->m_ggpj, pstRequestData->m_pstCode->m_ggpj, sizeof(pstRequestData->m_pstCode->m_ggpj));
	
	SRealUnit*	pstRealUnit = (SRealUnit*)(pstStatData + 1);
	pstRealUnit->m_puiData = (unsigned int*)(pstRealUnit + 1);
	unsigned int*	puiData = pstRealUnit->m_puiData;

	pstStatData->m_uiPrevClose = pQuotePrice->nZrsp;

	unsigned int uiIndex = 0;
	if ((pstCodeInfo->m_usMarketType & 0x0f) != ST_Index)
	{
		puiData[uiIndex++] = pQuotePrice->nZd;
		puiData[uiIndex++] = pQuotePrice->nZdf;
		puiData[uiIndex++] = pQuotePrice->nZrsp;
		puiData[uiIndex++] = pQuotePrice->nJrkp;
		puiData[uiIndex++] = pQuotePrice->nZgjg;

		puiData[uiIndex++] = pQuotePrice->nZdjg;
		puiData[uiIndex++] = pQuotePrice->nZtjg;
		puiData[uiIndex++] = pQuotePrice->nDtjg;
		puiData[uiIndex++] = pQuotePrice->nPjjg;
		puiData[uiIndex++] = pQuotePrice->nCjsl;

		puiData[uiIndex++] = pQuotePrice->nWb;
		puiData[uiIndex++] = pQuotePrice->nLb;
		puiData[uiIndex++] = pQuotePrice->nCjje*100; 
		puiData[uiIndex++] = pQuotePrice->nNp;

		puiData[uiIndex++] = pQuotePrice->nWp;
		puiData[uiIndex++] = pQuotePrice->nSyl1;
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = pQuotePrice->nSjw5;
		puiData[uiIndex++] = pQuotePrice->nSsl5;

		puiData[uiIndex++] = pQuotePrice->nSjw4;
		puiData[uiIndex++] = pQuotePrice->nSsl4;
		puiData[uiIndex++] = pQuotePrice->nSjw3;
		puiData[uiIndex++] = pQuotePrice->nSsl3;
		puiData[uiIndex++] = pQuotePrice->nSjw2;

		puiData[uiIndex++] = pQuotePrice->nSsl2;
		puiData[uiIndex++] = pQuotePrice->nSjw1;
		puiData[uiIndex++] = pQuotePrice->nSsl1;
		puiData[uiIndex++] = pQuotePrice->nZjjg;
		puiData[uiIndex++] = pQuotePrice->nZjcj;

		puiData[uiIndex++] = pQuotePrice->nBjw1;
		puiData[uiIndex++] = pQuotePrice->nBsl1;
		puiData[uiIndex++] = pQuotePrice->nBjw2;
		puiData[uiIndex++] = pQuotePrice->nBsl2;
		puiData[uiIndex++] = pQuotePrice->nBjw3;

		puiData[uiIndex++] = pQuotePrice->nBsl3;
		puiData[uiIndex++] = pQuotePrice->nBjw4;
		puiData[uiIndex++] = pQuotePrice->nBsl4;
		puiData[uiIndex++] = pQuotePrice->nBjw5;
		puiData[uiIndex++] = pQuotePrice->nBsl5;
	}
	else   //指数
	{
		// 将A股,B股,基金的成交金额提取出来
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = 0;

		puiData[uiIndex++] = pQuotePrice->nZjjg;
		puiData[uiIndex++] = pQuotePrice->nZd;
		puiData[uiIndex++] = pQuotePrice->nZdf;

		puiData[uiIndex++] = pQuotePrice->nCjje*10; 
		puiData[uiIndex++] = pQuotePrice->nCjsl;
		puiData[uiIndex++] = pQuotePrice->nZgjg;
		puiData[uiIndex++] = pQuotePrice->nZdjg;

		puiData[uiIndex++] = pQuotePrice->nBsl1; //?上涨和下跌
		puiData[uiIndex++] = pQuotePrice->nBsl3;
	}
	pstStatData->m_stRealUnit = *pstRealUnit;
	pstStatData->HostToNet((unsigned char*)pNewSendBuf);
	
	nLen = nSendBufSize;
	mpdel(pCltData);
	mpdel(pucUserDataBuf);
	return pNewSendBuf;
}
bool C2cc::OnHisKData(SMRequestData* pstMRequestData, unsigned char* pucData)
{
	SRequestData*	pstRequestData = (SRequestData*)pucData;
	pstRequestData->m_usType = pstMRequestData->m_usType;
	pstRequestData->m_usIndex = pstMRequestData->m_usIndex;
	pstRequestData->m_usMarketType = pstMRequestData->m_usMarketType;
	pstRequestData->m_usSize = pstMRequestData->m_usSize;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	SMCodeInfo*		pstMCodeInfo = (SMCodeInfo*)(pstMRequestData + 1);
		
	pstCodeInfo->m_usMarketType = pstMCodeInfo->m_usMarketType;
	memcpy(pstCodeInfo->m_acCode, pstMCodeInfo->m_acCode, CODE_LEN);
	pstCodeInfo->m_uiCode = atol(pstMCodeInfo->m_acCode);
	memcpy(pstCodeInfo->m_acName, pstMCodeInfo->m_acName, NAME_LEN);
	////////////////////////////////////////////////////////////////////////////////////////////
	int nCount = 0;
	char* pStockCode = NULL;
	bool bIsSpell = false;
	if (IsSpell(pstCodeInfo->m_acCode))  //如果是拼音简称
	{
		bIsSpell = true;
		pStockCode = g_StockInfo.GetStockCode(pstCodeInfo->m_acCode, nCount);
		if (pStockCode == NULL)
		{
			mpdel(pucData);
			return false;
		}
		if (nCount > 1) //发送代码列表
		{
			bool bRet = SendCodeList(pStockCode, nCount, pucData);
			mpdel(pStockCode);
			mpdel(pucData);
			return bRet;
		}
		else if (nCount <= 0)
		{
			mpdel(pStockCode);
			mpdel(pucData);
			return false;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////

	if (bIsSpell)
	{
		if (pstCodeInfo->m_usMarketType == 0)
		{
			char szSymbol[16] = {0};
			memcpy(szSymbol, ((tagStockInfo*)pStockCode)->szCode, 16);
			GetMktType(szSymbol, pstCodeInfo->m_usMarketType);
			memcpy(pstCodeInfo->m_acCode, szSymbol, 6);
		}
	}
	else
	{
		if (pstCodeInfo->m_usMarketType == 0)
			pstCodeInfo->m_usMarketType = g_StockInfo.GetMktType((char*)pstCodeInfo->m_acCode);
	}
	struct _new_req
	{
		CommxHead head;
		unsigned short funcno1;
		unsigned short reqnum;
		unsigned short funcno2;
		unsigned short symbol_num;
		char symbol[16];
		unsigned long nBeginDate;
		unsigned long nEndDate;
		unsigned short nHQMkt;  //还权标志  0正常，1还权

		unsigned short funcno3;		  //报价表请求
		unsigned short symbol_num2;
		char symbol2[16];
	} *preq;

	preq = (_new_req*)mpnew(sizeof(_new_req));
	memset(preq, 0x00, sizeof(_new_req));
	preq->head.Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	preq->head.SerialNo = onRequest(this);
	preq->head.Length = sizeof(_new_req) - sizeof(CommxHead);
	preq->nHQMkt = 0;
	
	InsertCltData(preq->head.SerialNo, (char*)pucData);

	switch(pstRequestData->m_usType & 0xff)
	{
	case DAY_HISK:
		preq->funcno1 = preq->funcno2 = PT_HKDATA_DAY;
		break;
	case WEEK_HISK:
		preq->funcno1 = preq->funcno2 = PT_HKDATA_WEEK;
		break;
	case MONTH_HISK:
		preq->funcno1 = preq->funcno2 = PT_HKDATA_MONTH;
		break;
	default:
		REPORT(MN, T("不能处理的k线数据类型:%d\n", (pstRequestData->m_usType & 0xff)), RPT_ERROR);
		return false;
	}
	preq->funcno3 = PT_QUOTEPRICE;
	preq->reqnum = 2;
	preq->symbol_num = preq->symbol_num2 = 1;
	if (bIsSpell)
	{
		memcpy(preq->symbol, ((tagStockInfo*)pStockCode)->szCode, 16);
		mpdel(pStockCode);
	}
	else
		GetSymbol(pstCodeInfo, preq->symbol);
	memcpy(preq->symbol2, preq->symbol, 16);
	//sprintf_s(preq->symbol, 16, "%s.%s", pstCodeInfo->m_acCode, getMarket(pstCodeInfo->m_usMarketType));

	CHECK_RUN(-1 == g_pcq->Write(preq, sizeof(_new_req)), MN, T("向行情服务器发送请求失败\n"), RPT_ERROR, return false);
	return true;
}

char* C2cc::AtHisKData(char *lpData, unsigned int &nLen, unsigned short serialno)
{
	char* pCltData = RemoveCltData(serialno);
	if (pCltData == NULL)
		return NULL;

	char* pBufPos  = lpData + 8;
	unsigned short* pStockCount = (unsigned short*)(lpData+6);
	if (*pStockCount <= 0)
	{
		mpdel(pCltData);
		return NULL;
	}

	char *lpSymbol = pBufPos;		//证券代码
	pBufPos += 16;
	unsigned short nHQMkt = 0;
	memcpy(&nHQMkt, pBufPos, sizeof(unsigned short));
	pBufPos += sizeof(unsigned short);

	unsigned int nKLineCount = 0;   //K线个数
	memcpy(&nKLineCount, pBufPos, sizeof(unsigned int));
	pBufPos += sizeof(unsigned int);

	SRequestData*	pstRequestData = (SRequestData*)pCltData;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	pstRequestData->m_pstCode = pstCodeInfo;

	int nKLineBufSize = nKLineCount * sizeof(SHisKUnit);
	char* pKLineBuf = NULL;
	if (nKLineCount > 0)
	{
		pKLineBuf = (char*)mpnew(nKLineBufSize);
		if (pKLineBuf == NULL)
		{
			REPORT(MN, T("mpnew() failed size=%d\n", nKLineBufSize), RPT_CRITICAL);
			mpdel(pCltData);
			return NULL;
		}
		memset(pKLineBuf, 0, nKLineBufSize);
		memcpy(pKLineBuf, pBufPos, nKLineBufSize);
		pBufPos += nKLineBufSize;
	}
	//////////////////////////////////////////////////////////////////////
	unsigned short* pPackType = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pPackType != PT_QUOTEPRICE)
	{
		if (pKLineBuf != NULL)
			mpdel(pKLineBuf);
		mpdel(pCltData);
		return NULL;
	}

	pStockCount = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pStockCount != 1)
	{
		if (pKLineBuf != NULL)
			mpdel(pKLineBuf);
		mpdel(pCltData);
		return NULL;
	}

	tagQuotedPrice* pQuotePrice = (tagQuotedPrice*)pBufPos;
	//发送到旧的客户端
	SHisKData hisKData = {0};
	hisKData.m_ucStructSize = sizeof(SHisKData);
	hisKData.m_usType = pstRequestData->m_usType;
	hisKData.m_usIndex=	pstRequestData->m_usIndex;
	hisKData.m_usMarketType = pstCodeInfo->m_usMarketType;
	memcpy(hisKData.m_acName, pQuotePrice->szName, NAME_LEN);
	memcpy(hisKData.m_acCode, pstCodeInfo->m_acCode, 6);
	hisKData.m_usSize = nKLineCount;
	hisKData.m_pstHisKUnit = (SHisKUnit *)pKLineBuf;
	unsigned short nBufNeedSize = hisKData.GetBufNeedSize(nKLineCount);
	
	int nSendBufSize = nBufNeedSize;
	char* pSendBuf = (char*)mpnew(nSendBufSize);
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nSendBufSize), RPT_CRITICAL);
		if (pKLineBuf != NULL)
			mpdel(pKLineBuf);
		mpdel(pCltData);
		return NULL;
	}
	char* pSendBufPos = pSendBuf;
	hisKData.HostToNet((unsigned char*)pSendBufPos);

	nLen = nSendBufSize;
	if (pKLineBuf != NULL)
		mpdel(pKLineBuf);
	mpdel(pCltData);
	return pSendBuf;
}

bool C2cc::OnSortData(SMRequestData* pstMRequestData, unsigned char* pucData)
{
	SMRequestSort*	pstMRequestSort = (SMRequestSort*)m_pucMRequestData;
	pstMRequestSort->NetToHost(pucData);

	SRequestSort*	pstRequestSort = (SRequestSort*)pucData;
	pstRequestSort->m_usType = pstMRequestSort->m_usType;
	pstRequestSort->m_usMarketType = pstMRequestSort->m_usMarketType;
	pstRequestSort->m_usIndex = pstMRequestSort->m_usIndex;
	pstRequestSort->m_usFirst = pstMRequestSort->m_usFirst;
	pstRequestSort->m_usSize = pstMRequestSort->m_usSize;
	
	int nDirection = 0;
	memcpy(&nDirection, pstMRequestSort->m_acRight, sizeof(int));

	char szField[512] = {0};
	GetField(pstMRequestData->m_usType, szField);
	REPORT(MN, T("%s", szField), RPT_INFO);
	char szSymbol[1024] = {0};
	GetSymbol(pstMRequestData->m_usMarketType, szSymbol);
	strcat(szSymbol, szField);

	int nPackSize = sizeof(CommxHead) + sizeof(unsigned short) + 1 + 
		2*sizeof(unsigned int) + (int)strlen(szSymbol)+1;
	char* pSendBuf = (char*)mpnew(nPackSize);
	memset(pSendBuf, 0x00, nPackSize);

	char* pSendBufPos = pSendBuf;
	CommxHead* pHead = (CommxHead*)pSendBufPos;
	pSendBufPos += sizeof(CommxHead);
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->SerialNo = onRequest(this);
	pHead->Length = nPackSize - sizeof(CommxHead);

	InsertCltData(pHead->SerialNo, (char*)pucData);

	unsigned short nPackType = PT_SORTDATA;
	memcpy(pSendBufPos, &nPackType, sizeof(unsigned short));
	pSendBufPos += sizeof(unsigned short);

	unsigned char SortType = 1; 
	if (nDirection == -1)
	{
		SortType = 1; 
		memcpy(pSendBufPos, &SortType, 1);
	}
	else
	{
		SortType = 0; 
		memcpy(pSendBufPos, &SortType, 1);
	}
	pSendBufPos += 1;

	memcpy(pSendBufPos, &pstRequestSort->m_usFirst, sizeof(unsigned int));
	pSendBufPos += sizeof(unsigned int);
	
	memcpy(pSendBufPos, &pstRequestSort->m_usSize, sizeof(unsigned int));
	pSendBufPos += sizeof(unsigned int);

	memcpy(pSendBufPos, szSymbol, (int)strlen(szSymbol));
	pSendBufPos += (int)strlen(szSymbol) + 1;

	CHECK_RUN(-1 == g_pcq->Write(pSendBuf, nPackSize), MN, T("向行情服务器发送请求失败\n"), RPT_ERROR, return false);
	return true;
}

char* C2cc::AtSortData(char *lpData, unsigned int &nLen, unsigned short serialno)
{
	unsigned int nFuc = 0;
	memcpy(&nFuc, lpData, sizeof(unsigned short));
	if (nFuc == 0xffff)
		return NULL;

	char* pCltData = RemoveCltData(serialno);
	if (pCltData == NULL)
		return NULL;

	SRequestData*	pstRequestData = (SRequestData*)pCltData;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	pstRequestData->m_pstCode = pstCodeInfo;

	char* pDataPos = lpData + 2;

	unsigned int nRecCount = pstRequestData->m_usSize;
	char* pSortData = (char*)mpnew(nRecCount * sizeof(SSortUnit));
	char* pSortDataPos = pSortData;
	int nLeaveSize = nLen - 2;
	int nCount = 0;
	SSortUnit* pSortUnit = (SSortUnit*)pSortDataPos;

	switch (pstRequestData->m_usType & 0xff)
	{
	case RISE_SORT:     //涨幅排行榜
		{
			for (int i=0; ; i++)
			{
				if (nLeaveSize <= 0)
					break;
				memset(&pSortUnit[i], 0, sizeof(SSortUnit));
				nCount++;
				memcpy(&pSortUnit[i].m_uiUpMargin, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);

				char szCode[32] = {0};
				memcpy(szCode, pDataPos, 32);
				char* pPos = strstr(szCode, ".");
				pPos[0] = 0;
				memcpy(&pSortUnit[i].m_acCode, szCode, 6);
				pDataPos += 32;
				nLeaveSize -= 32;
				
				memcpy(&pSortUnit[i].m_acName, pDataPos, 12);
				pDataPos += 32;
				nLeaveSize -= 32;
				
				memcpy(&pSortUnit[i].m_uiPrevClose, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiNewPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolume, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiCurVol, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSum, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiBuyPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSellPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiOpenPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMaxPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMinPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_iEntrustRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
			}
			break;
		}
	case SUM_SORT:  
		{//成交金额排行榜
			for (int i=0; ; i++)
			{
				if (nLeaveSize <= 0)
					break;
				memset(&pSortUnit[i], 0, sizeof(SSortUnit));
				nCount++;
				memcpy(&pSortUnit[i].m_uiSum, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);

				char szCode[32] = {0};
				memcpy(szCode, pDataPos, 32);
				char* pPos = strstr(szCode, ".");
				pPos[0] = 0;
				memcpy(&pSortUnit[i].m_acCode, szCode, 6);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_acName, pDataPos, 12);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_uiPrevClose, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiNewPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolume, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiCurVol, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiBuyPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSellPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiOpenPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMaxPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMinPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_iEntrustRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiUpMargin, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
			}
			//strcpy_s(pRecvBuf, 512, "cjje|code|name|zrsp|zjjg|cjsl|zjcj|np|wp|jrkp|zgjg|zdjg|lb|wb|zdf");
			break;
		}
	case VOLUME_SORT:   //成交量排行榜
		{
			for (int i=0; ; i++)
			{
				if (nLeaveSize <= 0)
					break;
				memset(&pSortUnit[i], 0, sizeof(SSortUnit));
				nCount++;
				memcpy(&pSortUnit[i].m_uiVolume, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);

				char szCode[32] = {0};
				memcpy(szCode, pDataPos, 32);
				char* pPos = strstr(szCode, ".");
				pPos[0] = 0;
				memcpy(&pSortUnit[i].m_acCode, szCode, 6);
				pDataPos += 32;
				nLeaveSize -= 32;
				
				memcpy(&pSortUnit[i].m_acName, pDataPos, 12);
				pDataPos += 32;
				nLeaveSize -= 32;
				
				memcpy(&pSortUnit[i].m_uiPrevClose, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiNewPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiCurVol, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSum, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiBuyPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSellPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiOpenPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMaxPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMinPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_iEntrustRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiUpMargin, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);  
				nLeaveSize -= sizeof(unsigned int);
			}
			//strcpy_s(pRecvBuf, 512, "cjsl|code|name|zrsp|zjjg|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|lb|wb|zdf");
			break;
		}
	case SWING_SORT:    //振幅排行榜
		{
			for (int i=0; ; i++)
			{
				if (nLeaveSize <= 0)
					break;
				memset(&pSortUnit[i], 0, sizeof(SSortUnit));
				nCount++;
				
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);

				char szCode[32] = {0};
				memcpy(szCode, pDataPos, 32);
				char* pPos = strstr(szCode, ".");
				pPos[0] = 0;
				memcpy(&pSortUnit[i].m_acCode, szCode, 6);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_acName, pDataPos, 12);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_uiPrevClose, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiNewPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolume, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiCurVol, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSum, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiBuyPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSellPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiOpenPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMaxPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMinPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_iEntrustRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiUpMargin, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
			}
			//strcpy_s(pRecvBuf, 512, "zf|code|name|zrsp|zjjg|cjsl|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|lb|wb|zdf");
			break;
		}
	case VOLRATIO_SORT: //量比排行榜
		{
			for (int i=0; ; i++)
			{
				if (nLeaveSize <= 0)
					break;
				memset(&pSortUnit[i], 0, sizeof(SSortUnit));
				nCount++;
				memcpy(&pSortUnit[i].m_uiVolRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);

				char szCode[32] = {0};
				memcpy(szCode, pDataPos, 32);
				char* pPos = strstr(szCode, ".");
				pPos[0] = 0;
				memcpy(&pSortUnit[i].m_acCode, szCode, 6);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_acName, pDataPos, 12);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_uiPrevClose, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiNewPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolume, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiCurVol, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSum, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiBuyPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSellPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiOpenPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMaxPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMinPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				
				memcpy(&pSortUnit[i].m_iEntrustRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiUpMargin, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);

			}
			//strcpy_s(pRecvBuf, 512, "lb|code|name|zrsp|zjjg|cjsl|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|wb|zdf");
			break;
		}
	case BUYSELL_SORT:  //委比排行
		{
			for (int i=0; ; i++)
			{
				if (nLeaveSize <= 0)
					break;
				memset(&pSortUnit[i], 0, sizeof(SSortUnit));
				nCount++;
				memcpy(&pSortUnit[i].m_iEntrustRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);

				char szCode[32] = {0};
				memcpy(szCode, pDataPos, 32);
				char* pPos = strstr(szCode, ".");
				pPos[0] = 0;
				memcpy(&pSortUnit[i].m_acCode, szCode, 6);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_acName, pDataPos, 12);
				pDataPos += 32;
				nLeaveSize -= 32;
				memcpy(&pSortUnit[i].m_uiPrevClose, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiNewPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolume, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiCurVol, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSum, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiBuyPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiSellPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiOpenPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMaxPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiMinPrice, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiVolRatio, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
				memcpy(&pSortUnit[i].m_uiUpMargin, pDataPos, sizeof(unsigned int));
				pDataPos += sizeof(unsigned int);
				nLeaveSize -= sizeof(unsigned int);
			}
			//strcpy_s(pRecvBuf, 512, "wb|code|name|zrsp|zjjg|cjsl|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|lb|zdf");
			break;
		}
	}

	unsigned short nBufNeedSize = SSortData::GetBufNeedSize((unsigned char)nCount, CODE_SORT);
	int nSendBufSize = nBufNeedSize;
	char* pNewSendBuf = (char*)mpnew(nSendBufSize);
	if (pNewSendBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nSendBufSize), RPT_CRITICAL);
		mpdel(pSortData);
		mpdel(pCltData);
		return NULL;
	}
	char* pSendBufPos = pNewSendBuf;

	SSortData sortData = {0};
	sortData.m_usType  = pstRequestData->m_usType;
	sortData.m_usIndex = pstRequestData->m_usIndex;
	sortData.m_usSize  = nCount;
	sortData.m_usTotal = nCount;
	sortData.m_pstSortUnit = (SSortUnit*)pSortData;
	
	sortData.HostToNet((unsigned char*)pSendBufPos, pstRequestData->m_usType & 0xff);
	
	mpdel(pSortData);
	mpdel(pCltData);
	nLen = nSendBufSize;
	return pNewSendBuf;
}

bool C2cc::OnStatData(SMRequestData* pstMRequestData, unsigned char* pucData)
{
	SRequestData*	pstRequestData = (SRequestData*)pucData;
	pstRequestData->m_usType = pstMRequestData->m_usType;
	pstRequestData->m_usIndex = pstMRequestData->m_usIndex;
	pstRequestData->m_usMarketType = pstMRequestData->m_usMarketType;
	pstRequestData->m_usSize = pstMRequestData->m_usSize;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	SMCodeInfo*		pstMCodeInfo = (SMCodeInfo*)(pstMRequestData + 1);
		
	pstCodeInfo->m_usMarketType = pstMCodeInfo->m_usMarketType;
	memcpy(pstCodeInfo->m_acCode, pstMCodeInfo->m_acCode, CODE_LEN);
	pstCodeInfo->m_uiCode = atol(pstMCodeInfo->m_acCode);
	memcpy(pstCodeInfo->m_acName, pstMCodeInfo->m_acName, NAME_LEN);
	////////////////////////////////////////////////////////////////////////////////////////////
	int nCount = 0;
	char* pStockCode = NULL;
	bool bIsSpell = false;
	if (IsSpell(pstCodeInfo->m_acCode))  //如果是拼音简称
	{
		bIsSpell = true;
		pStockCode = g_StockInfo.GetStockCode(pstCodeInfo->m_acCode, nCount);
		if (pStockCode == NULL)
		{
			mpdel(pucData);
			return false;
		}
		if (nCount > 1) //发送代码列表
		{
			bool bRet = SendCodeList(pStockCode, nCount, pucData);
			mpdel(pStockCode);
			mpdel(pucData);
			return bRet;
		}
		else if (nCount <= 0)
		{
			mpdel(pStockCode);
			mpdel(pucData);
			return false;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////

	if (bIsSpell)
	{
		if (pstCodeInfo->m_usMarketType == 0)
		{
			char szSymbol[16] = {0};
			memcpy(szSymbol, ((tagStockInfo*)pStockCode)->szCode, 16);
			GetMktType(szSymbol, pstCodeInfo->m_usMarketType);
			memcpy(pstCodeInfo->m_acCode, szSymbol, 6);
		}
	}
	else
	{
		if (pstCodeInfo->m_usMarketType == 0)
			pstCodeInfo->m_usMarketType = g_StockInfo.GetMktType((char*)pstCodeInfo->m_acCode);
	}

	struct _new_req
	{
		CommxHead head;
		unsigned short funcno1;
		unsigned short reqnum;
		
		unsigned short funcno2;		  //分时走势请求
		unsigned short symbol_num1;
		char symbol1[16];

		unsigned short funcno3;		  //报价表请求
		unsigned short symbol_num2;
		char symbol2[16];
	} *preq;

	preq = (_new_req*)mpnew(sizeof(_new_req));
	memset(preq, 0x00, sizeof(_new_req));
	preq->head.Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	preq->head.SerialNo = onRequest(this);
	preq->head.Length = sizeof(_new_req) - sizeof(CommxHead);
	preq->funcno1 = preq->funcno2 = PT_REAL_MINS;
	preq->reqnum = 2;
	preq->symbol_num1 = preq->symbol_num2 = 1;
	preq->funcno3 = PT_QUOTEPRICE;

	InsertCltData(preq->head.SerialNo, (char*)pucData);

	if (bIsSpell)
	{
		memcpy(preq->symbol1, ((tagStockInfo*)pStockCode)->szCode, 16);
		mpdel(pStockCode);
	}
	else
		GetSymbol(pstCodeInfo, preq->symbol1);
	memcpy(preq->symbol2, preq->symbol1, 16);

	CHECK_RUN(-1 == g_pcq->Write(preq, sizeof(_new_req)), MN, T("向行情服务器发送请求失败\n"), RPT_ERROR, return false);
	return true;
}

char* C2cc::AtStatData(char *lpData, unsigned int &nLen, unsigned short serialno)
{
	char* pCltData = RemoveCltData(serialno);
	if (pCltData == NULL)
		return NULL;

	char* pBufPos  = lpData + 8;
	unsigned short* pStockCount = (unsigned short*)(lpData+6);
	if (*pStockCount <= 0)
	{
		mpdel(pCltData);
		return NULL;
	}

	char *lpSymbol = pBufPos;		 //证券代码
	pBufPos += 16;
	unsigned short nMinsCount = 0;   //分钟数
	memcpy(&nMinsCount, pBufPos, sizeof(unsigned short));
	pBufPos += sizeof(unsigned short);

	SRequestData*	pstRequestData = (SRequestData*)pCltData;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	pstRequestData->m_pstCode = pstCodeInfo;

	int nMinsBufSize = nMinsCount * sizeof(SRealMinsUnit);
	char* pMinsBuf = NULL;
	if (nMinsCount > 0)
	{
		pMinsBuf = (char*)mpnew(nMinsBufSize);
		if (pMinsBuf == NULL)
		{
			REPORT(MN, T("mpnew() failed size=%d\n", nMinsBufSize), RPT_CRITICAL);
			mpdel(pCltData);
			return NULL;
		}
		memset(pMinsBuf, 0, nMinsBufSize);
		char* pMinsBufPos = pMinsBuf;

		for (unsigned int i=0; i<nMinsCount; i++)
		{
			tagRealMins* pRealMinsNew = (tagRealMins*)pBufPos;
			pBufPos += sizeof(tagRealMins);

			SRealMinsUnit* pMinsUnit = (SRealMinsUnit*)pMinsBufPos;
			pMinsBufPos += sizeof(SRealMinsUnit);

			pMinsUnit->m_uiNewPrice = pRealMinsNew->nNewPrice;
			pMinsUnit->m_uiVolume   = pRealMinsNew->nVolume;
			pMinsUnit->m_usTime     = (pRealMinsNew->nTime/60)*100 + (pRealMinsNew->nTime%60);
			pMinsUnit->m_uiAverage  = pRealMinsNew->AvgPrice;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////
	unsigned short* pPackType = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pPackType != PT_QUOTEPRICE)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}
	pStockCount = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pStockCount != 1)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}
	tagQuotedPrice* pQuotePrice = (tagQuotedPrice*)pBufPos;

	//发送数据到旧的客户端
	SRealMinsData realMins = {0};
	realMins.m_ucStructSize = sizeof(SRealMinsData);
	realMins.m_usType = pstRequestData->m_usType;
	realMins.m_usIndex=	pstRequestData->m_usIndex;
	realMins.m_usMarketType = pstRequestData->m_pstCode->m_usMarketType;
	memcpy(realMins.m_acName, pQuotePrice->szName, NAME_LEN);
	memcpy(realMins.m_acCode, pstRequestData->m_pstCode->m_acCode, 6);
	realMins.m_usMinsUnits = nMinsCount;
	realMins.m_pstMinsUnit = (SRealMinsUnit *)pMinsBuf;
	realMins.m_uiPrevClose = pQuotePrice->nZrsp;
	
	unsigned short	usTime;
	if (pstRequestData->m_pstCode->m_usMarketType == HK_Stock)
	{
		usTime = 600;//开市时间 第一次
		realMins.m_usFirstOpen = usTime/60*100 + usTime%60;

		usTime = 750;//闭市时间 第一次
		realMins.m_usFirstClose = usTime/60*100 + usTime%60;

		usTime =  870;//开市时间 第二次
		realMins.m_usSecondOpen = usTime/60*100 + usTime%60;

		usTime =  960;//闭市时间 第二次
		realMins.m_usSecondClose = usTime/60*100 + usTime%60;
	}
	else
	{
		// add by ljz 2004/06/22  ???
		usTime = 570;//开市时间 第一次
		realMins.m_usFirstOpen = usTime/60*100 + usTime%60;

		usTime = 690;//闭市时间 第一次
		realMins.m_usFirstClose = usTime/60*100 + usTime%60;

		usTime =  780;//开市时间 第二次
		realMins.m_usSecondOpen = usTime/60*100 + usTime%60;

		usTime =  900;//闭市时间 第二次
		realMins.m_usSecondClose = usTime/60*100 + usTime%60;
		// add by ljz 2004/06/22 end
	}

	unsigned short nBufNeedSize = realMins.GetBufNeedSize(nMinsCount, NEWREALMIN);
	
	char* pSendBuf = (char*)mpnew(nBufNeedSize);
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nBufNeedSize), RPT_CRITICAL);
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}

	realMins.HostToNet((unsigned char*)pSendBuf, NEWREALMIN);
	if (pMinsBuf != NULL)
		mpdel(pMinsBuf);
	/////////////////////////////////////////////////////////////////////////////////////////
	int uiSize = ((SStatData*)pSendBuf)->GetSize(pstCodeInfo->m_usMarketType & 0x0f);

	int nAllSendBufSize = uiSize + nBufNeedSize;
	char* pAllSendBuf = (char*)mpnew(nAllSendBufSize);
	if (pAllSendBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", nAllSendBufSize), RPT_CRITICAL);
		if (pMinsBuf != NULL)
		mpdel(pSendBuf);
		mpdel(pCltData);
		return NULL;
	}

	char* pucUserDataBuf = (char*)mpnew(uiSize);
	if (pucUserDataBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", uiSize), RPT_CRITICAL);
		mpdel(pAllSendBuf);
		mpdel(pCltData);
		return NULL;
	}

	SStatData*	pstStatData = (SStatData*)pucUserDataBuf;
	pstStatData->m_usIndex = pstRequestData->m_usIndex;
	pstStatData->m_usMarketType = pstCodeInfo->m_usMarketType;
	pstStatData->m_usType = pstRequestData->m_usType;
	memcpy(pstStatData->m_acName, pQuotePrice->szName, NAME_LEN);
	memcpy(pstStatData->m_acCode, pstRequestData->m_pstCode->m_acCode, CODE_LEN);
	memcpy(pstStatData->m_ggpj, pstRequestData->m_pstCode->m_ggpj, sizeof(pstRequestData->m_pstCode->m_ggpj));
	
	SRealUnit*	pstRealUnit = (SRealUnit*)(pstStatData + 1);
	pstRealUnit->m_puiData = (unsigned int*)(pstRealUnit + 1);
	unsigned int*	puiData = pstRealUnit->m_puiData;

	pstStatData->m_uiPrevClose = pQuotePrice->nZrsp;

	unsigned int uiIndex = 0;
	if ((pstCodeInfo->m_usMarketType & 0x0f) != ST_Index)
	{
		puiData[uiIndex++] = pQuotePrice->nZd;
		puiData[uiIndex++] = pQuotePrice->nZdf;
		puiData[uiIndex++] = pQuotePrice->nZrsp;
		puiData[uiIndex++] = pQuotePrice->nJrkp;
		puiData[uiIndex++] = pQuotePrice->nZgjg;

		puiData[uiIndex++] = pQuotePrice->nZdjg;
		puiData[uiIndex++] = pQuotePrice->nZtjg;
		puiData[uiIndex++] = pQuotePrice->nDtjg;
		puiData[uiIndex++] = pQuotePrice->nPjjg;
		puiData[uiIndex++] = pQuotePrice->nCjsl;

		puiData[uiIndex++] = pQuotePrice->nWb;
		puiData[uiIndex++] = pQuotePrice->nLb;
		puiData[uiIndex++] = pQuotePrice->nCjje*100; 
		puiData[uiIndex++] = pQuotePrice->nNp;

		puiData[uiIndex++] = pQuotePrice->nWp;
		puiData[uiIndex++] = pQuotePrice->nSyl1;
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = pQuotePrice->nSjw5;
		puiData[uiIndex++] = pQuotePrice->nSsl5;

		puiData[uiIndex++] = pQuotePrice->nSjw4;
		puiData[uiIndex++] = pQuotePrice->nSsl4;
		puiData[uiIndex++] = pQuotePrice->nSjw3;
		puiData[uiIndex++] = pQuotePrice->nSsl3;
		puiData[uiIndex++] = pQuotePrice->nSjw2;

		puiData[uiIndex++] = pQuotePrice->nSsl2;
		puiData[uiIndex++] = pQuotePrice->nSjw1;
		puiData[uiIndex++] = pQuotePrice->nSsl1;
		puiData[uiIndex++] = pQuotePrice->nZjjg;
		puiData[uiIndex++] = pQuotePrice->nZjcj;

		puiData[uiIndex++] = pQuotePrice->nBjw1;
		puiData[uiIndex++] = pQuotePrice->nBsl1;
		puiData[uiIndex++] = pQuotePrice->nBjw2;
		puiData[uiIndex++] = pQuotePrice->nBsl2;
		puiData[uiIndex++] = pQuotePrice->nBjw3;

		puiData[uiIndex++] = pQuotePrice->nBsl3;
		puiData[uiIndex++] = pQuotePrice->nBjw4;
		puiData[uiIndex++] = pQuotePrice->nBsl4;
		puiData[uiIndex++] = pQuotePrice->nBjw5;
		puiData[uiIndex++] = pQuotePrice->nBsl5;
	}
	else   //指数
	{
		// 将A股,B股,基金的成交金额提取出来
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = 0;

		puiData[uiIndex++] = pQuotePrice->nZjjg;
		puiData[uiIndex++] = pQuotePrice->nZd;
		puiData[uiIndex++] = pQuotePrice->nZdf;

		puiData[uiIndex++] = pQuotePrice->nCjje*10; 
		puiData[uiIndex++] = pQuotePrice->nCjsl;
		puiData[uiIndex++] = pQuotePrice->nZgjg;
		puiData[uiIndex++] = pQuotePrice->nZdjg;

		puiData[uiIndex++] = pQuotePrice->nBsl1; //?上涨和下跌
		puiData[uiIndex++] = pQuotePrice->nBsl3;
	}
	pstStatData->m_stRealUnit = *pstRealUnit;
	uiSize = pstStatData->HostToNet((unsigned char*)pAllSendBuf);
	////////////////////////////////////////////////////////////////////////////////////////
	memcpy(pAllSendBuf+uiSize, pSendBuf, nBufNeedSize);
	
	nLen = nAllSendBufSize;
	mpdel(pucUserDataBuf);
	mpdel(pSendBuf);
	mpdel(pCltData);
	return pAllSendBuf;
}

bool C2cc::OnRealMinsOverlapData(SMRequestData* pstMRequestData, unsigned char* pucData)
{
	SRequestData*	pstRequestData = (SRequestData*)pucData;
	pstRequestData->m_usType = pstMRequestData->m_usType;
	pstRequestData->m_usIndex = pstMRequestData->m_usIndex;
	pstRequestData->m_usMarketType = pstMRequestData->m_usMarketType;
	pstRequestData->m_usSize = pstMRequestData->m_usSize;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	SMCodeInfo*		pstMCodeInfo = (SMCodeInfo*)(pstMRequestData + 1);
		
	pstCodeInfo->m_usMarketType = pstMCodeInfo->m_usMarketType;
	memcpy(pstCodeInfo->m_acCode, pstMCodeInfo->m_acCode, CODE_LEN);
	pstCodeInfo->m_uiCode = atol(pstMCodeInfo->m_acCode);
	memcpy(pstCodeInfo->m_acName, pstMCodeInfo->m_acName, NAME_LEN);
	////////////////////////////////////////////////////////////////////////////////////////////
	int nCount = 0;
	char* pStockCode = NULL;
	bool bIsSpell = false;
	if (IsSpell(pstCodeInfo->m_acCode))  //如果是拼音简称
	{
		bIsSpell = true;
		pStockCode = g_StockInfo.GetStockCode(pstCodeInfo->m_acCode, nCount);
		if (pStockCode == NULL)
		{
			mpdel(pucData);
			return false;
		}
		if (nCount > 1) //发送代码列表
		{
			bool bRet = SendCodeList(pStockCode, nCount, pucData);
			mpdel(pStockCode);
			mpdel(pucData);
			return bRet;
		}
		else if (nCount <= 0)
		{
			mpdel(pStockCode);
			mpdel(pucData);
			return false;
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////
	if (bIsSpell)
	{
		if (pstCodeInfo->m_usMarketType == 0)
		{
			char szSymbol[16] = {0};
			memcpy(szSymbol, ((tagStockInfo*)pStockCode)->szCode, 16);
			GetMktType(szSymbol, pstCodeInfo->m_usMarketType);
			memcpy(pstCodeInfo->m_acCode, szSymbol, 6);
		}
	}
	else
	{
		if (pstCodeInfo->m_usMarketType == 0)
			pstCodeInfo->m_usMarketType = g_StockInfo.GetMktType((char*)pstCodeInfo->m_acCode);
	}

	struct _new_req
	{
		CommxHead head;
		unsigned short funcno1;
		unsigned short reqnum;
		
		unsigned short funcno2;		  //分时走势请求
		unsigned short symbol_num1;
		char symbol1[16];

		unsigned short funcno3;		  //报价表请求
		unsigned short symbol_num2;
		char symbol2[16];

		unsigned short funcno4;		  //大盘分时走势请求
		unsigned short symbol_num3;
		char symbol3[16];

		unsigned short funcno5;		  //大盘报价表请求
		unsigned short symbol_num4;
		char symbol4[16];
	} *preq;

	preq = (_new_req*)mpnew(sizeof(_new_req));
	memset(preq, 0x00, sizeof(_new_req));
	preq->head.Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	preq->head.SerialNo = onRequest(this);
	preq->head.Length = sizeof(_new_req) - sizeof(CommxHead);
	
	preq->funcno1 = preq->funcno2 = preq->funcno4 = PT_REAL_MINS;
	preq->reqnum = 4;
	preq->symbol_num1 = preq->symbol_num2 = preq->symbol_num3 = preq->symbol_num4 = 1;
	preq->funcno3 = preq->funcno5 = PT_QUOTEPRICE;

	InsertCltData(preq->head.SerialNo, (char*)pucData);

	if (bIsSpell)
	{
		memcpy(preq->symbol1, ((tagStockInfo*)pStockCode)->szCode, 16);
		mpdel(pStockCode);
	}
	else
		GetSymbol(pstCodeInfo, preq->symbol1);
	memcpy(preq->symbol2, preq->symbol1, 16);

	if ((pstCodeInfo->m_usMarketType & 0xf0f0) == 0x1000)      //上海
	{
		memcpy(preq->symbol3, "000001.SH", strlen("000001.SH"));
		memcpy(preq->symbol4, preq->symbol3, 16);
	}
	else if ((pstCodeInfo->m_usMarketType & 0xf0f0) == 0x1010) //深圳
	{
		memcpy(preq->symbol3, "399001.SZ", strlen("399001.SZ"));
		memcpy(preq->symbol4, preq->symbol3, 16);
	}

	CHECK_RUN(-1 == g_pcq->Write(preq, sizeof(_new_req)), MN, T("向行情服务器发送请求失败\n"), RPT_ERROR, return false);
	return true;
}

void C2cc::MakeStatData(char* pBuf, unsigned int& nLen, SRequestData* pstRequestData, tagQuotedPrice* pQuotePrice)
{
	unsigned int uiSize = ((SStatData*)pBuf)->GetSize(pstRequestData->m_usMarketType & 0x0f);

	char* pucUserDataBuf = (char*)mpnew(uiSize);
	if (pucUserDataBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", uiSize), RPT_CRITICAL);
		return;
	}

	SStatData*	pstStatData = (SStatData*)pucUserDataBuf;
	pstStatData->m_usIndex = pstRequestData->m_usIndex;
	pstStatData->m_usMarketType = pstRequestData->m_usMarketType;
	pstStatData->m_usType = pstRequestData->m_usType;
	memcpy(pstStatData->m_acName, pQuotePrice->szName, NAME_LEN);
	memcpy(pstStatData->m_acCode, pstRequestData->m_pstCode->m_acCode, CODE_LEN);
	memcpy(pstStatData->m_ggpj, pstRequestData->m_pstCode->m_ggpj, sizeof(pstRequestData->m_pstCode->m_ggpj));
	
	SRealUnit*	pstRealUnit = (SRealUnit*)(pstStatData + 1);
	pstRealUnit->m_puiData = (unsigned int*)(pstRealUnit + 1);
	unsigned int*	puiData = pstRealUnit->m_puiData;
	SCodeInfo*		pstCodeInfo = pstRequestData->m_pstCode;
	pstStatData->m_uiPrevClose = pQuotePrice->nZrsp;

	unsigned int uiIndex = 0;
	if ((pstCodeInfo->m_usMarketType & 0x0f) != ST_Index)
	{
		puiData[uiIndex++] = pQuotePrice->nZd;
		puiData[uiIndex++] = pQuotePrice->nZdf;
		puiData[uiIndex++] = pQuotePrice->nZrsp;
		puiData[uiIndex++] = pQuotePrice->nJrkp;
		puiData[uiIndex++] = pQuotePrice->nZgjg;

		puiData[uiIndex++] = pQuotePrice->nZdjg;
		puiData[uiIndex++] = pQuotePrice->nZtjg;
		puiData[uiIndex++] = pQuotePrice->nDtjg;
		puiData[uiIndex++] = pQuotePrice->nPjjg;
		puiData[uiIndex++] = pQuotePrice->nCjsl;

		puiData[uiIndex++] = pQuotePrice->nWb;
		puiData[uiIndex++] = pQuotePrice->nLb;
		puiData[uiIndex++] = pQuotePrice->nCjje*100; 
		puiData[uiIndex++] = pQuotePrice->nNp;

		puiData[uiIndex++] = pQuotePrice->nWp;
		puiData[uiIndex++] = pQuotePrice->nSyl1;
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = pQuotePrice->nSjw5;
		puiData[uiIndex++] = pQuotePrice->nSsl5;

		puiData[uiIndex++] = pQuotePrice->nSjw4;
		puiData[uiIndex++] = pQuotePrice->nSsl4;
		puiData[uiIndex++] = pQuotePrice->nSjw3;
		puiData[uiIndex++] = pQuotePrice->nSsl3;
		puiData[uiIndex++] = pQuotePrice->nSjw2;

		puiData[uiIndex++] = pQuotePrice->nSsl2;
		puiData[uiIndex++] = pQuotePrice->nSjw1;
		puiData[uiIndex++] = pQuotePrice->nSsl1;
		puiData[uiIndex++] = pQuotePrice->nZjjg;
		puiData[uiIndex++] = pQuotePrice->nZjcj;

		puiData[uiIndex++] = pQuotePrice->nBjw1;
		puiData[uiIndex++] = pQuotePrice->nBsl1;
		puiData[uiIndex++] = pQuotePrice->nBjw2;
		puiData[uiIndex++] = pQuotePrice->nBsl2;
		puiData[uiIndex++] = pQuotePrice->nBjw3;

		puiData[uiIndex++] = pQuotePrice->nBsl3;
		puiData[uiIndex++] = pQuotePrice->nBjw4;
		puiData[uiIndex++] = pQuotePrice->nBsl4;
		puiData[uiIndex++] = pQuotePrice->nBjw5;
		puiData[uiIndex++] = pQuotePrice->nBsl5;
	}
	else   //指数
	{
		// 将A股,B股,基金的成交金额提取出来
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = 0;
		puiData[uiIndex++] = 0;

		puiData[uiIndex++] = pQuotePrice->nZjjg;
		puiData[uiIndex++] = pQuotePrice->nZd;
		puiData[uiIndex++] = pQuotePrice->nZdf;

		puiData[uiIndex++] = pQuotePrice->nCjje*10; 
		puiData[uiIndex++] = pQuotePrice->nCjsl;
		puiData[uiIndex++] = pQuotePrice->nZgjg;
		puiData[uiIndex++] = pQuotePrice->nZdjg;

		puiData[uiIndex++] = pQuotePrice->nBsl1; //?上涨和下跌
		puiData[uiIndex++] = pQuotePrice->nBsl3;
	}
	pstStatData->m_stRealUnit = *pstRealUnit;
	uiSize = pstStatData->HostToNet((unsigned char*)pBuf);
	nLen = uiSize;
	mpdel(pucUserDataBuf);
}

void C2cc::MakeOverlapIndex(char* pBuf, unsigned int& nLen, unsigned int nRecords, char* pMinsBuf, 
							SRequestData* pstRequestData, tagQuotedPrice* pQuotePrice)
{
	unsigned int uiSize = ((SINDEXOVERLAP*)pBuf)->GetSize(nRecords);
	char* pucUserDataBuf = (char*)mpnew(uiSize);
	if (pucUserDataBuf == NULL)
	{
		REPORT(MN, T("mpnew() failed size=%d\n", uiSize), RPT_CRITICAL);
		return;
	}

	SINDEXOVERLAP*	pstRealMinsData = (SINDEXOVERLAP*)pucUserDataBuf;
	pstRealMinsData->m_uiClose = pQuotePrice->nZrsp;
	pstRealMinsData->m_ucUnits = nRecords;
 	pstRealMinsData->m_pstMinsUnit = (SRealMinsUnit*)(((char*)pstRealMinsData)+5);

	SRealMinsUnit*	pstRealMinsUnit = (SRealMinsUnit*)(((char*)pstRealMinsData)+5);
	memcpy(pstRealMinsUnit, pMinsBuf, nRecords * sizeof(SRealMinsUnit));

	unsigned short usType = pstRequestData->m_usType & 0xff00;
	uiSize = pstRealMinsData->HostToNet((unsigned char*)pBuf, usType);

	nLen = uiSize;
	mpdel(pucUserDataBuf);
}

char* C2cc::AtRealMinsOverlapData(char *lpData, unsigned int &nLen, unsigned short serialno)
{
	char pucDataBuf[1024*300];
	unsigned int uiSize = 0;
	unsigned int nLength = 0;

	/////////////////////////////////////////////////////////////////////////////////////
	char* pCltData = RemoveCltData(serialno);
	if (pCltData == NULL)
		return NULL;
	
	SRequestData*	pstRequestData = (SRequestData*)pCltData;
	SCodeInfo*		pstCodeInfo = (SCodeInfo*)(pstRequestData + 1);
	pstRequestData->m_pstCode = pstCodeInfo;

	char* pBufPos  = lpData + 8;
	unsigned short* pStockCount = (unsigned short*)(lpData+6);
	if (*pStockCount <= 0)
	{
		mpdel(pCltData);
		return NULL;
	}

	char *lpSymbol = pBufPos;		 //证券代码
	pBufPos += 16;
	unsigned short nMinsCount = 0;   //分钟数
	memcpy(&nMinsCount, pBufPos, sizeof(unsigned short));
	pBufPos += sizeof(unsigned short);

	int nMinsBufSize = nMinsCount * sizeof(SRealMinsUnit);
	char* pMinsBuf = NULL;
	if (nMinsCount > 0)
	{
		pMinsBuf = (char*)mpnew(nMinsBufSize);
		if (pMinsBuf == NULL)
		{
			REPORT(MN, T("mpnew() failed size=%d\n", nMinsBufSize), RPT_CRITICAL);
			mpdel(pCltData);
			return NULL;
		}
		memset(pMinsBuf, 0, nMinsBufSize);
		char* pMinsBufPos = pMinsBuf;

		for (unsigned int i=0; i<nMinsCount; i++)
		{
			tagRealMins* pRealMinsNew = (tagRealMins*)pBufPos;
			pBufPos += sizeof(tagRealMins);

			SRealMinsUnit* pMinsUnit = (SRealMinsUnit*)pMinsBufPos;
			pMinsBufPos += sizeof(SRealMinsUnit);

			pMinsUnit->m_uiNewPrice = pRealMinsNew->nNewPrice;
			pMinsUnit->m_uiVolume   = pRealMinsNew->nVolume;
			pMinsUnit->m_usTime     = (pRealMinsNew->nTime/60)*100 + (pRealMinsNew->nTime%60);
			pMinsUnit->m_uiAverage  = pRealMinsNew->AvgPrice;
		}
	}
/////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned short* pPackType = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pPackType != PT_QUOTEPRICE)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}
	pStockCount = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pStockCount != 1)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}
	tagQuotedPrice* pQuotePrice = (tagQuotedPrice*)pBufPos;
	pBufPos += sizeof(tagQuotedPrice);
	//////////////////////////////////////////////////////////////////////////////////////
	pPackType = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pPackType != PT_REAL_MINS)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}

	pStockCount = (unsigned short*)pBufPos;
	if (*pStockCount <= 0)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		mpdel(pCltData);
		return NULL;
	}
	pBufPos += sizeof(unsigned short);
	char *lpIdxSymbol = pBufPos;		 //证券代码
	pBufPos += 16;
	unsigned short nIdxMinsCount = 0;   //分钟数
	memcpy(&nIdxMinsCount, pBufPos, sizeof(unsigned short));
	pBufPos += sizeof(unsigned short);

	int nIdxMinsBufSize = nIdxMinsCount * sizeof(SRealMinsUnit);
	char* pIdxMinsBuf = NULL;
	if (nIdxMinsCount > 0)
	{
		pIdxMinsBuf = (char*)mpnew(nIdxMinsBufSize);
		if (pIdxMinsBuf == NULL)
		{
			REPORT(MN, T("mpnew() failed size=%d\n", nIdxMinsBufSize), RPT_CRITICAL);
			mpdel(pCltData);
			return NULL;
		}
		memset(pIdxMinsBuf, 0, nIdxMinsBufSize);
		char* pIdxMinsBufPos = pIdxMinsBuf;

		for (unsigned int i=0; i<nIdxMinsCount; i++)
		{
			tagRealMins* pRealMinsNew = (tagRealMins*)pBufPos;
			pBufPos += sizeof(tagRealMins);

			SRealMinsUnit* pMinsUnit = (SRealMinsUnit*)pIdxMinsBufPos;
			pIdxMinsBufPos += sizeof(SRealMinsUnit);

			pMinsUnit->m_uiNewPrice = pRealMinsNew->nNewPrice;
			pMinsUnit->m_uiVolume   = pRealMinsNew->nVolume;
			pMinsUnit->m_usTime     = (pRealMinsNew->nTime/60)*100 + (pRealMinsNew->nTime%60);
			pMinsUnit->m_uiAverage  = pRealMinsNew->AvgPrice;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////

	unsigned short* pIdxPackType = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pIdxPackType != PT_QUOTEPRICE)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		if (pIdxMinsBuf != NULL)
			mpdel(pIdxMinsBuf);
		mpdel(pCltData);
		return NULL;
	}
	pStockCount = (unsigned short*)pBufPos;
	pBufPos += sizeof(unsigned short);
	if (*pStockCount != 1)
	{
		if (pMinsBuf != NULL)
			mpdel(pMinsBuf);
		if (pIdxMinsBuf != NULL)
			mpdel(pIdxMinsBuf);
		mpdel(pCltData);
		return NULL;
	}
	tagQuotedPrice* pIdxQuotePrice = (tagQuotedPrice*)pBufPos;
	/////////////////////////////////////////////////////////////////////////////////////////

	MakeStatData(pucDataBuf, uiSize, pstRequestData, pQuotePrice);

	pstRequestData->m_usType = NEWREALMIN;
	//发送数据到旧的客户端
	SRealMinsData realMins = {0};
	realMins.m_ucStructSize = sizeof(SRealMinsData);
	realMins.m_usType = pstRequestData->m_usType;
	realMins.m_usIndex=	pstRequestData->m_usIndex;
	realMins.m_usMarketType = pstRequestData->m_pstCode->m_usMarketType;
	memcpy(realMins.m_acName, pQuotePrice->szName, NAME_LEN);
	memcpy(realMins.m_acCode, pstRequestData->m_pstCode->m_acCode, 6);
	realMins.m_usMinsUnits = nMinsCount;
	realMins.m_pstMinsUnit = (SRealMinsUnit *)pMinsBuf;
	realMins.m_uiPrevClose = pQuotePrice->nZrsp;

	unsigned short	usTime;
	if (pstRequestData->m_pstCode->m_usMarketType == HK_Stock)
	{
		usTime = 600;//开市时间 第一次
		realMins.m_usFirstOpen = usTime/60*100 + usTime%60;

		usTime = 750;//闭市时间 第一次
		realMins.m_usFirstClose = usTime/60*100 + usTime%60;

		usTime =  870;//开市时间 第二次
		realMins.m_usSecondOpen = usTime/60*100 + usTime%60;

		usTime =  960;//闭市时间 第二次
		realMins.m_usSecondClose = usTime/60*100 + usTime%60;
	}
	else
	{
		// add by ljz 2004/06/22  ???
		usTime = 570;//开市时间 第一次
		realMins.m_usFirstOpen = usTime/60*100 + usTime%60;

		usTime = 690;//闭市时间 第一次
		realMins.m_usFirstClose = usTime/60*100 + usTime%60;

		usTime =  780;//开市时间 第二次
		realMins.m_usSecondOpen = usTime/60*100 + usTime%60;

		usTime =  900;//闭市时间 第二次
		realMins.m_usSecondClose = usTime/60*100 + usTime%60;
		// add by ljz 2004/06/22 end
	}

	nLength = realMins.HostToNet((unsigned char*)pucDataBuf+uiSize, NEWREALMIN);
	uiSize += nLength;

	if (pMinsBuf != NULL)
		mpdel(pMinsBuf);

	pstRequestData->m_usType = REALMINOVERLAP;
	/////////////////////////////////////////////////////////////////////////////////////////
	if ((pstRequestData->m_pstCode->m_usMarketType & 0xf000) == 0x1000 &&
		(pstRequestData->m_pstCode->m_usMarketType & 0x000f) != 0x0001)
	{
		if ((pstRequestData->m_pstCode->m_usMarketType & 0xf0f0) == 0x1000)
		{
			pstRequestData->m_pstCode->m_usMarketType = 4097;
			pstRequestData->m_pstCode->m_uiCode = 1;
		}
		else if ((pstRequestData->m_pstCode->m_usMarketType & 0xf0f0) == 0x1010)
		{
			pstRequestData->m_pstCode->m_usMarketType = 4113;
			pstRequestData->m_pstCode->m_uiCode = 399001;
		}
		
		MakeOverlapIndex(pucDataBuf+uiSize, nLength, nIdxMinsCount, pIdxMinsBuf, pstRequestData, pIdxQuotePrice);
		uiSize += nLength;
	}
	else
	{
		*(unsigned short*)(pucDataBuf+1) = reverse_s(0x0b00);
	}
	if (pIdxMinsBuf != NULL)
		mpdel(pIdxMinsBuf);
	////////////////////////////////////////////////////////////////////////////////////////
	unsigned int nSendBufSize = uiSize;
	char* pSendBuf = (char*)mpnew(nSendBufSize);
	memcpy(pSendBuf, pucDataBuf, uiSize);
	
	nLen = nSendBufSize;

	mpdel(pCltData);
	return pSendBuf;
}

void C2cc::Answer(char *lpData, unsigned int nLength, unsigned short serialno)
{
	unsigned short funcno = 0;
	m_lpBuffer = NULL;
	if (nLength > 0)
	{
		if (serialno == 0)	  //发送代码列表
		{
			m_lpBuffer = lpData;
		}
		else
		{
			funcno = *(unsigned short*)lpData;
			switch (funcno)
			{//todo: 参照AtSymbolStatus增加各个应答协议的处理
			case PT_REAL_MINS:  //新分时协议
				{
					if (m_usType == REALMINSTAT)
						m_lpBuffer = AtStatData(lpData, nLength, serialno);
					else if (m_usType == REALMINOVERLAP)
						m_lpBuffer = AtRealMinsOverlapData(lpData, nLength, serialno);
					else
						m_lpBuffer = AtRealMinsData(lpData, nLength, serialno);
					break; 
				}
			case PT_QUOTEPRICE:  //自选股报价
				{
					if (m_usType == STATDATA)
						m_lpBuffer = AtReportStatData(lpData, nLength, serialno);
					else
						m_lpBuffer = AtReportData(lpData, nLength, serialno);  
					break;
				}
			case PT_HKDATA_DAY:   //历史K线数据
			case PT_HKDATA_WEEK:
			case PT_HKDATA_MONTH:
				{
					m_lpBuffer = AtHisKData(lpData, nLength, serialno);
					break;
				}
			case PT_SORTDATA:	//排行榜
				{
					m_lpBuffer = AtSortData(lpData, nLength, serialno);
					break;
				}
			default:
				{
					REPORT(MN, T("无法处理的报文类型0x%04x\n", funcno), RPT_WARNING);
				}
			}
		}
	}
	if (m_lpBuffer == NULL) //没有数据
	{
		REPORT(MN, T("没有有效的数据\n"), RPT_WARNING);
		nLength = 6;
		m_lpBuffer = (char*)mpnew(6);
		unsigned long nValue1 = 0x00000A2C;
		unsigned short nValue2 = 0x0002;
		memcpy(m_lpBuffer, &nValue1, sizeof(unsigned long));
		memcpy(m_lpBuffer + sizeof(unsigned long), &nValue2, sizeof(unsigned short));
	}

	//发送http包头和协议包头

	int nPos = sprintf(m_szHead, "HTTP/1.1 200 OK, Success\r\nContent-length: %d\r\ncontent-type: audio/mp3;charset=UTF-8\r\ncontent-source : caishentong\r\n\r\n", nLength+8);
	unsigned int *lpSize = (unsigned int*)(&m_szHead[nPos]);
	*lpSize = htonl(nLength);

	int nsize = strlen(m_szHead);
	if (-1 == Write(m_szHead, nPos + 8))
		return;

	//发送应答数据
	Write(m_lpBuffer, nLength);
}

void C2cc::SendErrorMsg(char* pMsg)
{
	//Close();
}

void C2cc::GetSymbol(unsigned short m_usType, char* pRecvBuf)
{
	switch (m_usType)
	{
	case 4097:
		strcpy_s(pRecvBuf, 1024, "md_shzs@");
		break;
	case 4098:
		strcpy_s(pRecvBuf, 1024, "md_shgz|md_shqz|md_shzz|md_shhg@");
		break;
	case 4099:
		strcpy_s(pRecvBuf, 1024, "md_shjj@");
		break;
	case 4100:
		strcpy_s(pRecvBuf, 1024, "md_shag@");
		break;
	case 4101:
		strcpy_s(pRecvBuf, 1024, "md_shbg@");
		break;
	case 4102:
		strcpy_s(pRecvBuf, 1024, "md_shetf|md_shlof|md_shwa@");
		break;
	case 4113:
		strcpy_s(pRecvBuf, 1024, "md_szzs@");
		break;
	case 4114:
		strcpy_s(pRecvBuf, 1024, "md_szgz|md_szqz|md_szzz|md_szhg@");
		break;
	case 4115:
		strcpy_s(pRecvBuf, 1024, "md_szjj@");
		break;
	case 4116:
		strcpy_s(pRecvBuf, 1024, "md_szag@");
		break;
	case 4117:
		strcpy_s(pRecvBuf, 1024, "md_szbg@");
		break;
	case 4118:
		strcpy_s(pRecvBuf, 1024, "md_szetf|md_szlof|md_szwa@");
		break;
	}
}

void C2cc::GetField(unsigned short m_usType, char* pRecvBuf)
{
	switch (m_usType & 0xff)
	{
	case RISE_SORT:     //涨幅排行榜
		strcpy_s(pRecvBuf, 512, "zdf|code|name|zrsp|zjjg|cjsl|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|lb|wb");
		break;
	case SUM_SORT:      //成交金额排行榜
		strcpy_s(pRecvBuf, 512, "cjje|code|name|zrsp|zjjg|cjsl|zjcj|np|wp|jrkp|zgjg|zdjg|lb|wb|zdf");
		break;
	case VOLUME_SORT:   //成交量排行榜
		strcpy_s(pRecvBuf, 512, "cjsl|code|name|zrsp|zjjg|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|lb|wb|zdf");
		break;
	case SWING_SORT:    //振幅排行榜
		strcpy_s(pRecvBuf, 512, "zf|code|name|zrsp|zjjg|cjsl|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|lb|wb|zdf");
		break;
	case VOLRATIO_SORT: //量比排行榜
		strcpy_s(pRecvBuf, 512, "lb|code|name|zrsp|zjjg|cjsl|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|wb|zdf");
		break;
	case BUYSELL_SORT:  //委比排行
		strcpy_s(pRecvBuf, 512, "wb|code|name|zrsp|zjjg|cjsl|zjcj|cjje|np|wp|jrkp|zgjg|zdjg|lb|zdf");
		break;
	}
}

unsigned short C2cc::GetCodeMarketType(char* pSymbol, SRequestData* pstRequestData)
{
	char* pMty = strstr(pSymbol, ".");
	pMty[0] = 0;
	pMty += 1;

	for(int i=0; i<pstRequestData->m_usSize; i++)
	{
		//pstRequestData->m_pstCode->NetToHost();

		if (_stricmp(pstRequestData->m_pstCode[i].m_acCode, pSymbol) == 0)
		{
			if (strcmp(pMty, "sz") == 0)
			{
				if (pstRequestData->m_pstCode[i].m_usMarketType > 4113)
					return pstRequestData->m_pstCode[i].m_usMarketType;
			}
			else if (strcmp(pMty, "SZ") == 0)
			{
				if (pstRequestData->m_pstCode[i].m_usMarketType == 4113)
					return pstRequestData->m_pstCode[i].m_usMarketType;
			}
			else if (strcmp(pMty, "sh") == 0)
			{
				if (pstRequestData->m_pstCode[i].m_usMarketType != 4097 && pstRequestData->m_pstCode[i].m_usMarketType < 4110)
					return pstRequestData->m_pstCode[i].m_usMarketType;
			}
			else if (strcmp(pMty, "SH") == 0)
			{
				if (pstRequestData->m_pstCode[i].m_usMarketType == 4097)
					return pstRequestData->m_pstCode[i].m_usMarketType;
			}
		}
	}

	return 0;
}

int C2cc::handle_write(char *lpData, unsigned int nLength, bool bSendOK)
{
	if (m_lpBuffer == lpData)
	{
		g_lockStat.lock();
		g_nSendCount++;
		g_nCurSendCount++;
		g_lockStat.unlock();

		mpdel(lpData);
	}

	return 0;
}

int C2cc::handle_timeout(ITimerCallbackBase *pTimerParam)
{
	REPORT(MN, T("handle_timeout()\n"), RPT_INFO);
	return 0;
}

int C2cc::handle_close(int iError)
{
	REPORT(MN, T("客户[%s]连接将关闭[Error:%d]\n", GetPeerAddress(), iError), RPT_INFO);
	return -1;
}

void C2cc::handle_release()
{
	onRelease(this);
	REPORT(MN, T("客户[%s]连接关闭\n", GetPeerAddress()), RPT_INFO);
	m_pcs->OnClientDisconnect(this);
	delete this;
}

//////////////////////////////////////////////////////////////////////////////////
char* C2q::GetRecvBuffer(int readID)
{
	if (0 == readID)
		return (char*)&m_head;
	else
		return m_szBuffer;
}

unsigned int C2q::GetRecvBufSize(int readID)
{
	if (0 == readID)
		return sizeof(CommxHead);
	else
		return sizeof(m_szBuffer);
}

int C2q::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	REPORT(MN, T("行情服务器[%s]连接已建立\n", GetPeerAddress()), RPT_INFO);
	RegisterTimer(&m_TimerParam, 0.0f, 60);

	// 收commx包头
	m_nReadID = 0;
	CHECK_RUN(-1 == Read(0, sizeof(CommxHead)), MN, T("读取CommxHead失败\n"), RPT_ERROR, return -1);

	return ANSWER_LENGTH;
}

int C2q::handle_read(char *lpBuffer, unsigned int nLength)
{
	if (0 == m_nReadID)
	{
		m_nReadID = 1;
		return Read(1, m_head.Length);
	}
	else
	{//已经收到完整的应答包
		unsigned short funcno = *(unsigned short*)m_szBuffer;
		if (funcno == PT_INIT && m_head.SerialNo == 0)   //股票初始化信息
		{
			g_StockInfo.Answer(m_szBuffer, m_head.Length);
		}
		else if (funcno != 0 && m_head.SerialNo != 0)
		{
			C2cc *pClient = onAnswer(m_head.SerialNo);
			try
			{
				if (pClient != NULL)
					pClient->Answer(m_szBuffer, m_head.Length, m_head.SerialNo);
			}
			catch (...)
			{
			}
		}
		m_nReadID = 0;
		CHECK_RUN(-1 == Read(0, sizeof(CommxHead)), MN, T("读取CommxHead失败\n"), RPT_ERROR, return -1);
	}
	return 0;
}

int C2q::handle_write(char *lpData, unsigned int nLength, bool bSendOK)
{
	mpdel(lpData);
	return 0;
}

int C2q::handle_timeout(ITimerCallbackBase *pTimerParam)
{
	if (pTimerParam->m_nTimerID == m_TimerParam.m_nTimerID)
	{
		g_lockStat.lock();
		if (g_nPeakCount < g_nCurSendCount)
			g_nPeakCount = g_nCurSendCount;

		REPORT(MN, T("接收数据包:[%d] 发送数据包:[%d] 共接收数据包:[%d] 共发送数据包:[%d] 峰值:[%d]\n",
			g_nCurRecvCount, g_nCurSendCount, g_nRecvCount, g_nSendCount, g_nPeakCount), RPT_ADDI_INFO|RPT_IGNORE);

		g_nCurRecvCount = 0;
		g_nCurSendCount = 0;
		g_lockStat.unlock();
		//REPORT(MN, T("HeartBeat\n"), RPT_IGNORE);
		g_StockInfo.UpdateStockInfo();
	}
	return 0;
}

int C2q::handle_close(int iError)
{//todo: 要求吴青不得主动断开这一条连接
	REPORT(MN, T("与行情服务器的连接断开\n"), RPT_INFO);
	CancelTimer(m_TimerParam.m_nTimerID);
	memset(&m_TimerParam, 0, sizeof(m_TimerParam));

	Connect(strQuoteAddress.c_str());

	//todo: 要测试当行情服务器主动断开的时候，程序是否会主动重连
	return 0;
}

void C2q::handle_release()
{
	delete this;
}
