// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define MN "Feed"

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#ifdef _DEBUG
//for memory leak check
#define _CRTDBG_MAP_ALLOC //使生成的内存dump包含内存块分配的具体代码为止
#include <stdlib.h> 
#include <crtdbg.h>
#define CheckMemoryLeak _CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG )| _CRTDBG_LEAK_CHECK_DF)
#endif

#include "../../public/commx/mery.h"
#include "../../public/commx/report.h"
#include "../../public/commx/commxapi.h"
#include "../../public/commx/encryption.h"
#include "../../public/feedinterface.h"

typedef bool(*pfnInitialize)(int argc, char *argv[], IFeedOwner *pOwner);
typedef bool(*pfnRequest)(char *lpData, unsigned int nLength, void *lpParam);
typedef void(*pfnRelease)();


#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd.lib")
#else
#pragma comment(lib, "../../public/lib/commxr.lib")
#endif

#define FEEDVER "$QuoteFeed#2.1.0$"

extern CRITICAL_SECTION g_csCount;
extern double g_nSendSuccess;
// TODO: 在此处引用程序需要的其他头文件
