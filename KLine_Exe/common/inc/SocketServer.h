//SocketServer.h
//
#ifndef _SOCKETSERVER_H_
#define _SOCKETSERVER_H_
#include <process.h>
#include "WorkThread.h"
#include <vector>
#include <map>
//////////////////////////////////////////////////////////////////////////////
#define RECV_BUF_SIZE 1024    //ע��: ���ջ������Ĵ�СӦ������Ҫ���ú��ʵĴ�С

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
	BOOL               m_bSerRun;       //accept�߳��Ƿ����ѭ��
	CVecCltCon         m_vecCltCon;     //����ͻ�������
	CRITICAL_SECTION   m_csVecCltCon;   //CVecCltCon ���ٽ���
};

//���ࡰCCltConEx��������CClientConnect������������д��Ҫ���麯��
class CCltConEx : public CClientConnect
{
public:
	CCltConEx();
	CCltConEx(SOCKET hCltSock, SOCKADDR_IN* pAddrClt); //ע���ʼ������Ĺ��캯��
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
	char*           m_pSendBuf;      //���ͻ�����
	int             m_nSendBufSize;	 //���ͻ�������С

	UINT            m_nFileCount;    //�����ļ�����������Ʊ����
	UINT            m_nPackSize;     //���Ͱ����ݴ�С����������ͷ
	char*           m_pSendBufPos;
	UINT            m_nRecordCount;  //���ͼ�¼�ĸ���
	
	std::map<UINT, UINT>  m_mapDate; //�������ڵ�map
	typedef std::pair<UINT, UINT> DatePari;
};
//////////////////////////////////////////////////////////////////////////////
#endif