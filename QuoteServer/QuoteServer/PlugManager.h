#pragma once
#include "stdafx.h"
#include "..\plug.h"
#include "windows.h"
#include <vector>
using namespace std; 
struct SPlugItem{
	unsigned int m_unPlugID;
	char m_acFile[MAX_PATH];
	HMODULE m_hHandle;
	SPlugMethod m_sMethod;
	unsigned int m_unBeginFuncNo;
	unsigned int m_unEndFuncNo;
	char m_acParam[256];
	int		m_nFarmType;//1表示是的
	int		m_nState;//状态，0表示为起用，1表示起用
};

class CPlugManager
{
public:
	CPlugManager(void);
	~CPlugManager(void);
public:
	vector<SPlugItem *> m_pItems;
public:
	SPlugItem * AddItem(SPlugItem * c_pItem);
	bool InitItem(SPlugItem * c_pItem);
	bool FreeItem(char * c_pFile);
	void FreeAllItems();
	int InitAllItems();
	void ReleaseAllItems();
	int UpdateData(char * c_pBuf,int c_iLen);
	int SendRequest(SMsgID * c_pID,unsigned int c_unFunc,char * c_pBuf,int c_iLen);
	void OnPlugQuit(unsigned int c_unPlugID);
};
