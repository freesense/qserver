
#include "stdafx.h"
#include "farm.h"
#include "config.h"
//#include "task.h"
//#include "addin_manager.h"
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
int CDataFarm::_maintain::OnElement(std::string *pSymbol, RINDEX *pIdx)
{
	char *pMkt = strlwr(strdup(pSymbol->substr(pSymbol->rfind(".")+1).c_str()));
	CConfig::CHKMKT::iterator iterCheck = pChkmkt->find(pMkt);
	if (iterCheck == pChkmkt->end())
	{
		free(pMkt);
		return 0;	// 市场还未收盘
	}

	switch (iterCheck->second)
	{
	case CConfig::_market::CLOSED:
		CloseSymbol(pSymbol->c_str(), pIdx);
		break;
	case CConfig::_market::OPENED:
		OpenSymbol(pSymbol->c_str(), pIdx);
		break;
	default:
		break;
	}
	free(pMkt);
	return 1;
}

bool CDataFarm::_maintain::IsSameWeek(unsigned int uiNow, unsigned int uiPrev)
{//比较今日和所给定日期是否在同一周内
	tm	stPrevTime,stNowTime;
	memset(&stPrevTime,0,sizeof(tm));
	stPrevTime.tm_year=uiPrev/10000-1900;
	stPrevTime.tm_mon=uiPrev%10000/100-1;
	stPrevTime.tm_mday=uiPrev%100;
	time_t stPrev=mktime(&stPrevTime);

	memset(&stNowTime,0,sizeof(tm));
	stNowTime.tm_year=uiNow/10000-1900;
	stNowTime.tm_mon=uiNow%10000/100-1;
	stNowTime.tm_mday=uiNow%100;
	time_t stNow=mktime(&stNowTime);

	if(stNow>stPrev && difftime(stNow,stPrev)>=7*24*3600)
		return	false;
	else if(stPrev>stNow && difftime(stPrev,stNow)>=7*24*3600)
		return	false;
	if(stNowTime.tm_wday<stPrevTime.tm_wday && stNow>stPrev)
		return	false;
	else if(stNowTime.tm_wday>stPrevTime.tm_wday && stNow<stPrev)
		return	false;
	return	true;
}

bool CDataFarm::_maintain::IsSameMonth(unsigned int uiNow, unsigned int uiPrev)
{//比较今日和所给定日期是否在同一月内
	unsigned char ucNowMonth=uiNow%10000/100;
	unsigned char ucPrevMonth=uiPrev%10000/100;
	if(uiNow/10000 != uiPrev/10000)
		return false;
	if(ucNowMonth!=ucPrevMonth)
		return false;
	return	true;
}

unsigned int CDataFarm::_maintain::SeekEnd(FILE *fp, unsigned int nBlock)
{
	fseek(fp, 0, SEEK_END);
	unsigned int nSize = ftell(fp) / sizeof(k_line);
	nBlock = min(nSize, nBlock);
	nSize = ftell(fp) - nBlock * sizeof(k_line);
	fseek(fp, nSize, SEEK_SET);
	return nBlock;
}

unsigned int CDataFarm::_maintain::Get5DayVol(const char *lpSymbol)
{
	return 0;
}

void CDataFarm::_maintain::OpenSymbol(const char *lpSymbol, RINDEX *pIndex)
{
	pIndex->cntMinK = 0;
	pIndex->cntTick = 0;

	Quote *pq = pFarm->GetQuote(pIndex->idxQuote);
	memset(((char*)pq) + QUOTEHEAD, 0x00, sizeof(Quote) - QUOTEHEAD);
	const char *lpMktCode = strchr(lpSymbol, '.') + 1;
	if (islower(lpMktCode[0]))	//指数不计算五日平均总手
		pq->day5pjzs = Get5DayVol(lpSymbol);

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
}

///////////////////////////////////////////////////////////////////////////////////////
CDataFarm::CDataFarm()
{
	m_pIndexHead = NULL;
	m_pMarketStatus = NULL;
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
	m_unMarketCount = nMarketCount;
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
	m_hFile = CreateFileA(lpszFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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

void CDataFarm::AddMink(RINDEX *pIdx, unsigned short now, unsigned int price, unsigned int volumn)
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
}

void CDataFarm::OnCloseSpecial()
{
}
