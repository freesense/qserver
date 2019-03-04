/** @defgroup commx ����ͨѶ��
 *
 *  ʹ����ɶ˿ڵ��첽ͨѶ��
 */

 /** @file   commxbase.h
 *	@brief   ����ͨѶ��ӿ�
 *  @ingroup commx
 *  ʹ��commxͨѶ����Ҫ�������ļ�
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

	/** @brief      ��ϵͳע��һ����ʱ��
	 *
	 *  ϵͳ���ڶ�ʱ�����ص�#ITimerBase::handle_timeout
	 *  @param      pTimerParam ��ʱ���ص�����
	 *  @param      tm_first ��ʱ���ص�ʱ�䣬��λ��
	 *  @param      tm_interval ��ʱ�����ʱ�䣬��λ��
	 *  @retval     long����ʱ��ID
	 *  @warning    ��ʱ���Ծ���ʱ��ע�ᣬ����ע�������ı䱾��ʱ�䣬�����Ŷ�ʱ����������
	 *  @exception  
	 */
	long RegisterTimer(ITimerCallbackBase *pTimerParam, float tm_first, float tm_interval);

	/**@brief ��ϵͳע��һ����ʱ��
	
	*  @param      pTimerParam ��ʱ���ص�����
	*  @param      tm_first ��ʱ���״λص�ʱ�䣬��ʽΪ"HHMMSS"���Ա���ʱ�����
	*  @param      tm_interval ��ʱ�����ʱ�䣬��λ��
	*  @retval     long����ʱ��ID
	 */
	long RegisterTimer(ITimerCallbackBase *pTimerParam, const char *tm_first, float tm_interval);

	void CancelTimer(long TimerID);
	void ChangeTimer(long TimerID, float tm_first, float tm_interval);

	virtual int handle_timeout(ITimerCallbackBase *pTimerParam) = 0;
};

