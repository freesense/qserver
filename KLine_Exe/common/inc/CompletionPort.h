/************************************************************************************
*
* 文件名  : CompletionPort.h
*
* 文件描述: 完成端口类
*
* 创建人  : dingjing, 2009-02-02
*
* 版本号  : 1.0
*
* 修改记录:
*
************************************************************************************/
#ifndef _COMPLETIONPORT_H_
#define _COMPLETIONPORT_H_
#include <string>
#include <Winsock2.h>
#include <mswsock.h>
#include "WorkThread.h"
#include "Container.h"


#define DB_MSG_BUF_LEN     400           //LogEvent处理日志缓冲区大小
#define MAX_THREAD_COUNT   10            //设置工作者线程的数量, 结果取MAX_THREAD_COUNT和SysInfo.dwNumberOfProcessors*2的最小值
#define BUF_SIZE           1024*50     //缓冲区大小
//#define RECV_COUNT		   5		 //AcceptEx后投递WSARecv的次数

//定义日志的级别
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

//IO 操作类型
#define IO_ACCEPT		0x01
#define IO_SEND         0x02
#define IO_RECV	        0x03

#pragma pack(push)
#pragma pack(1)
//单IO数据
typedef struct tagIo_Context
{
	OVERLAPPED      overlapped;
	WSABUF          DataBuf;
	char            Buffer[BUF_SIZE];
	SOCKET          hCltSock;
	unsigned char   byIoType;
	time_t          tmPreCheck;        //上次检测时间
}IO_CONTEXT, *LPIO_CONTEXT;

//完成键（单句炳数据）
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
public:	//构造
	CCompletionPort();
	virtual ~CCompletionPort();

public: //属性

public:	//操作
	/********************************************************************************
	* 函数名: StartServer
	* 参  数:
	*		  UINT    nPort      [IN] : 监听端口
	*         int nAcceptRecvSize[IN] : AcceptEx时接收的数据大小，默认-1是缓冲区大小
	*         LPCTSTR pLocalIP   [IN] : 给监听套接字绑定的IP地址，如果本地有多个IP地址，可选其一, 默认为NULL
	* 功  能: 启动完成端口服务器
	* 返回值: 连接成功返回true, 否则false
	********************************************************************************/
	BOOL StartServer(UINT nPort, int nAcceptRecvSize = -1,  LPCSTR pLocalIP = NULL);
	void StopServer();

	/********************************************************************************
	* 函数名: SetAcceptTimeOut
	* 参  数:
	*		  UINT nAccept [IN] : 超时时间，单位：秒 如果连接在设置的时间范围内没有发送数据，就关闭该连接
	*		  UINT nRecv [IN]   : 接收数据超时时间，单位：秒
	* 功  能: 设置超时时间
	********************************************************************************/
	void SetTimeOut(int nAccept, int nRecv){m_nAcceptTimeOut = nAccept; m_nRecvTimeout = nRecv;}
	
	/********************************************************************************
	* 函数名: SendPack
	* 参  数:
	*		  SOCKET hSock     [IN] : 待发送数据的套接字
	*		  void* pData      [IN] : 数据缓冲区
	*		  DWORD dwDataSize [IN] : 待发送数据大小
	* 功  能: 发送数据
	* 返回值: 连接成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL SendPack(SOCKET hSock, void* pData, DWORD dwDataSize);
	BOOL RecvPack(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext, int nLen, int nReaded=0);

	std::string GetPeerIP(SOCKET hSock, UINT& nPort);
protected:
	/********************************************************************************
	* 函数名: LogEvent
	* 参  数:
	*		  WORD nLogType [IN] : 日志的类型
	*                              RPT_DEBUG   : 表示是调试信息
	*						       RPT_INFO    : 表示是提示信息
	*							   RPT_WARNING : 表示是警告信息
	*                              RPT_ERROR   : 表示是错误信息
	* 功  能: 处理日志
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
	
private: //属性
	UINT                  m_nPort;          //端口号
	std::string           m_strLocalIP;     //与监听套接字绑定的IP
	HANDLE                m_hIOComPort;		//完成端口句柄
	CWorkThread*          m_pAcceptThread;  //监听线程
	SOCKET                m_hListenSocket;  //监听套接字
	HANDLE                m_hEventListen;	//监听事件
	BOOL                  m_bRun;           //运行标记
	UINT                  m_nThreadCount;   //工作者线程的个数
	int                   m_nAcceptTimeOut; //设置SO_CONNECT_TIME
	int					  m_nRecvTimeout;   //recv超时时间

	int                   m_nAcceptRecvSize; //AcceptEx接收数据的大小

	LPFN_ACCEPTEX             m_pAcceptEx;		//AcceptEx函数指针
	LPFN_GETACCEPTEXSOCKADDRS m_pGetAcceptExSockaddrs;

	CDeque<CWorkThread*>      m_dequeThread;	//保存CWorkThread*指针
	CDeque<LPIO_CONTEXT>      m_dequeIo;	    //保存未用LPIO_CONTEXT指针
	CDeque<LPHANDLE_CONTEXT>  m_dequeHandle;	//保存未用LPHANDLE_CONTEXT指针
	CMapEx<LPIO_CONTEXT>	  m_mapIo;		    //保存正在使用LPIO_CONTEXT指针
	CMapEx<LPHANDLE_CONTEXT>  m_mapHandle;      //保存正在使用LPHANDLE_CONTEXT指针
};

/////////////////////////////////////////////////////////////////////////////////////
#endif