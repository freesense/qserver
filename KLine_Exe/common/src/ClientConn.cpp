//ClientConn.cpp
#include "stdafx.h"
#include "ClientConn.h"

//////////////////////////////////////////////////////////////////////
CClientConn::CClientConn()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_hCltSock = NULL;
}

CClientConn::~CClientConn()
{
	DisConnect();
	WSACleanup();
}

bool CClientConn::Connect(char *pSerIP, unsigned int nPort)
{
	if(m_hCltSock != NULL)
		DisConnect();

	m_hCltSock = socket(AF_INET, SOCK_STREAM, 0);
	if(m_hCltSock == INVALID_SOCKET)
		return false;

	m_addr.sin_family			= AF_INET;
	m_addr.sin_addr.S_un.S_addr	= inet_addr(pSerIP);
	m_addr.sin_port				= htons(nPort);	

	if(connect(m_hCltSock, (LPSOCKADDR)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
		return false;
	else
		return true;
}

void CClientConn::DisConnect()
{
	if(m_hCltSock != NULL)
	{
		if (SOCKET_ERROR == shutdown(m_hCltSock, SD_SEND))
			REPORT(MN, T("shutdown() failed with error code:%d", WSAGetLastError()), RPT_ERROR);

		if (SOCKET_ERROR == closesocket(m_hCltSock))
			REPORT(MN, T("closesocket() failed with error code:%d", WSAGetLastError()), RPT_ERROR);
		m_hCltSock = NULL;
	}
}

bool CClientConn::SendPacket(char *pSendBuf, int nPackLen)
{
	if(m_hCltSock == NULL)
		return false;

	int nLeaveSize = nPackLen;
	char* pBufPos = pSendBuf;
	while(nLeaveSize > 0)
	{
		int nRet = send(m_hCltSock, pBufPos, nLeaveSize, 0);
		if (nRet == SOCKET_ERROR)
			return false;
		nLeaveSize -= nRet;
		pBufPos    += nRet;
	}
	
	return true;
}

int CClientConn::RecvPacket(char *pRecvBuf, int nBufLen)
{
	if(m_hCltSock == NULL)
		return -1;

	int nLeaveSize = nBufLen;
	char* pBufPos = pRecvBuf;
	while(nLeaveSize > 0)
	{
		int nRet = recv(m_hCltSock, pBufPos, nLeaveSize, 0);
		if (nRet == SOCKET_ERROR)
			return -1;

		if (nRet == 0)
			break;

		nLeaveSize -= nRet;
		pBufPos    += nRet;
	}

	return nBufLen - nLeaveSize;
}