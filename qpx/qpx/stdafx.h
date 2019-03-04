// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#define MN "qpx"

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <stdio.h>
#include <tchar.h>
#include <map>
#include <string>
#include "../../public/commx/commxapi.h"
#include "../../public/commx/report.h"
#include "../../public/commx/synch.h"
#include "../../public/commx/mery.h"

#ifdef _DEBUG
//for memory leak check
#define _CRTDBG_MAP_ALLOC //使生成的内存dump包含内存块分配的具体代码为止
#include <stdlib.h> 
#include <crtdbg.h>
#define CheckMemoryLeak _CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG )| _CRTDBG_LEAK_CHECK_DF)
#endif

#include "condef.h"

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif // _DEBUG

extern std::string strQuoteAddress, strHostAddress;

class C2cc;
extern unsigned short g_serialno;
extern std::map<unsigned short, C2cc*> g_mapClient;
extern LockSingle g_lockClients;

extern unsigned short onRequest(C2cc *pClient);
extern C2cc* onAnswer(unsigned short serialno);
extern void onRelease(C2cc *pClient);

#pragma warning(disable:4819)

#define QPXVER "$Qpx#2.1.9$"

extern std::map<unsigned short, char*> g_mapCltData;
extern void InsertCltData(unsigned short nSerialNo, char* pData);
extern char* RemoveCltData(unsigned short nSerialNo);

#define	RPT_HEARTBEAT  0xAA	//心跳日志，用来监控线程
#define	RPT_ADDI_INFO  0x90 //附加信息，可以报告设备的状态等信息
// TODO: 在此处引用程序需要的其他头文件
