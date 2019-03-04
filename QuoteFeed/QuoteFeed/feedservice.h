
#ifndef __FEED_SERVICE_H__
#define __FEED_SERVICE_H__

#include <set>
#include "../../public/commx/synch.h"
#include "../../public/commx/commxbase.h"
#include "../../public/protocol.h"

#define MAX_SERVICE_LENGTH 1024

class CFeedServer;

class CFeedService : public ICommX
{
public:
	CFeedService(CFeedServer *pServer);
	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);
	virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);
	virtual int handle_read(char *lpBuffer, unsigned int nLength);
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);
	virtual int handle_close(int iError);
	virtual void handle_release();

	void SendQuote(void *lpSendData = NULL, unsigned int nLen = 0);

private:
	CFeedServer *m_pServer;
	unsigned int m_nSendBytes;
	unsigned short m_nSerialNo;

	CommxHead m_recvHead;
	char m_szRecvData[MAX_SERVICE_LENGTH];
	bool m_bRecvStep;
};

class CFeedServer : public ICommX
{
public:
	virtual ICommX* handle_accept(const char *lpLocalAddr, const char *lpPeerAddr);
	void Close();

	void SendQuote(CFeedService *pService = NULL, void *lpData = NULL, unsigned int nLen = 0);
	void OnDataWrited(char *lpData, unsigned int nLength);
	void OnServiceClosed(CFeedService *pService);

private:
	std::set<CFeedService*> m_setFeedService;
	LockRW m_lockFeedService;
};

#endif
