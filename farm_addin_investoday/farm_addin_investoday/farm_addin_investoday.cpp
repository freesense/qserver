// farm_addin_investoday.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "config.h"
#include "workthread.h"
#include "farm.h"
#include <map>
#include <string>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

std::map<std::string, unsigned int> mpSymbols;
CWorkThread gWorkThread;
CDataFarm *gpFarm;

struct farm_each
{
	void convertDBfield(Quote *pq, char &val_cl, char &mkt_cl)
	{
		if (!strcmp(pq->szStockKind, "md_hkzs"))
		{/** �۹�ָ�� [7/1/2009 xinl] */
			val_cl = 'Z';
			mkt_cl = 'H';
		}
		else if (!strcmp(pq->szStockKind, "md_hkgp"))
		{/** �۹ɹ�Ʊ [7/1/2009 xinl] */
			val_cl = 'T';
			mkt_cl = 'H';
		}
		else if (!strcmp(pq->szStockKind, "md_shzs"))
		{/** �Ϻ�ָ�� [7/1/2009 xinl] */
			val_cl = 'Z';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shgz") || !strcmp(pq->szStockKind, "md_shqz"))
		{/** �Ϻ�ծȯ [7/1/2009 xinl] */
			val_cl = 'D';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shlof") || !strcmp(pq->szStockKind, "md_shjj"))
		{/** �Ϻ����� [7/1/2009 xinl] */
			val_cl = 'K';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shag"))
		{/** �Ϻ�A�� [7/1/2009 xinl] */
			val_cl = 'A';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shbg"))
		{/** �Ϻ�B�� [7/1/2009 xinl] */
			val_cl = 'B';
			mkt_cl = 'S';
		}
		else if (strstr(pq->szStockKind, "md_sh"))
		{/** �Ϻ����� [7/1/2009 xinl] */
			val_cl = 'T';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_szzs"))
		{/** ����ָ�� [7/1/2009 xinl] */
			val_cl = 'Z';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szgz") || !!strcmp(pq->szStockKind, "md_szqz"))
		{/** ����ծȯ [7/1/2009 xinl] */
			val_cl = 'D';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szlof") || !strcmp(pq->szStockKind, "md_szjj"))
		{/** ���ڻ��� [7/1/2009 xinl] */
			val_cl = 'K';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szag"))
		{/** ����A�� [7/1/2009 xinl] */
			val_cl = 'A';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szbg"))
		{/** ����A�� [7/1/2009 xinl] */
			val_cl = 'B';
			mkt_cl = 'Z';
		}
		else if (strstr(pq->szStockKind, "md_sz"))
		{/** �������� [7/1/2009 xinl] */
			val_cl = 'T';
			mkt_cl = 'Z';
		}
		else
		{
			val_cl = 'X';
			mkt_cl = 'X';
		}
	}

	int OnElement(std::string *pSymbol, RINDEX *pIdx)
	{
		Quote *pq = gpFarm->GetQuote(pIdx->idxQuote);
		char *pMkt = strdup(pSymbol->substr(pSymbol->rfind(".")+1).c_str());
		char val_cl, mkt_cl;
		convertDBfield(pq, val_cl, mkt_cl);

		if (!strcmp(strlwr(pMkt), "hk"))
		{/** д��۹���k�� [7/1/2009 xinl] */
			char *pSql = (char*)mpnew(1024);
			CHECK_RUN(!pSql, MN, T("�����ڴ�ʧ��\n"), RPT_CRITICAL, return 0);

			sprintf(pSql, cfg.m_vSql[1].c_str(), cfg.m_strDbName.c_str(),
				pSymbol->substr(0, pSymbol->rfind('.')).c_str(), val_cl, mkt_cl,
				(float)pq->jrkp/1000, (float)pq->zgjg/1000, (float)pq->zdjg/1000, (float)pq->zjcj/1000,
				pq->cjsl, (float)pq->cjje);
			gWorkThread.AddSql(pSql);
		}

		/** д���ʽ��� [7/1/2009 xinl] */
		char *pSql = (char*)mpnew(1024);
		CHECK_RUN(!pSql, MN, T("�����ڴ�ʧ��\n"), RPT_CRITICAL, return 0);

		sprintf(pSql, cfg.m_vSql[2].c_str(), cfg.m_strDbName.c_str(),
			pSymbol->substr(0, pSymbol->rfind('.')).c_str(), mkt_cl, val_cl,
			(lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday,
			pq->np, pq->wp, pq->np-pq->wp, (float)pq->pjjg/1000);
		gWorkThread.AddSql(pSql);

		free(pMkt);
		return 1;
	}

	struct tm *lt;
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

extern "C" __declspec(dllexport) void GetModuleVer(char** pBuf)
{
	static std::string strVersion;
	strVersion = gVer;

	char *lpCommxVer = NULL;
	CommxVer(&lpCommxVer);
	strVersion += lpCommxVer;
	*pBuf = (char*)strVersion.c_str();
}

extern "C" __declspec(dllexport) int Initialize(int iMode, void *pFarm)
{
	gpFarm = (CDataFarm*)pFarm;
	REPORT(MN, T("��ʼ��������չ��...\n"), RPT_INFO);
	cfg.Load("QuoteFarm.xml");
	CHECK_RUN(-1 == gWorkThread.open(), MN, T("���������߳�ʧ��\n"), RPT_ERROR, return -1);
	return 0;
}

extern "C" __declspec(dllexport) void Release()
{
	gWorkThread.close();
	gWorkThread.wait();
}

extern "C" __declspec(dllexport) void Sym_Begin()
{}

extern "C" __declspec(dllexport) void OnQuote(int imode, const char *symbol, int *pFlag, Quote *quote, TickUnit *tickunit)
{
	/** ֻ�Ը۹�ָ��,2823��A50�й���,����300����ʱ������� [7/1/2009 xinl] */
	std::string s = symbol;
	if (!quote)
		return;
	if (s != "002823.hk" && s != "399300.SZ" && strcmp(quote->szStockKind, "md_hkzs"))
		return;

	if (tickunit->Time == 0)
		return;

	if (imode == 0)
	{/** ɾ����Ʊ [7/1/2009 xinl] */
		mpSymbols.erase(symbol);
		return;
	}

	REPORT(MN, T("%s TICK.\n", symbol), RPT_DEBUG);

	char *pSql = (char*)mpnew(1024);
	CHECK_RUN(!pSql, MN, T("�����ڴ�ʧ��\n"), RPT_CRITICAL, return);

	time_t today;
	time(&today);
	struct tm *lt = localtime(&today);

	sprintf(pSql, cfg.m_vSql[0].c_str(), cfg.m_strDbName.c_str(),
		lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday,
		tickunit->Time/60, tickunit->Time%60, s.substr(0, s.rfind('.')).c_str(),
		(float)tickunit->Price/1000, tickunit->Volume, (float)(quote->cjje - mpSymbols[symbol]));
	gWorkThread.AddSql(pSql);

	mpSymbols[symbol] = quote->cjje;
}

extern "C" __declspec(dllexport) void Sym_Commit()
{}

extern "C" __declspec(dllexport)void OnTimer(short timer)
{
	if (timer < 1631)
		return;

	/** �۹���k����� [7/1/2009 xinl] */
	/** �ʽ������ [7/1/2009 xinl] */
	farm_each fe;
	time_t today;
	time(&today);
	fe.lt = localtime(&today);
	gpFarm->m_hashIdx.for_each(fe);
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
