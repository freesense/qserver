// farm_addin_investoday.cpp : 定义 DLL 应用程序的入口点。
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
		{/** 港股指数 [7/1/2009 xinl] */
			val_cl = 'Z';
			mkt_cl = 'H';
		}
		else if (!strcmp(pq->szStockKind, "md_hkgp"))
		{/** 港股股票 [7/1/2009 xinl] */
			val_cl = 'T';
			mkt_cl = 'H';
		}
		else if (!strcmp(pq->szStockKind, "md_shzs"))
		{/** 上海指数 [7/1/2009 xinl] */
			val_cl = 'Z';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shgz") || !strcmp(pq->szStockKind, "md_shqz"))
		{/** 上海债券 [7/1/2009 xinl] */
			val_cl = 'D';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shlof") || !strcmp(pq->szStockKind, "md_shjj"))
		{/** 上海基金 [7/1/2009 xinl] */
			val_cl = 'K';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shag"))
		{/** 上海A股 [7/1/2009 xinl] */
			val_cl = 'A';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_shbg"))
		{/** 上海B股 [7/1/2009 xinl] */
			val_cl = 'B';
			mkt_cl = 'S';
		}
		else if (strstr(pq->szStockKind, "md_sh"))
		{/** 上海其他 [7/1/2009 xinl] */
			val_cl = 'T';
			mkt_cl = 'S';
		}
		else if (!strcmp(pq->szStockKind, "md_szzs"))
		{/** 深圳指数 [7/1/2009 xinl] */
			val_cl = 'Z';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szgz") || !!strcmp(pq->szStockKind, "md_szqz"))
		{/** 深圳债券 [7/1/2009 xinl] */
			val_cl = 'D';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szlof") || !strcmp(pq->szStockKind, "md_szjj"))
		{/** 深圳基金 [7/1/2009 xinl] */
			val_cl = 'K';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szag"))
		{/** 深圳A股 [7/1/2009 xinl] */
			val_cl = 'A';
			mkt_cl = 'Z';
		}
		else if (!strcmp(pq->szStockKind, "md_szbg"))
		{/** 深圳A股 [7/1/2009 xinl] */
			val_cl = 'B';
			mkt_cl = 'Z';
		}
		else if (strstr(pq->szStockKind, "md_sz"))
		{/** 深圳其他 [7/1/2009 xinl] */
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
		{/** 写入港股日k线 [7/1/2009 xinl] */
			char *pSql = (char*)mpnew(1024);
			CHECK_RUN(!pSql, MN, T("分配内存失败\n"), RPT_CRITICAL, return 0);

			sprintf(pSql, cfg.m_vSql[1].c_str(), cfg.m_strDbName.c_str(),
				pSymbol->substr(0, pSymbol->rfind('.')).c_str(), val_cl, mkt_cl,
				(float)pq->jrkp/1000, (float)pq->zgjg/1000, (float)pq->zdjg/1000, (float)pq->zjcj/1000,
				pq->cjsl, (float)pq->cjje);
			gWorkThread.AddSql(pSql);
		}

		/** 写入资金流 [7/1/2009 xinl] */
		char *pSql = (char*)mpnew(1024);
		CHECK_RUN(!pSql, MN, T("分配内存失败\n"), RPT_CRITICAL, return 0);

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
	REPORT(MN, T("初始化今日扩展库...\n"), RPT_INFO);
	cfg.Load("QuoteFarm.xml");
	CHECK_RUN(-1 == gWorkThread.open(), MN, T("创建工作线程失败\n"), RPT_ERROR, return -1);
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
	/** 只对港股指数,2823（A50中国）,沪深300做分时走势入库 [7/1/2009 xinl] */
	std::string s = symbol;
	if (!quote)
		return;
	if (s != "002823.hk" && s != "399300.SZ" && strcmp(quote->szStockKind, "md_hkzs"))
		return;

	if (tickunit->Time == 0)
		return;

	if (imode == 0)
	{/** 删除股票 [7/1/2009 xinl] */
		mpSymbols.erase(symbol);
		return;
	}

	REPORT(MN, T("%s TICK.\n", symbol), RPT_DEBUG);

	char *pSql = (char*)mpnew(1024);
	CHECK_RUN(!pSql, MN, T("分配内存失败\n"), RPT_CRITICAL, return);

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

	/** 港股日k线入库 [7/1/2009 xinl] */
	/** 资金流入库 [7/1/2009 xinl] */
	farm_each fe;
	time_t today;
	time(&today);
	fe.lt = localtime(&today);
	gpFarm->m_hashIdx.for_each(fe);
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
