/************************************************************************************
*
* �ļ���  : CompletionPort.h
*
* �ļ�����: ��ɶ˿���
*
* ������  : dingjing, 2009-02-02
*
* �汾��  : 1.0
*
* �޸ļ�¼:
*
************************************************************************************/
#ifndef _COMPLETIONPORT_H_
#define _COMPLETIONPORT_H_
#include <string>
#include <Winsock2.h>
#include <mswsock.h>
#include "WorkThread.h"
#include "Container.h"


#define DB_MSG_BUF_LEN     400           //LogEvent������־��������С
#define MAX_THREAD_COUNT   10            //���ù������̵߳�����, ���ȡMAX_THREAD_COUNT��SysInfo.dwNumberOfProcessors*2����Сֵ
#define BUF_SIZE           1024*50     //��������С
//#define RECV_COUNT		   5		 //AcceptEx��Ͷ��WSARecv�Ĵ���

//������־�ļ���
#ifndef	RPT_DEBUG
#define RPT_DEBUG		0x10
#endif

#ifndef	RPT_INFO
#define RPT_INFO		0x20
#endif

#ifndef	RPT_WARNING
#define RPT_WARNING		0x40
#endif

#ifndef	RPT_ERROR
#define RPT_ERROR		0x80
#endif

#ifndef	RPT_HEARTBEAT
#define RPT_HEARTBEAT    0xAA
#endif

//IO ��������
#define IO_ACCEPT		0x01
#define IO_SEND         0x02
#define IO_RECV	        0x03

#pragma pack(push)
#pragma pack(1)
//��IO����
typedef struct tagIo_Context
{
	OVERLAPPED      overlapped;
	WSABUF          DataBuf;
	char            Buffer[BUF_SIZE];
	SOCKET          hCltSock;
	unsigned char   byIoType;
	time_t          tmPreCheck;        //�ϴμ��ʱ��
}IO_CONTEXT, *LPIO_CONTEXT;

//��ɼ�����������ݣ�
typedef struct tagHandle_Context
{
	SOCKET          hSocket;
	SOCKADDR_IN		addr;
}HANDLE_CONTEXT, *LPHANDLE_CONTEXT;
#pragma pack(pop)

#pragma comment(lib, "Ws2_32.lib")

static GUID g_GUIDAcceptEx = WSAID_ACCEPTEX;
static GUID g_GUIDGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
/////////////////////////////////////////////////////////////////////////////////////
class CCompletionPort
{
public:	//����
	CCompletionPort();
	virtual ~CCompletionPort();

public: //����

public:	//����
	/********************************************************************************
	* ������: StartServer
	* ��  ��:
	*		  UINT    nPort      [IN] : �����˿�
	*         int nAcceptRecvSize[IN] : AcceptExʱ���յ����ݴ�С��Ĭ��-1�ǻ�������С
	*         LPCTSTR pLocalIP   [IN] : �������׽��ְ󶨵�IP��ַ����������ж��IP��ַ����ѡ��һ, Ĭ��ΪNULL
	* ��  ��: ������ɶ˿ڷ�����
	* ����ֵ: ���ӳɹ�����true, ����false
	********************************************************************************/
	BOOL StartServer(UINT nPort, int nAcceptRecvSize = -1,  LPCSTR pLocalIP = NULL);
	void StopServer();

	/********************************************************************************
	* ������: SetAcceptTimeOut
	* ��  ��:
	*		  UINT nAccept [IN] : ��ʱʱ�䣬��λ���� ������������õ�ʱ�䷶Χ��û�з������ݣ��͹رո�����
	*		  UINT nRecv [IN]   : �������ݳ�ʱʱ�䣬��λ����
	* ��  ��: ���ó�ʱʱ��
	********************************************************************************/
	void SetTimeOut(int nAccept, int nRecv){m_nAcceptTimeOut = nAccept; m_nRecvTimeout = nRecv;}
	
