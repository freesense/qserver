// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#pragma warning(disable:4311 4312)

#define MN "Farm"

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif
#pragma comment(lib, "../../public/lib/kvisitor")

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <stdio.h>
#include <set>
#include <tchar.h>
#include <Windows.h>
#include "../../public/commx/synch.h"
#include "../../public/commx/report.h"
#include "../../public/commx/commxapi.h"
#include "../../public/commx/encryption.h"
#include "../../public/data_struct.h"
#include "../../public/kvisitor.h"
#include "communication.h"

const char gVer[] = "$Quotefarm.exe#2.0.62$";

extern HANDLE gQuit;
typedef std::map<std::string, std::vector<RINDEX*> > MAPSPECIAL;
typedef std::map<std::string, std::vector<RINDEX*> >::iterator SPECIALITER;
extern MAPSPECIAL g_mapCloseSpecial;

//以下是统计数据
extern atomint g_nFeedRecvLength;		/** 从Feed收到的数据长度(每个计时周期) [5/22/2009 xinl] */
extern atomint g_nTotalRecv;			/** 从Feed收到的数据长度(总长度) [5/22/2009 xinl] */
extern atomint g_nFeedPacketCount;		/** 从Feed收到的数据包总数 [5/22/2009 xinl] */

extern void OnAnswer(unsigned int msgid, CommxHead *pHead);	//插件qextend的请求回调函数
extern void* ConnectFeed(const char *lpFeedName, const char *lpFeedAddress, CFeedSocket *pSocket = NULL);
extern int _internal_main();

//版本函数
typedef void(*pfnGetModuleVer)(char **lpVer);

struct _find_close_special
{
	int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);
	std::string symbol;
};
