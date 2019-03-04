// qextend.cpp : ���� DLL �ĳ�ʼ�����̡�
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
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// CqextendApp

BEGIN_MESSAGE_MAP(CqextendApp, CWinApp)
END_MESSAGE_MAP()

// CqextendApp ����
CqextendApp::CqextendApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
	theApp.m_onQuoteChanged = NULL;
	theApp.m_onBusiness = NULL;
}

// Ψһ��һ�� CqextendApp ����
CqextendApp theApp;

// CqextendApp ��ʼ��
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
	REPORT(MN, T("��ʼ��������չ��...\n"), RPT_INFO);

	gbWorkDay = false;
	gbTradeTime = false;
	theApp.m_nMode = iMode;

	//��ʼ���ڴ��
	//CHECK_RUN(!InitializeMemoryPool(), MN, T("��ʼ���ڴ��ʧ��\n"), RPT_ERROR, return -1);
	//CHECK_RUN(!CreateMemBlock(64, 10), MN, T("Ԥ�����ڴ�[64]ʧ��\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(1024, 10), MN, T("Ԥ�����ڴ�[1024]ʧ��\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(10240, 10), MN, T("Ԥ�����ڴ�[10240]ʧ��\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(102400, 5), MN, T("Ԥ�����ڴ�[102400]ʧ��\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	//CHECK_RUN(!CreateMemBlock(409600, 2), MN, T("Ԥ�����ڴ�[409600]ʧ��\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});

	gbIsWorkTime = false;
	gbIsTradeTime = false;
	memset(gacTradeTime,0,sizeof(gacTradeTime));

	cfg.m_nStopSvcBegin = 1501;
	cfg.m_nStopSvcEnd = 1531;
	//���������ļ�
	cfg.Load("QuoteFarm.xml");
	//����˽�е������ļ�
	cfg.Load("TradeMatch.xml");

	REPORT(MN,T("ֹͣ����ʱ������[%d][%d]\n",cfg.m_nStopSvcBegin,cfg.m_nStopSvcEnd),RPT_INFO);
	CHECK_RUN(!gFarm.mapfile(FILE_INDEX, cfg.m_nMarketCount,cfg.m_nSymbolCount / 2, cfg.m_nSymbolCount, cfg.m_nTickCount, cfg.m_nMinkCount),
		MN, T("ӳ�����������ļ�%sʧ��\n", FILE_INDEX), RPT_ERROR, return -1);
	::CoInitialize(NULL);
	//if (GetWorkState() < 0)
	//	return -1;

	REPORT(MN,T("[IP:%s][DB:%s][U:%s][P:%s]\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);
/*	if (gDBRead.Open(cfg.m_strServerIP,cfg.m_strDataBase,cfg.m_strUser,cfg.m_strPass))
	{
		REPORT(MN,T("�������ݿ�ɹ���\n"),RPT_INFO);
	}
	else
		REPORT(MN,T("�������ݿ�ʧ��[%s][%s][%s][%s]��\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);
*/
/*
	if (gDBWrite.Open(cfg.m_strServerIP,cfg.m_strDataBase,cfg.m_strUser,cfg.m_strPass))
	{
		REPORT(MN,T("�������ݿ�ɹ���\n"),RPT_INFO);
	}
	else
		REPORT(MN,T("�������ݿ�ʧ��[%s][%s][%s][%s]��\n",cfg.m_strServerIP.c_str(),cfg.m_strDataBase.c_str(),cfg.m_strUser.c_str(),cfg.m_strPass.c_str()),RPT_INFO);
*/
	//���������߳�
	gWorkThread.open();
	//��������߳���
	gWriteThread.open();
	//
	gInfoThread.open();
	return 0;
}

extern "C" __declspec(dllexport) void Release()
{
	REPORT(MN, T("�ӵ�֪ͨ�Ƴ�\n"), RPT_INFO);
	gbExitFlag = true;
	gWorkThread.close();
	gWriteThread.close();
	gWorkThread.wait();
	gWriteThread.wait();
	REPORT(MN, T("�����߳����˳�\n"), RPT_INFO);
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
