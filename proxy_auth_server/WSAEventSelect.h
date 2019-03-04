//WSAEventSelect.h
#ifndef _WSAEVENTSELECT_H_
#define _WSAEVENTSELECT_H_
//#include <winsock2.h>
#include <string>
#include "server_auth.h"
#include "../public/protocol.h"
#include "../public/commx/commxbase.h"
#include "../public/auth/auth_struct.h"
#include "../public/xml/xmlparser.h"
#include "../public/devmonitor/auth_xml_protocol.h"

#define BUF_SIZE 65536

/////////////////////////////////////////////////////////////////////////////////
class CProxySocket : public ICommX
{
public:
	CProxySocket() : m_bQuit(false) {m_timer.m_nTimerID = 0;};
	void finalize();

	virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);
	virtual int handle_close(int iError);
	virtual void handle_release();
	virtual int handle_read(char *lpBuffer, unsigned int nLength);
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);
	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);

	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);

	CAuthProtoclXML m_ap;
	bool m_bValid;

private:
	void ParseStart(TiXmlElement *pNode);
	void ParseStop(TiXmlElement *pNode);
	void ParseDownloadXml(TiXmlElement *pNode);
	void ParseUploadXml(TiXmlElement *pNode);
	void ChangeBinStringToString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize);

	int m_nReadID;
	char m_szBuffer[BUF_SIZE];
	char m_szHeartbeatBuffer[512];
	CommxHead *m_pHead;

	bool m_bQuit;
	ITimerCallbackBase m_timer;
};

extern CProxySocket *g_pProxySocket;

#endif