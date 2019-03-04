// myservice.cpp

#include "stdafx.h"
#include "myservice.h"

CMyService::CMyService()
:CNTService("QuoteFarm")
{
	m_iMajorVersion = 2;
	m_iMinorVersion = 1;
}

BOOL CMyService::OnInit()
{
	return TRUE;
}

int CMyService::Run()
{
	return _internal_main();
}

void CMyService::OnStop()
{
	::SetEvent(gQuit);
}

void CMyService::OnShutdown()
{
	OnStop();
}

BOOL CMyService::OnUserControl(DWORD dwOpcode)
{
    switch (dwOpcode)
	{
    case SERVICE_CONTROL_USER + 0:
        return TRUE;

    default:
        break;
    }
    return FALSE; // say not handled
}
