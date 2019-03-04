// WatchDog.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DogService.h"
#include "ReportLog.h"

int main(int argc, char* argv[])
{
	std::string s = GetExecutePath();
	unsigned int pos = (unsigned int)s.rfind("\\");
	s = s.substr(0, pos+1);
	SetCurrentDirectory(s.c_str());

#ifdef CONSOLEAPP
	CReportBase *pr1 = new CReportBase;
	CReportBox::Instance()->AddReportUnit(pr1);
#endif

	gExecutePath = GetExecutePath();
	gExecutePath = gExecutePath.substr(0, gExecutePath.rfind('\\') + 1);

	CReportFile *pr2 = new CReportFile(MODULE_NAME);
	CReportBox::Instance()->AddReportUnit(pr2);

	g_pLog = new CReportLog();
	CReportBox::Instance()->AddReportUnit(g_pLog);

	cfg.Load((gExecutePath + "WatchDog.xml").c_str());
	CDogService MyService;

#ifdef CONSOLEAPP
	MyService.Run();
#else
	if (!MyService.ParseStandardArgs(argc, argv))
		MyService.StartService();
#endif

	CReportBox::Instance()->Close();
	return MyService.m_Status.dwWin32ExitCode;
}
