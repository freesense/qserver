//QpxService.cpp
#include "stdafx.h"
#include "QpxService.h"

extern int start();
extern void Stop();
///////////////////////////////////////////////////////////////////////////////
CQpxService::CQpxService()
{
}

CQpxService::~CQpxService()
{
}

void CQpxService::Run()
{
	if (start() == -1)
	{
		REPORT(MN, T("start() failed\n"), RPT_ERROR);
		return;
	}
	CNTService::Run();
}

void CQpxService::Stop()
{
	CNTService::Stop();
	::Stop();
}