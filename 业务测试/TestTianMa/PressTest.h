//PressTest.h
#ifndef _PRESSTEST_H_
#define _PRESSTEST_H_
#include "WSAEventSelect.h"
#include "..\..\public\commx\highperformancecounter.h"
#include "..\..\public\protocol.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
class CPressTest : public CWSAEventSelect
{
public:
	CPressTest(CWnd* pMainDlg);
	virtual~ CPressTest();

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual void OnWrite();
	virtual void OnRead();
	virtual void OnTimeOut();

	virtual void OnConnectFailed();
	virtual void OnReconnect();

	virtual void OnReadFailed();
	virtual void OnWriteFailed();

	unsigned int m_nPackType;
	double       m_time;
protected:

private:
	CWnd*          m_pMainDlg;
	char*          m_pRecvBuf;
	int            m_nBufLen;

	CommxHead      m_packHead;
	char*          m_pBufPos;
	int            m_nLeaveSize;
};
/////////////////////////////////////////////////////////////////////////////////////////////////
#endif