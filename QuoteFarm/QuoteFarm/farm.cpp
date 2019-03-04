
#include "stdafx.h"
#include "farm.h"
#include "config.h"
#include "task.h"
#include "addin_manager.h"
#include "../../public/commx/mery.h"
#include <direct.h>
#include <algorithm>

#pragma warning(disable:4311 4312)

///////////////////////////////////////////////////////////////////////////////////////
void CDataFarm::_findpy::OnBucket(std::string pydm, PYNODE pyNode)
{
	symbols += pyNode.symbol;
	symbols += " ";
}

///////////////////////////////////////////////////////////////////////////////////////
CDataFarm::_maintain::_maintain()
: pFarm(NULL), pChkmkt(NULL), now(0)
{}

int CDataFarm::_maintain::CheckSymbol(const char *lpSymbol, RINDEX *pIndex)
{
	Quote *pq = pFarm->GetQuote(pIndex->idxQuote);
	if (pq->szStockKind[0])
	{
		CHECK_RUN(!pq->szStockName[0], MN, T("SelfCheck: %s 名称错误\n", lpSymbol), RPT_WARNING, return 1);
		CHECK_RUN(!pq->zrsp, MN, T("SelfCheck: %s 昨收为0\n", lpSymbol), RPT_WARNING, return 1);
	}

	int nCount = 0;
	unsigned int nTime = 0;

	MinBlock *pmb = pFarm->GetMinK(pIndex->idxMinK);
	while (nCount < pIndex->cntMinK)
	{
		MinUnit *pmu = &pmb->unit[nCount % MINK_PERBLOCK];
		if (pmu->Time > nTime)
			nTime = pmu->Time;
		else
		{
			REPORT(MN, T("SelfCheck: %s 分时走势时间错误[%d-%d]\n", lpSymbol, nTime, pmu->Time), RPT_WARNING);
			return 1;
		}
		if (++nCount % MINK_PERBLOCK == 0 && pmb->next)
			pmb = pFarm->GetMinK(pmb->next);
	}

	nTime = 0;
	nCount = 0;
	TickBlock *ptb = pFarm->GetTick(pIndex->idxTick);
	while (nCount < pIndex->cntTick)
	{
		TickUnit *ptu = &ptb->unit[nCount % TICK_PERBLOCK];
		if (ptu->Time >= nTime)
			nTime = ptu->Time;
		else
		{
			REPORT(MN, T("SelfCheck: %s 分笔成交时间错误[%d-%d]\n", lpSymbol, nTime, ptu->Time), RPT_WARNING);
			return 1;
		}
		if (++nCount % TICK_PERBLOCK == 0 && ptb->next)
			ptb = pFarm->GetTick(ptb->next);
	}

	return 0;
}

int CDataFarm::_maintain::OnElement(std::string *pSymbol, RINDEX *pIdx)
{
	char *pMkt = strlwr(strdup(pSymbol->substr(pSymbol->rfind(".")+1).c_str()));
	CConfig::CHKMKT::iterator iterCheck = pChkmkt->find(pMkt);
	if (iterCheck == pChkmkt->end())
	{
		free(pMkt);
		return 0;	// 市场还未收盘
	}

	switch (atoi(iterCheck->second.c_str()))
	{
	case CConfig::CLOSED:
		CloseSymbol(pSymbol->c_str(), pIdx);
		break;
	case CConfig::OPENED:
		try
		{
			OpenSymbol(pSymbol->c_str(), pIdx);
		}
		catch (...)
		{
			REPORT(MN, T("%s开盘错误！\n", pSymbol->c_str()), RPT_WARNING);
		}
		break;
	case CConfig::CHECK:
		free(pMkt);
		return CheckSymbol(pSymbol->c_str(), pIdx);
	default:
		break;
	}
	free(pMkt);
	return 1;
}

void CDataFarm::_maintain::OpenSymbol(const char *lpSymbol, RINDEX *pIndex)
{
	pIndex->cntMinK = 0;
	pIndex->cntTick = 0;

	Quote *pq = pFarm->GetQuote(pIndex->idxQuote);
	memset(((char*)pq)+QUOTEHEAD+sizeof(unsigned int), 0x00, sizeof(Quote)-QUOTEHEAD-QUOTETAIL-sizeof(unsigned int));
	const char *lpMktCode = strchr(lpSymbol, '.') + 1;
	if (islower(lpMktCode[0]))	//指数不计算五日平均总手
		pq->day5pjzs = pFarm->Get5DayVol(lpSymbol);

	/** 回收所有多余的内存块，只给每只symbol留下一个必要块 [7/17/2009 xinl] */
	if (pFarm->GetMinK(pIndex->idxMinK)->next)
	{
		unsigned int idx = pIndex->idxMinK;
		while (pFarm->GetMinK(idx)->next)
			idx = pFarm->GetMinK(idx)->next;
		pFarm->GetMinK(idx)->next = pFarm->m_pIndexHead->nMinkIndex;
		pFarm->m_pIndexHead->nMinkIndex = pFarm->GetMinK(pIndex->idxMinK)->next;
	}

	if (pFarm->GetTick(pIndex->idxTick)->next)
	{
		unsigned int idx = pIndex->idxTick;
		while (pFarm->GetTick(idx)->next)
			idx = pFarm->GetTick(idx)->next;
		pFarm->GetTick(idx)->next = pFarm->m_pIndexHead->nTickIndex;
		pFarm->m_pIndexHead->nTickIndex = pFarm->GetTick(pIndex->idxTick)->next;
	}

	MinBlock *pm = pFarm->GetMinK(pIndex->idxMinK);
	memset(pm, 0x00, sizeof(MinBlock));

	TickBlock *pt = pFarm->GetTick(pIndex->idxTick);
	memset(pt, 0x00, sizeof(TickBlock));
}

