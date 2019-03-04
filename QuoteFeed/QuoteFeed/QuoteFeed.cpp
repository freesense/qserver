// QuoteFeed.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "config.h"
#include "feedowner.h"
#include "feedservice.h"
#include <string>
#include <iostream>
#include "UseLogData.h"
#include "../../public/devmonitor/ReportServer.h"
#include "../../public/devmonitor/DevMonitor.h"
#include "QuoteFeedService.h"

/*
#ifndef _DEBUG
#define SERVICE    //����Ϊ����
#endif 
//*/
#define SERVICE_NAME "QuoteFeed"

#pragma comment(lib, "MPR.LIB")
////////////////////////////////////////////////////////////////////////////////
CQuoteFeedService g_Service;

#ifndef SERVICE
CReportBase   *g_prpt_stdout = NULL;
#endif
CReportFile   *g_prpt_file = NULL;
CReportServer *g_prpt_server = NULL;

CFeedServer g_objServer;
CFeedOwner g_objFeedOwner(&g_objServer);
CUseLogData g_useLogData(&g_objFeedOwner);

int InitFeed()
{
	CRC32::prepareCrc32Table();

	std::string strExecutePath = GetExecutePath();
	strExecutePath = strExecutePath.substr(0, strExecutePath.rfind("\\") + 1);
	std::string strXmlPath = strExecutePath + "QuoteFeed.xml";
	g_cfg.Load(strXmlPath.c_str());

	//*//�豸���
	BOOL bRet = g_DevMonitor.Init(g_cfg.m_strDevServIP, 
		g_cfg.m_nDevServPort, g_cfg.m_strAppName);
	CHECK_RUN(!bRet, MN, T("��ʼ���豸���ģ��ʧ��\n"), RPT_ERROR, {});
	//end  //*/

	CHECK_RUN(!InitializeMemoryPool(), MN, T("��ʼ���ڴ��ʧ��\n"), RPT_CRITICAL, {CReportBox::Instance()->Close(); return -1;});

	CHECK_RUN(-1 == start_comm_loop(), MN, T("����ͨѶ�߳�ʧ��\n"), RPT_ERROR, {CReportBox::Instance()->Close(); return -1;});
	g_objServer.Listen(g_cfg.m_strServiceAddress.c_str(), 2);

	char szDllVer[100];
	char* pDllVerPos = szDllVer;
	int nLen = 0;
	if (!g_cfg.m_bUserDBF)
	{
		if (!g_useLogData.Init())
			REPORT(MN, T("ʹ����־�ļ�����Դ��ʼ��ʧ��\n"), RPT_ERROR);
		else
			REPORT(MN, T("ʹ����־�ļ�����Դ��ʼ���ɹ�\n"), RPT_INFO);
	}
	else
	{
		for (unsigned int i = 0; i < g_cfg.m_vDll.size(); i++)
		{
			if (!g_cfg.m_vDll[i]->m_bLoad)
				continue;
			pfnInitialize _Initialize = (pfnInitialize)g_cfg.m_vDll[i]->Symbol("Initialize");
			if (_Initialize == NULL)
				continue;

			unsigned int nParamsCount = (unsigned int)g_cfg.m_vDll[i]->m_strParams.size();
			nParamsCount++; //���ӳ�ʱʱ��
			char ** ppParams = new char*[nParamsCount];
			CHECK_RUN(!ppParams, MN, T("�����ڴ�ʧ��\n"), RPT_CRITICAL, {CReportBox::Instance()->Close(); return -1;});
																
			for (unsigned int j = 0; j < nParamsCount-1; j++)
				ppParams[j] = (char*)g_cfg.m_vDll[i]->m_strParams[j].c_str();

			char szTimeout[20];
			itoa(g_DevMonitor.GetTimeOut(), szTimeout, 10);	//��ʱʱ��
			ppParams[nParamsCount-1] = szTimeout;

			if (!_Initialize(nParamsCount, ppParams, &g_objFeedOwner))
				REPORT(MN, T("��ʼ��%sʧ��\n", g_cfg.m_vDll[i]->m_strName.c_str()), RPT_ERROR);

			//���ģ��汾��Ϣ
			pfnGetModuleVer _GetModuleVer = (pfnGetModuleVer)g_cfg.m_vDll[i]->Symbol("GetModuleVer");
			char* pVer = NULL;
			_GetModuleVer(&pVer);
			strcpy_s(pDllVerPos, 100 - nLen, pVer);
			nLen += (int)strlen(pVer);
			pDllVerPos += strlen(pVer);
			//

			delete []ppParams;
		}
	}

	char* pCommxVer = NULL;
	CommxVer(&pCommxVer);
	REPORT(MN, T("%s%s%s%s\n", FEEDVER, szDllVer, g_DevMonitor.m_strVer.c_str(), pCommxVer), RPT_VERSION);

	return 0;
}

