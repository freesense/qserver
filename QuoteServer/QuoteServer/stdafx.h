// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#include <stdio.h>
#include <tchar.h>

#define SHAREMEM_NO
#define HTTPREQ_NO

#define MN "QS"

#include "iomodel.h"
#include "farm.h"
#include "Realdata.h"
#include "DataPool.h"
#include "..\\plug.h"
#include "PlugManager.h"
#include "..\\..\\public\\commx\\report.h"

// TODO: 在此处引用程序需要的其他头文件
extern CDataFarm * g_pDataFarm;
extern CompletionPortModel  * g_pIo;
extern CPlugManager * g_pPlugs;
extern CIndexSummary g_IndexSummary;
extern CDataPool g_DataPool;
extern CReportFile * g_pReportFile;
extern CStockKindManager g_StockKindManager;
extern std::string g_strNoLineTest;
