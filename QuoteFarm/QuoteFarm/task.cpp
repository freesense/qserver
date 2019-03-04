
#include "stdafx.h"
#include "task.h"
#include "addin_manager.h"
#include "../../public/commx/mery.h"
#include "../../public/devmonitor/DevMonitor.h"
#include "../../public/devmonitor/ReportServer.h"
#include <algorithm>

#pragma warning(disable:4267 4309)

#define QFSET(F, B) ((F)[((B)/32)] |= (1 << ((B)%32)))
#define QFGET(F, B) (((F)[((B)/32)]) & (1 << ((B)%32)))

const double ROUND = 0.00005;
const int MULTIPLE = 10000;

CFarmTask gtask;

///////////////////////////////////////////////////////////////////////////////////////
int CFarmTask::_avg_of_index::OnElement(std::string *pSymbol, RINDEX *pIdx)
{
	std::string pMkt = pSymbol->substr(pSymbol->rfind('.')+1);
	if (pMkt[0] >= 'A' && pMkt[0] <= 'Z')
	{
		vecIndexCode.push_back(*pSymbol);
		return 0;		/** 指数不参与计算 [7/31/2009 xinl] */
	}

	if (strMktCode == pMkt)
	{
		Quote *pq = pFarm->GetQuote(pIdx->idxQuote);
		nSumPrevClose += (pq->zrsp ? pq->zrsp : pq->zjjg);
		nSumLastPrice += (pq->zjjg ? pq->zjjg : pq->zrsp);
	}

	return 1;
}

int CFarmTask::_avg_of_index::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	Quote *pq = pFarm->GetQuote(pri->idxQuote);
	pq->pjjg = (unsigned int)(pq->zrsp * scale);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
int CFarmTask::_find_symbol::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	nSize = 0;
	pIndex = pri;
	Quote *pq = NULL;

	if (bAdd && pHead->nFeedType != 0x02)
	{
		m_pTask->m_pFarm->AddSymbolIndex(lpSymbol->c_str(), pri);
		pq = m_pTask->m_pFarm->GetQuote(pri->idxQuote);
		strcpy(pq->szStockCode, lpSymbol->c_str());
		strcpy(pq->szStockKind, g_cfg.GetPattern(lpSymbol->c_str()));
	}

	nSize += sizeof(FeedHead);
	TickUnit tick;
	memset(&tick, 0x00, sizeof(tick));

	switch (pHead->nFeedType)
	{
	case 0x02:/*删除股票*/
		for (unsigned int i = 0; i < gAddin.m_vOnQuote.size(); i++)
			gAddin.m_vOnQuote[i]._onQuote(0, lpSymbol->c_str(), NULL, NULL, &tick);
		m_pTask->m_pFarm->DeleteSymbolIndex(lpSymbol->c_str(), pri);
		break;
	case 0x01:/*增加股票*/
		pq = m_pTask->m_pFarm->GetQuote(pri->idxQuote);
		strcpy(pq->szStockName, (char*)pHead + sizeof(FeedHead));
		m_pTask->m_pFarm->AddPydm(pq->szStockCode, pq->szStockName);
		nSize += 17;
		OnQuoteData(pHead->nCount, pq, &tick);
		for (unsigned int i = 0; i < gAddin.m_vOnQuote.size(); i++)
			gAddin.m_vOnQuote[i]._onQuote(1, lpSymbol->c_str(), m_nChangeFlag, pq, &tick);
		break;
	case 0x00:/*普通行情*/
		pq = m_pTask->m_pFarm->GetQuote(pri->idxQuote);
		// TODO: 如果是先启动Feed后启动Farm，这里可能不能获得symbol name
		OnQuoteData(pHead->nCount, pq, &tick);
		for (unsigned int i = 0; i < gAddin.m_vOnQuote.size(); i++)
			gAddin.m_vOnQuote[i]._onQuote(2, lpSymbol->c_str(), m_nChangeFlag, pq, &tick);
		break;
	default:
		HEXDUMP(MN, pHead, nTotalPacketSize, T("未知业务%c\n", pHead->nFeedType), RPT_WARNING);
		break;
	}
	return 0;
}

