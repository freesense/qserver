// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#define MN "Feed"

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#ifdef _DEBUG
//for memory leak check
#define _CRTDBG_MAP_ALLOC //ʹ���ɵ��ڴ�dump�����ڴ�����ľ������Ϊֹ
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
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
