// stdafx.cpp : source file that includes just the standard includes
//	WatchDog.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "../../public/devmonitor/proxy_auth_server_base.h"

std::string        gExecutePath;
CLoginDevMap       g_mapDevList;
CRITICAL_SECTION   g_csDevList;
extern IServerAuthProxy *g_pAuthProxy;

void AddDev(const char* pDevName, unsigned long dwProID)
{
	::EnterCriticalSection(&g_csDevList);

	CLoginDevMap::iterator find = g_mapDevList.find(dwProID);
	if (find == g_mapDevList.end())
	{
		g_mapDevList.insert(std::make_pair(dwProID, pDevName));
	}

	::LeaveCriticalSection(&g_csDevList);
}

void DelDev(unsigned long dwProID)
{
	::EnterCriticalSection(&g_csDevList);

	CLoginDevMap::iterator find = g_mapDevList.find(dwProID);
	if (find != g_mapDevList.end())
	{									 
		g_mapDevList.erase(find);
	}

	::LeaveCriticalSection(&g_csDevList);
}

void SendDevList()
{
	std::string strDevList;
	::EnterCriticalSection(&g_csDevList);

	CLoginDevMap::iterator pos = g_mapDevList.begin();
	char szProID[50] = {0};
	while(pos != g_mapDevList.end())
	{
		strDevList += "$";
		strDevList += pos->second;
		strDevList += "#";
		sprintf_s(szProID, 50, "%d", pos->first);
		strDevList += szProID;
		strDevList += "$";
		pos++;
	}

	::LeaveCriticalSection(&g_csDevList);

	g_pAuthProxy->OnServerLogin(MODULE_NAME, strDevList.c_str());
}
// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
