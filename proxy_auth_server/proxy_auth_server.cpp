/** @file    proxy_auth_server.cpp
 *	@brief   定义服务器认证代理的引出函数
 *  @ingroup proxy_auth_server
 *  
 */

#include "stdafx.h"
#include "server_auth.h"
#include "WSAEventSelect.h"
#include "../public/commx/xexception.h"
#include "../public/commx/mery.h"

char szVer[100] = {0};
unsigned int g_nTimeout = 60;

/** @brief      释放认证代理
 *
 *  
 *  @retval     void
 *  @warning    
 *  @see        
 *  @ingroup    
 *  @exception  
 */
extern "C" __declspec(dllexport) void Uninitialize()
{
	g_pProxySocket->finalize();
	while (g_pProxySocket)
		Sleep(20);

 	if (g_pAuth)
	{
		delete g_pAuth;
		g_pAuth = NULL;
	}

	UninitializeTimerQueue();
	stop_comm_loop();

	PSEH->Close();
	ReleaseMemoryPool();
}

/** @brief      初始化认证代理
 *
 *  
 *  @param      nTimeout，重连时间和心跳时间（秒）
 *  @param      lpAuthAddress，认证服务器域名或IP
 *  @param      nPort，认证服务器开放端口
 *  @param      pAuthCallback，服务器实现的代理回调指针，如果为NULL，则所有回调都不会进行
 *  @retval     IServerAuthProxy*，代理接口指针，如果为NULL，表示初始化失败
 *  @warning    
 *  @see        
 *  @ingroup    
 *  @exception  
 */
extern "C" __declspec(dllexport) IServerAuthProxy* Initialize(unsigned int nTimeout,
											   const char *lpAuthAddress,
											   unsigned short nPort,
											   IServerAuthBase *pAuthCallback)
{
	g_nTimeout = nTimeout;
	if (!g_pAuth)
	{
		InitializeMemoryPool();

		SehReportBase *pSehReport = new SehReport;
		SehThrowBase *pSehThrow = new SehThrowStd;
		CHECK_RUN(!pSehReport || !pSehThrow, MODULE_NAME, T("分配内存失败\n"), RPT_CRITICAL, return 0);
		SEH::Initialize(pSehReport, pSehThrow);
		PSEH->DoCatch();
		PSEH->DoCatchCpp();

		start_comm_loop(1);
		InitializeTimerQueue();

		g_pAuth = new CServerAuth(pAuthCallback);
		CHECK_RUN(!g_pAuth, MODULE_NAME, T("分配内存[CServerAuth]失败\n"), RPT_CRITICAL, return NULL);

		g_pProxySocket = new CProxySocket;
		CHECK_RUN(!g_pProxySocket, MODULE_NAME, T("分配内存[CProxySocket]失败\n"),
			RPT_CRITICAL, {delete g_pAuth; g_pAuth = NULL; return NULL;});

		char szAddress[51];
		sprintf(szAddress, "%s:%d", lpAuthAddress, nPort);
		g_pProxySocket->Connect(szAddress);
	}

	return g_pAuth;
}

extern "C" __declspec(dllexport) void GetModuleVer(char** pBuf)
{
	char* pCommxVer = NULL;
	CommxVer(&pCommxVer);

	strcpy_s((char*)szVer,	100, pCommxVer);
	int nLen = strlen(szVer);
	strcpy_s((char*)szVer+nLen, 100-nLen, PROXY_VER);

	*pBuf = (char*)szVer;
}
