/** @file    proxy_auth_server_base.h
 *	@brief   �����������֤�ӿ�
 *  @ingroup proxy_auth_server
 *  
 */
#ifndef __PROXY_AUTH_SERVER_INTERFACE_H__
#define __PROXY_AUTH_SERVER_INTERFACE_H__

/** @brief   ��������Ҫʵ�ֵ���֤�ص��ӿ�
 *  @ingroup proxy_auth_server
 *  
 */
class IServerAuthBase
{
public:
	/** @brief      �û���¼�ص�
	 *
	 *  ��¼�������֤����������û��ĵ�¼��Ϣ�󣬻����û�Ȩ�޸ı�󣬴�����ô˽ӿ�֪ͨ������
	 *  @param      lpUser���û���¼����
	 *  @retval     void
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual void OnUserLogin(const char *lpUser) = 0;

	/** @brief      �û��ǳ�
	 *
	 *  ��¼�������֤���������֪ͨ���û��������ߣ����ô˽ӿ�֪ͨ������
	 *  @param      lpUser���û���¼����
	 *  @param      lpNoticeMessage��֪ͨ��Ϣ��һ��������ԭ��
	 *  @retval     void
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual void OnUserLogout(const char *lpUser, const char *lpNoticeMessage) = 0;

	virtual void OnExecute(const char *lpExecutor, const char *lpParams, const char *lpWorkDir) = 0;
	virtual void OnStop(const char *lpExecutor, const char *lpParams, const char *lpWorkDir) = 0;
	virtual void OnDownloadXml(const char *lpDevName, const char *lpPath) = 0;
	virtual void OnUploadXml(const char *lpPath, const char *pDataBuf, int nDataSize) = 0;
};

/** @brief   ������ʹ�õ���֤����ӿ�
 *  @ingroup proxy_auth_server
 *  
 *	��֤�������������Ŀͻ���֤�����������ؿͻ�Ȩ�����ݸ�������
 */
class IServerAuthProxy
{
public:
	/** @brief      ��������¼
	 *
	 *  ֪ͨ��֤�������µķ�������¼
	 *  @param      lpFingerPrinter��������ָ��
	 *  @retval     int���Ƿ�ɹ��������û���¼������
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnServerLogin(const char *lpFingerPrinter, const char *lpDevList) = 0;

	/** @brief      �û���¼
	 *
	 *  ֪ͨ��֤���������û���¼�������û�Ȩ��
	 *  @param      lpUser���û���¼����
	 *  @param      lpPwd����¼����
	 *  @retval     int���Ƿ�ɹ��������û���¼������
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnUserLogin(const char *lpUser, const char *lpPwd, const char *lpServerFingerPrinter) = 0;

	/** @brief      �û��ǳ�
	 *
	 *  
	 *  @param      lpUser���û���¼����
	 *  @retval     int
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnUserLogout(const char *lpUser, const char *lpServerFingerPrinter) = 0;

	/** @brief      ת��
	 *
	 *  
	 *  @param      lpData��Ҫת��������ָ��
	 *  @param      nLength��ת�����ݳ���
	 *  @retval     int
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnTransmit(char *lpData, unsigned int nLength) = 0;

	/** @brief      ����û�Ȩ��
	 *
	 *  
	 *  @param      lpUser���û���¼����
	 *  @retval     void
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
// 	virtual void GetUserRights(const char *lpUser) = 0;
};

typedef IServerAuthProxy*(*pfnInit)(unsigned int, const char*, unsigned short, IServerAuthBase*);
typedef void(*pfnUninit)();
typedef void(*pfnGetModuleVer)(char**);

#endif
