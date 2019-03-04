// HKFeed.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "../../public/feedinterface.h"
#include "ScanThread_hk.h"
#include "constdef_hk.h"
#include "../../public/commx/mery.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

CScanThread    g_thread;			//扫描线程
IFeedOwner*    g_pOwner = NULL;
int            g_nTimeout = 0;      //超时时间

bool Initialize(int argc, char *argv[], IFeedOwner *pOwner)
{
	g_pOwner = pOwner;
	if(argc < 4)
	{
		REPORT_Ex("argc < 4, Initialize failed!\n", RPT_ERROR);
		return FALSE;
	}
	{
		char szDBFPath_HK[MAX_PATH];

		for(int i=0; i<argc-1; i++)
		{
			char *pNew = strstr(argv[i], "-");
			if (pNew != NULL)
			{
				pNew[0] = 0;
				if(_stricmp(argv[0], "HK") == 0)
				{
					strcpy_s(szDBFPath_HK, MAX_PATH, ++pNew);
					continue;
				}
				else
				{
					REPORT_Ex("argv[0] dbf path is invalid, Initialize failed!\n", RPT_ERROR);
					return FALSE;
				}
			}

			char *pStartTime = strstr(argv[i], "开始时间:");
			if (pStartTime != NULL)
			{
				pStartTime += strlen("开始时间:");
				g_thread.m_nStartTime = atoi(pStartTime);
				continue;
			}

			char *pStopTime = strstr(argv[i], "结束时间:");
			if (pStopTime != NULL)
			{
				pStopTime += strlen("结束时间:");
				g_thread.m_nStopTime = atoi(pStopTime);
			}
		}

		g_nTimeout = atoi(argv[3]);

		//预分配内存
		if (!CreateMemBlock(64, 10))
		{
			REPORT_Ex("CreateMemBlock(64, 10) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024, 20))
		{
			REPORT_Ex("CreateMemBlock(1024, 20) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024*100, 40))
		{
			REPORT_Ex("CreateMemBlock(1024*100, 40) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024*200, 40))
		{
			REPORT_Ex("CreateMemBlock(1024*200, 40) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024*500, 10))
		{
			REPORT_Ex("CreateMemBlock(1024*500, 10) failed!\n", RPT_ERROR);
			return FALSE;
		}
		//
		if(!g_thread.SetDBFPath(szDBFPath_HK))
			return FALSE;
	}

	if(!g_thread.Start())
		return FALSE;
		  
	return TRUE;
}

bool Request(char *lpData, unsigned int nLength, void *lpParam)
{
	return TRUE;
}

void Release()
{
	g_thread.Stop();
}

void GetModuleVer(char** pBuf)
{
	*pBuf = (char*)HKFEEDVER;
}