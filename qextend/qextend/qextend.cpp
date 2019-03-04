// qextend.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "qextend.h"
#include "../../public/commx/commxapi.h"
#include "../../public/commx/code_convert.h"
#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>
#include <WinSock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 唯一的一个 CqextendApp 对象
CqextendApp theApp;

CqextendApp::CqextendApp()
{
	m_nMode = 0;
	theApp.m_onBusiness = NULL;
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
	REPORT(MN, T("初始化行情扩展库...\n"), RPT_INFO);

	theApp.m_nMode = iMode;

	//初始化内存池
	CHECK_RUN(!InitializeMemoryPool(), MN, T("初始化内存池失败\n"), RPT_ERROR, return -1);
	CHECK_RUN(!CreateMemBlock(64, 10), MN, T("预分配内存[64]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	CHECK_RUN(!CreateMemBlock(1024, 10), MN, T("预分配内存[1024]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	CHECK_RUN(!CreateMemBlock(10240, 10), MN, T("预分配内存[10240]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	CHECK_RUN(!CreateMemBlock(102400, 5), MN, T("预分配内存[102400]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});
	CHECK_RUN(!CreateMemBlock(409600, 2), MN, T("预分配内存[409600]失败\n"), RPT_ERROR, {ReleaseMemoryPool();return -1;});

	//解析配置文件
	cfg.Load("qextend.xml");
	cfg.Load("QuoteFarm.xml");

	gThread.open();
	return 0;
}

extern "C" __declspec(dllexport) void Release()
{
	gThread.close();
	gThread.wait();
	REPORT(MN, T("工作线程已退出\n"), RPT_INFO);
	if (theApp.m_nMode)
		ReleaseMemoryPool();
}

extern "C" __declspec(dllexport) void RegisterCallback(pfnOnQuoteChanged funcQuote, pfnOnBusiness funcBusiness)
{
	theApp.m_onBusiness = funcBusiness;
}

extern "C" __declspec(dllexport) bool OnRequest(unsigned int msgid, CommxHead *pRequest)
{
	unsigned int len = pRequest->Length;
	/** 跳过包头+4字节 [7/17/2009 xinl] */
	unsigned short *pusFunc = (unsigned short*)((char*)pRequest + sizeof(CommxHead)/* + 2 * sizeof(short)*/);
	unsigned short usFunc = *pusFunc;
	if (CommxHead::NETORDER == pRequest->GetByteorder())
	{
		len = ntohl(len);
		usFunc = ntohs(usFunc);
	}

	//不支持的外部业务
	if (usFunc != 6000 && usFunc != 6001 && usFunc != 6002 && usFunc != 6003 && usFunc != 6004)
		return false;

	char *pBuf = (char*)mpnew(sizeof(CommxHead) + len + sizeof(unsigned int) + 1);
	CHECK_RUN(!pBuf, MN, T("分配内存[%d]失败！\n", len), RPT_CRITICAL, return false);
	memcpy(pBuf, pRequest, sizeof(CommxHead));
	memcpy(pBuf + sizeof(CommxHead), pusFunc, len);
	pBuf[sizeof(CommxHead) + len] = 0x00;
	*(unsigned int*)(pBuf + sizeof(CommxHead) + len + 1) = msgid;
	gQueue.Push((CommxHead*)pBuf);
	return true;
}

extern "C" __declspec(dllexport)void OnTimer(short timer)
{
}
