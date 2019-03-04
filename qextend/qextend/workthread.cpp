
#include "stdafx.h"
#include "qextend.h"
#include "workthread.h"
#include <algorithm>
#include <assert.h>
#include <WinSock2.h>
#include "../../public/commx/highperformancecounter.h"

using std::make_pair;

#pragma warning(disable:4267)

////////////////////////////////////////////////////////////////////////////////////////////
int _symbol_zjjg::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	Quote *pq = m_pFarm->GetQuote(pri->idxQuote);
	zdf = pq->zdf;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
unsigned int GetQuote(Quote *pq, const char *col)
{
	if (!strcmp(col, "zrsp"))
		return pq->zrsp;
	else if (!strcmp(col, "jrkp"))
		return pq->jrkp;
	else if (!strcmp(col, "ztjg"))
		return pq->ztjg;
	else if (!strcmp(col, "dtjg"))
		return pq->dtjg;
	else if (!strcmp(col, "syl1"))
		return pq->syl1;
	else if (!strcmp(col, "syl2"))
		return pq->syl2;
	else if (!strcmp(col, "zgjg"))
		return pq->zgjg;
	else if (!strcmp(col, "zdjg"))
		return pq->zdjg;
	else if (!strcmp(col, "zjjg"))
		return pq->zjjg;
	else if (!strcmp(col, "zjcj"))
		return pq->zjcj;
	else if (!strcmp(col, "cjsl"))
		return pq->cjsl;
	else if (!strcmp(col, "cjje"))
		return pq->cjje;
	else if (!strcmp(col, "cjbs"))
		return pq->cjbs;
	else if (!strcmp(col, "bp1"))
		return pq->BP1;
	else if (!strcmp(col, "bm1"))
		return pq->BM1;
	else if (!strcmp(col, "bp2"))
		return pq->BP2;
	else if (!strcmp(col, "bm2"))
		return pq->BM2;
	else if (!strcmp(col, "bp3"))
		return pq->BP3;
	else if (!strcmp(col, "bm3"))
		return pq->BM3;
	else if (!strcmp(col, "bp4"))
		return pq->BP4;
	else if (!strcmp(col, "bm4"))
		return pq->BM4;
	else if (!strcmp(col, "bp5"))
		return pq->BP5;
	else if (!strcmp(col, "bm5"))
		return pq->BM5;
	else if (!strcmp(col, "sp1"))
		return pq->SP1;
	else if (!strcmp(col, "sm1"))
		return pq->SM1;
	else if (!strcmp(col, "sp2"))
		return pq->SP2;
	else if (!strcmp(col, "sm2"))
		return pq->SM2;
	else if (!strcmp(col, "sp3"))
		return pq->SP3;
	else if (!strcmp(col, "sm3"))
		return pq->SM3;
	else if (!strcmp(col, "sp4"))
		return pq->SP4;
	else if (!strcmp(col, "sm4"))
		return pq->SM4;
	else if (!strcmp(col, "sp5"))
		return pq->SP5;
	else if (!strcmp(col, "sm5"))
		return pq->SM5;
	else if (!strcmp(col, "day5pjzs"))
		return pq->day5pjzs;
	else if (!strcmp(col, "pjjg"))
		return pq->pjjg;
	else if (!strcmp(col, "wb"))
		return pq->wb;
	else if (!strcmp(col, "lb"))
		return pq->lb;
	else if (!strcmp(col, "np"))
		return pq->np;
	else if (!strcmp(col, "wp"))
		return pq->wp;
	else if (!strcmp(col, "zdf"))
		return pq->zdf;
	else if (!strcmp(col, "zf"))
		return pq->zf;
	else
		REPORT_RUN(MN, T("无效的行情请求列[%s]\n", col), RPT_ERROR, return 0);
}

////////////////////////////////////////////////////////////////////////////////////////////
_find_symbol::_find_symbol(CDataFarm *pFarm)
{
	m_pFarm = pFarm;
	m_pvSymbol = NULL;
	m_symbol_data = NULL;
}

int _find_symbol::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	memcpy(&m_symbol_data->pq, m_pFarm->GetQuote(pri->idxQuote), sizeof(Quote));
	return 0;
}

int _find_symbol::OnElement(std::string *pSymbol, RINDEX *pIdx)
{
	_symbol_data sd;
	memcpy(&sd.pq, m_pFarm->GetQuote(pIdx->idxQuote), sizeof(Quote));
	if (strlen(sd.pq.szStockKind) && m_strGroup.find(sd.pq.szStockKind) != std::string::npos)
	{
		sd.symbol = sd.pq.szStockCode;
		m_pvSymbol->push_back(sd);
		return 1;
	}
	return 0;
}

void _find_symbol::operator()(_symbol_data &sd)
{
	m_symbol_data = &sd;
	m_pFarm->m_hashIdx.cast(sd.symbol, *this, true);
}

////////////////////////////////////////////////////////////////////////////////////////////
less_symbol::less_symbol(std::string &sortcol, int ascend, CDataFarm *pFarm)
: sort_colname(sortcol), nAscending(ascend), m_pFarm(pFarm)
{}