	/********************************************************************************
	* ������: SendPack
	* ��  ��:
	*		  SOCKET hSock     [IN] : ���������ݵ��׽���
	*		  void* pData      [IN] : ���ݻ�����
	*		  DWORD dwDataSize [IN] : ���������ݴ�С
	* ��  ��: ��������
	* ����ֵ: ���ӳɹ�����TRUE, ����FALSE
	********************************************************************************/
	BOOL SendPack(SOCKET hSock, void* pData, DWORD dwDataSize);
	BOOL RecvPack(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext, int nLen, int nReaded=0);

	std::string GetPeerIP(SOCKET hSock, UINT& nPort);
protected:
	/********************************************************************************
	* ������: LogEvent
	* ��  ��:
	*		  WORD nLogType [IN] : ��־������
	*                              RPT_DEBUG   : ��ʾ�ǵ�����Ϣ
	*						       RPT_INFO    : ��ʾ����ʾ��Ϣ
	*							   RPT_WARNING : ��ʾ�Ǿ�����Ϣ
	*                              RPT_ERROR   : ��ʾ�Ǵ�����Ϣ
	* ��  ��: ������־
	********************************************************************************/
	//virtual void LogEvent(WORD nLogType, LPCSTR pFormat, ...);

	BOOL InitServer();
	static UINT _stdcall _WorkThread(void* pParam);
	static UINT _stdcall _AcceptThread(void* pParam);
	UINT WorkThread();
	UINT AcceptThread();
	BOOL PostAcceptEx();

	virtual void OnDisconnect(LPHANDLE_CONTEXT pHandleContext);
	virtual BOOL OnRecv(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext,
		DWORD dwDataSize);

	virtual void OnSend(LPIO_CONTEXT pIoContext);
	virtual void OnConnect(SOCKET hSock);
	virtual void OnTimeOut(LPIO_CONTEXT pIoContext);

	void DisConnected(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext);

	LPIO_CONTEXT GetIoContext();
	LPHANDLE_CONTEXT GetHandleContext();
	void ReleaseIoContext(LPIO_CONTEXT pData);
	void ReleaseHandleContext(LPHANDLE_CONTEXT pData);
	void OnAccept(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext, DWORD dwTransCount);
	
private: //����
	UINT                  m_nPort;          //�˿ں�
	std::string           m_strLocalIP;     //������׽��ְ󶨵�IP
	HANDLE                m_hIOComPort;		//��ɶ˿ھ��
	CWorkThread*          m_pAcceptThread;  //�����߳�
	SOCKET                m_hListenSocket;  //�����׽���
	HANDLE                m_hEventListen;	//�����¼�
	BOOL                  m_bRun;           //���б��
	UINT                  m_nThreadCount;   //�������̵߳ĸ���
	int                   m_nAcceptTimeOut; //����SO_CONNECT_TIME
	int					  m_nRecvTimeout;   //recv��ʱʱ��

	int                   m_nAcceptRecvSize; //AcceptEx�������ݵĴ�С

	LPFN_ACCEPTEX             m_pAcceptEx;		//AcceptEx����ָ��
	LPFN_GETACCEPTEXSOCKADDRS m_pGetAcceptExSockaddrs;

	CDeque<CWorkThread*>      m_dequeThread;	//����CWorkThread*ָ��
	CDeque<LPIO_CONTEXT>      m_dequeIo;	    //����δ��LPIO_CONTEXTָ��
	CDeque<LPHANDLE_CONTEXT>  m_dequeHandle;	//����δ��LPHANDLE_CONTEXTָ��
	CMapEx<LPIO_CONTEXT>	  m_mapIo;		    //��������ʹ��LPIO_CONTEXTָ��
	CMapEx<LPHANDLE_CONTEXT>  m_mapHandle;      //��������ʹ��LPHANDLE_CONTEXTָ��
};

/////////////////////////////////////////////////////////////////////////////////////
#endif