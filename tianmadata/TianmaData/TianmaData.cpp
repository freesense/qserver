// TianmaData.cpp : 定义 DLL 应用程序的入口点。
//

#include "stdafx.h"
#include "TianmaDataExport.h"
#include "DataProcess.h"
#include "DataBaseEx.h"

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

/////////////////////////////////////////////////////////////////////////////

//平台提供的回调函数
_SendReply      pSendReply;
_GetSymbolQuote pGetSymbolQuote;
_PlugQuit       pPlugQuit;
_GetSymbolData  pGetSymbolData;

CDataProcess    g_DataProc;
CDataBaseEx     g_database;


char szVer[100] = {0};
//////////////////////////////////////////////////////////////////////////////
int _OnInit(unsigned int c_unPlugID,const char * c_pParam)
{
	return g_DataProc.OnInit(c_unPlugID,c_pParam);
}

void _OnRelease()
{
	g_DataProc.OnRelease();
}

int _OnUpdateData(char* c_pBuf, int c_iLen)
{
	//添加代码
	return 0;
}

int _OnRequest(SMsgID* c_pMsgID, char* c_pBuf, int c_iLen)
{
	return g_DataProc.OnRequest(c_pMsgID, c_pBuf, c_iLen);
}

void _Set_SendReplay(_SendReply c_pFunc)
{
	//设置发送函数指针
	pSendReply = c_pFunc;
}

void _Set_GetSymbolQuote(_GetSymbolQuote c_pFunc)
{
	//设置获取股票行情数据指针
	pGetSymbolQuote = c_pFunc;
}

void _Set_GetSymbolData(_GetSymbolData c_pFunc)
{
	//设置获取股票数据函数指针
	pGetSymbolData = c_pFunc;
}

void _Set_PlugQuit(_PlugQuit c_pFunc)
{
	//设置通知退出处理函数指针
	pPlugQuit = c_pFunc;
}

void GetModuleVer(char** pBuf)
{
	char* pCommxVer = NULL;
	CommxVer(&pCommxVer);

	strcpy_s((char*)szVer,	100, pCommxVer);
	int nLen = (int)strlen(szVer);
	strcpy_s((char*)szVer+nLen, 100-nLen, TIANMADATAVER);

	*pBuf = (char*)szVer;
}


