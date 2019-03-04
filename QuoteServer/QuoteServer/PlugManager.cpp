#include "StdAfx.h"
#include "atlconv.h"
#include <stdio.h>
#include <iostream>
#include "PlugManager.h"
#include "..\\..\\public\\commx\\synch.h"
#include "..\\..\\public\\commx\\report.h"

using namespace std;
extern Quote * GetSymbolQuote(char * c_pSymbol);
extern int SendReply(SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);
extern void PlugQuit(unsigned int c_unPlugID);
extern int GetSymbolData(char * c_pSymbol,char * c_pBuf,int c_iMaxLen);

extern void OnQuoteChanged(const char *symbol, const int *notify); //4个int
extern void OnBusiness(unsigned int c_ID ,char *c_pBuf);

#define MODULE_NAME "QS"
CPlugManager::CPlugManager(void)
{
}

CPlugManager::~CPlugManager(void)
{
	vector<SPlugItem *>::iterator iter;
	ReleaseAllItems();
	FreeAllItems();
	while(m_pItems.size() > 0)
	{
		iter = m_pItems.begin();
		delete (*iter);
		m_pItems.erase(iter);
	}
}
SPlugItem * CPlugManager::AddItem(SPlugItem * c_pItem)
{
	c_pItem->m_unBeginFuncNo = 0;
	c_pItem->m_unEndFuncNo = 1000;
	vector<SPlugItem *>::iterator iter;
	c_pItem->m_unPlugID = 1;
	for(iter = m_pItems.begin(); iter != m_pItems.end(); iter++)
	{
		if ((*iter)->m_unPlugID > c_pItem->m_unPlugID)
		{
			c_pItem->m_unPlugID = (*iter)->m_unPlugID + 1;
		}
	}
	m_pItems.push_back(c_pItem);
	return c_pItem;

}
bool CPlugManager::InitItem(SPlugItem * c_pItem)
{
	if (c_pItem->m_nState == 0)
		return false;
	USES_CONVERSION;
	c_pItem->m_hHandle = LoadLibrary(A2W(c_pItem->m_acFile));
	//REPORT(MODULE_NAME,T("PLUG name=%s handle=%d\n",strFileName.c_str(),pItem->m_hHandle),RPT_INFO);
	if (c_pItem->m_hHandle == 0)
	{
		return false;
	}

	c_pItem->m_sMethod.m_FarmOnInitialize = (_FarmOnInitialize)GetProcAddress(c_pItem->m_hHandle,"Initialize");
	//REPORT(MODULE_NAME,T("m_FarmOnInitialize=%d\n",pItem->m_sMethod.m_FarmOnInitialize),RPT_INFO);
	c_pItem->m_sMethod.m_FarmRelease = (_FarmRelease)GetProcAddress(c_pItem->m_hHandle,"Release");
	//REPORT(MODULE_NAME,T("m_FarmRelease=%d\n",pItem->m_sMethod.m_FarmRelease),RPT_INFO);			  
	c_pItem->m_sMethod.m_FarmRegisterCallBack = (_FarmRegisterCallBack)GetProcAddress(c_pItem->m_hHandle,"RegisterCallback");
	//REPORT(MODULE_NAME,T("m_FarmRegisterCallBack=%d\n",pItem->m_sMethod.m_FarmRegisterCallBack),RPT_INFO);
	c_pItem->m_sMethod.m_FarmOnRequest = (_FarmOnRequest)GetProcAddress(c_pItem->m_hHandle,"OnRequest");
	//REPORT(MODULE_NAME,T("m_FarmOnRequest=%d\n",pItem->m_sMethod.m_FarmOnRequest),RPT_INFO);
	/*
	if (pItem->m_sMethod.m_FarmOnInitialize == 0
		||pItem->m_sMethod.m_FarmOnRequest == 0
		||pItem->m_sMethod.m_FarmRegisterCallBack == 0
		||pItem->m_sMethod.m_FarmRelease == 0)
	{
		REPORT(MODULE_NAME,T("%s GetProcess Fail\n",strFileName.c_str()),RPT_INFO);
		delete pItem;
		pItem = NULL;
		return false;
	}
	*/
	c_pItem->m_sMethod.m_OnInit = (_OnInit)GetProcAddress(c_pItem->m_hHandle,"_OnInit");
	//REPORT(MODULE_NAME,T("_OnInit=%d\n",pItem->m_sMethod.m_OnInit),RPT_INFO);
	c_pItem->m_sMethod.m_OnRelease = (_OnRelease)GetProcAddress(c_pItem->m_hHandle,"_OnRelease");
	//REPORT(MODULE_NAME,T("_OnRelease=%d\n",pItem->m_sMethod.m_OnRelease),RPT_INFO);
	c_pItem->m_sMethod.m_OnRequest = (_OnRequest)GetProcAddress(c_pItem->m_hHandle,"_OnRequest");
	//REPORT(MODULE_NAME,T("_OnRequest=%d\n",pItem->m_sMethod.m_OnRequest),RPT_INFO);
	c_pItem->m_sMethod.m_OnUpdateData = (_OnUpdateData)GetProcAddress(c_pItem->m_hHandle,"_OnUpdateData");
	//REPORT(MODULE_NAME,T("_OnUpdateData=%d\n",pItem->m_sMethod.m_OnUpdateData),RPT_INFO);
	c_pItem->m_sMethod.m_Set_GetSymbolQuote = (_Set_GetSymbolQuote)GetProcAddress(c_pItem->m_hHandle,"_Set_GetSymbolQuote");
	//REPORT(MODULE_NAME,T("_Set_GetSymbolQuote=%d\n",pItem->m_sMethod.m_Set_GetSymbolQuote),RPT_INFO);
	c_pItem->m_sMethod.m_Set_SendReplay = (_Set_SendReplay)GetProcAddress(c_pItem->m_hHandle,"_Set_SendReplay");
	//REPORT(MODULE_NAME,T("_Set_SendReplay=%d\n",pItem->m_sMethod.m_Set_SendReplay),RPT_INFO);
	c_pItem->m_sMethod.m_Set_PlugQuit = (_Set_PlugQuit)GetProcAddress(c_pItem->m_hHandle,"_Set_PlugQuit");
	//REPORT(MODULE_NAME,T("_Set_PlugQuit=%d\n",pItem->m_sMethod.m_Set_PlugQuit),RPT_INFO);
	c_pItem->m_sMethod.m_Set_GetSymbolData = (_Set_GetSymbolData)GetProcAddress(c_pItem->m_hHandle,"_Set_GetSymbolData");
	//REPORT(MODULE_NAME,T("_Set_GetSymbolData=%d\n",pItem->m_sMethod.m_Set_GetSymbolData),RPT_INFO);
	/*
	if (pItem->m_sMethod.m_Set_GetSymbolData == 0
		|| pItem->m_sMethod.m_OnInit == 0
		|| pItem->m_sMethod.m_OnRelease == 0
		|| pItem->m_sMethod.m_OnRequest == 0
		|| pItem->m_sMethod.m_OnUpdateData == 0
		|| pItem->m_sMethod.m_Set_GetSymbolQuote == 0
		|| pItem->m_sMethod.m_Set_SendReplay == 0
		|| pItem->m_sMethod.m_Set_PlugQuit == 0
		|| pItem->m_sMethod.m_Set_GetSymbolData == 0)
	{
		REPORT(MODULE_NAME,T("%s GetProcess Fail\n",strFileName.c_str()),RPT_INFO);
		delete pItem;
		pItem = NULL;
		return false;
	}*/

	c_pItem->m_sMethod.m_GetModuleVer = (_GetModuleVer)GetProcAddress(c_pItem->m_hHandle,"GetModuleVer");
	REPORT(MODULE_NAME,T("_GetModuleVer=%d\n",c_pItem->m_sMethod.m_GetModuleVer),RPT_INFO);

	return true;
}

