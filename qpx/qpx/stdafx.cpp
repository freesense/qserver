// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// qpx.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

std::string strQuoteAddress, strHostAddress;
unsigned short g_serialno = 0;
std::map<unsigned short, C2cc*> g_mapClient;
LockSingle g_lockClients;

unsigned short onRequest(C2cc *pClient)
{
	WGUARD(LockSingle, g_lockClients, grd);
	g_mapClient[++g_serialno] = pClient;
	return g_serialno;
}

C2cc* onAnswer(unsigned short serialno)
{
	C2cc *pClient = NULL;
	WGUARD(LockSingle, g_lockClients, grd);
	std::map<unsigned short, C2cc*>::iterator iter = g_mapClient.find(serialno);
	if (iter != g_mapClient.end())
		pClient = iter->second;
	return pClient;
}

void onRelease(C2cc *pClient)
{
	WGUARD(LockSingle, g_lockClients, grd);
	std::map<unsigned short, C2cc*>::iterator iter = g_mapClient.begin();
	while (iter != g_mapClient.end())
	{
		if (iter->second == pClient)
		{
			g_mapClient.erase(iter++);
			continue;
		}
		iter++;
	}
}

std::map<unsigned short, char*> g_mapCltData;
LockSingle  g_lockCltData;

void InsertCltData(unsigned short nSerialNo, char* pData)
{
	WGUARD(LockSingle, g_lockCltData, grd);
	g_mapCltData.insert(std::make_pair(nSerialNo, pData));
}

char* RemoveCltData(unsigned short nSerialNo)
{
	WGUARD(LockSingle, g_lockCltData, grd);
	std::map<unsigned short, char*>::iterator find = g_mapCltData.find(nSerialNo);
	if (find != g_mapCltData.end())
	{	
		char* pData = find->second;
		g_mapCltData.erase(find);
		return pData;
	}
	else
		return NULL;
}

// TODO: �� STDAFX.H ��
// �����κ�����ĸ���ͷ�ļ����������ڴ��ļ�������
