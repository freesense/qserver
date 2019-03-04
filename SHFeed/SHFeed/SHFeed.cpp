// SHFeed.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "../../public/feedinterface.h"
#include "ScanThread.h"
#include "constdef.h"
#include "../../public/commx/mery.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

CScanThread    g_thread;			//扫描线程
IFeedOwner*    g_pOwner = NULL;
int            g_nTimeout = 0;      //超时时间
std::string    g_strSZRegex;        //深圳国债回购正则表达式
std::string    g_strSHRegex;        //上海国债回购正则表达式


bool Initialize(int argc, char *argv[], IFeedOwner *pOwner)
{
	g_pOwner = pOwner;
	if(argc < 5)
	{
		REPORT_Ex("argc < 5, Initialize failed!\n", RPT_ERROR);
		return FALSE;
	}

	{
		char szDBFPath_SZ[MAX_PATH];
		char szDBFPath_SH[MAX_PATH];

		char szMarket[3];
		for (int i=0; i<argc-1; i++)
		{
			memset(szMarket, 0, 3);
			memcpy(szMarket, argv[i], 2);

			if (_stricmp(szMarket, "SZ") == 0)
			{
				char *pNew = strstr(argv[i], "-");
				if (pNew != NULL)
				{
					pNew[0] = 0;
					strcpy_s(szDBFPath_SZ, MAX_PATH, ++pNew);
					continue;
				}
			}
			else if (_stricmp(szMarket, "SH") == 0)
			{
				char *pNew = strstr(argv[i], "-");
				if (pNew != NULL)
				{
					pNew[0] = 0;
					strcpy_s(szDBFPath_SH, MAX_PATH, ++pNew);
					continue;
				}
			}
			else
			{
				char *pNew = strstr(argv[i], "深圳国债回购");
				if (pNew != NULL)
				{
					g_strSZRegex = pNew + strlen("深圳国债回购");
					continue;
				}

				pNew = strstr(argv[i], "上海国债回购");
				if (pNew != NULL)
				{
					g_strSHRegex = pNew + strlen("上海国债回购");
					continue;
				}
			}

			REPORT_Ex( (T("argv[%d] path [%s] is invalid, Initialize failed!\n", i, argv[i])).c_str(), RPT_ERROR);
			return FALSE;
		}
		g_nTimeout = atoi(argv[4]);

		//预分配内存
		if (!CreateMemBlock(64, 10))
		{
			REPORT_Ex("CreateMemBlock(64, 100) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024, 20))
		{
			REPORT_Ex("CreateMemBlock(1024, 200) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024*100, 40))
		{
			REPORT_Ex("CreateMemBlock(1024*100, 400) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024*200, 40))
		{
			REPORT_Ex("CreateMemBlock(1024*200, 400) failed!\n", RPT_ERROR);
			return FALSE;
		}
		if (!CreateMemBlock(1024*500, 10))
		{
			REPORT_Ex("CreateMemBlock(1024*500, 100) failed!\n", RPT_ERROR);
			return FALSE;
		}
		//
		if(!g_thread.SetDBFPath(szDBFPath_SZ, szDBFPath_SH))
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
	*pBuf = (char*)SHFEEDVER;
}