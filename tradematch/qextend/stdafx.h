// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif
//#pragma comment(lib, "../../public/lib/sqlite3")

#define MN "TRADEMATCH"

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

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将是显式的

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE 类
#include <afxodlgs.h>       // MFC OLE 对话框类
#include <afxdisp.h>        // MFC 自动化类
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC 数据库类
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
//#include <afxdao.h>			// MFC DAO 数据库类
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>
#include <afxoledb.h>
#include <atlplus.h>
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF") 

#include "../../public/protocol.h"
#include "../../public/data_struct.h"
#include "config.h"
#include "workthread.h"
#include "DataBaseEx.h"
#include "DataStore.h"
#include "../../public/messagequeue.h"
#include "../../public/commx/synch.h"
#include "../../public/commx/report.h"
#include "../../public/commx/mery.h"
#include "../../public/commx/code_convert.h"
#include "../../public/commx/highperformancecounter.h"

const char gVer[] = "$tradematch.dll#2.1.3$";

extern CWorkThread gWorkThread;
extern CWriteThread gWriteThread;
extern CInfoThread gInfoThread;
extern CDataBaseEx gDBRead;
extern CDataBaseEx gDBWrite;
extern CMessageQueue<CommxHead*> gQueue;
extern CDataStore gDataStore;
extern CDataFarm gFarm;
extern int gnTradeTime;//交易时间
extern char gacTradeTime[4];//交易时间,跟踪上证指数的交易时间
extern bool gbIsTradeTime;//是否是交易时间,上证指数9:30到11:30及下午13:00到15:00为交易时间.这区间有成交两变化则进行撮合
extern bool gbIsWorkTime;//是否是工作时间,下午3点到3点半为非工作时间,不允许提交委托
extern CMessageQueue<Consign*> gConsignQueue;//委托单库,等待写入委托成交表
extern CMessageQueue<Consign*> gConsignQueueWait;//委托单库,等待写入委托成交表
extern bool gbExitFlag;//退出
extern SumInfo gSumInfo;//统计信息
extern bool gbWorkDay;//是否是工作日
extern bool gbTradeTime;//交易时间