void CFarmTask::_find_symbol::OnQuoteData(unsigned int nCount, Quote *pq, TickUnit *ptick)
{
	memset(m_nChangeFlag, 0x00, sizeof(m_nChangeFlag));
	bool bzf = false;					// 是否需要计算振幅
	int nTick = 0;
	char *pData = (char*)pHead + nSize;
	char szMktCode[7];
	strcpy(szMktCode, strchr(pq->szStockCode, '.') + 1);
	unsigned int oldbuy1 = 0, oldsell1 = 0;				// 原始买一卖一价

	//五档买卖盘暂存
	bool b5Quote = false;
	unsigned int _tmp_bs10[20];
	memset(&_tmp_bs10, 0x00, sizeof(_tmp_bs10));

	for (unsigned int i = 0; i < nCount; i++)
	{
		unsigned short col = *(unsigned short*)pData;
		pData += sizeof(short);
		switch (col)
		{
		case FD_ZRSP:
			pq->zrsp = *(unsigned int*)pData;
			break;
		case FD_JRKP:
			pq->jrkp = *(unsigned int*)pData;
			if (pq->zjjg == 0)
			{
				nTick = 1;
				pq->zgjg = pq->zdjg = pq->zjjg = pq->jrkp;
			}
			break;
		case FD_ZJCJ:
			pq->zjjg = *(unsigned int*)pData;
			if (pq->zgjg == 0)
				pq->zgjg = pq->zjjg;
			if (pq->zdjg == 0)
				pq->zdjg = pq->zjjg;
			if (pq->jrkp == 0)
			{
				nTick = 1;
				pq->jrkp = pq->zjjg;
			}

			{//计算涨跌幅
				unsigned int price;
				if (pq->zrsp)
					price = pq->zrsp;
				else if (pq->jrkp)
					price = pq->jrkp;
				else
					price = pq->zjjg;

				// 没有最新价，可能是停盘，涨跌幅维持在0不变
				if (pq->zjjg == 0 || pq->zjjg == price)
					pq->zdf = 0;
				else if (pq->zjjg < price)
					pq->zdf = (int)(((((float)((int)pq->zjjg - (int)price)) / price) - ROUND) * MULTIPLE);
				else if (pq->zjjg > price)
					pq->zdf = (int)(((((float)((int)pq->zjjg - (int)price)) / price) + ROUND) * MULTIPLE);
			}
			break;
		case FD_CJSL:
			{
				unsigned int old_cjsl = pq->cjsl;
				pq->zjcj = *(unsigned int*)pData - pq->cjsl;
				pq->cjsl = *(unsigned int*)pData;
				if (nTick == 1)
					pq->zjcj = pq->cjsl;
				else if (pq->zjcj > 0)
					nTick = 2;
			}
			break;
		case FD_CJJE:
			{
				unsigned int old_cjje = pq->cjje;
				pq->cjje = *(unsigned int*)pData;
				/** 预防部分特殊代码只有金额变化而没有成交量的变化 [8/14/2009 xinl] */
				if (pq->cjje > old_cjje)
					nTick = 2;
			}
			break;
		case FD_CJBS:
			pq->cjbs = *(unsigned int*)pData;
			break;
		case FD_ZGCJ:
			bzf = true;
			pq->zgjg = *(unsigned int*)pData;
			break;
		case FD_ZDCJ:
			bzf = true;
			pq->zdjg = *(unsigned int*)pData;
			break;
		case FD_SYL1:
			pq->syl1 = *(unsigned int*)pData;
			break;
		case FD_SYL2:
			pq->syl2 = *(unsigned int*)pData;
			break;
		case FD_JSD1:
		case FD_JSD2:
		case FD_HYCC:
		case FD_XXJYDW:
		case FD_XXMGMZ:
		case FD_XXZFXL:
		case FD_XXLTGS:
		case FD_XXSNLR:
		case FD_XXBNLR:
		case FD_XXJSFL:
		case FD_XXYHSL:
		case FD_XXGHFL:
		case FD_XXMBXL:
		case FD_XXBLDW:
		case FD_XXSLDW:
		case FD_XXJGDW:
		case FD_XXJHCS:
		case FD_XXLXCS:
		case FD_XXXJXZ:
		case FD_XXZHBL:
			break;
		case FD_XXZTJG:
			pq->ztjg = *(unsigned int*)pData;
			break;
		case FD_XXDTJG:
			pq->dtjg = *(unsigned int*)pData;
			break;
		case FD_SJW5:
			b5Quote = true;
			_tmp_bs10[0] = *(unsigned int*)pData;
			break;
		case FD_SJW4:
			b5Quote = true;
			_tmp_bs10[2] = *(unsigned int*)pData;
			break;
		case FD_SJW3:
			b5Quote = true;
			_tmp_bs10[4] = *(unsigned int*)pData;
			break;
		case FD_SJW2:
			b5Quote = true;
			_tmp_bs10[6] = *(unsigned int*)pData;
			break;
		case FD_SJW1:
			b5Quote = true;
			_tmp_bs10[8] = *(unsigned int*)pData;
			break;
		case FD_SSL5:
			b5Quote = true;
			_tmp_bs10[1] = *(unsigned int*)pData;
			break;
		case FD_SSL4:
			b5Quote = true;
			_tmp_bs10[3] = *(unsigned int*)pData;
			break;
		case FD_SSL3:
			b5Quote = true;
			_tmp_bs10[5] = *(unsigned int*)pData;
			break;
		case FD_SSL2:
			b5Quote = true;
			_tmp_bs10[7] = *(unsigned int*)pData;
			break;
		case FD_SSL1:
			b5Quote = true;
			_tmp_bs10[9] = *(unsigned int*)pData;
			break;
		case FD_BJW5:
			b5Quote = true;
			_tmp_bs10[10] = *(unsigned int*)pData;
			break;
		case FD_BJW4:
			b5Quote = true;
			_tmp_bs10[12] = *(unsigned int*)pData;
			break;
		case FD_BJW3:
			b5Quote = true;
			_tmp_bs10[14] = *(unsigned int*)pData;
			break;
		case FD_BJW2:
			b5Quote = true;
			_tmp_bs10[16] = *(unsigned int*)pData;
			break;
		case FD_BJW1:
			b5Quote = true;
			_tmp_bs10[18] = *(unsigned int*)pData;
			break;
		case FD_BSL5:
			b5Quote = true;
			_tmp_bs10[11] = *(unsigned int*)pData;
			break;
		case FD_BSL4:
			b5Quote = true;
			_tmp_bs10[13] = *(unsigned int*)pData;
			break;
		case FD_BSL3:
			b5Quote = true;
			_tmp_bs10[15] = *(unsigned int*)pData;
			break;
		case FD_BSL2:
			b5Quote = true;
			_tmp_bs10[17] = *(unsigned int*)pData;
			break;
		case FD_BSL1:
			b5Quote = true;
			_tmp_bs10[19] = *(unsigned int*)pData;
			break;
		case FD_AVERAGE:
			pq->pjjg = *(unsigned int*)pData;
			break;
		default:
			REPORT(MN, T("[%s]非法行情数据类型：%d\n", pq->szStockCode, col), RPT_WARNING);
			break;
		}

		QFSET(m_nChangeFlag, col);
		pData += sizeof(int);
	}

	if (b5Quote)
	{
		int i = 0;
		for (; i < 10; i++)
		{
			unsigned int jw = _tmp_bs10[i*2];
			unsigned int sl = _tmp_bs10[i*2+1];
			if ((jw && !sl) || (!jw && sl))
				break;		// 价格和数量其中之一为0，不改变5档行情
		}

		if (i == 10)
		{
			if (_tmp_bs10[0] && _tmp_bs10[1])
			{
				pq->SP5 = _tmp_bs10[0];
				pq->SM5 = _tmp_bs10[1];
			}
			if (_tmp_bs10[2] && _tmp_bs10[3])
			{
				pq->SP4 = _tmp_bs10[2];
				pq->SM4 = _tmp_bs10[3];
			}
			if (_tmp_bs10[4] && _tmp_bs10[5])
			{
				pq->SP3 = _tmp_bs10[4];
				pq->SM3 = _tmp_bs10[5];
			}
			if (_tmp_bs10[6] && _tmp_bs10[7])
			{
				pq->SP2 = _tmp_bs10[6];
				pq->SM2 = _tmp_bs10[7];
			}
			if (_tmp_bs10[8] && _tmp_bs10[9])
			{
				oldsell1 = pq->SP1;
				pq->SP1 = _tmp_bs10[8];
				pq->SM1 = _tmp_bs10[9];
			}

			if (_tmp_bs10[10] && _tmp_bs10[11])
			{
				pq->BP5 = _tmp_bs10[10];
				pq->BM5 = _tmp_bs10[11];
			}
			if (_tmp_bs10[12] && _tmp_bs10[13])
			{
				pq->BP4 = _tmp_bs10[12];
				pq->BM4 = _tmp_bs10[13];
			}
			if (_tmp_bs10[14] && _tmp_bs10[15])
			{
				pq->BP3 = _tmp_bs10[14];
				pq->BM3 = _tmp_bs10[15];
			}
			if (_tmp_bs10[16] && _tmp_bs10[17])
			{
				pq->BP2 = _tmp_bs10[16];
				pq->BM2 = _tmp_bs10[17];
			}
			if (_tmp_bs10[18] && _tmp_bs10[19])
			{
				oldbuy1 = pq->BP1;
				pq->BP1 = _tmp_bs10[18];
				pq->BM1 = _tmp_bs10[19];
			}
		}
	}

	pq->flag[0] |= m_nChangeFlag[0];
	pq->flag[1] |= m_nChangeFlag[1];
	pq->flag[2] |= m_nChangeFlag[2];
	pq->flag[3] |= m_nChangeFlag[3];

	if (bzf)
	{
		unsigned int tmp = pq->zrsp;
		if (tmp == 0)
			tmp = pq->jrkp;
		if (tmp == 0)
			tmp = pq->zdjg;
		if (tmp == 0)
			pq->zf = 0;
		else if (pq->zgjg == 0 && pq->zdjg == 99999999)
			pq->zf = 0;
		else
			pq->zf = (int)(((double)(pq->zgjg-pq->zdjg)/tmp+ROUND)*MULTIPLE);
	}

	if (nTick && pq->zjjg && pq->jrkp)
	{
		// 增加分笔成交
		ptick->Way = 0;
		ptick->Time = pHead->nTime;
		ptick->Price = pq->zjjg;
		ptick->Volume = pq->zjcj;

		// 计算均价
// 		pq->pjjg = (unsigned int)(((double)pq->cjje) / pq->cjsl * 10000);

		if (szMktCode[0] >= 'a' && szMktCode[0] <= 'z')
		{// 非指数品种
			// 计算量比
			int n1=g_cfg.GetMinPerDay(szMktCode);
			int n2=g_cfg.GetQuoteMin(szMktCode, pHead->nTime);
			if (pq->day5pjzs == 0)
				pq->day5pjzs = m_pTask->m_pFarm->Get5DayVol(pq->szStockCode);
			double fAvg5DayVol = ((double)pq->day5pjzs)/n1*n2;
			pq->lb = (unsigned int)((((double)pq->cjsl + ROUND) * 10000) / fAvg5DayVol);

			// 计算内外盘
			unsigned int isell = 0;
			if (oldsell1 == 0)
				oldsell1 = pq->SP1;
			if (oldbuy1 == 0)
				oldbuy1 = pq->BP1;
			unsigned int avgprice = (oldbuy1 + oldsell1) / 2;

			if (pq->zjjg == avgprice)
				isell = int(ptick->Volume / 2);
			else if (ptick->Price > avgprice)
			{
				ptick->Way = 1;
				isell = ptick->Volume;
			}
			else
				ptick->Way = 2;
			pq->wp += isell;
			pq->np = pq->cjsl - pq->wp;
		}

		if (nNodeType == 1)
		{/** 有了新的tick才有新的分钟k线，只有实时行情才能增加分笔和分时数据 [7/6/2009 xinl] */
			m_pTask->m_pFarm->AddTick(pIndex, ptick);
			m_pTask->m_pFarm->AddMink(pIndex, pHead->nTime, pq->zjjg, pq->zjcj, pq->pjjg);
		}
	}

	if (szMktCode[0] >= 'a' && szMktCode[0] <= 'z')
	{// 非指数品种，计算委比
		int fbuy = pq->BM1+pq->BM2+pq->BM3+pq->BM4+pq->BM5;
		int fsell = pq->SM1+pq->SM2+pq->SM3+pq->SM4+pq->SM5;
		if (fbuy == fsell)
			pq->wb = 0;
		else if (fbuy > fsell)
			pq->wb = (int)(((fbuy - fsell) / ((double)fbuy + (double)fsell) + ROUND) * MULTIPLE);
		else
			pq->wb = (int)(((fbuy - fsell) / ((double)fbuy + (double)fsell) - ROUND) * MULTIPLE);
	}
	else
	{// 指数的委比设置为0
		pq->wb = 0;
	}

	nSize = (unsigned int)(pData - (char*)pHead);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CFarmTask::SetFarmPtr(CDataFarm *pFarm)
{
	m_pFarm = pFarm;
}

void CFarmTask::SetSharedServer(CSharedServer *pServer)
{
	m_pServer = pServer;
}

void CFarmTask::AddFeed(FeedHead *pHead, unsigned int nLen, unsigned int nType)
{
	_NODE node;
	node.nLength = nLen;
	node.pHead = pHead;
	node.nType = nType;
	m_qFeed.Push(node);
}

unsigned int CFarmTask::svc(void* lpParam)
{
	_NODE node;
	char szTmp[15];
	FeedHead *pHead = NULL, *pTmp = NULL;
	_find_symbol fs;
	fs.m_pTask = this;
	m_dwPreTick = GetTickCount();

	while (1)
	{
		if (-1 == m_qFeed.Pop(node, g_DevMonitor.m_nTimeOut * 1000))
		{
			m_dwPreTick = GetTickCount();
			REPORT(MN, T(""), RPT_HEARTBEAT|RPT_IGNORE);
			continue;
		}

		DWORD dwCurTick = GetTickCount();
		if (dwCurTick - m_dwPreTick >= (DWORD)g_DevMonitor.m_nTimeOut * 1000)
		{
			REPORT(MN, T(""), RPT_HEARTBEAT|RPT_IGNORE);
			m_dwPreTick = dwCurTick;
		}

		pTmp = node.pHead;
		if (node.pHead->nFeedType == 0xffff && node.pHead->nTime == 0xffff)
		{//退出线程
			mpdel(pTmp);
			break;
		}
		else if (node.pHead->nFeedType == 0xff00)
		{//系统定时事件
			m_lock.lock();
			m_pFarm->OnTask(m_chkmkt, *(time_t*)((char*)node.pHead + sizeof(FeedHead)));
			m_chkmkt.clear();
			m_lock.unlock();
		}
		else
		{//收到行情数据
			CConfig::MKT_SECOND mkt_status = g_cfg.GetMarketStatus(node.pHead->szMarketCode);
			if (mkt_status == CConfig::CLOSED)
			{// 收盘后将收到的行情缓存到文件里面
				FILE *fp = fopen(node.pHead->szMarketCode, "a+b");
				if (!fp)
					REPORT(MN, T("无法打开收盘缓存文件%s\n", node.pHead->szMarketCode), RPT_ERROR);
				else
				{
					fwrite(&node.nLength, 1, sizeof(node.nLength), fp);
					fwrite(node.pHead, 1, node.nLength, fp);
					fclose(fp);
				}
			}
			else if (mkt_status == CConfig::OPENED)
			{
				unsigned int nSize = 0;
				fs.nSize = 0;
				for (unsigned int i = 0; i < gAddin.m_vOnQuote.size(); i++)
					gAddin.m_vOnQuote[i]._begin();
				while (nSize < node.nLength)
				{
					node.pHead = (FeedHead*)((char*)node.pHead + fs.nSize);
					node.pHead->nTime = g_cfg.ConvergenceTime(node.pHead->szMarketCode, node.pHead->nTime);
					fs.nFeedType = node.pHead->nFeedType;
					fs.nNodeType = node.nType;
					fs.pHead = node.pHead;
					fs.nTotalPacketSize = node.nLength;
					sprintf(szTmp, "%s.%s", node.pHead->szSymbolCode, node.pHead->szMarketCode);
					if (-1 == m_pFarm->m_hashIdx.cast(szTmp, fs, false))
					{
						REPORT(MN, T("行情包错误，退出本包处理\n"), RPT_WARNING);
						for (unsigned int i = 0; i < gAddin.m_vOnQuote.size(); i++)
							gAddin.m_vOnQuote[i]._commit();
						mpdel(pTmp);
						break;
					}
					nSize += fs.nSize;
				}

				for (unsigned int i = 0; i < gAddin.m_vOnQuote.size(); i++)
					gAddin.m_vOnQuote[i]._commit();

				/** 已经解析一个完整的业务包，计算指数均价 [7/31/2009 xinl] */
				_avg_of_index aoi;
				aoi.nSumLastPrice = aoi.nSumPrevClose = 0;
				aoi.pFarm = m_pFarm;
				aoi.strMktCode = node.pHead->szMarketCode;
				std::transform(aoi.strMktCode.begin(), aoi.strMktCode.end(), aoi.strMktCode.begin(), tolower);
				m_pFarm->m_hashIdx.for_each(aoi);
				aoi.scale = aoi.nSumPrevClose == 0 ? 1 : (aoi.nSumLastPrice/aoi.nSumPrevClose);
				for (unsigned int i = 0; i < aoi.vecIndexCode.size(); i++)
					m_pFarm->m_hashIdx.cast(aoi.vecIndexCode[i], aoi);
			}
			pTmp = (FeedHead*)((char*)pTmp - sizeof(unsigned int));
		}
		mpdel(pTmp);
	}
	return 0;
}

void CFarmTask::close()
{
	for (unsigned int i = 0; i < m_nThreadNum; i++)
	{
		FeedHead *pHead = (FeedHead*)mpnew(sizeof(FeedHead));
		pHead->nFeedType = 0xffff;
		pHead->nTime = 0xffff;
		AddFeed(pHead, sizeof(FeedHead));
	}
}
