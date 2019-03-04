//ClientConn.h
#ifndef _CLIENTCONN_H
#define _CLIENTCONN_H

#include <Winsock2.h>
///////////////////////////////////////////////////////////////////////
class CClientConn
{
public:
	CClientConn();
	virtual ~CClientConn();

	bool Connect(char* pSerIP, unsigned int nPort);
	void DisConnect();

	bool SendPacket(char* pSendBuf, int nPackLen);
	int  RecvPacket(char* pRecvBuf, int nBufLen);
protected:

private:
	SOCKET         m_hCltSock;
	sockaddr_in    m_addr;
};

////////////////////////////////////////////////////////////////////////
#endif