/** @defgroup commx 基础通讯库
 *
 *  使用完成端口的异步通讯库
 */

 /** @file   commxbase.h
 *	@brief   定义通讯库接口
 *  @ingroup commx
 *  使用commx通讯库需要包含本文件
 */
#ifndef __COMMX_WINNT_BASE_ICOMMX_H__
#define __COMMX_WINNT_BASE_ICOMMX_H__

#include "commxapi.h"

class IAsyncOverlappedResult;
class CAsyncAcceptResult;
class CAsyncConnectResult;
class CAsyncReadResult;
class CAsyncWriteResult;
class LockRW;

class COMMXAPI ITimerBase
{
public:
	struct ITimerCallbackBase
	{
		ITimerCallbackBase() : m_pCallback(NULL), m_nTimerID(0) {};
		ITimerBase *m_pCallback;
		unsigned int m_nTimerID;
	};

	/** @brief      向系统注册一个定时器
	 *
	 *  系统将在定时到达后回调#ITimerBase::handle_timeout
	 *  @param      pTimerParam 定时器回调对象
	 *  @param      tm_first 定时器回调时间，单位秒
	 *  @param      tm_interval 定时器间隔时间，单位秒
	 *  @retval     long，定时器ID
	 *  @warning    定时器以绝对时间注册，所以注册后如果改变本地时间，将干扰定时器正常运行
	 *  @exception  
	 */
	long RegisterTimer(ITimerCallbackBase *pTimerParam, float tm_first, float tm_interval);

	/**@brief 向系统注册一个定时器
	
	*  @param      pTimerParam 定时器回调对象
	*  @param      tm_first 定时器首次回调时间，格式为"HHMMSS"，以本地时间计算
	*  @param      tm_interval 定时器间隔时间，单位秒
	*  @retval     long，定时器ID
	 */
	long RegisterTimer(ITimerCallbackBase *pTimerParam, const char *tm_first, float tm_interval);

	void CancelTimer(long TimerID);
	void ChangeTimer(long TimerID, float tm_first, float tm_interval);

	virtual int handle_timeout(ITimerCallbackBase *pTimerParam) = 0;
};

/** @brief   通讯处理器基类
 *  @ingroup commx
 *  所有处理连接的类都必须从本类派生
 */
class COMMXAPI ICommX : public ITimerBase
{
	friend class CAsyncAcceptResult;
	friend class CAsyncConnectResult;
	friend class CAsyncReadResult;
	friend class CAsyncWriteResult;

public:
	ICommX();
	virtual ~ICommX();

