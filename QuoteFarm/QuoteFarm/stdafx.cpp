// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// QuoteFarm.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

// TODO: �� STDAFX.H ��
// �����κ�����ĸ���ͷ�ļ����������ڴ��ļ�������

HANDLE gQuit = NULL;

atomint g_nFeedRecvLength = 0;
atomint g_nTotalRecv = 0;
atomint g_nFeedPacketCount = 0;
MAPSPECIAL g_mapCloseSpecial;

void* ConnectFeed(const char *lpFeedName, const char *lpFeedAddress, CFeedSocket *pSocket)
{
	if (!pSocket)
	{
		pSocket = new CFeedSocket(lpFeedName);
		CHECK_RUN(!pSocket, MN, T("�����ڴ�ʧ��\n"), RPT_CRITICAL, return NULL);
	}
	pSocket->Connect(lpFeedAddress);
	return pSocket;
}

int _find_close_special::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	g_mapCloseSpecial[symbol].push_back(pri);
	return 0;
}
