// qpx.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "socketx.h"
#include "config.h"
#include "../../public/devmonitor/ReportServer.h"
#include "../../public/devmonitor/DevMonitor.h"
#include "QpxService.h"
//暂时不使用工作线程，因为完成端口模型已经有多线程支持，且整个业务过程中cpu都是全速运行
// #include "thread.h"

//*
#ifndef _DEBUG
#define SERVICE    //定义为服务
#endif
//*/   
#define SERVICE_NAME "qpx"

#ifndef SERVICE
CReportBase   *g_prpt_stdout = NULL;
#endif
CReportFile   *g_prpt_file = NULL;
CReportServer *g_prpt_server = NULL;

CQpxService g_Service;

int start()
{
	std::string strExecutePath = GetExecutePath();
	strExecutePath = strExecutePath.substr(0, strExecutePath.rfind("\\") + 1);
	std::string strXmlPath = strExecutePath + "qpx.xml";
	g_cfg.Load(strXmlPath.c_str());

	strQuoteAddress = g_cfg.m_strQuoteAddress;
	strHostAddress  = g_cfg.m_strHostAddress;

	//*//设备监控
	BOOL bRet = g_DevMonitor.Init(g_cfg.m_strDevServIP, 
		g_cfg.m_nDevServPort, g_cfg.m_strAppName);
	CHECK_RUN(!bRet, MN, T("初始化设备监控模块失败\n"), RPT_ERROR, {});
	//end  //*/

	InitializeMemoryPool();
	CreateMemBlock(256, 32);
	CreateMemBlock(512, 32);
	CreateMemBlock(1024, 32);
	CreateMemBlock(2048, 32);
	CreateMemBlock(4096, 32);

	start_comm_loop();

	g_pcq = new C2q;
	CHECK_RUN(!g_pcq, MN, T("分配内存[%d]失败\n", sizeof(C2q)), RPT_CRITICAL, return -1);
	g_pcq->Connect(strQuoteAddress.c_str());
	
	g_pcs = new C2cs;
	CHECK_RUN(!g_pcq, MN, T("分配内存[%d]失败\n", sizeof(C2cs)), RPT_CRITICAL, return -1);
	g_pcs->Listen(strHostAddress.c_str(), 100);

	//版本信息
	char* pCommxVer = NULL;
	CommxVer(&pCommxVer);
	REPORT(MN, T("%s%s%s\n", QPXVER, g_DevMonitor.m_strVer.c_str(), pCommxVer), RPT_VERSION);
	return 0;
}

void Stop()
{
	g_pcq->Close();
	g_pcs->Close();
	//设备监控
	g_DevMonitor.Uninit();
	//end

	stop_comm_loop();
	ReleaseMemoryPool();
	CReportBox::Instance()->Close();

	delete g_pcq;
}

int _tmain(int argc, _TCHAR* argv[])
{
	#ifdef _DEBUG
	CheckMemoryLeak;
#endif
	//todo: 修改成debug版为命令行程序，release版使用服务的模型
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
	start();
#endif

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
	std::string strCommand;
	while (1)
	{
		std::cin >> strCommand;
		if (strCommand == "quit")
			break;
	}

	Stop();
#endif
	//todo: 加强程序退出，客户端连接大批断开，随机断开的测试
	return 0;
}
