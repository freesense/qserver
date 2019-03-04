/** @file    proxy_auth_server_base.h
 *	@brief   定义服务器认证接口
 *  @ingroup proxy_auth_server
 *  
 */
#ifndef __PROXY_AUTH_SERVER_INTERFACE_H__
#define __PROXY_AUTH_SERVER_INTERFACE_H__

/** @brief   服务器方要实现的认证回调接口
 *  @ingroup proxy_auth_server
 *  
 */
class IServerAuthBase
{
public:
	/** @brief      用户登录回调
	 *
	 *  登录代理从认证服务器获得用户的登录信息后，或者用户权限改变后，代理调用此接口通知服务器
	 *  @param      lpUser，用户登录名称
	 *  @retval     void
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual void OnUserLogin(const char *lpUser) = 0;

	/** @brief      用户登出
	 *
	 *  登录代理从认证服务器获得通知，用户必须下线，调用此接口通知服务器
	 *  @param      lpUser，用户登录名称
	 *  @param      lpNoticeMessage，通知消息，一般是下线原因
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

/** @brief   服务器使用的认证代理接口
 *  @ingroup proxy_auth_server
 *  
 *	认证代理代理服务器的客户认证工作，并返回客户权限数据给服务器
 */
class IServerAuthProxy
{
public:
	/** @brief      服务器登录
	 *
	 *  通知认证服务器新的服务器登录
	 *  @param      lpFingerPrinter，服务器指纹
	 *  @retval     int，是否成功发出新用户登录的请求
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnServerLogin(const char *lpFingerPrinter, const char *lpDevList) = 0;

	/** @brief      用户登录
	 *
	 *  通知认证服务器新用户登录，请求用户权限
	 *  @param      lpUser，用户登录名称
	 *  @param      lpPwd，登录密码
	 *  @retval     int，是否成功发出新用户登录的请求
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnUserLogin(const char *lpUser, const char *lpPwd, const char *lpServerFingerPrinter) = 0;

	/** @brief      用户登出
	 *
	 *  
	 *  @param      lpUser，用户登录名称
	 *  @retval     int
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnUserLogout(const char *lpUser, const char *lpServerFingerPrinter) = 0;

	/** @brief      转发
	 *
	 *  
	 *  @param      lpData，要转发的数据指针
	 *  @param      nLength，转发数据长度
	 *  @retval     int
	 *  @warning    
	 *  @see        
	 *  @ingroup    
	 *  @exception  
	 */
	virtual int OnTransmit(char *lpData, unsigned int nLength) = 0;

	/** @brief      获得用户权限
	 *
	 *  
	 *  @param      lpUser，用户登录名称
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
