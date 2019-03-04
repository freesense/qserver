/** @file    proxy_auth_server.cpp
 *	@brief   �����������֤�������������
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

/** @brief      �ͷ���֤����
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

/** @brief      ��ʼ����֤����
 *
 *  
 *  @param      nTimeout������ʱ�������ʱ�䣨�룩
 *  @param      lpAuthAddress����֤������������IP
 *  @param      nPort����֤���������Ŷ˿�
 *  @param      pAuthCallback��������ʵ�ֵĴ���ص�ָ�룬���ΪNULL�������лص����������
 *  @retval     IServerAuthProxy*������ӿ�ָ�룬���ΪNULL����ʾ��ʼ��ʧ��
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
		CHECK_RUN(!pSehReport || !pSehThrow, MODULE_NAME, T("�����ڴ�ʧ��\n"), RPT_CRITICAL, return 0);
		SEH::Initialize(pSehReport, pSehThrow);
		PSEH->DoCatch();
		PSEH->DoCatchCpp();

		start_comm_loop(1);
		InitializeTimerQueue();

		g_pAuth = new CServerAuth(pAuthCallback);
		CHECK_RUN(!g_pAuth, MODULE_NAME, T("�����ڴ�[CServerAuth]ʧ��\n"), RPT_CRITICAL, return NULL);

		g_pProxySocket = new CProxySocket;
		CHECK_RUN(!g_pProxySocket, MODULE_NAME, T("�����ڴ�[CProxySocket]ʧ��\n"),
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
