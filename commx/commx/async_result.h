
#ifndef __ASYNC_OVERLAPPED_RESULT_H__
#define __ASYNC_OVERLAPPED_RESULT_H__

#include <list>
#include "../public/synch.h"
#include "../public/commxbase.h"

/** @brief   �����첽����������ɲ���
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
	SOCKET m_socket;				/**< �������ӵ��׽���(�Ǽ����׽���) */
	char *m_lpBuffer;				/**< ��AcceptEx�ṩ�Ľ��ջ�����ָ�� */
	unsigned int m_nBufferSize;		/**< ��AcceptEx�ṩ�Ľ��ջ��������� */

};

class CAsyncConnectResult : public IAsyncOverlappedResult
{
	friend class ICommX;

public:
	virtual void complete(ICommX *pCommX, unsigned long nTransCount);

private:
	SOCKET m_socket;				/**< ���ӵ��׽��� */
};

/** @brief   �����첽����ɲ���
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

	WSABUF m_wb;				/**< ���ݽ��ջ��壬ʵ�ʴ洢�ص������������ */
	char *m_lpReceiveFlag;		/**< ��һ�ν��յ��ַ�����־ */
	int m_nReceiveSize;			/**< ��һ�ν�Ҫ���յĳ��� or ��һ�ν��յ��ַ�����־�ĳ���*/
	unsigned int m_nFlagRepeatTimes;	/**< ��һ�ν��յ��ַ�����־���ظ����� */
	unsigned int m_nReceivedSize;	/**< �Ѿ����գ�δͶ�ݸ����������ַ����� */
	char *m_lpReadBuffer;		/**< ���ݻ�������� */

	FILE *m_fp;					/**< ����debug�����ļ� */
};

/** @brief   �����첽д��ɲ���
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
	WSABUF m_wb;					/**< ���ݷ��ͻ��壬ʵ�ʴ洢�ص���Ӧ�þ��� */
	LockSingle m_lock;				/**< �������ݶ����� */
	std::list<_pre_data> m_lstData;	/**< �������ݶ��� */

	FILE *m_fp;						/**< ����debug�����ļ� */
};

#endif