bool CPlugManager::FreeItem(char * c_pFile)
{
	vector<SPlugItem *>::iterator iter;
	for(iter = m_pItems.begin(); iter != m_pItems.end(); iter++)
	{
		if (strcmp((*iter)->m_acFile,c_pFile) == 0)
		{
			if ((*iter)->m_hHandle != 0)
				FreeLibrary((*iter)->m_hHandle);
			(*iter)->m_hHandle = 0;
		}
	}
	return true;
}

void CPlugManager::FreeAllItems()
{
	vector<SPlugItem *>::iterator iter;
	for(iter = m_pItems.begin(); iter != m_pItems.end(); iter++)
	{
		if ((*iter)->m_hHandle != 0)
			FreeLibrary((*iter)->m_hHandle);
		(*iter)->m_hHandle = 0;
	}
}
extern std::string g_strVer;//版本信息
int CPlugManager::InitAllItems()
{
	vector<SPlugItem *>::iterator iter;
	iter = m_pItems.begin();
	while(iter != m_pItems.end())
	{
		if (InitItem(*iter))
		{
			if ((*iter)->m_sMethod.m_FarmOnInitialize != 0)
			{
				if ((*iter)->m_sMethod.m_FarmOnInitialize != 0)
				{
					//REPORT(MODULE_NAME,T("To m_FarmOnInitialize \n"),RPT_INFO);
					int iRet = (*iter)->m_sMethod.m_FarmOnInitialize(1);
					//REPORT(MODULE_NAME,T("m_FarmOnInitialize Ret=%d\n",iRet),RPT_INFO);
					if (iRet < 0)
						return -1;
					
				}
				if ((*iter)->m_sMethod.m_FarmRegisterCallBack != 0)
				{
					//REPORT(MODULE_NAME,T("to OnQuoteChanged and OnBusiness \n"),RPT_INFO);
					(*iter)->m_sMethod.m_FarmRegisterCallBack(OnQuoteChanged,OnBusiness);
					//REPORT(MODULE_NAME,T("Set OnQuoteChanged and OnBusiness End\n"),RPT_INFO);
				}
			}
			else
			{
				if ((*iter)->m_sMethod.m_Set_SendReplay != 0)
				{
					(*iter)->m_sMethod.m_Set_SendReplay(SendReply);
					REPORT(MODULE_NAME,T("m_Set_SendReplay OK\n"),RPT_INFO);
				}
				if ((*iter)->m_sMethod.m_Set_GetSymbolQuote != 0)
				{					
					(*iter)->m_sMethod.m_Set_GetSymbolQuote(GetSymbolQuote);
					REPORT(MODULE_NAME,T("m_Set_GetSymbolQuote OK \n"),RPT_INFO);
				}
				if ((*iter)->m_sMethod.m_Set_PlugQuit != 0)
				{				
					(*iter)->m_sMethod.m_Set_PlugQuit(PlugQuit);
					REPORT(MODULE_NAME,T("m_Set_PlugQuit OK \n"),RPT_INFO);
				}
				if ((*iter)->m_sMethod.m_Set_GetSymbolData != 0)
				{
					(*iter)->m_sMethod.m_Set_GetSymbolData(GetSymbolData);
					REPORT(MODULE_NAME,T("m_Set_GetSymbolData OK \n"),RPT_INFO);
				}
				if ((*iter)->m_sMethod.m_OnInit != 0)
				{
					int iRet = (*iter)->m_sMethod.m_OnInit((*iter)->m_unPlugID,(*iter)->m_acParam);
					if (iRet < 0)
						return -1;
				}
			}
			if ((*iter)->m_sMethod.m_GetModuleVer != 0)
			{
				char acBuf[256];
				char * pBuf;
				pBuf = acBuf;
				memset(pBuf,0,256);
				(*iter)->m_sMethod.m_GetModuleVer(&pBuf);
				g_strVer += pBuf;
				REPORT(MODULE_NAME,T("VER=[%s][%s] \n",(*iter)->m_acFile,pBuf),RPT_INFO);
			}
			iter++;
		}
		else
		{
			delete *iter;
			iter = m_pItems.erase(iter);
		}
	}
	return 0;
}