bool less_symbol::operator()(_symbol_data &s1, _symbol_data &s2)
{
	assert(m_pFarm);
	if (sort_colname == "gxd")
	{
		if (0 == nAscending)
			return s1.reserved1 < s2.reserved1;
		else
			return s1.reserved1 > s2.reserved1;
	}
	else if (sort_colname == "zdf" || sort_colname == "wb")
	{
		if (0 == nAscending)
			return (int)(GetQuote(&s1.pq, sort_colname.c_str())) < (int)(GetQuote(&s2.pq, sort_colname.c_str()));
		else
			return (int)(GetQuote(&s1.pq, sort_colname.c_str())) > (int)(GetQuote(&s2.pq, sort_colname.c_str()));
	}
	else
	{
		if (0 == nAscending)
			return GetQuote(&s1.pq, sort_colname.c_str()) < GetQuote(&s2.pq, sort_colname.c_str());
		else
			return GetQuote(&s1.pq, sort_colname.c_str()) > GetQuote(&s2.pq, sort_colname.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////
index_stat::index_stat(CDataFarm *pFarm, unsigned int *u, unsigned int *d, unsigned int *e)
: up(u), dn(d), eq(e), m_pFarm(pFarm)
{
	*up = 0;
	*dn = 0;
	*eq = 0;
}

void index_stat::operator()(_symbol_data &s)
{
	_find_symbol fs(m_pFarm);
	fs.m_symbol_data = &s;
	m_pFarm->m_hashIdx.cast(s.symbol, fs, true);

	if (s.pq.zdf > 0)
		(*up)++;
	else if (s.pq.zdf < 0)
		(*dn)++;
	else
		(*eq)++;
}

////////////////////////////////////////////////////////////////////////////////////////////
_remove_symbol::_remove_symbol(const char *lpColName)
{
	sort_col = lpColName;
}

bool _remove_symbol::operator()(_symbol_data &sd)
{
	if (sort_col == "gxd" || sort_col == "zdf" || sort_col == "zf")
		return false;

	if (0 == GetQuote(&sd.pq, sort_col.c_str()))
	{
		if (sort_col == "zjjg" && GetQuote(&sd.pq, "jrkp"))
		{
			sd.pq.zjjg = GetQuote(&sd.pq, "jrkp");
			return false;
		}
		else if (sort_col == "zjjg" && GetQuote(&sd.pq, "zrsp"))
		{
			sd.pq.zjjg = GetQuote(&sd.pq, "zrsp");
			return false;
		}
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////
CWorkThread::CWorkThread()
{
	m_nUpdateDbDate = 0;
}

void CWorkThread::close()
{
	memset(&m_head, 0x00, sizeof(CommxHead));
	gQueue.Push(&m_head);
}

char* CWorkThread::Malloc(unsigned int nSize, CommxHead *pHead)
{
	unsigned short *pFunc = (unsigned short*)(pHead + 1);
//	nSize += (sizeof(CommxHead) + 3 * sizeof(unsigned short));
	nSize += (sizeof(CommxHead) + 1 * sizeof(unsigned short));
	char *lpData = (char*)mpnew(nSize);

	if (lpData)
	{
		nSize -= sizeof(CommxHead);
		memcpy(lpData, pHead, sizeof(CommxHead));
		CommxHead *pAnsHead = (CommxHead*)lpData;
		unsigned short *pShort = (unsigned short*)(pAnsHead + 1);

		if (CommxHead::NETORDER == pHead->GetByteorder())
		{
			pShort[0] = htons(*pFunc);
// 			pShort[0] = pShort[2] = htons(*pFunc);
// 			pShort[1] = htons(1);
			pAnsHead->Length = htonl(nSize);
		}
		else
		{
			pShort[0] = *pFunc;
// 			pShort[0] = pShort[2] = *pFunc;
// 			pShort[1] = 1;
			pAnsHead->Length = nSize;
		}
	}
//	return lpData + sizeof(CommxHead) + 3 * sizeof(unsigned short);
	return lpData + sizeof(CommxHead) + 1 * sizeof(unsigned short);
}

void CWorkThread::UpdateDatabase(bool bForce)
{
	std::map<unsigned short, bool>::iterator iter = cfg.m_mpBusinessNo.find(6002);
	if (iter == cfg.m_mpBusinessNo.end() || !iter->second)
		return;

	time_t now;
	time(&now);
	struct tm *lt = localtime(&now);
	unsigned int nday = (lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday;
	unsigned int ntime = lt->tm_hour*100+lt->tm_min;

	if (bForce || (nday != m_nUpdateDbDate && ntime >= 900))
	{
		m_nUpdateDbDate = nday;

		WGUARD(LockRW, m_lock_zixun, grd1);
		WGUARD(LockRW, m_lock_vocation, grd2);
		WGUARD(LockRW, m_lock_vocsymbol, grd3);

		m_zixun.clear();
		m_vocation.clear();
		m_vocsymbol.clear();

		VisitMarketValue();
		VisitComponent();
		VisitVocation();

		REPORT_RUN(MN, T("结束数据库更新\n"), RPT_INFO, return);
	}
}

unsigned int CWorkThread::svc(void* lpParam)
{
	CommxHead *pHead = NULL;
	int timeout = INFINITE;
	m_dwPreTick = GetTickCount();

	UpdateDatabase(true);

#ifdef _DEBUG
	CHighPerformanceCounter hpc;
#endif

	if (theApp.m_nMode)
	{
		timeout = 1000;
		CHECK_RUN(!m_farm.mapfile(FILE_INDEX, cfg.m_nMarketCount, cfg.m_nSymbolCount / 2, cfg.m_nSymbolCount, cfg.m_nTickCount, cfg.m_nMinkCount),
			MN, T("映射行情数据文件%s失败\n", FILE_INDEX), RPT_ERROR, return -1);
	}

	DWORD dwNow = GetTickCount();
	while (1)
	{
		int nSize = gQueue.Pop(pHead, timeout);
		if (-1 != nSize)
		{
			if (pHead->Prop == 0 && pHead->Feed == 0 && pHead->Length == 0 && pHead->SerialNo == 0)
				break;

			char *pAnswer = NULL;
			char *lpData = (char*)pHead + sizeof(CommxHead);
			unsigned short *pFuncNo = (unsigned short*)lpData;
			if (CommxHead::NETORDER == pHead->GetByteorder())
			{
				*pFuncNo = ntohs(*pFuncNo);
				pHead->Length = ntohl(pHead->Length);
			}

#ifdef _DEBUG
			hpc.count();
#endif

			REPORT(MN, T("Func:%d\n", *pFuncNo), RPT_INFO);
			switch (*pFuncNo)
			{
			case 6000://请求标准排行榜
			case 6002://请求指数贡献度或行业排行
				pAnswer = do_sort(pHead);
				break;
			case 6001://请求大盘统计数据
				pAnswer = do_6001(pHead);
				break;
			case 6003:/** 请求单个行业涨跌幅 [7/17/2009 xinl] */
				pAnswer = do_6003(pHead);
				break;
			case 6004://请求证券品种所包含的所有symbol代码
				pAnswer = do_6004(pHead);
				break;
			default:
				REPORT(MN, T("错误的业务代码[%d]\n", *pFuncNo), RPT_WARNING);
				break;
			}

			DEBUG_REPORT(MN, T("Func:%d, Queue:%d, Time:%f\n", *pFuncNo, nSize, hpc.count()), RPT_INFO);
			REPORT(MN, T("Func:%d OK.\n", *pFuncNo), RPT_INFO);

			if (pAnswer)
			{
// 				pAnswer -= (sizeof(CommxHead) + 3 * sizeof(unsigned short));
				pAnswer -= (sizeof(CommxHead) + 1 * sizeof(unsigned short));
				unsigned int msgid = *(unsigned int*)((char*)pHead + sizeof(CommxHead) + pHead->Length + 1);
				theApp.m_onBusiness(msgid, (CommxHead*)pAnswer);
				mpdel(pAnswer);
				REPORT(MN, T("Func:%d returned.\n", *pFuncNo), RPT_INFO);
			}

			mpdel(pHead);
		}

		OnTimer(dwNow);
	}

	return 0;
}

void CWorkThread::OnTimer(DWORD dwNow)
{
	UpdateDatabase(false);
	DWORD dwCurTick = GetTickCount();
	if (dwCurTick - m_dwPreTick >= (DWORD)cfg.m_nDevTimeout * 1000)
	{
		REPORT(MN, T(""), RPT_HEARTBEAT|RPT_IGNORE);
		m_dwPreTick = dwCurTick;
	}
}

char* CWorkThread::do_6004(CommxHead *pHead)
{
	char *lpData = (char*)(pHead+1)+sizeof(unsigned short);
	unsigned int nLen = pHead->Length - sizeof(unsigned short);
	if (CommxHead::UTF16LE == pHead->GetEncoding())
		ICONV(CONV_UTF16LE, lpData, nLen, CONV_GBK, lpData, nLen);

	std::vector<_symbol_data> vSymbol;
	SelSymbols(lpData, nLen, vSymbol);

	std::string s;
	for (unsigned int i = 0; i < vSymbol.size(); i++)
		s += (vSymbol[i].symbol + "|");

	if (CommxHead::UTF16LE == pHead->GetEncoding())
		nLen = s.length() * 2;
	else
		nLen = s.length();

	char *pAnswer = Malloc(nLen, pHead);
	CHECK_RUN(!pAnswer, MN, T("内存[%d]分配失败\n", nLen), RPT_CRITICAL, return NULL);

	if (CommxHead::UTF16LE == pHead->GetEncoding())
		ICONV(CONV_GBK, s.c_str(), s.length(), CONV_UTF16LE, pAnswer, s.length() * 2);
	else
		memcpy(pAnswer, s.c_str(), s.length());

	return pAnswer;
}

char* CWorkThread::do_6003(CommxHead *pHead)
{
	char *lpVocationCode = (char*)(pHead+1)+sizeof(unsigned short);

	if (CommxHead::UTF16LE == pHead->GetEncoding())
		ICONV(CONV_UTF16LE, lpVocationCode, pHead->Length-sizeof(unsigned short), CONV_GBK, lpVocationCode, pHead->Length-sizeof(unsigned short));

	MAPVOCATION::iterator iterb1 = m_vocation.begin();
	for (; iterb1 != m_vocation.end(); ++iterb1)
	{
		if (iterb1->second.strVocationCode == lpVocationCode)
			break;
	}

	CHECK_RUN(iterb1 == m_vocation.end(), MN, T("行业代码[%s]不存在\n", lpVocationCode),
		RPT_WARNING, return MakeError(pHead, "行业代码不存在"));

	// 获得行业代码
	_symbol_data sd;
	sd.reserved2 = 0.0f;
	sd.symbol = lpVocationCode;
	strcpy(sd.pq.szStockCode, sd.symbol.c_str());
 	strcpy(sd.pq.szStockName, iterb1->second.strVocationName.c_str());

	// 计算行业的当前流通市值
	MAPVOCATIONSYMBOL::iterator iterb2 = m_vocsymbol.lower_bound(sd.symbol);
	MAPVOCATIONSYMBOL::iterator itere2 = m_vocsymbol.upper_bound(sd.symbol);
	for (; iterb2 != itere2; ++iterb2)
	{// 获得行业所属symbol
		_symbol_zjjg sz;
		sz.m_pFarm = &m_farm;
		m_farm.m_hashIdx.cast(iterb2->second, sz);

		// 获得symbol流通市值
		MAPZIXUN::iterator iter = m_zixun.find(iterb2->second);
		if (iter != m_zixun.end())
			sd.reserved2 += (iter->second.fFlowValue*sz.zdf);
	}

	// 计算行业的当前涨跌幅
	double fVocationValue = iterb1->second.fMarketValue;
	char szTmp[1024];
	sprintf(szTmp, "%.2f", fVocationValue);

	unsigned int nSize = sizeof(int);
	if (CommxHead::ANSI == pHead->GetEncoding())
		nSize += (strlen(szTmp) + 16);
	else
		nSize += (2 * strlen(szTmp) + 32);

	char *lpData = Malloc(nSize, pHead);
	CHECK_RUN(!lpData, MN, T("分配内存失败\n"), RPT_CRITICAL, return NULL);
	memset(lpData, 0x00, nSize);
	char *lpAnswer = lpData;

	/** 行业名称 [7/22/2009 xinl] */
	if (CommxHead::ANSI == pHead->GetEncoding())
	{
		strcpy(lpData, sd.pq.szStockName);
		lpData += 16;
	}
	else if (CommxHead::UTF16LE == pHead->GetEncoding())
	{
		ICONV(CONV_GBK, sd.pq.szStockName, strlen(sd.pq.szStockName), CONV_UTF16LE, lpData, 32);
		lpData += 32;
	}

	/** 行业涨跌幅 [7/22/2009 xinl] */
	int *zdf = (int*)lpData;
	lpData += sizeof(int);
	*zdf = (int)(sd.reserved2/fVocationValue);
	if (CommxHead::NETORDER == pHead->GetByteorder())
		*zdf = htonl(*zdf);

	/** 行业市值 [7/22/2009 xinl] */
	if (CommxHead::ANSI == pHead->GetEncoding())
		memcpy(lpData, szTmp, strlen(szTmp));
	else if (CommxHead::UTF16LE == pHead->GetEncoding())
		ICONV(CONV_GBK, szTmp, strlen(szTmp), CONV_UTF16LE, lpData, 2*strlen(szTmp));

	return lpAnswer;
}

char* CWorkThread::do_sort(CommxHead *pHead)
{
	char *lpData = (char*)(pHead+1)+sizeof(unsigned short);

	unsigned char bOrder = lpData[0];
	lpData++;
	unsigned int nBegin = *(unsigned int*)lpData;
	if (CommxHead::NETORDER == pHead->GetByteorder())
		nBegin = ntohl(nBegin);
	lpData += sizeof(unsigned int);
	unsigned int nCount = *(unsigned int*)lpData;
	if (CommxHead::NETORDER == pHead->GetByteorder())
		nCount = ntohl(nCount);
	lpData += sizeof(unsigned int);

	if (CommxHead::UTF16LE == pHead->GetEncoding())
	{
		unsigned int offset = sizeof(unsigned short) + sizeof(char) + sizeof(unsigned int ) * 2;
		ICONV(CONV_UTF16LE, lpData, pHead->Length - offset, CONV_GBK, lpData, pHead->Length - offset);
	}

	// 解析参加排序的symbol集合定义
	char *symbol_kinds = strtok(lpData, "@");
	CHECK_RUN(!symbol_kinds, MN, T("无排序集合定义\n"), RPT_WARNING, return MakeError(pHead, "无排序集合定义"));
	lpData += (strlen(symbol_kinds)+1);

	// 解析返回的数据列
	std::vector<std::string> vCol;
	std::vector<_symbol_data> vSymbol;
	char *token = strtok(lpData, "|");
	while (token && (token < ((char*)pHead) + sizeof(CommxHead) + pHead->Length))
	{
		vCol.push_back(strlwr(token));
		token = strtok(NULL, "|");
	}

	CHECK_RUN(!vCol.size(), MN, T("无排序列定义\n"), RPT_WARNING, return MakeError(pHead, "无排序列定义"));
	CHECK_RUN((vCol[0] == "code" || vCol[0] == "name"), MN, T("排序列不能为%s\n", vCol[0].c_str()),
		RPT_WARNING, return MakeError(pHead, T("排序列不能为%s\n", vCol[0].c_str()).c_str()));

	if (vCol.size())
	{
		char cOrginOrder = bOrder;
		if (!strncmp(symbol_kinds, "vocation", 8))	// 三级行业、四级行业按涨跌幅排序
			CalcVocation(symbol_kinds, vSymbol);
		else if (!strncmp(symbol_kinds, "cfg_", 4))	// 查询指数的成份股
			SelGxd(symbol_kinds, vSymbol);
		else										// 标准的排行榜，取排序symbol集合的行情
			SelSymbols(symbol_kinds, strlen(symbol_kinds), vSymbol);

		if (nBegin >= vSymbol.size())				//不需要排序，直接返回空集合
			return MakeSort(pHead, vSymbol, vCol);

		//去掉排序数据为0的symbol
		std::vector<_symbol_data>::iterator send = std::remove_if(vSymbol.begin(), vSymbol.end(), _remove_symbol(vCol[0].c_str()));
		vSymbol.erase(send, vSymbol.end());
		if (vSymbol.size() == 0)
			return MakeSort(pHead, vSymbol, vCol);

		if (nBegin + nCount > vSymbol.size())		//返回部分集合
			nCount = (unsigned int)(vSymbol.size() - nBegin);

		//优化排序范围
		if (nBegin >= vSymbol.size() / 2)
		{
			bOrder = !bOrder;
			nBegin = (unsigned int)(vSymbol.size() - nBegin - nCount);
		}

		//排序并产生符合查询条件的symbol集合
		less_symbol lessObj(vCol[0], bOrder, &m_farm);
		std::partial_sort(vSymbol.begin(), vSymbol.begin()+nBegin+nCount, vSymbol.end(), lessObj);
		vSymbol = std::vector<_symbol_data>(vSymbol.begin()+nBegin, vSymbol.begin()+nBegin+nCount);
		if (cOrginOrder != bOrder)
			std::reverse(vSymbol.begin(), vSymbol.end());
	}

	return MakeSort(pHead, vSymbol, vCol);
}

char* CWorkThread::do_6001(CommxHead *pHead)
{
	char *lpData = (char*)(pHead+1)+sizeof(unsigned short);

	char szReqSymbol[STOCK_CODE_LEN*2];
	memset(szReqSymbol, 0x00, sizeof(szReqSymbol));
	memcpy(szReqSymbol, lpData, STOCK_CODE_LEN*2);
	if (CommxHead::UTF16LE == pHead->GetEncoding())
		ICONV(CONV_UTF16LE, szReqSymbol, STOCK_CODE_LEN*2, CONV_GBK, szReqSymbol, STOCK_CODE_LEN*2);

	unsigned int up, dn, eq;
	std::vector<_symbol_data> vSymbol;
	const char *group_name = cfg.PrepareStatistics(szReqSymbol);
	SelSymbols(group_name, strlen(group_name), vSymbol);
	std::for_each(vSymbol.begin(), vSymbol.end(), index_stat(&m_farm, &up, &dn, &eq));

	char *pAnswer = (char*)Malloc(3 * sizeof(unsigned int), pHead);
	CHECK_RUN(!pAnswer, MN, T("分配内存失败\n"), RPT_CRITICAL, return NULL);

	unsigned int *pStat = (unsigned int*)pAnswer;
	pStat[0] = up;
	pStat[1] = dn;
	pStat[2] = eq;

	if (CommxHead::NETORDER == pHead->GetByteorder())
	{
		pStat[0] = htonl(pStat[0]);
		pStat[1] = htonl(pStat[1]);
		pStat[2] = htonl(pStat[2]);
	}

	return pAnswer;
}

unsigned int CWorkThread::GetPacketSize(std::vector<std::string> &vCol)
{
	unsigned int nSize = 0;
	for (unsigned int i = 0; i < vCol.size(); i++)
	{
		if (vCol[i] == "name" || vCol[i] == "code")
			nSize += STOCK_NAME_LEN * 2;
		else
			nSize += sizeof(unsigned int);
	}
	return nSize;
}

char* CWorkThread::MakeError(CommxHead *pHead, const char *lpMsg)
{
	unsigned short *pFunc = (unsigned short*)(pHead+1);
	*pFunc = -1;
	char *lpBuffer = NULL;
	if (CommxHead::UTF16LE == pHead->GetEncoding())
		lpBuffer = Malloc(2 * strlen(lpMsg), pHead);
	else
		lpBuffer = Malloc(strlen(lpMsg), pHead);
	CHECK_RUN(!lpBuffer, MN, T("分配内存失败:MsgLen=%d\n", strlen(lpMsg)), RPT_CRITICAL, return NULL);

	if (CommxHead::UTF16LE == pHead->GetEncoding())
		ICONV(CONV_GBK, lpMsg, strlen(lpMsg), CONV_UTF16LE, lpBuffer, 2*strlen(lpMsg));
	else
		strcpy(lpBuffer, lpMsg);

	return lpBuffer;
}

char* CWorkThread::MakeSort(CommxHead *pHead, std::vector<_symbol_data> &vSymbol, std::vector<std::string> &vCol)
{
	unsigned int nSize = GetPacketSize(vCol);
	char *lpData = Malloc((unsigned int)vSymbol.size() * nSize, pHead);
	CHECK_RUN(!lpData, MN, T("分配内存失败:nCount=%d,nSize=%d\n", vSymbol.size(), nSize), RPT_CRITICAL, return NULL);
	memset(lpData, 0x00, (unsigned int)vSymbol.size() * nSize);
	char *lpAnswer = lpData;

	for (unsigned int i = 0; i < vSymbol.size(); i++)
	{
		for (unsigned int j = 0; j < vCol.size(); j++)
		{
			if (vCol[j] == "code")
			{
				if (CommxHead::UTF16LE == pHead->GetEncoding())
					ICONV(CONV_GBK, vSymbol[i].pq.szStockCode, strlen(vSymbol[i].pq.szStockCode), CONV_UTF16LE, lpData, STOCK_CODE_LEN*2);
				else
					strcpy(lpData, vSymbol[i].pq.szStockCode);
				lpData += STOCK_CODE_LEN*2;
			}
			else if (vCol[j] == "name")
			{
				if (CommxHead::UTF16LE == pHead->GetEncoding())
					ICONV(CONV_GBK, vSymbol[i].pq.szStockName, strlen(vSymbol[i].pq.szStockName), CONV_UTF16LE, lpData, STOCK_CODE_LEN*2);
				else
					strcpy(lpData, vSymbol[i].pq.szStockName);
				lpData += STOCK_NAME_LEN*2;
			}
			else if (vCol[j] == "gxd")
			{
				if (CommxHead::NETORDER == pHead->GetByteorder())
					*(int*)lpData = htonl(vSymbol[i].reserved1);
				else
					*(int*)lpData = vSymbol[i].reserved1;
				lpData += sizeof(int);
			}
			else
			{
				if (CommxHead::NETORDER == pHead->GetByteorder())
					*(unsigned int*)lpData = htonl(GetQuote(&vSymbol[i].pq, vCol[j].c_str()));
				else
					*(unsigned int*)lpData = GetQuote(&vSymbol[i].pq, vCol[j].c_str());
				lpData += sizeof(unsigned int);
			}
		}
	}

	return lpAnswer;
}

bool CWorkThread::VisitComponent()
{
	REPORT(MN, T("取指数成份股\n"), RPT_INFO);
	CDblib mssql;
	unsigned int nSize = 0;
	bool bsql = mssql.Open(cfg.m_component_sql.strDbAddress.c_str(),
		cfg.m_component_sql.strUser.c_str(),
		cfg.m_component_sql.strPassword.c_str());
	CHECK_RUN(!bsql, MN, T("打开数据库[%s]失败: %s::%s\n",
		cfg.m_component_sql.strDbAddress.c_str(), cfg.m_component_sql.strUser.c_str(), cfg.m_component_sql.strPassword.c_str()),
		RPT_ERROR, return false);

	mssql.Use(cfg.m_component_sql.strDbName.c_str());
	CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});

	//沪深300
	bsql = mssql.Run(cfg.m_component_sql.vSql[0].c_str());
	CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});

	while (mssql.MoveNext())
	{
		std::string scode = mssql.GetColData(0);
		if (scode[0] == '6')
			scode += ".sh";
		else
			scode += ".sz";
		nSize++;
		m_vocsymbol.insert(make_pair("cfg_hs300", scode));
	}
	REPORT(MN, T("沪深300共有%d支成份股\n", nSize), RPT_INFO);
	nSize = 0;

	//深证成指
	bsql = mssql.Run(cfg.m_component_sql.vSql[1].c_str());
	CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});

	while (mssql.MoveNext())
	{
		std::string scode = mssql.GetColData(0);
		if (scode[0] == '6')
			scode += ".sh";
		else
			scode += ".sz";
		nSize++;
		m_vocsymbol.insert(make_pair("cfg_szcz", scode));
	}
	REPORT(MN, T("深证成指共有%d支成份股\n", nSize), RPT_INFO);
	nSize = 0;

	mssql.Close();

	// 更新指数的总市值和流通市值
	MAPVOCATIONSYMBOL::iterator iterb, itere;
	MAPZIXUN::iterator iter;
	_symbol_zixun _zixun;

	memset(&_zixun, 0x00, sizeof(_zixun));
	iterb = m_vocsymbol.lower_bound("cfg_szzs");
	itere = m_vocsymbol.upper_bound("cfg_szzs");
	for (; iterb != itere; ++iterb)
	{
		iter = m_zixun.find(iterb->second);
		if (iter != m_zixun.end())
		{
			_zixun.fMarketValue += iter->second.fMarketValue;
			_zixun.fFlowValue += iter->second.fFlowValue;
		}
	}
	m_zixun["000001.SH"] = _zixun;
	REPORT(MN, T("000001.SH/市值%.2f/流通市值%.2f\n", _zixun.fMarketValue, _zixun.fFlowValue), RPT_INFO);

	memset(&_zixun, 0x00, sizeof(_zixun));
	iterb = m_vocsymbol.lower_bound("cfg_szcz");
	itere = m_vocsymbol.upper_bound("cfg_szcz");
	for (; iterb != itere; ++iterb)
	{
		iter = m_zixun.find(iterb->second);
		if (iter != m_zixun.end())
		{
			_zixun.fMarketValue += iter->second.fMarketValue;
			_zixun.fFlowValue += iter->second.fFlowValue;
		}
	}
	m_zixun["399001.SZ"] = _zixun;
	REPORT(MN, T("399001.SZ/市值%.2f/流通市值%.2f\n", _zixun.fMarketValue, _zixun.fFlowValue), RPT_INFO);

	memset(&_zixun, 0x00, sizeof(_zixun));
	iterb = m_vocsymbol.lower_bound("cfg_hs300");
	itere = m_vocsymbol.upper_bound("cfg_hs300");
	for (; iterb != itere; ++iterb)
	{
		iter = m_zixun.find(iterb->second);
		if (iter != m_zixun.end())
		{
			_zixun.fMarketValue += iter->second.fMarketValue;
			_zixun.fFlowValue += iter->second.fFlowValue;
		}
	}
	m_zixun["399300.SZ"] = _zixun;
	REPORT(MN, T("399300.SZ/市值%.2f/流通市值%.2f\n", _zixun.fMarketValue, _zixun.fFlowValue), RPT_INFO);

	return true;
}