void UninitFeed()
{
	//�豸���
	g_DevMonitor.Uninit();
	//end

	if (g_cfg.m_bUserDBF)
	{
		for (unsigned int i = 0; i < g_cfg.m_vDll.size(); i++)
		{
			pfnRelease _Release = (pfnRelease)g_cfg.m_vDll[i]->Symbol("Release");
			if (_Release != NULL)
				_Release();
		}
	}
	else
	{
		g_useLogData.Release();
	}

	stop_comm_loop();
	g_objServer.Close(); 

	ReleaseMemoryPool();  
	CReportBox::Instance()->Close();
}

int _tmain(int argc, _TCHAR* argv[])
{	
#ifdef _DEBUG
	CheckMemoryLeak;
#endif

	std::string strExecutePath = GetExecutePath();
	std::string strWorkPath = strExecutePath.substr(0, strExecutePath.rfind("\\"));
	::SetCurrentDirectory(strWorkPath.c_str());

#ifndef SERVICE
	g_prpt_stdout = new CReportBase;
#endif
	g_prpt_file  = new CReportFile(MN);
	g_prpt_server = new CReportServer;

#ifndef SERVICE	   
	CReportBox::Instance()->AddReportUnit(g_prpt_stdout);
#endif		 
	CReportBox::Instance()->AddReportUnit(g_prpt_file);
	CReportBox::Instance()->AddReportUnit(g_prpt_server);

#ifndef SERVICE	 
	InitFeed();
#endif		 
	//////////////////////////////////////////////////////////////
#ifdef SERVICE
	if (argc <= 1)
	{
		g_Service.StartServiceMain(SERVICE_NAME, &g_Service);
		return 0;
	}

	if (_stricmp(argv[1], "-i") == 0)
	{
		if(!g_Service.Install(SERVICE_NAME))
			REPORT(MN, T("Install Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
		else
			REPORT(MN, T("Install Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
	}
	else if (_stricmp(argv[1], "-u") == 0)
	{
		if(!g_Service.Uninstall(SERVICE_NAME))
			REPORT(MN, T("UnInstall Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
		else
			REPORT(MN, T("UnInstall Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
	}
	else if (_stricmp(argv[1], "-o") == 0)
	{
		if(!g_Service.StartService(SERVICE_NAME))
			REPORT(MN, T("Start Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
		else
			REPORT(MN, T("Start Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
	}
	else if (_stricmp(argv[1], "-c") == 0)
	{
		if(!g_Service.StopService(SERVICE_NAME))
			REPORT(MN, T("Stop Service [%s] failed\n", SERVICE_NAME), RPT_ERROR);
		else
			REPORT(MN, T("Stop Service [%s] successed\n", SERVICE_NAME), RPT_INFO);
	}
#else

	////////////////////////////////////////////////////////
	/*
	while(1)
	{
		static int nindex = 1;
		REPORT(MN, T("Test %d\n", nindex++), RPT_INFO);
		REPORT(MN, T("Test %d\n", nindex++), RPT_WARNING);
		//REPORT(MN, T("Test %d\n", nindex++), RPT_ERROR);
		//REPORT(MN, T("Test %d\n", nindex++), RPT_DEBUG);
	}  //*/
	////////////////////////////////////////////////////////
	std::string strCommand;
	while (1)
	{
		std::cin >> strCommand;
		if (strCommand == "quit")
			break;
	}

	UninitFeed();
#endif	
	return 0;
}



