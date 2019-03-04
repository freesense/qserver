//IOCPServer.h
#ifndef _IOCPSERVER_H_
#define _IOCPSERVER_H_

#include "CompletionPort.h"
#include "../../public/protocol.h"
#include "../../public/xml/tinyxml.h"

#define WM_DEVSERVLOGIN   WM_USER + 187
#define WM_DEVSERVLOGOUT  WM_USER + 188

#define WM_DEVLOGIN   WM_USER + 189
#define WM_DEVLOGOUT  WM_USER + 190

#define WM_DEVSERV_TIMEOUT WM_USER + 191
#define WM_UPDATE_DEV_STATUS WM_USER + 192

struct tagConnect
{
	char         szIP[20];
	UINT         nPort;
	SOCKET       hSock;
};

struct tagDevLogin
{
	char         szIP[20];
	DWORD        dwProID;
};

struct tagRecvData
{
	CommxHead head;
	UINT      nReaded;
};
/////////////////////////////////////////////////////////////////////////////////
class CIOCPServer : public CCompletionPort
{
public:
	CIOCPServer();
	virtual ~CIOCPServer();

	void SetMsgWnd(HWND hWnd){m_hMsgWnd = hWnd;}
protected:
	//virtual void LogEvent(WORD nLogType, LPCSTR pFormat, ...);

	virtual void OnDisconnect(LPHANDLE_CONTEXT pHandleContext);
	virtual BOOL OnRecv(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext, 
		DWORD dwDataSize);
	virtual void OnSend(LPIO_CONTEXT pIoContext);
	virtual void OnConnect(SOCKET hSock);
	virtual void OnTimeOut(LPIO_CONTEXT pIoContext);
private:
	BOOL ProcessRequest(LPHANDLE_CONTEXT pHandleContext, void* pData, CommxHead* pHead);
	BOOL ParseServerLogin(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext);
	BOOL ParseServerLogout(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext);
	BOOL ParseLog(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext);
	BOOL ParseLoginDevList(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext);
	BOOL ParseXmlData(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext);
	void ChangeBinStringToString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize);

	// Ù–‘
	map<SOCKET, tagRecvData*>  m_mapSocket;
	HWND        m_hMsgWnd;
	CCriticalSection m_csMap;
};
/////////////////////////////////////////////////////////////////////////////////
#endif