void CPlugManager::ReleaseAllItems()
{
	vector<SPlugItem *>::iterator iter;
	for(iter = m_pItems.begin(); iter != m_pItems.end(); iter++)
	{
		if ((*iter)->m_hHandle != 0)
		{
			if ((*iter)->m_sMethod.m_FarmRelease != 0)
				(*iter)->m_sMethod.m_FarmRelease();
			else
			if ((*iter)->m_sMethod.m_OnRelease != 0)
				(*iter)->m_sMethod.m_OnRelease();
		}
	}
}

int CPlugManager::UpdateData(char * c_pBuf,int c_iLen)
{
	vector<SPlugItem *>::iterator iter;
	int iRet = 0;
	for(iter = m_pItems.begin(); iter != m_pItems.end(); iter++)
	{
		if ((*iter)->m_hHandle != 0)
			iRet = (*iter)->m_sMethod.m_OnUpdateData(c_pBuf,c_iLen);
	}
	return iRet;
}
int CPlugManager::SendRequest(SMsgID * c_pID,unsigned int c_unFuncNo,char * c_pBuf,int c_iLen)
{
	vector<SPlugItem *>::iterator iter;
	SPlugItem * pItem;
	int iRet = -1;
	//REPORT(MODULE_NAME,T("SendRequest %d\n",c_unFuncNo),RPT_INFO);
	for(iter = m_pItems.begin(); iter != m_pItems.end(); iter++)
	{
		pItem = *iter;
		if (pItem->m_hHandle != 0 && (*iter)->m_unBeginFuncNo <= c_unFuncNo && (*iter)->m_unEndFuncNo > c_unFuncNo)
		{
			//REPORT(LockSingle,MODULE_NAME,T("before m_OnRequest %s\n",(*iter)->m_acFile),RPT_INFO);
			if ((*iter)->m_sMethod.m_OnRequest != 0)
				iRet = (*iter)->m_sMethod.m_OnRequest(c_pID,c_pBuf,c_iLen);
			else
			if ((*iter)->m_sMethod.m_FarmOnRequest != 0)
			{
				//REPORT(MODULE_NAME,T("before m_FarmOnRequest %d\n",c_pID->m_unID),RPT_INFO);
				iRet = (*iter)->m_sMethod.m_FarmOnRequest(c_pID->m_unID,c_pBuf);
				//REPORT(MODULE_NAME,T("after m_FarmOnRequest %d ret=%d\n",c_pID->m_unID,iRet),RPT_INFO);
			}
			//cout<<"SendRequest ret = "<< iRet<< endl;
			//REPORT(LockSingle,MODULE_NAME,T("after m_OnRequest %s\n",(*iter)->m_acFile),RPT_INFO);
		}
	}
	return iRet;
}
void CPlugManager::OnPlugQuit(unsigned int c_unPlugID)
{
	vector<SPlugItem *>::iterator iter;
	for(iter = m_pItems.begin(); iter != m_pItems.end(); iter++)
	{
		if ((*iter)->m_hHandle != 0 && (*iter)->m_unPlugID == c_unPlugID)
		{
			FreeLibrary((*iter)->m_hHandle);
			delete (*iter);
			m_pItems.erase(iter);
		}
	}
}
