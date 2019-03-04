/** @file    server_auth.h
 *	@brief   ʵ�ַ�������֤����ӿ�
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

/** @brief   �����������֤����
 *  @ingroup proxy_auth_server
 *  
 */
class CServerAuth : public IServerAuthProxy
{
	struct _UserRights
	{
		char Group[4];		/**< �û������ͣ�0-�����û��飬1-��ʽ�շ���ͨ�û��飬2-��ʽ�շ�Vip�� */
		char Expiration[12];/**< �û���ֹ���� */
		string Market;		/**< �û��г����� */
		string Data;		/**< �û����ݴ��� */
		string Function;	/**< �û����ܴ��� */
	};

public:	//add by dj
	typedef std::map<std::string, _UserRights> Mp_UserRights;			/**< �ض����¼�û�Ȩ���������ͣ��û���-�û�Ȩ�� */
	typedef std::map<std::string, Mp_UserRights> Mp_SvrUser;			/**< �ض����¼�û�Ȩ���������ͣ�������ָ��-�û� */
	LockRW m_lock;					/**< �û�Ȩ�ޱ������ */

public:
	CServerAuth(IServerAuthBase *pSvrAuthBase);
	virtual ~CServerAuth();

	virtual int OnServerLogin(const char *lpFingerPrinter, const char *lpDevList);
	virtual int OnUserLogin(const char *lpUser, const char *lpPwd, const char *lpServerFingerPrinter);
	virtual int OnUserLogout(const char *lpUser, const char *lpServerFingerPrinter);
	virtual int OnTransmit(char *lpData, unsigned int nLength);

	IServerAuthBase *m_pCallback;	/**< �ص����� */

private:
	Mp_SvrUser m_mpServerUser;		/**< �û�Ȩ�ޱ� */

	std::string  m_strFingerPrinter;
	std::string  m_strDevTable;
};

extern CServerAuth *g_pAuth;		/**< ��������֤����ȫ��Ψһ����ָ�� */

#endif
