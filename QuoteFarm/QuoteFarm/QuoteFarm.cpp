// QuoteFarm.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <vld.h>
#include "config.h"
#include "farm.h"
#include "task.h"
#include "addin_manager.h"
#include "communication.h"
#include "farmtimer.h"
#include "myservice.h"
#include "../../public/devmonitor/ReportServer.h"
#include "../../public/devmonitor/DevMonitor.h"
#include "../../public/commx/mery.h"
#include <direct.h>

struct _mod_ver
{
	std::string name;
	std::string version;

	_mod_ver(const char *lpVer)
	{
		std::string s = lpVer;
		unsigned int pos = (unsigned int)s.find("#");
		name = s.substr(s.find("$")+1, pos);
		version = s.substr(pos+1);
	}

	bool operator==(const _mod_ver &mv) const
	{
		return (name == mv.name && version == mv.version);
	}

	bool operator<(const _mod_ver &mv)
	{
		return (name < mv.name);
	}
};

void LoadAddins()
{
	std::string sVersion = gVer;
	char *pVer = NULL;
	CommxVer(&pVer);
	sVersion += pVer;
	sVersion += g_DevMonitor.m_strVer;

	for (unsigned int i = 0; i < g_cfg.m_vAddin.size(); i++)
	{
		if (g_cfg.m_vAddin[i].bStatus)
		{
			REPORT(MN, T("װ�ز��%s......\n", g_cfg.m_vAddin[i].strPath.c_str()), RPT_INFO);
			char *pVer = NULL;
			gAddin.LoadAddin(g_cfg.m_vAddin[i], &pVer);
			REPORT(MN, T("װ�ز���ɹ�\n"), RPT_INFO);
			sVersion += pVer;
		}
	}

	std::list<_mod_ver> mvs;
	char *lpToken = strtok((char*)sVersion.c_str(), "$");
	while (lpToken)
	{
		mvs.push_back(_mod_ver(lpToken));
		lpToken = strtok(NULL, "$");
	}
	mvs.sort();
	mvs.unique();

	sVersion = "";
	for (std::list<_mod_ver>::iterator iter = mvs.begin(); iter != mvs.end(); ++iter)
		sVersion += "$" + iter->name + "#" + iter->version + "$";

	//�ϱ��汾��Ϣ
	REPORT(MN, T("%s\n", sVersion.c_str()), RPT_VERSION);
}

