// qextend.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "tradematch.h"
#include "regexpr2.h"
#include "../../public/commx/commxapi.h"
#include "../../public/commx/code_convert.h"
#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>
#include <WinSock2.h>
#include "datastore.h"

using namespace regex;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CqextendApp

BEGIN_MESSAGE_MAP(CqextendApp, CWinApp)
END_MESSAGE_MAP()

// CqextendApp 构造
CqextendApp::CqextendApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	theApp.m_onQuoteChanged = NULL;
	theApp.m_onBusiness = NULL;
}

// 唯一的一个 CqextendApp 对象
CqextendApp theApp;

// CqextendApp 初始化
BOOL CqextendApp::InitInstance()
{
	m_nMode = 0;
	gbExitFlag = false;
	m_onQuoteChanged = NULL;
	gSumInfo.m_nBargain = 0;
	gSumInfo.m_nConsign = 0;
	gSumInfo.m_nMaxBargain = 0;
	gSumInfo.m_nMaxConsign = 0;
	CWinApp::InitInstance();
	return TRUE;
}

extern "C" __declspec(dllexport) void GetModuleVer(char** pBuf)
{
	static std::string strVersion;
	strVersion = gVer;

	char *lpCommxVer = NULL;
	CommxVer(&lpCommxVer);
	strVersion += lpCommxVer;

	strVersion += "$";

	*pBuf = (char*)strVersion.c_str();
}

extern "C" __declspec(dllexport) int Initialize(int iMode,void *)
{
	REPORT(MN, T("初始化行情扩展库...\n"), RPT_INFO);

	gbWorkDay = false;
	gbTradeTime = false;
	theApp.m_nMode = iMode;

	//初始化内存池
	//CHECK_RUN(!InitializeMemoryPool(), MN, T("初始化内存池失败\n"), RPT_ERROR, return -1);
	//CHECK_RUN(!CreateMemBlock(64, 10), MN, T("预分配内存[64]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(1024, 10), MN, T("预分配内存[1024]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(10240, 10), MN, T("预分配内存[10240]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(102400, 5), MN, T("预分配内存[102400]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(409600, 2), MN, T("预分配内存[409600]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});

	gbIsWorkTime = false;
	gbIsTradeTime = false;
	memset(gacTradeTime,0,sizeof(gacTradeTime));

	cfg.m_nStopSvcBegin = 1501;
	cfg.m_nStopSvcEnd = 1531;
	//解析配置文件
	cfg.Load("QuoteFarm.xml");
	//解析私有的配置文件
	cfg.Load("TradeMatch.xml");

	REPORT(MN,T("停止服务时间区间[%d][%d]\n",cfg.m_nStopSvcBegin,cfg.m_nStopSvcEnd),RPT_INFO);
	CHECK_RUN(!gFarm.mapfile(FILE_INDEX, cfg.m_nMarketCount,cfg.m_nSymbolCount / 2, cfg.m_nSymbolCount, cfg.m_nTickCount, cfg.m_nMinkCount),
		MN, T("映射行情数据文件%s失败\n", FILE_INDEX), RPT_ERROR, return -1);
	::CoInitialize(NULL);
	//if (GetWorkState() < 0)
	//	return -1;

	REPORT(MN,T("[IP:%s][DB:%s][U:%s][P:%s]\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);
/*	if (gDBRead.Open(cfg.m_strServerIP,cfg.m_strDataBase,cfg.m_strUser,cfg.m_strPass))
	{
		REPORT(MN,T("连接数据库成功！\n"),RPT_INFO);
	}
	else
		REPORT(MN,T("连接数据库失败[%s][%s][%s][%s]！\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);
*/
/*
	if (gDBWrite.Open(cfg.m_strServerIP,cfg.m_strDataBase,cfg.m_strUser,cfg.m_strPass))
	{
		REPORT(MN,T("连接数据库成功！\n"),RPT_INFO);
	}
	else
		REPORT(MN,T("连接数据库失败[%s][%s][%s][%s]！\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);
*/
	//启动工作线程
	gWorkThread.open();
	//启动入库线程序
	gWriteThread.open();
	//
	gInfoThread.open();
	return 0;
}

extern "C" __declspec(dllexport) void Release()
{
	REPORT(MN, T("接到通知推出\n"), RPT_INFO);
	gbExitFlag = true;
	gWorkThread.close();
	gWriteThread.close();
	gWorkThread.wait();
	gWriteThread.wait();
	REPORT(MN, T("工作线程已退出\n"), RPT_INFO);
	//if (theApp.m_nMode)
	//	ReleaseMemoryPool();
	::CoUninitialize();
}

extern "C" __declspec(dllexport) void RegisterCallback(pfnOnQuoteChanged funcQuote, pfnOnBusiness funcBusiness)
{
	theApp.m_onQuoteChanged = funcQuote;
	theApp.m_onBusiness = funcBusiness;
}

#ifdef _DEBUG
CHighPerformanceCounter ghpc;
#endif // _DEBUG

extern "C" __declspec(dllexport) void Sym_Begin()
{
#ifdef _DEBUG
	ghpc.count();
#endif // _DEBUG
	gDataStore.OnBegin();
}

extern "C" __declspec(dllexport) void OnQuote(int imode, const char *symbol, int *pFlag, Quote * quote, TickUnit *tickunit)
{/*
	char acBuf[7];
	strncpy(acBuf,quote->szStockCode,6);
	acBuf[6] = 0;
	if(atoi(acBuf) >= 300001 && atoi(acBuf) <= 300028)
		if (strcmp(quote->szStockKind,"md_szag") == 0)
			strcpy(quote->szStockKind,"md_cyb");*/
	switch (imode)
	{
	case 1:
		gDataStore.AddSymbol(symbol, pFlag);
		break;
	case 2:
		//REPORT(MN,T("OnQuote [%d][%s][%d]\n",imode,symbol,*pFlag),RPT_INFO);
		if (tickunit->Time > 0 && tickunit->Price > 0 && tickunit->Volume > 0)
			gDataStore.UpdateSymbol(symbol, pFlag,quote,tickunit);
		break;
	default:
		if (strstr(symbol,".sh") || strstr(symbol,".sz"))
			gDataStore.DelSymbol(symbol);
		break;
	}
}

extern "C" __declspec(dllexport) void Sym_Commit()
{
	//DEBUG_REPORT(MN, T("Prepare:%fs\n", ghpc.count()), RPT_DEBUG);
 	gDataStore.OnEnd();
	//DEBUG_REPORT(MN, T("Commit:%fs\n", ghpc.count()), RPT_DEBUG);
}

extern "C" __declspec(dllexport) bool OnRequest(unsigned int msgid, CommxHead *pRequest)
{
	return true;
}

extern "C" __declspec(dllexport)void OnTimer(short timer)
{
	REPORT(MN,T("TradeMatcher Accept OnTimer [%d]\n",timer),RPT_INFO);
}
