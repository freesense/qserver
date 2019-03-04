// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__86ED7D59_E4EC_4A43_9F5D_9618872EDC62__INCLUDED_)
#define AFX_STDAFX_H__86ED7D59_E4EC_4A43_9F5D_9618872EDC62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DOGVER "$WatchDog#2.2.8$"

#ifdef _DEBUG
#define CONSOLEAPP
#endif

#define WIN32_LEAN_AND_MEAN
#define MODULE_NAME "WatchDog"
#define TIME_OUT 60  //超时时间 单位：秒

#pragma warning(disable:4786)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:428)

// TODO: reference additional headers your program requires here
#include <Windows.h>
#include <stdlib.h>
#include <Winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "../../public/commx/synch.h"
#include "../../public/commx/report.h"
#include "config.h"
#include <map>

extern std::string gExecutePath;  

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd.lib")
#else
#pragma comment(lib, "../../public/lib/commxr.lib")
#endif	

typedef std::map<unsigned long, std::string> CLoginDevMap;
extern void AddDev(const char* pDevName, unsigned long dwProID);
extern void DelDev(unsigned long dwProID);
extern void SendDevList();
extern CRITICAL_SECTION   g_csDevList;
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__86ED7D59_E4EC_4A43_9F5D_9618872EDC62__INCLUDED_)
