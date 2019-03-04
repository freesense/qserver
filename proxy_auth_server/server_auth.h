/** @file    server_auth.h
 *	@brief   实现服务器认证代理接口
 *  @ingroup proxy_auth_server
 *  
 */
#ifndef __SERVER_AUTH_H__
#define __SERVER_AUTH_H__

#pragma warning(disable:4786)

#include <map>
#include <string>
#include "../public/commx/synch.h"
#include "../public/devmonitor/proxy_auth_server_base.h"

using std::map;
using std::string;

/** @brief   定义服务器认证代理
 *  @ingroup proxy_auth_server
 *  
 */
class CServerAuth : public IServerAuthProxy
{
	struct _UserRights
	{
		char Group[4];		/**< 用户组类型，0-试用用户组，1-正式收费普通用户组，2-正式收费Vip组 */
		char Expiration[12];/**< 用户截止日期 */
		string Market;		/**< 用户市场代码 */
		string Data;		/**< 用户数据代码 */
		string Function;	/**< 用户功能代码 */
	};

public:	//add by dj
	typedef std::map<std::string, _UserRights> Mp_UserRights;			/**< 重定义登录用户权限数据类型，用户名-用户权限 */
	typedef std::map<std::string, Mp_UserRights> Mp_SvrUser;			/**< 重定义登录用户权限数据类型，服务器指纹-用户 */
	LockRW m_lock;					/**< 用户权限表访问锁 */

public:
	CServerAuth(IServerAuthBase *pSvrAuthBase);
	virtual ~CServerAuth();

	virtual int OnServerLogin(const char *lpFingerPrinter, const char *lpDevList);
	virtual int OnUserLogin(const char *lpUser, const char *lpPwd, const char *lpServerFingerPrinter);
	virtual int OnUserLogout(const char *lpUser, const char *lpServerFingerPrinter);
	virtual int OnTransmit(char *lpData, unsigned int nLength);

	IServerAuthBase *m_pCallback;	/**< 回调对象 */

private:
	Mp_SvrUser m_mpServerUser;		/**< 用户权限表 */

	std::string  m_strFingerPrinter;
	std::string  m_strDevTable;
};

extern CServerAuth *g_pAuth;		/**< 服务器认证代理全局唯一对象指针 */

#endif
