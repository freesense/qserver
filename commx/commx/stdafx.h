
#ifndef __STDAFX_COMMX_H__
#define __STDAFX_COMMX_H__

#pragma once

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <mswsock.h>

#pragma warning(disable:4005 4786 4996)

#define _DLL
#define MN "COMMX"
const char gVer[] = "$commx.dll#2.1.1$";

extern HANDLE g_hIocp;	/**< Íê³É¶Ë¿Ú */
extern LPFN_ACCEPTEX lpfnAcceptEx;
extern LPFN_CONNECTEX lpfnConnectEx;
extern HANDLE g_hTimerQueue;

#endif
