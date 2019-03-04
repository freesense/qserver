//WSAEventSelect.h
#ifndef _WSAEVENTSELECT_H_
#define _WSAEVENTSELECT_H_
#include <winsock2.h>
#include <string>

#define BUF_SIZE 8192
/////////////////////////////////////////////////////////////////////////////////
class CWSAEventSelect
{
public:
	CWSAEventSelect();
	virtual ~CWSAEventSelect();

	BOOL StartThread();
	void StopThread();

	BOOL Connect(string strIP, UINT nPort);
	void Disconnect();
	void SetTimeOut(UINT nTimeOut){m_nTimeOut = nTimeOut;}

	void Reconnect();
	BOOL Read(const char* pBuf, int nReadLen);
	BOOL Write(const char* pBuf, int nLen);

protected:
	static UINT __stdcall _ThreadProc(LPVOID lpParam);
	void ThreadProc();

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual void OnWrite();
	virtual void OnRead();
	virtual void OnTimer();  //每隔超时时间执行一次

private:
	SOCKET           m_hSock;
	SOCKADDR_IN		 m_addr;
	WSAEVENT         m_EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	HANDLE           m_hThread;  //线程句柄
	BOOL             m_bRun;
	UINT             m_nTimeOut;
	UINT             m_nSockCount;
	WSANETWORKEVENTS m_NetworkEvents;
	string           m_strIP;
    UINT             m_nPort;

	DWORD            m_dwPreTick;
	char             m_szWriteBuf[BUF_SIZE];
	CRITICAL_SECTION m_csLock;
};

/////////////////////////////////////////////////////////////////////////////////
#endif