void CDataFarm::_maintain::CloseSymbol(std::vector<RINDEX*> vri, unsigned int &cjsl, unsigned int &cjje)
{
	cjsl = cjje = 0;
	for (unsigned int i = 0; i < vri.size(); i++)
	{
		Quote *pq = pFarm->GetQuote(vri[i]->idxQuote);
		cjsl += pq->cjsl;
		cjje += pq->cjje;
	}
}

void CDataFarm::_maintain::CloseSymbol(const char *lpSymbol, RINDEX *pIndex)
{
	/** 当日没有交易，不收盘 [7/14/2009 xinl] */
	if (pIndex->cntTick == 0)
		return;

	Quote *pq = pFarm->GetQuote(pIndex->idxQuote);
	if (pq->zjjg == 0 && pq->zjcj == 0)
		return;
	if (pq->cjsl == 0)
		return;

	dayk knew;
	strcpy(knew.szStockCode, lpSymbol);
	knew.day = g_cfg.GetMarketDay(now, strchr(lpSymbol, '.') + 1);
	knew.open = pq->jrkp;
	knew.high = pq->zgjg;
	knew.low = pq->zdjg;
	knew.close = pq->zjjg;

	REPORT(MN, T("[%s] 收盘日 [%d]\n", knew.szStockCode, knew.day), RPT_WARNING);

	SPECIALITER iter = g_mapCloseSpecial.find(lpSymbol);
	if (iter != g_mapCloseSpecial.end())
		CloseSymbol(iter->second, knew.volume, knew.amount);
	else
	{
		knew.volume = pq->cjsl;
		knew.amount = pq->cjje;
	}

	string sfn = (char*)strchr(lpSymbol, '.') + 1;
	sfn += ".close";
	string line = kdb_addkline(knew);
	FILE *fp = fopen(sfn.c_str(), "a+");
	fwrite(line.c_str(), 1, line.length(), fp);	
	fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////////////
CDataFarm::CDataFarm()
{
	m_pIndexHead = NULL;
	m_pMarketStatus = NULL;
	m_nFarmOpenDate = 0;
}

CDataFarm::~CDataFarm()
{
#ifdef _WIN32
	UnmapViewOfFile(m_lpData);
	CloseHandle(m_hMappingFile);
	CloseHandle(m_hFile);
#elif defined _POSIX_C_SOURCE
#endif
}

bool CDataFarm::mapfile(const char* lpszFileName, unsigned int nMarketCount, unsigned int nBuckets, unsigned int nSymbols, unsigned int nTickCount, unsigned int nMinkCount)
{
	IndexHead tmpIdxhead;
	memset(&tmpIdxhead, 0x00, sizeof(tmpIdxhead));
	tmpIdxhead.Version = 0x01;
	tmpIdxhead.nBucketCount = (unsigned short)GetNearPrime(nBuckets);
	tmpIdxhead.nSymbolCount = nSymbols;
	tmpIdxhead.nTickCount = nTickCount;
	tmpIdxhead.nMinkCount = nMinkCount;
	tmpIdxhead.nQuoteIndex = 0;
	tmpIdxhead.nTickIndex = 0;
	tmpIdxhead.nMinkIndex = 0;
	unsigned long nHashIndexSize = HASHIDX::GetSize(0, tmpIdxhead.nBucketCount, tmpIdxhead.nSymbolCount);
	unsigned long nHashPySize = HASHPY::GetSize(0, tmpIdxhead.nBucketCount, tmpIdxhead.nSymbolCount);

	unsigned int nSize = sizeof(IndexHead) + nMarketCount * sizeof(MarketStatus) + nHashIndexSize + nHashPySize;
	tmpIdxhead.nIndexOffset = sizeof(tmpIdxhead) + nMarketCount * sizeof(MarketStatus);
	tmpIdxhead.nPyOffset = tmpIdxhead.nIndexOffset + nHashIndexSize;

#ifdef _WIN32
	m_hFile = CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	m_hMappingFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, nSize, NULL);
	m_lpData = (char*)MapViewOfFile(m_hMappingFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#elif defined _POSIX_C_SOURCE
	m_fd = open(lpszFileName, O_RDWR|O_CREAT);
	m_lpData = (char*)mmap(0, nSize, PROT_READ|PROT_READ, MAP_SHARED, m_fd, 0);
#endif

	m_pIndexHead = (IndexHead*)m_lpData;
	m_pMarketStatus = (MarketStatus*)(m_lpData + sizeof(IndexHead));
	if (m_lpData[0] == 0x00)
		memcpy(m_pIndexHead, &tmpIdxhead, sizeof(IndexHead));

	char szTmp[11], szIdx[21], szPy[21];
	m_hashIdx.malloc(m_pIndexHead->nBucketCount, m_pIndexHead->nSymbolCount);
	for (int i = 0; i < m_pIndexHead->nBucketCount; i++)
	{
		strcpy(szIdx, "qot_idx");
		m_hashIdx.m_pLock[i].Initialize(strcat(szIdx, itoa(i, szTmp, 10)));
	}
	m_hashIdx.mapmem(m_lpData + m_pIndexHead->nIndexOffset, NULL, 0);

	m_hashPy.malloc(m_pIndexHead->nBucketCount, m_pIndexHead->nSymbolCount);
	for (int i = 0; i < m_pIndexHead->nBucketCount; i++)
	{
		strcpy(szPy, "qot_py");
		m_hashPy.m_pLock[i].Initialize(strcat(szPy, itoa(i, szTmp, 10)));
	}
	m_hashPy.mapmem(m_lpData + m_pIndexHead->nPyOffset, NULL, 0);

	m_tbLevel0.mapfile(FILE_LEVEL0, m_pIndexHead->nSymbolCount);
	m_tbTick.mapfile(FILE_TICK, m_pIndexHead->nTickCount/TICK_PERBLOCK);
	m_tbMinK.mapfile(FILE_MINK, m_pIndexHead->nMinkCount);

// 	/** 初始化每个市场的昨收和与现价和 [7/31/2009 xinl] */
// 	_sum_of_index soi;
// 	soi.pFarm = this;
// 	m_hashIdx.for_each(soi);
	return true;
}

void CDataFarm::AddSymbolIndex(const char *lpSymbol, RINDEX *pIndex)
{
	/// 增加索引编码
	memset(pIndex, 0x00, sizeof(RINDEX));
	pIndex->idxQuote = GetNextQuote();
	pIndex->idxMinK = GetNextMink();
	pIndex->idxTick = GetNextTick();
	memset(GetQuote(pIndex->idxQuote), 0x00, sizeof(Quote));
	memset(GetMinK(pIndex->idxMinK), 0x00, sizeof(MinBlock));
	memset(GetTick(pIndex->idxTick), 0x00, sizeof(TickBlock));
}

void CDataFarm::AddPydm(const char *lpSymbol, const char *lpStockName)
{
	/// 增加拼音代码
	std::string spy = GetHzpy(lpStockName);
	if (spy.length())
	{
		PYNODE node;
		strcpy(node.symbol, lpSymbol);
		m_hashPy.AddElement(spy, node);
	}
}

void CDataFarm::DeleteSymbolIndex(const char *lpSymbol, RINDEX *pIndex)
{
	*(unsigned int*)m_tbLevel0[pIndex->idxQuote] = m_pIndexHead->nQuoteIndex;
	m_pIndexHead->nQuoteIndex = pIndex->idxQuote;

	unsigned int idx = pIndex->idxMinK;
	while (GetMinK(idx)->next)
		idx = GetMinK(idx)->next;
	GetMinK(idx)->next = m_pIndexHead->nMinkIndex;
	m_pIndexHead->nMinkIndex = pIndex->idxMinK;

	idx = pIndex->idxTick;
	while (GetTick(idx)->next)
		idx = GetTick(idx)->next;
	GetTick(idx)->next = m_pIndexHead->nTickIndex;
	m_pIndexHead->nTickIndex = pIndex->idxTick;

	m_hashPy.DelElement(lpSymbol);
	m_hashIdx.DelElement(lpSymbol);
}

void CDataFarm::AddTick(RINDEX *pIdx, TickUnit *ptick)
{
	unsigned int i = pIdx->cntTick / TICK_PERBLOCK;
	unsigned int j = pIdx->cntTick % TICK_PERBLOCK;
	unsigned int idx = pIdx->idxTick;
	TickBlock *ptb = GetTick(idx);
	for (unsigned int m = 0; m < i; m++)
	{
		ptb = GetTick(ptb->next);
		idx = ptb->next;
	}

	memcpy(&ptb->unit[j], ptick, sizeof(TickUnit));
	pIdx->cntTick++;

	if ((pIdx->cntTick % TICK_PERBLOCK) == 0)
	{
		ptb->next = GetNextTick();
		ptb = GetTick(ptb->next);
		memset(ptb, 0x00, sizeof(TickBlock));
	}
}

void CDataFarm::AddMink(RINDEX *pIdx, unsigned short now, unsigned int price, unsigned int volumn, unsigned int avg_price)
{
	unsigned int count = pIdx->cntMinK ? pIdx->cntMinK - 1 : 0;
	unsigned int i = count / MINK_PERBLOCK;
	unsigned int j = count % MINK_PERBLOCK;

	MinBlock *pmb = GetMinK(pIdx->idxMinK);
	for (unsigned int m = 0; m < i; m++)
		pmb = GetMinK(pmb->next);

	if (pmb->unit[j].Time == now)
	{// 同一分钟，直接改写最后一根k线
		pmb->unit[j].MaxPrice = max(pmb->unit[j].MaxPrice, price);
		pmb->unit[j].MinPrice = min(pmb->unit[j].MinPrice, price);
		pmb->unit[j].NewPrice = price;
		pmb->unit[j].Volume += volumn;
		pmb->unit[j].AvgPrice = avg_price;
	}
	else
	{// 和最后一根k线不是同一分钟，需要另开k线
		if (pmb->unit[j].Time != 0)
		{// 最后一根k线不是空k线，进入一个新开的MinkBlock会出现这种情况
			if (j != MINK_PERBLOCK - 1)
				j++;
			else
			{
				pmb->next = GetNextMink();
				pmb = GetMinK(pmb->next);
				memset(pmb, 0x00, sizeof(MinBlock));
				j = 0;
			}
		}
		pmb->unit[j].Time = now;
		pmb->unit[j].OpenPrice = pmb->unit[j].MaxPrice = pmb->unit[j].MinPrice = pmb->unit[j].NewPrice = price;
		pmb->unit[j].Volume = volumn;
		pmb->unit[j].AvgPrice = avg_price;

		// 增加k线的总数
		pIdx->cntMinK++;
	}
}

std::string CDataFarm::GetPySymbol(const char *lpPy)
{
	_findpy obj;
	m_hashPy.FindBucket(lpPy, obj);
	return obj.symbols;
}

unsigned int CDataFarm::GetNextQuote()
{
	unsigned int nr = m_pIndexHead->nQuoteIndex;
	unsigned int nIdx = *(unsigned int*)m_tbLevel0[m_pIndexHead->nQuoteIndex];
	if (nIdx == 0)
		m_pIndexHead->nQuoteIndex++;
	else
		m_pIndexHead->nQuoteIndex = nIdx;
	return nr;
}

unsigned int CDataFarm::GetNextTick()
{
	unsigned int nr = m_pIndexHead->nTickIndex;
	if (m_tbTick[m_pIndexHead->nTickIndex]->next == 0)
		m_pIndexHead->nTickIndex++;
	else
		m_pIndexHead->nTickIndex = m_tbTick[m_pIndexHead->nTickIndex]->next;
	return nr;
}

unsigned int CDataFarm::GetNextMink()
{
	unsigned int nr = m_pIndexHead->nMinkIndex;
	if (m_tbMinK[m_pIndexHead->nMinkIndex]->next == 0)
		m_pIndexHead->nMinkIndex++;
	else
		m_pIndexHead->nMinkIndex = m_tbMinK[m_pIndexHead->nMinkIndex]->next;
	return nr;
}

unsigned int CDataFarm::Get5DayVol(const char *lpSymbol)
{
	time_t now;
	time(&now);
	tm *lt = localtime(&now);
	return kdb_get5dayvol(lpSymbol, (lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday);
}

unsigned int CDataFarm::SeekEnd(FILE *fp, unsigned int nBlock)
{
	fseek(fp, 0, SEEK_END);
	unsigned int nSize = ftell(fp) / sizeof(dayk);
	nBlock = min(nSize, nBlock);
	nSize = ftell(fp) - nBlock * sizeof(dayk);
	fseek(fp, nSize, SEEK_SET);
	return nBlock;
}

Quote* CDataFarm::GetQuote(unsigned int idx)
{
	return m_tbLevel0[idx];
}

TickBlock* CDataFarm::GetTick(unsigned int idx)
{
	return m_tbTick[idx];
}

MinBlock* CDataFarm::GetMinK(unsigned int idx)
{
	return m_tbMinK[idx];
}

unsigned int CDataFarm::GetHashSize()
{
	return m_hashIdx.size();
}

std::string CDataFarm::GetHzpy(const char *lpSymbol)
{
	std::string s = lpSymbol;
	char *pTmp = strupr(strdup(converthztopy((char*)s.c_str()).c_str()));
	s = pTmp;
	free(pTmp);
	return s;
}

std::string CDataFarm::converthztopy(char *as_hzstring)
{
	std::string s = as_hzstring;
	std::string::size_type pos;
	while ((pos = s.find(' ')) != std::string::npos)
		s.erase(pos, 1);
	while ((pos = s.find('-')) != std::string::npos)
		s.erase(pos, 1);
	while ((pos = s.find('*')) != std::string::npos)
		s.erase(pos, 1);
	while ((pos = s.find("SS")) != std::string::npos)
		s.replace(pos, 2, "s");
// 	while (s[0] >= '0' && s[0] <= '9')
// 		s.erase(0, 1);

	static int li_secposvalue[] = {1601,1637,1833,2078,2274,2302,2433,2594,2787,3106,3212,3472,3635,3722,3730,3858,4027,4086,4390,4558,4684,4925,5249};
	static char* lc_firstletter[] = {"a","b","c","d","e","f","g","h","j","k","l","m","n","o","p","q","r","s","t","w","x","y","z"};
	static char* ls_secondsectable =
		"cjwgnspgcgne[y[btyyzdxykygt[jnnjqmbsgzscyjsyy[pgkbzgy[ywjkgkljywkpjqhy[w[dzlsgmrypywwcckznkyygttnjjnykkzytcjnmcylqlypyqfqrpzslwbtgkjfyxjwzltbncxjjjjtxdttsqzycdxxhgck[phffss[ybgxlppbyll[hlxs[zm[jhsojnghdzqyklgjhsgqzhxqgkezzwyscscjxyeyxadzpmdssmzjzqjyzc[j[wqjbyzpxgznzcpwhkxhqkmwfbpbydtjzzkqhy"
		"lygxfptyjyyzpszlfchmqshgmxxsxj[[dcsbbqbefsjyhxwgzkpylqbgldlcctnmayddkssngycsgxlyzaybnptsdkdylhgymylcxpy[jndqjwxqxfyyfjlejpzrxccqwqqsbnkymgplbmjrqcflnymyqmsqyrbcjthztqfrxqhxmjjcjlxqgjmshzkbswyemyltxfsydswlycjqxsjnqbsctyhbftdcyzdjwyghqfrxwckqkxebptlpxjzsrmebwhjlbjslyysmdxlclqkxlhxjrzjmfqhxhwy"
		"wsbhtrxxglhqhfnm[ykldyxzpylgg[mtcfpajjzyljtyanjgbjplqgdzyqyaxbkysecjsznslyzhsxlzcghpxzhznytdsbcjkdlzayfmydlebbgqyzkxgldndnyskjshdlyxbcghxypkdjmmzngmmclgwzszxzjfznmlzzthcsydbdllscddnlkjykjsycjlkwhqasdknhcsganhdaashtcplcpqybsdmpjlpzjoqlcdhjjysprchn[nnlhlyyqyhwzptczgwwmzffjqqqqyxaclbhkdjxdgmmy"
		"djxzllsygxgkjrywzwyclzmssjzldbyd[fcxyhlxchyzjq[[qagmnyxpfrkssbjlyxysyglnscmhzwwmnzjjlxxhchsy[[ttxrycyxbyhcsmxjsznpwgpxxtaybgajcxly[dccwzocwkccsbnhcpdyznfcyytyckxkybsqkkytqqxfcwchcykelzqbsqyjqcclmthsywhmktlkjlycxwheqqhtqh[pq[qscfymndmgbwhwlgsllysdlmlxpthmjhwljzyhzjxhtxjlhxrswlwzjcbxmhzqxsdzp"
		"mgfcsglsxymjshxpjxwmyqksmyplrthbxftpmhyxlchlhlzylxgsssstclsldclrpbhzhxyyfhb[gdmycnqqwlqhjj[ywjzyejjdhpblqxtqkwhlchqxagtlxljxmsl[htzkzjecxjcjnmfby[sfywybjzgnysdzsqyrsljpclpwxsdwejbjcbcnaytwgmpapclyqpclzxsbnmsggfnzjjbzsfzyndxhplqkzczwalsbccjx[yzgwkypsgxfzfcdkhjgxdlqfsgdslqwzkxtmhsbgzmjzrglyjb"
		"pmlmsxlzjqqhzyjczydjwbmyklddpmjegxyhylxhlqyqhkycwcjmyyxnatjhyccxzpcqlbzwwytwbqcmlpmyrjcccxfpznzzljplxxyztzlgdldcklyrzzgqtgjhhgjljaxfgfjzslcfdqzlclgjdjcsnzlljpjqdcclcjxmyzftsxgcgsbrzxjqqctzhgyqtjqqlzxjylylbcyamcstylpdjbyregklzyzhlyszqlznwczcllwjqjjjkdgjzolbbzppglghtgzxyghzmycnqsycyhbhgxkamtx"
		"yxnbskyzzgjzlqjdfcjxdygjqjjpmgwgjjjpkqsbgbmmcjssclpqpdxcdyyky[cjddyygywrhjrtgznyqldkljszzgzqzjgdykshpzmtlcpwnjafyzdjcnmwescyglbtzcgmssllyxqsxsbsjsbbsgghfjlypmzjnlyywdqshzxtyywhmzyhywdbxbtlmsyyyfsxjc[dxxlhjhf[sxzqhfzmzcztqcxzxrttdjhnnyzqqmnqdmmg[ydxmjgdhcdyzbffallztdltfxmxqzdngwqdbdczjdxbzgs"
		"qqddjcmbkzffxmkdmdsyyszcmljdsynsbrskmkmpcklgdbqtfzswtfgglyplljzhgj[gypzltcsmcnbtjbqfkthbyzgkpbbymtdssxtbnpdkleycjnyddykzddhqhsdzsctarlltkzlgecllkjlqjaqnbdkkghpjtzqksecshalqfmmgjnlyjbbtmlyzxdcjpldlpcqdhzycbzsczbzmsljflkrzjsnfrgjhxpdhyjybzgdlqcsezgxlblgyxtwmabchecmwyjyzlljjyhlg[djlslygkdzpzxj"
		"yyzlwcxszfgwyydlyhcljscmbjhblyzlycblydpdqysxqzbytdkyxjy[cnrjmpdjgklcljbctbjddbblblczqrppxjcjlzcshltoljnmdddlngkaqhqhjgykheznmshrp[qqjchgmfprxhjgdychghlyrzqlcyqjnzsqtkqjymszswlcfqqqxyfggyptqwlmcrnfkkfsyylqbmqammmyxctpshcptxxzzsmphpshmclmldqfyqxszyydyjzzhqpdszglstjbckbxyqzjsgpsxqzqzrqtbdkyxzk"
		"hhgflbcsmdldgdzdblzyycxnncsybzbfglzzxswmsccmqnjqsbdqsjtxxmbltxzclzshzcxrqjgjylxzfjphymzqqydfqjjlzznzjcdgzygctxmzysctlkphtxhtlbjxjlxscdqxcbbtjfqzfsltjbtkqbxxjjljchczdbzjdczjdcprnpqcjpfczlclzxzdmxmphjsgzgszzqlylwtjpfsyasmcjbtzkycwmytcsjjljcqlwzmalbxyfbpnlsfhtgjwejjxxglljstgshjqlzfkcgnnnszfdeq"
		"fhbsaqtgylbxmmygszldydqmjjrgbjtkgdhgkblqkbdmbylxwcxyttybkmrtjzxqjbhlmhmjjzmqasldcyxyqdlqcafywyxqhz";

	std::string result = "";
	int h,l,w;

	for (unsigned i = 0; i < s.length(); i++)
	{
		h = (unsigned char)(s[i]);
		l = (unsigned char)(s[i+1]);
		if (h < 0xa1 || l < 0xa1)
		{
			result += s[i];
			continue;
		}
		else
			w = (h - 160) * 100 + l - 160;

		if (w > 315 && w < 326)
		{
			result += ('0' + w - 316);
			i++;
		}
		else if (w > 332 && w < 359)
		{
			result += ('a' + w - 333);
			i++;
		}
		else if (w > 1600 && w < 5590)
		{
			for (int j = 22; j >= 0; j --)
			{
				if(w >= li_secposvalue[j])
				{
					result += lc_firstletter[j];
					i++;
					break;
				}
			}
			continue;
		}
		else
		{
			i++;
			w = (h - 160 - 56) * 94 + l - 161;
			if (w >= 0 && w <= 3007)
				result += ls_secondsectable[w];
			else
			{
				result += (char)h;
				result += (char)l;
			}
		}
	}
	return result;
}

void CDataFarm::OnTask(CConfig::CHKMKT &chkmkt, time_t now)
{
	struct tm *lt = localtime(&now);
	unsigned int today = (lt->tm_year+1900)*10000 + (lt->tm_mon+1)*100 + lt->tm_mday;

	CConfig::CHKMKT::iterator iter = chkmkt.find("DETAIL");
	if (iter != chkmkt.end())
	{// 保存历史明细
		REPORT(MN, T("保存今日明细数据...\n"), RPT_INFO);
		chkmkt.erase(iter);
		char szDirName[MAX_PATH], szFileName[MAX_PATH];
		sprintf(szDirName, "%s/%s/%d",
			(g_cfg.m_strHisPath.substr(0, g_cfg.m_strHisPath.rfind('\\') + 1) + HIS_DETAIL).c_str(),
			HIS_DETAIL, today);
		_mkdir(szDirName);

		sprintf(szFileName, "%s/%s", szDirName, FILE_INDEX);
		CopyFile(FILE_INDEX, szFileName, FALSE);
		sprintf(szFileName, "%s/%s", szDirName, FILE_TICK);
		CopyFile(FILE_TICK, szFileName, FALSE);
		sprintf(szFileName, "%s/%s", szDirName, FILE_MINK);
		CopyFile(FILE_MINK, szFileName, FALSE);
		REPORT(MN, T("今日明细数据已保存\n"), RPT_INFO);
	}

	for (iter = chkmkt.lower_bound("WEIGHT"); iter != chkmkt.upper_bound("WEIGHT");)
	{
		kdb_update_weight();
		chkmkt.erase(iter++);
	}

	for (unsigned int i = 0; i < g_cfg.m_vAddin.size(); i++)
	{
		iter = chkmkt.find(g_cfg.m_vAddin[i].strPath);
		if (iter != chkmkt.end())
		{// 插件定时器
			REPORT(MN, T("%s Timer Trigged.\n", g_cfg.m_vAddin[i].strPath.c_str()), RPT_INFO);
			pfnOnTimer _onTimer = reinterpret_cast<pfnOnTimer>(atoi(iter->second.c_str()));
			_onTimer(lt->tm_hour * 100 + lt->tm_min);
			chkmkt.erase(iter);
		}
	}

	if (chkmkt.size() == 0)
		return;

	_maintain obj;
	obj.pFarm = this;
	obj.now = now;
	obj.pChkmkt = &chkmkt;
	OnCloseSpecial();

	for (iter = chkmkt.begin(); iter != chkmkt.end(); ++iter)
		REPORT(MN, T("市场[%s]数据维护中...\n", iter->first.c_str()), RPT_INFO);
	unsigned long nCount = m_hashIdx.for_each(obj);		/** 逐symbol维护 [2/26/2010 xinl] */
	g_cfg.UpdateMarketStatus(m_pMarketStatus);

	for (iter = chkmkt.begin(); iter != chkmkt.end(); ++iter)
	{
		char *pMkt = strlwr(strdup(iter->first.c_str()));
		CConfig::MKT_SECOND mkts = (CConfig::MKT_SECOND)atoi(iter->second.c_str());
		if (mkts == CConfig::CLOSED)
		{
			string sfn = pMkt;
			sfn += ".close";
			FILE *fp = fopen(sfn.c_str(), "r");
			CHECK_RUN(!fp, MN, T("打开收市文件 %s 失败\n", sfn.c_str()), RPT_WARNING, {free(pMkt); continue;});

			fseek(fp, 0, SEEK_END);
			unsigned int fsize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			char *pTmp = (char*)mpnew(fsize+1);
			CHECK_RUN(!pTmp, MN, T("分配内存失败[%d]\n", fsize+1), RPT_CRITICAL, {free(pMkt); continue;});

			fread(pTmp, 1, fsize, fp);
			pTmp[fsize] = 0x00;
			fclose(fp);
			string strTmp = pTmp;
			mpdel(pTmp);

			kdb_addayk(pMkt, strTmp);					/** 提交维护 [2/26/2010 xinl] */
			kdb_update_history(pMkt);
			REPORT(MN, T("市场[%s]收盘\n", pMkt), RPT_INFO);
			remove(sfn.c_str());
		}
		else if (mkts == CConfig::OPENED)
		{/// 开盘后将昨日收盘后的缓存行情文件读入，并删除
			FILE *fp = fopen(pMkt, "rb");
			if (!fp)
				REPORT(MN, T("市场[%s]昨日盘后无行情数据\n", pMkt), RPT_INFO);
			else
			{
				REPORT(MN, T("市场[%s]开盘\n", pMkt), RPT_INFO);
				unsigned int nSize = 0;
				FeedHead *pHead = NULL;
				while (1)
				{
					if (sizeof(nSize) != fread(&nSize, 1, sizeof(nSize), fp))
						break;
					char *pTmp = (char*)mpnew(nSize + sizeof(unsigned int));
					if (!pTmp)
						REPORT_RUN(MN, T("分配内存%d失败\n", nSize + sizeof(unsigned int)), RPT_CRITICAL, {free(pMkt); exit(1);});
					pHead = (FeedHead*)(pTmp + sizeof(unsigned int));
					fread(pHead, 1, nSize, fp);
					gtask.AddFeed(pHead, nSize, 0);
				}
				fclose(fp);

				remove(pMkt);//删除缓存行情文件
				REPORT(MN, T("市场[%s]开盘结束\n", pMkt), RPT_INFO);
			}
		}
		free(pMkt);
	}
}

bool CDataFarm::SelfCheck(const char *pMkts)
{
	_maintain obj;
	obj.pFarm = this;

	char szTmp[256];
	CConfig::CHKMKT ckm;
	std::string s;
	
	if (pMkts)
		s = pMkts;

	char *lpToken = strtok((char*)s.c_str(), "|");
	while (lpToken)
	{
		ckm.insert(std::make_pair(lpToken, itoa(CConfig::CHECK, szTmp, 10)));
		lpToken = strtok(NULL, "|");
	}
	obj.pChkmkt = &ckm;

	REPORT(MN, T("系统自检...\n"), RPT_INFO);
	dumpHashIndex();
	CHECK(m_pIndexHead->nSymbolCount - GetHashSize() <= 100, MN,
		T("Symbol空间不足[%d]\n", GetHashSize()), RPT_WARNING);
// 	CHECK(m_pIndexHead->nMinkCount - m_pIndexHead->nMinkIndex <= 100, MN,
// 		T("Mink空间不足[%d]\n", m_pIndexHead->nMinkIndex), RPT_WARNING);
// 	CHECK(m_pIndexHead->nTickCount - m_pIndexHead->nTickIndex <= 100, MN,
// 		T("Tick空间不足[%d]\n", m_pIndexHead->nTickIndex), RPT_WARNING);

	unsigned long nCount = m_hashIdx.for_each(obj);
	REPORT_RUN(MN, T("系统自检完毕\n"), RPT_INFO, return true);
}

void CDataFarm::dumpHashIndex()
{
	REPORT(MN, T("Dump IndexHead...\n"), RPT_DEBUG);
	REPORT(MN, T("版本号: %d\n", m_pIndexHead->Version), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("桶的数量: %d\n", m_pIndexHead->nBucketCount), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("Symbol数量: %d\n", m_pIndexHead->nSymbolCount), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("Tick结构数量: %d\n", m_pIndexHead->nTickCount), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("分钟k线数量: %d\n", m_pIndexHead->nMinkCount), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("可用Quote索引: %d\n", m_pIndexHead->nQuoteIndex), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("可用Tick索引: %d\n", m_pIndexHead->nTickIndex), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("可用Mink索引: %d\n", m_pIndexHead->nMinkIndex), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("索引表偏移: %d\n", m_pIndexHead->nIndexOffset), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("拼音索引偏移: %d\n", m_pIndexHead->nPyOffset), RPT_DEBUG|RPT_NOPREFIX);
	REPORT(MN, T("Dump IndexHead end.\n"), RPT_DEBUG);

	MarketStatus *pms = (MarketStatus*)(m_pIndexHead + 1);
	REPORT(MN, T("Dump Market Status...\n"), RPT_DEBUG);
	for (unsigned int i = 0; i < g_cfg.m_vMarket.size(); i++)
	{
		REPORT(MN, T("市场代码: %s\n", pms[i].szMarket), RPT_DEBUG|RPT_NOPREFIX);
		REPORT(MN, T("开盘日期: %d\n", pms[i].dateOpen), RPT_DEBUG|RPT_NOPREFIX);
		REPORT(MN, T("收盘日期: %d\n", pms[i].dateClose), RPT_DEBUG|RPT_NOPREFIX);
	}
	REPORT(MN, T("Dump Market Status end.\n"), RPT_DEBUG);

	CHECK_RUN(m_pIndexHead->nQuoteIndex >= (unsigned int)m_pIndexHead->nSymbolCount-1,
		MN, T("Quote index error!\n"), RPT_WARNING,
		{_rebuildQuoteIndex(); return;});
	CHECK_RUN(m_pIndexHead->nTickIndex >= m_pIndexHead->nTickCount-1, MN, T("Tick index error!\n"), RPT_ERROR, return);
	CHECK_RUN(m_pIndexHead->nMinkIndex >= m_pIndexHead->nMinkCount-1, MN, T("Mink index error!\n"), RPT_ERROR, return);
}

void CDataFarm::OnCloseSpecial()
{
	_find_close_special fcs;
	for (SPECIALITER iter = g_mapCloseSpecial.begin(); iter != g_mapCloseSpecial.end(); ++iter)
	{
		iter->second.clear();
		fcs.symbol = iter->first;
		if (iter->first == "000001.SH")
		{
			m_hashIdx.cast("000001.SH", fcs);
			m_hashIdx.cast("000011.SH", fcs);
		}
		else if (iter->first == "399001.SZ")
		{
			m_hashIdx.cast("399106.SZ", fcs);
			m_hashIdx.cast("399305.SZ", fcs);
		}
		else
		{
			m_hashIdx.cast(iter->first, fcs);
			m_hashIdx.cast("399305.SZ", fcs);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
struct _rebuild
{
	int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
	{
		REPORT(MN, T("%s: %d-%d, %d, %d\n", lpSymbol->c_str(), idx, pri->idxQuote, pri->idxMinK, pri->idxTick), RPT_DEBUG);
		return 0;
	}
	unsigned int idx;
};

void CDataFarm::_rebuildQuoteIndex()
{
// 	REPORT(MN, T("index of quote: %d\n", m_tbLevel0.GetIndex()), RPT_DEBUG);
// 	REPORT(MN, T("index of mink: %d\n", m_tbMinK.GetIndex()), RPT_DEBUG);
// 	REPORT(MN, T("index of tick: %d\n", m_tbTick.GetIndex()), RPT_DEBUG);
// 
// 	REPORT(MN, T("桶的个数: %d\n", *m_hashIdx.m_pulBucketCount), RPT_DEBUG);
// 	REPORT(MN, T("节点的个数: %d\n", *m_hashIdx.m_pulNodeCount), RPT_DEBUG);
// 	REPORT(MN, T("节点向量中下一个可用节点的索引: %d\n", *m_hashIdx.m_pulIndex), RPT_DEBUG);
// 	REPORT(MN, T("元素的个数: %d\n", *m_hashIdx.m_pulElementCount), RPT_DEBUG);

// 	unsigned int idx = *m_hashIdx.m_pulIndex;
// 	while (1)
// 	{
// 		REPORT(MN, T("idx=%d, next=%d, key=%s\n", idx, m_hashIdx.m_pNode[idx].next, m_hashIdx.m_pNode[idx].key.c_str()), RPT_DEBUG);
// 		idx = m_hashIdx.m_pNode[idx].next;
// 		if (idx == 0)
// 			break;
// 	}

// 	for (unsigned int i = 0; i < *m_hashIdx.m_pulBucketCount; i++)
// 		REPORT(MN, T("%d, bucket-idx=%d\n", i, m_hashIdx.m_pBucket[i]), RPT_DEBUG);

// 	_rebuild rb;
	for (unsigned int i = 0; i < g_cfg.m_nSymbolCount; i++)
	{
		if (*(unsigned int*)m_tbLevel0[i] == 0)
		{
			REPORT(MN, T("Rebuild quote index from %d to %d.\n", m_pIndexHead->nQuoteIndex, i), RPT_INFO);
			m_pIndexHead->nQuoteIndex = i;
			break;
		}

// 		Quote *pq = m_tbLevel0[i];
// 		rb.idx = i;
// 		if (*(unsigned int*)pq == 0)
// 			REPORT(MN, T("index %d is zero.\n", i), RPT_DEBUG);
// 		else
// 			unsigned long idx = m_hashIdx.cast(pq->szStockCode, rb);
	}
}
