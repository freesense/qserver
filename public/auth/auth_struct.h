/** @file    auth_struct.h
 *	@brief   定义认证业务数据结构
 *  @ingroup 
 *  
 */
#ifndef __AUTHRIZE_DATA_STRUCT_H__
#define __AUTHRIZE_DATA_STRUCT_H__

#include <map>
#include <vector>
#include <string>
#include "../../public/commx/synch.h"

using std::map;
using std::multimap;
using std::string;

#pragma pack(1)

////////////////////////////////////////////////////////////////////////////////////////////////////////

/// 枚举认证数据类型
enum DataType {	VERSION = 'V',		/// 版本
				SERVER  = 'S',		/// 服务器
				MARKET  = 'M',		/// 大市场
				FUNC	= 'F',		/// 功能包
				SubMarket= 'S',		/// 子市场
				HQDATA	= 'Q',		/// 行情数据种类
				USER	= 'U'};		/// 用户信息

struct _subclass
{
	int subMarket;
	int subHqdata;
};

inline bool operator==(const _subclass &s1, const _subclass &s2)
{
	return (s1.subHqdata == s2.subHqdata) && (s1.subMarket == s2.subMarket);
}

struct VersionData
{
	DataType flag;						/**< 数据类型标志，在这里是'V' */
	struct Version
	{
		char LatestVersion[8];			/**< 最新版本号 */
		char UpgradeType[44];			/**< 升级类型，0-暂不升级，1-可选升级，2-强制升级 */
		char UpgradeContent[504];		/**< 升级内容 */
	};

	typedef map<string, Version> MapVersion;	/**< 定义版本信息存放类型 */
	MapVersion m_mpVersion;				/**< 从数据库中取得的版本信息存放在此 */
	LockSingle m_lock;					/**< 版本信息访问锁 */

	void Update(const MapVersion &mv)
	{
		m_lock.lock();
		m_mpVersion = mv;
		m_lock.unlock();
	}
};

struct ServerData
{
	DataType flag;						/**< 数据类型标志，在这里是'S' */
	struct Server
	{
		UINT ServerLevel;				/**< 服务器级别，与用户的组级别对应：0-试用用户组，1-正式收费普通用户组，2-正式收费Vip组 */
		char ServerIp[24];				/**< 服务器IP */
		int ServerPort;					/**< 服务器端口 */
		unsigned long OnlineUser;		/**< 在线人数 */
		unsigned long MaxConnect;		/**< 最大连接数 */
		char ServerStatus[4];			/**< 服务器状态，0-未启动，1-正常运行，2-暂停 */
		long lparams;					/**< 参数，保留给界面使用(HTREEITEM) */
		char serviceName[32];           /**< 参数,保留服务器名称*/
		std::vector<_subclass> SubData;	/**< 服务器支持的数据子类别 */
	};

	typedef map<string, Server> MapServer;	/**< 定义服务器信息存放类型 */
	MapServer m_mpServer;				/**< 从数据库中取得的服务器信息存放在此 */
	LockSingle m_lock;					/**< 服务器信息访问锁 */

	void Update(MapServer &ms)
	{
		m_lock.lock();
		m_mpServer = ms;
		m_lock.unlock();
	}
};

struct UserData
{
	DataType flag;						/**< 数据类型标志，在这里是'U' */
	struct User
	{
		char UserPassword[8];			/**< 账号密码 */
		char UserStatus[4];				/**< 账号状态，0-开通，1-暂停，2-停止 */
		UINT UserLevel;					/**< 0-试用用户组，1-正式收费普通用户组，2-正式收费Vip组 */
		UINT UserOnline;				/**< 是否在线，0-不在线，>0-在线 */
		UINT Expiration;				/**< 截止日期 */
		UINT SuiteID;					/**< 用户套餐ID */
		std::vector<_subclass> SubData;	/**< 用户支持的数据子类别 */
	};

	typedef map<string, User> MapUser;	/**< 定义用户信息存放类型 */
	MapUser m_mpUser;					/**< 从数据库中取得的用户信息存放在此 */
	LockRW m_lock;						/**< 用户信息访问锁 */

	void Update(const char *lpUserName, const User &data)
	{
		m_lock.WaitWrite();
		m_mpUser[lpUserName] = data;
		m_lock.EndWrite();
	}
};

struct MarketData 
{
	DataType flag;						/**< 数据类型标志，在这里是'M' */
	struct Market
	{
		char Code[8];
		char CnName[44];
		char EnName[44];
	};

	typedef map<UINT, Market> MapMarket;/**< 定义版本信息存放类型 */
	MapMarket m_mpMarket;				/**< 从数据库中取得的版本信息存放在此 */
	LockSingle m_lock;					/**< 市场信息访问锁 */
	
	void Update(const MapMarket &mv)
	{
		m_lock.lock();
		m_mpMarket = mv;
		m_lock.unlock();
	}
};

struct FuncData 
{
	DataType flag;						/**< 数据类型标志，在这里是'F' */
	struct Func
	{
		char Code[8];
		char Name[44];
	};
	
	typedef multimap<UINT, Func> MapFunc;	/**< 定义版本信息存放类型 */
	MapFunc m_mpFunc;					/**< 从数据库中取得的版本信息存放在此 */
	LockSingle m_lock;					/**< 市场信息访问锁 */
	
	void Update(const MapFunc &mv)
	{
		m_lock.lock();
		m_mpFunc = mv;
		m_lock.unlock();
	}
};

struct SubMarketData 
{
	DataType flag;						/**< 数据类型标志，在这里是'S' */
	struct SubMkt
	{
		int nMktCode;					/**< 大市场代码 */
		char Name[44];					/**< 子市场名称 */
	};
	typedef map<UINT, SubMkt> MapSubMarket;	/**< 定义版本信息存放类型 */
	MapSubMarket m_mpSubMarket;			/**< 从数据库中取得的版本信息存放在此 */
	LockSingle m_lock;					/**< 市场信息访问锁 */
	
	void Update(const MapSubMarket &mv)
	{
		m_lock.lock();
		m_mpSubMarket = mv;
		m_lock.unlock();
	}
};

struct HqData 
{
	DataType flag;						/**< 数据类型标志，在这里是'Q' */
	typedef map<UINT, std::string> MapHq;	/**< 定义版本信息存放类型 */
	MapHq m_mpHq;						/**< 从数据库中取得的版本信息存放在此 */
	LockSingle m_lock;					/**< 市场信息访问锁 */
	
	void Update(const MapHq &mv)
	{
		m_lock.lock();
		m_mpHq = mv;
		m_lock.unlock();
	}
};

#pragma pack()

#endif
