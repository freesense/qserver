// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../../public/lib/ntwdblib")

#define MN "HQEXTEND"
#define WIN32_LEAN_AND_MEAN

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 头中排除极少使用的资料
#endif

// 如果必须将位于下面指定平台之前的平台作为目标，请修改下列定义。
// 有关不同平台对应值的最新信息，请参考 MSDN。
#ifndef WINVER				// 允许使用特定于 Windows XP 或更高版本的功能。
#define WINVER 0x0501		// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINDOWS		// 允许使用特定于 Windows 98 或更高版本的功能。
#define _WIN32_WINDOWS 0x0410 // 将此值更改为适当的值，以指定将 Windows Me 或更高版本作为目标。
#endif

#ifndef _WIN32_IE			// 允许使用特定于 IE 6.0 或更高版本的功能。
#define _WIN32_IE 0x0600	// 将此值更改为相应的值，以适用于 IE 的其他版本。
#endif

#include <Windows.h>
#include "../../public/protocol.h"
#include "../../public/data_struct.h"
#include "config.h"
#include "workthread.h"
#include "../../public/messagequeue.h"
#include "../../public/commx/synch.h"
#include "../../public/commx/report.h"
#include "../../public/commx/mery.h"
#include "../../public/commx/code_convert.h"
#include "../../public/commx/highperformancecounter.h"

const char gVer[] = "$qextend.dll#2.1.26$";

extern CWorkThread gThread;
extern CMessageQueue<CommxHead*> gQueue;