bool CWorkThread::VisitMarketValue()
{
	REPORT(MN, T("查询个股总市值,流通市值,流通股\n"), RPT_INFO);
	CDblib mssql;
	bool bsql = mssql.Open(cfg.m_marketvalue_sql.strDbAddress.c_str(),
		cfg.m_marketvalue_sql.strUser.c_str(),
		cfg.m_marketvalue_sql.strPassword.c_str());
	CHECK_RUN(!bsql, MN, T("打开数据库[%s]失败: %s::%s\n",
		cfg.m_marketvalue_sql.strDbAddress.c_str(), cfg.m_marketvalue_sql.strUser.c_str(), cfg.m_marketvalue_sql.strPassword.c_str()),
		RPT_ERROR, return false);

	mssql.Use(cfg.m_marketvalue_sql.strDbName.c_str());
	CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});

	bsql = mssql.Run(cfg.m_marketvalue_sql.vSql[0].c_str());
	CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});

	while (mssql.MoveNext())
	{
		std::string code = mssql.GetColData(0);
		std::string ltg;
		if (code.rfind('A') != std::string::npos)
			ltg = mssql.GetColData(3);
		else
			ltg = mssql.GetColData(4);
		if (code[code.length()-2] == 'Z')
			code = code.substr(0, code.length()-2) + ".sz";
		else if (code[code.length()-2] == 'S')
		{
			code = code.substr(0, code.length()-2) + ".sh";
			m_vocsymbol.insert(make_pair("cfg_szzs", code));
		}
		else
			continue;
		std::string zsz = mssql.GetColData(1);
		std::string ltsz = mssql.GetColData(2);
		zsz = zsz.length() ? zsz : "0";
		ltsz = ltsz.length() ? ltsz : "0";
		ltg = ltg.length() ? ltg : "0";

		_symbol_zixun zixun;
		zixun.fFlowShare = atof(ltg.c_str());
		zixun.fFlowValue = atof(ltsz.c_str());;
		zixun.fMarketValue = atof(zsz.c_str());
		m_zixun[code] = zixun;
	}
	REPORT(MN, T("已更新%d条市值数据\n", m_zixun.size()), RPT_INFO);

	mssql.Close();
	return true;
}

