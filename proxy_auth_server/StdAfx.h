// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__DF4D8F48_1AB1_444A_A680_4699387EAC93__INCLUDED_)
#define AFX_STDAFX_H__DF4D8F48_1AB1_444A_A680_4699387EAC93__INCLUDED_

#pragma warning(disable:4786 4503)

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#define MODULE_NAME "proxyAuth"
#define PROXY_VER "$proxy_auth_server#2.2.9$"

#include "../public/commx/synch.h"
#include "../public/commx/report.h"

#ifdef _DEBUG
#pragma comment(lib, "../public/lib/commxd.lib")
#else
#pragma comment(lib, "../public/lib/commxr.lib")
#endif

#define _DLL
#include "../public/commx/commxapi.h"

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__DF4D8F48_1AB1_444A_A680_4699387EAC93__INCLUDED_)