/** @brief   ͨѶ����������
 *  @ingroup commx
 *  ���д������ӵ��඼����ӱ�������
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

	/** @brief      �����ӽ���ʱ��ǰ�����ص�
     *
     *  �����µĿͻ�������ʱ���ô˺�������ȷ������˫���ĵ�ַ��Ϣ
     *  @retval     ICommX*�����������ӵ����Ӵ�������ָ�룬���ΪNULL����ʾ�����������ܴ�����
     */
	virtual ICommX* handle_accept(const char *lpLocalAddr, const char *lpPeerAddr);

    /** @brief      �����ӽ���ʱ��ǰ�����ص�
     *
     *  ���½����������ϻص��������������ڴ˽���һЩ���ӽ����ĳ�ʼ��������
     *  @retval     int������Ҫ���յ�һ������ҵ�������󳤶ȣ�-1��ʾʧ�ܣ����ӽ����Ͽ�
     *  @warning    ����ֵ��ʾ����Ҫ���յİ��е����İ����ȣ�ϵͳ�������ֵ�������ڲ����ջ���Ĵ�С����
	                ��ֵ�����������С������г���������ȵĲ��ֻᱻ�ص����������в���Ԥ�ϵĽ��
     */
    virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);

	/** @brief      �����ӽ����ر�ʱ������
	 *
	 *  ������ǰ�������ã�Ҳ������Ӧ�������ر�ʱ���á�Ĭ�ϲ����ǵ���#handle_write��ʹӦ���л���������ͻ�����
	 *  @param      iError����ɹرյĴ�����룬0��ʾû�м�⵽����
	 *  @retval     void
	 *  @warning    Ӧ�����ظú���ʱ�������ں������ڵ���#ICommX::handle_close����������ͻ�����
	 */
	virtual int handle_close(int iError);

	virtual void handle_release();

	/** @brief      �����յ���������ʱ��ǰ�����ص�
	 *
	 *  ������ǽ��յ���һ��������ҵ�������Ҫ�ں����е���#ICommX::Read��������һ�ζ�ȡ
	 *  @param      nLength ���յ�������������
	 *  @retval     int��0-������-1-ɾ��������
	 *  @warning    ��������������û��Լ�����#ICommX::Read()������ʣ��İ�����һ����
	 */
	virtual int handle_read(char *lpBuffer, unsigned int nLength);

	/** @brief      ��������һ�������İ�ʱ��ǰ�����ص�
	 *
	 *  
	 *  @param			lpData
	 *  @param			nLength
	 *  @param			bSendOK
	 *  @retval		int
	 */
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);

	/** @brief      ��ʱ��ʱ�䵽
	 *
	 *  
	 *  @param      pTimerParam ע�ᶨʱ��ʱ�ṩ�Ĳ���
	 *  @retval     void
	 */
	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);

	/** @brief      �رմ�����
	 *
	 *  ���ñ���������ͼɾ���ײ�Ĵ�����
	 *  @retval     int��0-�ײ�ͨѶ����ɾ����1-�ײ�ͨѶ����δɾ��
	 */
	void Close();

	bool checkdump_read_buffer(unsigned short nLength, const char *lpCompare, unsigned short nCmpLen);

	int Listen(const char *lpszAddress, unsigned short nAcceptCount, unsigned int nInitialRead = 0);
	int Connect(const char *lpszAddress, const char *lpSendBuffer = 0, unsigned int nSendLen = 0);

	/** @brief      �������ж�ȡ����
	 *
	 *  Ͷ��һ�����������ڽ��ջ�������Ӧ���ṩ�����Ӧ��������ά�����ջ�������Ŀǰ�Ļ����ǣ�
	 *	1.Ӧ�ñ����ṩ���ջ�������IOCP���յ�������ֱ����䵽������ջ������С�
	 *	2.ÿ��#Read()���ö�����Ӧ��Ҫ����ջ�������Ӧ���Լ���������ָ�����������ݡ�
	 *	3.Ӧ������#GetRecvBuffer()����ʵ�ֽ��ջ���Ĺ���
	 *  @param		nLength
					- =-1�������յ��������ݣ��������û�����
					- >0��lpszString=0��Ҫ��ȡ�����ݵĳ��ȣ�ϵͳ�����յ��㹻���ȵ����ݺ��ٽ����ݽ����û�
					- >0��lpszString!=0������ַ������ȣ�ϵͳ�����յ�nCount������ַ���֮���ٽ����ݽ����û�
	 *  @param		readID ��������־����Ӧ���Լ�ά��
	 *  @param		nLength Ҫ��ȡ�����ݳ���
	 *  @param		lpszString ��Ϊ��ǵ��ַ����������������ַ�
	 *  @param		nCount ����ظ�����
	 *  @retval	int��0-�ɹ���-1-ʧ��
	 *  @see		Read(0)-�����������ݣ�Read(-1)-������һ�εĽ��������������գ����յ�����һ�ν��յ�����β��
	 */
	int Read(int readID, int nLength, unsigned int nCount = 1, const char *lpszString = NULL);

	long handle_result(IAsyncOverlappedResult *pResult);

	/** @brief      �����ӵĶԷ���������
	 *
	 *  �첽���������ú�����̷��أ�����ʱ���ݲ�һ���Ѿ������ͳ�ȥ
	 Ĭ�ϲ�����ʾ���Ͷ����еĴ�������
	 *  @param      lpData Ҫ���͵�����ָ��
	 *  @param      nLength Ҫ���͵����ݳ���
	 *  @retval     int 0-�ɹ���-1-ʧ��
	 */
	int Write(void *lpData = NULL, unsigned int nLength = 0);

protected:
	int accept(unsigned short nAcceptCount, unsigned int nInitialRead);
	int accept(IAsyncOverlappedResult *pResult);
	UINT GetSocket();
	const char* GetPeerAddress();
	const char* GetLocalAddress();
	long GetAsyncCount();

	char m_szLocalAddress[22];				/**< ����IP��ַ�Ͷ˿ں� */
	char m_szPeerAddress[22];				/**< Զ��IP��ַ�Ͷ˿ںţ�����Ǽ���������ֶ�Ϊ�� */
	bool m_bRelease;						/**< �Ƿ�رձ����� */

	bool m_bDebug;							/**< �Ƿ��ӡ�շ����� */

private:
	IAsyncOverlappedResult* get_read_result();
	IAsyncOverlappedResult* get_write_result();
	IAsyncOverlappedResult* get_connect_result();

	IAsyncOverlappedResult *m_pResult;		/**< �첽��ɽ������ */
	LockRW *m_lock_result;					/**< �첽��ɽ�������� */
	UINT m_socket;							/**< socket�׽��� */
	long m_nAsyncCount;						/**< ��Ͷ�ݵ��첽���������� */
};

/** @brief   �ص�������ɻ���
*  @ingroup commx
*  ���ö�̬�ԣ��ַ���ɲ���������ò����Ķ���
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