bool CWorkThread::VisitVocation()
{
	REPORT(MN, T("查询行业数据\n"), RPT_INFO);
	CDblib mssql;
	bool bsql = mssql.Open(cfg.m_vocation_sql.strDbAddress.c_str(),
		cfg.m_vocation_sql.strUser.c_str(),
		cfg.m_vocation_sql.strPassword.c_str());
	CHECK_RUN(!bsql, MN, T("打开数据库[%s]失败: %s::%s\n",
		cfg.m_vocation_sql.strDbAddress.c_str(), cfg.m_vocation_sql.strUser.c_str(), cfg.m_vocation_sql.strPassword.c_str()),
		RPT_ERROR, return false);

	mssql.Use(cfg.m_vocation_sql.strDbName.c_str());
	CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});

	std::map<std::string, std::string> vocations;

	// 取三四级行业代码和行业名称
	bsql = mssql.Run(cfg.m_vocation_sql.vSql[0].c_str());
	CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});
	while (mssql.MoveNext())
	{
		std::string code = mssql.GetColData(0);
 		std::string name = mssql.GetColData(1);
		vocations[code] = name;
	}

 	char szSql[1024];

	// 取行业所属股票
	char *level = NULL;					/**< 行业级别 */
	std::map<std::string, std::string>::iterator iter1 = vocations.begin();
	for (; iter1 != vocations.end(); ++iter1)
	{
		if (iter1->first.length() == 8)
		{// 四级行业
			sprintf(szSql, cfg.m_vocation_sql.vSql[1].c_str(), "cmpt_cl", iter1->first.c_str());
		}
		else
		{// 三级行业
			int nCount = 0;
			for (unsigned int i = 0; i < cfg.m_vocation_sql.vSql[1].length(); i++)
			{
				if (cfg.m_vocation_sql.vSql[1][i] == '%')
					nCount+=1;
			}
			if (nCount == 2)
				sprintf(szSql, cfg.m_vocation_sql.vSql[1].c_str(), "indus3_cl", iter1->first.c_str());
			else
				sprintf(szSql, cfg.m_vocation_sql.vSql[1].c_str(), iter1->first.c_str());
		}

		bsql = mssql.Run(szSql);
		CHECK_RUN(!bsql, MN, T("%s\n", mssql.GetErrorMsg()), RPT_ERROR, {mssql.Close(); return false;});

		_vocation_list voc;
		voc.strVocationCode = iter1->first;
		voc.strVocationName = iter1->second;
		voc.fMarketValue = 0.0f;

		while (mssql.MoveNext())
		{
			std::string s;
			std::string scode = mssql.GetColData(0);
			if (scode.substr(0, 2) == "60")
				s = scode + ".sh";
			else if (scode.substr(0, 2) == "90")
				s = scode + ".sh";
			else if (scode.substr(0, 2) == "20")
				s = scode + ".sz";
			else
				s = scode + ".sz";
			m_vocsymbol.insert(make_pair(iter1->first, s));

			MAPZIXUN::iterator iter2 = m_zixun.find(s);
			if (iter2 != m_zixun.end())
				voc.fMarketValue += iter2->second.fFlowValue;
		}
		m_vocation.insert(make_pair(iter1->first.length() == 6 ? 3 : 4, voc));
	}

	mssql.Close();
	return true;
}