int _internal_main()
{
	time_t now;
	time(&now);
	struct tm *lt = localtime(&now);

	{/// Ϊ������ĳɽ����ͳɽ�����㷨��׼��
		std::vector<RINDEX*> tmp;
		g_mapCloseSpecial["399001.SZ"] = tmp;
		g_mapCloseSpecial["000001.SH"] = tmp;
	}

	/** ��ʼ��CRC32�� [7/10/2009 xinl] */
	CRC32::prepareCrc32Table();

	CReportBase *rpt_stdout = new CReportBase;
	CReportFile *rpt_file = new CReportFile(MN);
	CReportServer *rpt_server = new CReportServer;
	CReportBox::Instance()->AddReportUnit(rpt_stdout);
	CReportBox::Instance()->AddReportUnit(rpt_file);
 	CReportBox::Instance()->AddReportUnit(rpt_server);
	REPORT(MN, T("---------- %04d-%02d-%02d ----------\n", lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday), RPT_INFO|RPT_NOPREFIX);

	std::string strExecutePath = GetExecutePath();
	strExecutePath = strExecutePath.substr(0, strExecutePath.rfind("\\") + 1);
	g_cfg.m_strXmlFile = strExecutePath + "QuoteFarm.xml";
	g_cfg.Load(g_cfg.m_strXmlFile.c_str());
	REPORT(MN, T("װ�������ļ�%s......OK\n", g_cfg.m_strXmlFile.c_str()), RPT_INFO);

	BOOL bRet = g_DevMonitor.Init(g_cfg.m_strDevServIP, g_cfg.m_nDevServPort, g_cfg.m_strAppName);
	CHECK_RUN(!bRet, MN, T("��ʼ���豸���ģ��ʧ��\n"), RPT_ERROR, );

	InitializeMemoryPool();
	for (std::map<unsigned int, unsigned int>::iterator iter = g_cfg.m_mpMemPool.begin(); iter != g_cfg.m_mpMemPool.end(); ++iter)
		CHECK_RUN(!CreateMemBlock(iter->first, iter->second), MN, T("��ʼ���ڴ��[%d*%d]ʧ��\n", iter->first, iter->second), RPT_CRITICAL, {ReleaseMemoryPool(); return -1;});
	REPORT(MN, T("��ʼ���ڴ��......OK\n"), RPT_INFO);

	CDataFarm farm;
	CHECK_RUN(!farm.mapfile(FILE_INDEX, (unsigned int)g_cfg.m_vMarket.size(),
		g_cfg.m_nSymbolCount / 2, g_cfg.m_nSymbolCount, g_cfg.m_nTickCount, g_cfg.m_nMinCount),
		MN, T("�������������ļ�%sʧ��\n", FILE_INDEX), RPT_ERROR, {CReportBox::Instance()->Close(); return -1;});
	g_cfg.UpdateMarketStatus(farm.m_pMarketStatus);
	REPORT(MN, T("��ʼ�����������ļ�......OK\n"), RPT_INFO);
	farm.SelfCheck();

	_mkdir(g_cfg.m_strHisPath.c_str());
// 	_mkdir((g_cfg.m_strHisPath + "/" + HISK_DAY).c_str());
// 	_mkdir((g_cfg.m_strHisPath + "/" + HISK_WEK).c_str());
// 	_mkdir((g_cfg.m_strHisPath + "/" + HISK_MON).c_str());
	_mkdir((g_cfg.m_strHisPath.substr(0, g_cfg.m_strHisPath.rfind('\\') + 1) + HIS_DETAIL).c_str());
	REPORT(MN, T("�������ݲֿ�Ŀ¼......OK\n"), RPT_INFO);

	gtask.SetFarmPtr(&farm);
	gtask.open(0, g_cfg.m_nTaskCount);
	CHECK_RUN(-1 == start_comm_loop(1), MN, T("���������߳�ʧ��\n"), RPT_ERROR, {CReportBox::Instance()->Close(); return -1;});
	REPORT(MN, T("���������߳�......OK\n"), RPT_INFO);

	gAddin.SetFarm(&farm);
	LoadAddins();

	for (unsigned int i = 0; i < g_cfg.m_vFeed.size(); i++)
	{
		if (g_cfg.m_vFeed[i].bStatus)
			g_cfg.m_vFeed[i].pHandler = ConnectFeed(g_cfg.m_vFeed[i].strName.c_str(), g_cfg.m_vFeed[i].strAddress.c_str());
	}
	REPORT(MN, T("��ʼ������Feedͨ��......OK\n"), RPT_INFO);

	CSharedServer server;
	gtask.SetSharedServer(&server);
	server.m_pFarm = &farm;
	CHECK_RUN(-1 == server.Listen(g_cfg.m_strCheckAddress.c_str(), 2), MN,
		T("���������ַ[%s]ʧ��\n", g_cfg.m_strCheckAddress.c_str()), RPT_ERROR,
		{CReportBox::Instance()->Close(); return -1;});
	REPORT(MN, T("��[%s]�ϵķ���......OK\n", g_cfg.m_strCheckAddress.c_str()), RPT_INFO);

	CFarmTimer ftimer(&server, &farm);
	ftimer.RegisterTimer(&ftimer.m_tcb, (float)g_cfg.m_nHeartbeat, (float)g_cfg.m_nHeartbeat);
	REPORT(MN, T("��ʼ�����ж�ʱ��......OK\n"), RPT_INFO);

	gQuit = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	while (::WaitForSingleObject(gQuit, 10 * 1000) == WAIT_TIMEOUT)
	{
		time_t now;
		time(&now);
		gtask.m_lock.lock();
		if (gtask.m_chkmkt.size() == 0)
		{
			g_cfg.CheckTaskOnTime(now, gtask.m_chkmkt);
			if (gtask.m_chkmkt.size())
			{
				char *pChkmkt = (char*)mpnew(sizeof(FeedHead) + sizeof(time_t));
				CHECK_RUN(!pChkmkt, MN, T("�����ڴ�[%d]ʧ��\n", sizeof(FeedHead) + sizeof(time_t)), RPT_CRITICAL, exit(1));
				memset(pChkmkt, 0x00, sizeof(FeedHead) + sizeof(time_t));
				FeedHead *pHead = (FeedHead*)pChkmkt;
				pHead->nFeedType = 0xff00;
				struct tm *lt = localtime(&now);
				unsigned short lnow = lt->tm_hour * 100 + lt->tm_min;
				pHead->nTime = lnow;
				*(time_t*)(pChkmkt + sizeof(FeedHead)) = now;
				gtask.AddFeed(pHead, sizeof(FeedHead) + sizeof(time_t));
			}
		}
		gtask.m_lock.unlock();
	}

	server.Close();

	g_cfg.m_feedLock.WaitWrite();
	for (unsigned int i = 0; i != g_cfg.m_vFeed.size(); i++)
	{
		CFeedSocket *pHandle = (CFeedSocket*)g_cfg.m_vFeed[i].pHandler;
		if (pHandle)
			pHandle->Close();
	}
	g_cfg.m_feedLock.EndWrite();
	Sleep(200);

	ftimer.CancelTimer(ftimer.m_tcb.m_nTimerID);
	gtask.close();
	stop_comm_loop();
	gtask.wait();

	gAddin.ReleaseAddin();

	::CloseHandle(gQuit);
	g_DevMonitor.Uninit();

	Sleep(3000);
	ReleaseMemoryPool();
	CReportBox::Instance()->Close();
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::string s = GetExecutePath();
	unsigned int pos = (unsigned int)s.rfind("\\");
	s = s.substr(0, pos+1);
	SetCurrentDirectory(s.c_str());

	// �����������
	CMyService MyService;
// 	return MyService.Run();

#ifdef _DEBUG
	return MyService.Run();
#else
	// ������׼���� (��װ, ж��, �汾��.)
	if (!MyService.ParseStandardArgs(argc, argv))
	{
		// δ�����κα�׼������������������
		// ȡ������ DebugBreak �����е�ע�ͣ�
		// ����������������������
		//DebugBreak();
		MyService.StartService();
	}

	// ����������Ѿ�ֹͣ
	return MyService.m_Status.dwWin32ExitCode;
#endif
}
