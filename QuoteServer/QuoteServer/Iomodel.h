/*++

Copyright (c) 2004 

模块名: 

     iomodel.h

模块描述:

     Winsock 完成端口类头文件

作者:

     PPP elssann@hotmail.com

开发环境:
    
     Visual C++ 6.0, Windows 2000.
     
修订记录:

     创建于: 2004.1.16

最后修改日期:
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

//定义连接类型
typedef enum _SOCKET_TYPE
{
	SocketServer,
	SocketClient
}SOCKET_TYPE, *PSOCKET_TYPE;

//
//自定义结构，即“完成键”(单句柄数据)
//
typedef struct _PER_HANDLE_CONTEXT
{
	SOCKET                     IoSocket;
}PER_HANDLE_CONTEXT, *PPER_HANDLE_CONTEXT;


//
//单IO数据，扩展的WSAOVERLAPPED
//
typedef struct _PER_IO_CONTEXT
{
	WSAOVERLAPPED              ol;
	char					 * m_pBuf;//缓冲区
	unsigned int			   m_nBufLen;
	WSABUF                     wsaBuffer;
	SOCKET                     sClient;
	unsigned int               unId;
	unsigned int				unIdMain;//主要
	IO_OPERATION               IoOperation;
	SOCKET_TYPE					SocketType;
	time_t						m_timeCheck;//上次检测时间
	int							m_nNeedLineTest;//是否需要线路测试
	time_t						m_timeLineTest;//线路测试时间
	int							m_nOverLen;
	int							m_nRequireLen;
	bool						m_bRealPush;
	char					  *	m_pReqBuf;//实时推送请求
	unsigned int				m_nReqBufLen;
	int							m_nSocketError;//0表示正常，否则表示此连接失效，每次CheckTimeOut会加1，大于3则将此节点释放

	unsigned short				m_usProtocolType;//是否是http协议,0未知，1非http协议，2http协议
	char						m_acHttpHead[HTTP_HEADER_SIZE + 1];
	int							m_nHeadLen;//http头的长度
}PER_IO_CONTEXT, *PPER_IO_CONTEXT;

//运行信息
typedef struct _RUN_INFO
{
	unsigned int				m_unAccept;//连接数量
	unsigned int				m_unRecv;//接收数据
	unsigned int				m_unSend;//发送数据

	unsigned int				m_unAccept_S;//秒接受连接数量
	unsigned int				m_unRecv_S;//秒接收
	unsigned int				m_unSend_S;//秒发送

	time_t						m_tmRec;//当前记录的时间
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
//完成端口模型类
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
	BOOL				m_bThreadLoopExit;//ThreadLoop是否退出
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

	RWLock					m_lockVReq;//请求队列锁
	//vector<char * >				m_vReq;//请求
	list<PPER_IO_CONTEXT>			m_listReq;//请求

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

	int						m_nRecvTimeout;//recv超时时间
	int						m_nAcceptTimeout;//accept建立超时时
	int						m_nLineTest;//线路测试包发送间隔

	int						m_nError;
	time_t					m_timeFarmUpdate;//最新的数据更新时间
	time_t					m_timeRealPush;//最新的数据推送时间
	int						m_nReleaseFail;

	std::vector<PRUN_INFO>		m_vpRun_Info;
	RWLock						m_lockRun_Info;
public:
	bool IsNeedLineTest(SOCKET c_id);
	//设置超时
	void				SetTimeout(int c_nAcceptTimeout,int c_nRecvTimeout);
	//设置线路测试包发送时间
	void SetLineTestTime(int c_nTime);
	// 异步发送数据
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