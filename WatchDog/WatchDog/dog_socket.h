
#ifndef __DOG_SOCKET_H__
#define __DOG_SOCKET_H__

#pragma warning(disable:4786)

#include <set>
#include "../../public/commx/synch.h"
#include "../../public/commx/commxbase.h"
#include "../../public/devmonitor/auth_xml_protocol.h"
#include "../../public/protocol.h"

#define MAX_DOGBUF 16*1024

class CDogListener;

class CDogSocket : public ICommX
{
	enum ReadPart {PART_HEAD = 1, PART_BODY};

public:
	CDogSocket(CDogListener* pServer);

	virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);
	virtual int handle_close(int iError);
	virtual void handle_release();
	virtual int handle_read(char *lpBuffer, unsigned int nLength);
	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);

	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);

private:
	int CheckPackageType(char *lpData, unsigned int nLength);

	struct _TimerCallback : public ITimerCallbackBase
	{
		unsigned long m_nOldHeartbeat;
	} m_timer;
	unsigned long m_nHeartbeatCount;

	char m_szBuffer[MAX_DOGBUF];
	CDogListener *m_ptrServer;
	ReadPart m_part;

	unsigned long m_nProID;
	char m_lpFingerPrinter[50];
	CAuthProtoclXML  m_DogSerAp;
};

class CDogListener : public ICommX
{
public:
	virtual ~CDogListener();
	virtual ICommX* handle_accept(const char *lpLocalAddr, const char *lpPeerAddr);
	void OnSocketClosed(CDogSocket *pHandle);

private:
	LockSingle m_lock;
	std::set<CDogSocket*> m_setHandle;
};

#endif
