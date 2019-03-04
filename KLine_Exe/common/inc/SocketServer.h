//SocketServer.h
//
#ifndef _SOCKETSERVER_H_
#define _SOCKETSERVER_H_
#include <process.h>
#include "WorkThread.h"
#include <vector>
#include <map>
//////////////////////////////////////////////////////////////////////////////
#define RECV_BUF_SIZE 1024    //注意: 接收缓冲区的大小应根据需要设置合适的大小

class CClientConnect
{
public:
	CClientConnect();
	CClientConnect(SOCKET hCltSock, SOCKADDR_IN* pAddrClt);
	virtual ~CClientConnect();

	void CloseSocket();
	virtual void StartRecvData();
	virtual BOOL SendData(const char* pData, int nLen);

	void SetSocketInfo(SOCKET hCltSock, SOCKADDR_IN* pAddrClt);
	BOOL IsIdle(){return m_hCltSock == INVALID_SOCKET ? TRUE:FALSE;}

	HANDLE  m_hThread;
protected:
	virtual void LogEvent(const char* pFormat, ...);
	virtual void ParseData(const char* pData, int nLen);

private:
	SOCKET         m_hCltSock;
	SOCKADDR_IN    m_addrClt;
	char           m_chRecvBuf[RECV_BUF_SIZE];
};
/////////////////////////////////////////////////////////////////////////////
typedef std::vector<CClientConnect*> CVecCltCon;

class CSocketServer
{
public:
	CSocketServer();
	virtual ~CSocketServer();

	BOOL StartServer(UINT nPort);
	void StopServer();
protected:
	//virtual void LogEvent(const char* pFormat, ...);
	static UINT WINAPI _AcceptThread(LPVOID pParam);
	static UINT WINAPI _ClientThread(LPVOID pParam);
	void WaitForSingleObject(HANDLE hHandle);

private:
	SOCKET             m_hServSock;
	CWorkThread*       m_pAcceptThread;
	BOOL               m_bSerRun;       //accept线程是否结束循环
	CVecCltCon         m_vecCltCon;     //保存客户端连接
	CRITICAL_SECTION   m_csVecCltCon;   //CVecCltCon 用临界区
};

//用类“CCltConEx”派生“CClientConnect”，在类中重写需要的虚函数
class CCltConEx : public CClientConnect
{
public:
	CCltConEx();
	CCltConEx(SOCKET hCltSock, SOCKADDR_IN* pAddrClt); //注意初始化基类的构造函数
	virtual ~CCltConEx();

protected:
	virtual void ParseData(const char* pData, int nLen);
	//virtual void LogEvent(const char* pFormat, ...);

	void FindAllFile(CString strPath);
	void GetFileCount(CString strPath);
	void FillDataToPack(CString strFileName);
	BOOL FindDate(UINT nDate);
	CString ChangeFileNameFormat(CString strFileName, UINT& nStockType);
	void SetMapSecondValue(UINT nValue);
	BOOL IsToday(UINT nDate);
	void FillTodayData();

private:
	char*           m_pSendBuf;      //发送缓冲区
	int             m_nSendBufSize;	 //发送缓冲区大小

	UINT            m_nFileCount;    //日线文件个数，即股票个数
	UINT            m_nPackSize;     //发送包数据大小，不包括包头
	char*           m_pSendBufPos;
	UINT            m_nRecordCount;  //发送记录的个数
	
	std::map<UINT, UINT>  m_mapDate; //保存日期的map
	typedef std::pair<UINT, UINT> DatePari;
};
//////////////////////////////////////////////////////////////////////////////
#endif