void CWorkThread::SelSymbols(const char *group, unsigned int nLength, std::vector<_symbol_data> &vSymbol)
{
	_find_symbol fs(&m_farm);
	fs.m_pvSymbol = &vSymbol;
	fs.m_strGroup.assign(group, nLength);
	m_farm.m_hashIdx.for_each(fs);
}

void CWorkThread::CalcVocation(const char *group, std::vector<_symbol_data> &vSymbol)
{
	int nLevel = group[strlen(group)-1]-'0';

	RGUARD(LockRW, m_lock_vocation, grd1);
	RGUARD(LockRW, m_lock_vocsymbol, grd2);
	RGUARD(LockRW, m_lock_zixun, grd3);

	MAPVOCATION::iterator iterb1 = m_vocation.lower_bound(nLevel);
	MAPVOCATION::iterator itere1 = m_vocation.upper_bound(nLevel);

	for (; iterb1 != itere1; ++iterb1)
	{
		// 获得行业代码
		_symbol_data sd;
		sd.reserved2 = 0.0f;
		sd.symbol = iterb1->second.strVocationCode;
		strcpy(sd.pq.szStockCode, sd.symbol.c_str());
		strcpy(sd.pq.szStockName, iterb1->second.strVocationName.c_str());

		// 计算行业的当前流通市值
		MAPVOCATIONSYMBOL::iterator iterb2 = m_vocsymbol.lower_bound(sd.symbol);
		MAPVOCATIONSYMBOL::iterator itere2 = m_vocsymbol.upper_bound(sd.symbol);
		for (; iterb2 != itere2; ++iterb2)
		{// 获得行业所属symbol
			_symbol_zjjg sz;
			sz.m_pFarm = &m_farm;
			m_farm.m_hashIdx.cast(iterb2->second, sz);

			// 获得symbol流通市值
			MAPZIXUN::iterator iter = m_zixun.find(iterb2->second);
			if (iter != m_zixun.end())
				sd.reserved2 += (iter->second.fFlowValue*sz.zdf);
		}

		// 计算行业的当前涨跌幅
		double fVocationValue = iterb1->second.fMarketValue;
		sd.pq.zdf = (int)(sd.reserved2/fVocationValue);
		vSymbol.push_back(sd);
	}
}

