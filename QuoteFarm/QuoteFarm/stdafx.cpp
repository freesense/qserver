// stdafx.cpp : 只包括标准包含文件的源文件
// QuoteFarm.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用

HANDLE gQuit = NULL;

atomint g_nFeedRecvLength = 0;
atomint g_nTotalRecv = 0;
atomint g_nFeedPacketCount = 0;
MAPSPECIAL g_mapCloseSpecial;

void* ConnectFeed(const char *lpFeedName, const char *lpFeedAddress, CFeedSocket *pSocket)
{
	if (!pSocket)
	{
		pSocket = new CFeedSocket(lpFeedName);
		CHECK_RUN(!pSocket, MN, T("分配内存失败\n"), RPT_CRITICAL, return NULL);
	}
	pSocket->Connect(lpFeedAddress);
	return pSocket;
}

int _find_close_special::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	g_mapCloseSpecial[symbol].push_back(pri);
	return 0;
}
