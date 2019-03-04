/*++

Copyright (c) 2004 

ģ����: 

     iomodel.h

ģ������:

     Winsock ��ɶ˿���ͷ�ļ�

����:

     PPP elssann@hotmail.com

��������:
    
     Visual C++ 6.0, Windows 2000.
     
�޶���¼:

     ������: 2004.1.16

����޸�����:
      2004.1.23


--*/

#ifndef      _IOMODEL_H
#define      _IOMODEL_H

//
//Head files
//
#ifndef _WIN_SOCK_DEF
#define _WIN_SOCK_DEF
#include <winsock2.h>
#include <mswsock.h>
#endif
#include <vector>
#include <list>
#include <map>
#include "rwlock.h"
#include "clientsocket.h"
#include "qs_data_struct.h"
#include "..\\..\\public\\commx\\synch.h"

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
using namespace std; 

#ifdef __cplusplus
extern "C" {
#endif

#define    MAXTHREAD_COUNT       8
#define		MAXTHREAD_REALPUSH	 1

#define    IO_READ_COMPLETION    100
#define    IO_WRITE_COMPLETION   200
#define    IO_ACCEPT_COMPLETION  300
#define		IO_EXTRA_SEND_COMPLETION	 400

#define HTTP_HEADER_SIZE 8*1024

//������������
typedef enum _SOCKET_TYPE
{
	SocketServer,
	SocketClient
}SOCKET_TYPE, *PSOCKET_TYPE;

//
//�Զ���ṹ��������ɼ���(���������)
//
typedef struct _PER_HANDLE_CONTEXT
{
	SOCKET                     IoSocket;
}PER_HANDLE_CONTEXT, *PPER_HANDLE_CONTEXT;


//
//��IO���ݣ���չ��WSAOVERLAPPED
//
typedef struct _PER_IO_CONTEXT
{
	WSAOVERLAPPED              ol;
	char					 * m_pBuf;//������
	unsigned int			   m_nBufLen;
	WSABUF                     wsaBuffer;
	SOCKET                     sClient;
	unsigned int               unId;
	unsigned int				unIdMain;//��Ҫ
	IO_OPERATION               IoOperation;
	SOCKET_TYPE					SocketType;
	time_t						m_timeCheck;//�ϴμ��ʱ��
	int							m_nNeedLineTest;//�Ƿ���Ҫ��·����
	time_t						m_timeLineTest;//��·����ʱ��
	int							m_nOverLen;
	int							m_nRequireLen;
	bool						m_bRealPush;
	char					  *	m_pReqBuf;//ʵʱ��������
	unsigned int				m_nReqBufLen;
	int							m_nSocketError;//0��ʾ�����������ʾ������ʧЧ��ÿ��CheckTimeOut���1������3�򽫴˽ڵ��ͷ�

	unsigned short				m_usProtocolType;//�Ƿ���httpЭ��,0δ֪��1��httpЭ�飬2httpЭ��
	char						m_acHttpHead[HTTP_HEADER_SIZE + 1];
	int							m_nHeadLen;//httpͷ�ĳ���
}PER_IO_CONTEXT, *PPER_IO_CONTEXT;

//������Ϣ
typedef struct _RUN_INFO
{
	unsigned int				m_unAccept;//��������
	unsigned int				m_unRecv;//��������
	unsigned int				m_unSend;//��������

	unsigned int				m_unAccept_S;//�������������
	unsigned int				m_unRecv_S;//�����
	unsigned int				m_unSend_S;//�뷢��

	time_t						m_tmRec;//��ǰ��¼��ʱ��
}RUN_INFO, * PRUN_INFO;
//
// global var
//

static 	GUID g_GUIDAcceptEx = WSAID_ACCEPTEX;
static  GUID g_GUIDTransmitFile = WSAID_TRANSMITFILE;

DWORD __stdcall   ReqRoutine(LPVOID);
DWORD __stdcall   CompletionRoutine(LPVOID);
DWORD __stdcall	  TimeoutRoutine(LPVOID);
DWORD __stdcall   MyThreadLoop(LPVOID);
//
//��ɶ˿�ģ����
//
class CRealData;
class CompletionPortModel
{
public:
	CompletionPortModel();
	~CompletionPortModel();

	BOOL                Init();
	BOOL				CreateMyThreadLoop();
	void				UnInit();
	BOOL                AllocEventMessage();
	void				EndEventMessage();
	BOOL                PostAcceptEx();
	
	virtual BOOL        HandleData(
									PPER_IO_CONTEXT lpPerIoContext, 
								    int nFlags,
									CRealData * c_pRealData
									);
	virtual BOOL        DataAction(
							    	PPER_IO_CONTEXT lpPerIoContext,
							    	PPER_HANDLE_CONTEXT lpNewperHandletext
							    	);

	int					CloseAllSocket();
	int					CheckTimeout(bool c_bRealPush);
	int					CountNode();
	//void                InsertNode(PPER_IO_CONTEXT pNode, PPER_HANDLE_CONTEXT pHandleNode);
	void                InsertNode(PPER_IO_CONTEXT pNode);
	bool                ReleaseNode(PPER_IO_CONTEXT pNode);
	int					CountLookaside();
	int					CountHandleaside();
	void                InsertToLookaside(PPER_IO_CONTEXT lpIoNode, PPER_HANDLE_CONTEXT lpHandleNode);

	PPER_IO_CONTEXT     GetIoFromLookaside();
	PPER_HANDLE_CONTEXT GetHandleFromLookaside();

	BOOL				m_bExit;
	BOOL				m_bMainExit;
	BOOL				m_bThreadLoopExit;//ThreadLoop�Ƿ��˳�
	HANDLE              m_hCOP;
	SOCKET              m_ListenSocket;
	
	RWLock				m_rwLockConnect;
	RWLock				m_rwLockIo;
	RWLock				m_rwLockHandle;
	RWLock				m_rwLockRealReq;
	
	int					m_nRecvCount;
	LPFN_TRANSMITFILE             lpTransmitFile;

	vector<PPER_IO_CONTEXT>	  m_vIoLookaside;
	vector<PPER_HANDLE_CONTEXT> m_vHandleLookaside;
	PPER_HANDLE_CONTEXT m_lpListenHandleContext;

	CClientSocket			m_clientToFarmer;

	RWLock					m_lockVReq;//���������
	//vector<char * >				m_vReq;//����
	list<PPER_IO_CONTEXT>			m_listReq;//����

	DWORD m_dwPreTick;//for monitor
public:
	BOOL                InitWinsock();
	BOOL                BindAndListenSocket();
	void                CloseThreadHandle();
	void                GetAddressAndPort();
	
	UINT                uPort;
	char                szAddress[20];
	
	HANDLE              m_hThreadArray[MAXTHREAD_COUNT];
	HANDLE				m_hThreadTimeout;
	HANDLE				m_hThreadLoop;
	HANDLE				m_hThreadReq[20];
	HANDLE              m_hEvent;
	
	volatile LONG                 m_lAcceptExCounter;
	
	map<unsigned int,PPER_IO_CONTEXT>	  m_mapConnect;

	LPFN_ACCEPTEX                 lpAcceptEx;
	LPFN_GETACCEPTEXSOCKADDRS		m_lpGetAcceptExSockaddrsFun;
	unsigned int					m_unIo;
	unsigned int					m_unHandle;

	int						m_nRecvTimeout;//recv��ʱʱ��
	int						m_nAcceptTimeout;//accept������ʱʱ
	int						m_nLineTest;//��·���԰����ͼ��

	int						m_nError;
	time_t					m_timeFarmUpdate;//���µ����ݸ���ʱ��
	time_t					m_timeRealPush;//���µ���������ʱ��
	int						m_nReleaseFail;

	std::vector<PRUN_INFO>		m_vpRun_Info;
	RWLock						m_lockRun_Info;
public:
	bool IsNeedLineTest(SOCKET c_id);
	//���ó�ʱ
	void				SetTimeout(int c_nAcceptTimeout,int c_nRecvTimeout);
	//������·���԰�����ʱ��
	void SetLineTestTime(int c_nTime);
	// �첽��������
	bool ASYSendBuffer(PPER_IO_CONTEXT c_lpIo);
	int ConnectTo(char * c_pHost, int c_nPort);
	int ConnectToFarmer(char * c_pHost, int c_nPort);
	void OnSocketClose(unsigned int c_unID,SOCKET c_s);
	PPER_IO_CONTEXT FindIO(unsigned int c_ID,SOCKET c_socket);
	PPER_IO_CONTEXT FindIO(unsigned int c_ID);

	PPER_IO_CONTEXT GetNewReq();
	void PushNewReq(PPER_IO_CONTEXT c_lpIo);
	void PushNewReq(SOCKET c_sClient,char * c_pReq);
	int DealReq(PPER_IO_CONTEXT c_lpIO,CRealData * c_pRealData);
	void OnFarmUpdateData(char * c_pBuf);//
	void ProcessCount(CRealData &c_RealData);
	void OutputState(int &c_nAccept);
	void AddRunInfo(PRUN_INFO c_pRun_Info);
	void RemoveRunInfo(PRUN_INFO c_pRun_Info);
	void TotalRunInfo(PRUN_INFO c_pRun_Info);
	char* memstr(const void* buf, unsigned int buflen, const void* lpfind, unsigned int findlen);
};


#ifdef __cplusplus
}
#endif

#endif  //_IOMODEL_H