	static char* GetHostname(char *lpBuffer, unsigned int nBufSize);
	static char* GetAddrFromSockaddr(char *lpBuffer, unsigned int nBufSize, void *lp_sockaddr_in);
	static char* GetAddrFromHostname(char *lpBuffer, unsigned int nBufSize, const char *lpHostname, void **lpp_addr_info);
	static void* GetSockaddrFromIp(void *lp_sockaddr_in, char *lpNetaddr);

	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);

	/** @brief      当连接进入时被前摄器回调
     *
     *  接受新的客户端连接时调用此函数，以确定连接双方的地址信息
     *  @retval     ICommX*，处理新连接的连接处理器的指针，如果为NULL，表示服务器不接受此连接
     */
	virtual ICommX* handle_accept(const char *lpLocalAddr, const char *lpPeerAddr);

    /** @brief      当连接建立时被前摄器回调
     *
     *  在新建立的连接上回调本函数，可以在此进行一些连接建立的初始化工作。
     *  @retval     int，返回要接收的一个完整业务包的最大长度，-1表示失败，连接将被断开
     *  @warning    返回值表示所有要接收的包中的最大的包长度，系统根据这个值来设置内部接收缓冲的大小。这
	                个值如果比最大包长小，则包中超过这个长度的部分会被截掉，并可能有不可预料的结果
     */
    virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);

	/** @brief      当连接将被关闭时被调用
	 *
	 *  可能是前摄器调用，也可能是应用主动关闭时调用。默认操作是调用#handle_write，使应用有机会清除发送缓冲区
	 *  @param      iError，造成关闭的错误代码，0表示没有检测到错误
	 *  @retval     void
	 *  @warning    应用重载该函数时，必须在函数体内调用#ICommX::handle_close，以清除发送缓冲区
	 */
	virtual int handle_close(int iError);

	virtual void handle_release();

	/** @brief      当接收到完整数据时被前摄器回调
	 *
	 *  如果不是接收到了一个完整的业务包，需要在函数中调用#ICommX::Read来进行下一次读取
	 *  @param      nLength 接收的数据完整长度
	 *  @retval     int，0-正常，-1-删除处理器
	 *  @warning    在这个函数里面用户自己调用#ICommX::Read()来接收剩余的包或下一个包
	 */
	virtual int handle_read(char *lpBuffer, unsigned int nLength);

	/** @brief      当发送完一个完整的包时被前摄器回调
	 *
	 *  
	 *  @param			lpData
	 *  @param			nLength
	 *  @param			bSendOK
	 *  @retval		int
	 */
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);

	/** @brief      定时器时间到
	 *
	 *  
	 *  @param      pTimerParam 注册定时器时提供的参数
	 *  @retval     void
	 */
	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);

	/** @brief      关闭处理器
	 *
	 *  调用本函数会试图删除底层的处理器
	 *  @retval     int，0-底层通讯对象删除，1-底层通讯对象还未删除
	 */
	void Close();

	bool checkdump_read_buffer(unsigned short nLength, const char *lpCompare, unsigned short nCmpLen);

	int Listen(const char *lpszAddress, unsigned short nAcceptCount, unsigned int nInitialRead = 0);
	int Connect(const char *lpszAddress, const char *lpSendBuffer = 0, unsigned int nSendLen = 0);

	/** @brief      从连接中读取数据
	 *
	 *  投递一个读请求。由于接收缓冲区由应用提供，因此应用有责任维护接收缓冲区。目前的机制是：
	 *	1.应用必须提供接收缓冲区，IOCP将收到的数据直接填充到这个接收缓冲区中。
	 *	2.每次#Read()调用都会向应用要求接收缓冲区，应用自己管理缓冲区指针和里面的数据。
	 *	3.应用重载#GetRecvBuffer()，来实现接收缓冲的管理
	 *  @param		nLength
					- =-1，不管收到多少数据，都交给用户处理
					- >0且lpszString=0，要收取的数据的长度，系统会在收到足够长度的数据后再将数据交给用户
					- >0且lpszString!=0，标记字符串长度，系统会在收到nCount个标记字符串之后再将数据交给用户
	 *  @param		readID 读操作标志，由应用自己维护
	 *  @param		nLength 要读取的数据长度
	 *  @param		lpszString 作为标记的字符串，可以是任意字符
	 *  @param		nCount 标记重复次数
	 *  @retval	int，0-成功，-1-失败
	 *  @see		Read(0)-接收任意数据，Read(-1)-按照上一次的接收条件继续接收，接收点在上一次接收的数据尾部
	 */
	int Read(int readID, int nLength, unsigned int nCount = 1, const char *lpszString = NULL);

	long handle_result(IAsyncOverlappedResult *pResult);

	/** @brief      向连接的对方发送数据
	 *
	 *  异步函数，调用后会立刻返回，但此时数据不一定已经被发送出去
	 默认参数表示发送队列中的待发数据
	 *  @param      lpData 要发送的数据指针
	 *  @param      nLength 要发送的数据长度
	 *  @retval     int 0-成功，-1-失败
	 */
	int Write(void *lpData = NULL, unsigned int nLength = 0);

protected:
	int accept(unsigned short nAcceptCount, unsigned int nInitialRead);
	int accept(IAsyncOverlappedResult *pResult);
	UINT GetSocket();
	const char* GetPeerAddress();
	const char* GetLocalAddress();
	long GetAsyncCount();

	char m_szLocalAddress[22];				/**< 本地IP地址和端口号 */
	char m_szPeerAddress[22];				/**< 远程IP地址和端口号，如果是监听，这个字段为空 */
	bool m_bRelease;						/**< 是否关闭本对象 */

	bool m_bDebug;							/**< 是否打印收发数据 */

private:
	IAsyncOverlappedResult* get_read_result();
	IAsyncOverlappedResult* get_write_result();
	IAsyncOverlappedResult* get_connect_result();

	IAsyncOverlappedResult *m_pResult;		/**< 异步完成结果对象 */
	LockRW *m_lock_result;					/**< 异步完成结果对象锁 */
	UINT m_socket;							/**< socket套接字 */
	long m_nAsyncCount;						/**< 已投递的异步操作的数量 */
};

/** @brief   重叠操作完成基类
*  @ingroup commx
*  利用多态性，分发完成操作到发起该操作的对象
*/
class COMMXAPI IAsyncOverlappedResult : public OVERLAPPED
{
	friend class ICommX;

public:
	IAsyncOverlappedResult();
	virtual ~IAsyncOverlappedResult();

	virtual void complete(ICommX *pCommX, unsigned long nTransCount) = 0;

private:
	IAsyncOverlappedResult *m_pNext;
};

COMMXAPI int start_comm_loop(unsigned short nThreadNum = 0);
COMMXAPI void stop_comm_loop();
COMMXAPI BOOL post_completion(unsigned int, unsigned long, IAsyncOverlappedResult*);
COMMXAPI bool InitializeTimerQueue();
COMMXAPI void UninitializeTimerQueue();

#endif
