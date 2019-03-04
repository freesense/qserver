
#ifndef __ASYNC_OVERLAPPED_RESULT_H__
#define __ASYNC_OVERLAPPED_RESULT_H__

#include <list>
#include "../public/synch.h"
#include "../public/commxbase.h"

/** @brief   定义异步接受连接完成操作
 *
 */
class CAsyncAcceptResult : public IAsyncOverlappedResult
{
	friend class ICommX;

public:
	CAsyncAcceptResult(unsigned int nInitialSize);
	~CAsyncAcceptResult();
	virtual void complete(ICommX *pCommX, unsigned long nTransCount);

private:
	SOCKET m_socket;				/**< 接受连接的套接字(非监听套接字) */
	char *m_lpBuffer;				/**< 向AcceptEx提供的接收缓冲区指针 */
	unsigned int m_nBufferSize;		/**< 向AcceptEx提供的接收缓冲区长度 */

};

class CAsyncConnectResult : public IAsyncOverlappedResult
{
	friend class ICommX;

public:
	virtual void complete(ICommX *pCommX, unsigned long nTransCount);

private:
	SOCKET m_socket;				/**< 连接的套接字 */
};

/** @brief   定义异步读完成操作
 *
 */
class CAsyncReadResult : public IAsyncOverlappedResult
{
	friend class ICommX;

public:
	CAsyncReadResult(const char *lpFileName);
	~CAsyncReadResult();
	virtual void complete(ICommX *pCommX, unsigned long nTransCount);

private:
	char* memstr(void* buf, unsigned int buflen, void* lpfind, unsigned int findlen, unsigned int nCount);

	WSABUF m_wb;				/**< 数据接收缓冲，实际存储地点由派生类决定 */
	char *m_lpReceiveFlag;		/**< 下一次接收的字符流标志 */
	int m_nReceiveSize;			/**< 下一次将要接收的长度 or 下一次接收的字符流标志的长度*/
	unsigned int m_nFlagRepeatTimes;	/**< 下一次接收的字符流标志的重复次数 */
	unsigned int m_nReceivedSize;	/**< 已经接收，未投递给处理句柄的字符长度 */
	char *m_lpReadBuffer;		/**< 数据缓冲区起点 */

	FILE *m_fp;					/**< 数据debug缓存文件 */
};

/** @brief   定义异步写完成操作
 *
 */
class CAsyncWriteResult : public IAsyncOverlappedResult
{
	friend class ICommX;

	struct _pre_data
	{
		void *m_pData;
		unsigned int m_nLen;
	};

public:
	CAsyncWriteResult(const char *lpFileName);
	~CAsyncWriteResult();
	virtual void complete(ICommX *pCommX, unsigned long nTransCount);

	unsigned int AddPreSend(void *lpData, unsigned int nLen);
	unsigned int RmvSendData(void **ppData);
	unsigned int GetSendData(void **ppData);
	void OnClose(ICommX *pCommX);

private:
	WSABUF m_wb;					/**< 数据发送缓冲，实际存储地点由应用决定 */
	LockSingle m_lock;				/**< 待发数据队列锁 */
	std::list<_pre_data> m_lstData;	/**< 待发数据队列 */

	FILE *m_fp;						/**< 数据debug缓存文件 */
};

#endif