void CWorkThread::SelGxd(const char *group, std::vector<_symbol_data> &vSymbol)
{
	char *lpIndexSymbol;
	_symbol_data sdindex;
	_find_symbol fs(&m_farm);

	if (!strcmp(group, "cfg_szzs"))
		lpIndexSymbol = "000001.SH";
	else if (!strcmp(group, "cfg_hs300"))
		lpIndexSymbol = "399300.SZ";
	else
		lpIndexSymbol = "399001.SZ";

	/// 取指数最新价格
	sdindex.symbol = lpIndexSymbol;
	fs.m_symbol_data = &sdindex;
	memset(&sdindex.pq, 0x00, sizeof(Quote));
	m_farm.m_hashIdx.cast(sdindex.symbol, fs);

	RGUARD(LockRW, m_lock_vocsymbol, grd2);
	RGUARD(LockRW, m_lock_zixun, grd3);

	/// 获得指数流通市值/总市值
	double fIndexValue;
	MAPZIXUN::iterator iter = m_zixun.find(lpIndexSymbol);

	if (!strcmp(lpIndexSymbol, "000001.SH"))
		fIndexValue = iter->second.fMarketValue;
	else
		fIndexValue = iter->second.fFlowValue;

	/// 获得指数的成份股
	MAPVOCATIONSYMBOL::iterator iterb = m_vocsymbol.lower_bound(group);
	MAPVOCATIONSYMBOL::iterator itere = m_vocsymbol.upper_bound(group);

	for (; iterb != itere; ++iterb)
	{
		_symbol_data sd;
		sd.symbol = iterb->second;

		/// 获得symbol流通市值/总市值
		double fSymbolValue;
		iter = m_zixun.find(sd.symbol);
		if (iter == m_zixun.end())
		{
			REPORT(MN, T("%s成员%s不存在\n", group, sd.symbol.c_str()), RPT_WARNING);
			continue;
		}

		if (!strcmp(lpIndexSymbol, "000001.SH"))
			fSymbolValue = iter->second.fMarketValue;
		else
			fSymbolValue = iter->second.fFlowValue;


		/// 取symbol行情
 		sd.reserved2 = fSymbolValue/fIndexValue;
 		fs.m_symbol_data = &sd;
 		m_farm.m_hashIdx.cast(sd.symbol, fs);

 		if (fs.m_symbol_data->pq.zjjg)
		{
			sd.reserved1 = (int)(sd.reserved2*fs.m_symbol_data->pq.zdf/10000*sdindex.pq.zjjg);
			vSymbol.push_back(sd);
		}
	}
}
