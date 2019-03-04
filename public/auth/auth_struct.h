/** @file    auth_struct.h
 *	@brief   ������֤ҵ�����ݽṹ
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

/// ö����֤��������
enum DataType {	VERSION = 'V',		/// �汾
				SERVER  = 'S',		/// ������
				MARKET  = 'M',		/// ���г�
				FUNC	= 'F',		/// ���ܰ�
				SubMarket= 'S',		/// ���г�
				HQDATA	= 'Q',		/// ������������
				USER	= 'U'};		/// �û���Ϣ

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
	DataType flag;						/**< �������ͱ�־����������'V' */
	struct Version
	{
		char LatestVersion[8];			/**< ���°汾�� */
		char UpgradeType[44];			/**< �������ͣ�0-�ݲ�������1-��ѡ������2-ǿ������ */
		char UpgradeContent[504];		/**< �������� */
	};

	typedef map<string, Version> MapVersion;	/**< ����汾��Ϣ������� */
	MapVersion m_mpVersion;				/**< �����ݿ���ȡ�õİ汾��Ϣ����ڴ� */
	LockSingle m_lock;					/**< �汾��Ϣ������ */

	void Update(const MapVersion &mv)
	{
		m_lock.lock();
		m_mpVersion = mv;
		m_lock.unlock();
	}
};

struct ServerData
{
	DataType flag;						/**< �������ͱ�־����������'S' */
	struct Server
	{
		UINT ServerLevel;				/**< �������������û����鼶���Ӧ��0-�����û��飬1-��ʽ�շ���ͨ�û��飬2-��ʽ�շ�Vip�� */
		char ServerIp[24];				/**< ������IP */
		int ServerPort;					/**< �������˿� */
		unsigned long OnlineUser;		/**< �������� */
		unsigned long MaxConnect;		/**< ��������� */
		char ServerStatus[4];			/**< ������״̬��0-δ������1-�������У�2-��ͣ */
		long lparams;					/**< ����������������ʹ��(HTREEITEM) */
		char serviceName[32];           /**< ����,��������������*/
		std::vector<_subclass> SubData;	/**< ������֧�ֵ���������� */
	};

	typedef map<string, Server> MapServer;	/**< �����������Ϣ������� */
	MapServer m_mpServer;				/**< �����ݿ���ȡ�õķ�������Ϣ����ڴ� */
	LockSingle m_lock;					/**< ��������Ϣ������ */

	void Update(MapServer &ms)
	{
		m_lock.lock();
		m_mpServer = ms;
		m_lock.unlock();
	}
};

struct UserData
{
	DataType flag;						/**< �������ͱ�־����������'U' */
	struct User
	{
		char UserPassword[8];			/**< �˺����� */
		char UserStatus[4];				/**< �˺�״̬��0-��ͨ��1-��ͣ��2-ֹͣ */
		UINT UserLevel;					/**< 0-�����û��飬1-��ʽ�շ���ͨ�û��飬2-��ʽ�շ�Vip�� */
		UINT UserOnline;				/**< �Ƿ����ߣ�0-�����ߣ�>0-���� */
		UINT Expiration;				/**< ��ֹ���� */
		UINT SuiteID;					/**< �û��ײ�ID */
		std::vector<_subclass> SubData;	/**< �û�֧�ֵ���������� */
	};

	typedef map<string, User> MapUser;	/**< �����û���Ϣ������� */
	MapUser m_mpUser;					/**< �����ݿ���ȡ�õ��û���Ϣ����ڴ� */
	LockRW m_lock;						/**< �û���Ϣ������ */

	void Update(const char *lpUserName, const User &data)
	{
		m_lock.WaitWrite();
		m_mpUser[lpUserName] = data;
		m_lock.EndWrite();
	}
};

struct MarketData 
{
	DataType flag;						/**< �������ͱ�־����������'M' */
	struct Market
	{
		char Code[8];
		char CnName[44];
		char EnName[44];
	};

	typedef map<UINT, Market> MapMarket;/**< ����汾��Ϣ������� */
	MapMarket m_mpMarket;				/**< �����ݿ���ȡ�õİ汾��Ϣ����ڴ� */
	LockSingle m_lock;					/**< �г���Ϣ������ */
	
	void Update(const MapMarket &mv)
	{
		m_lock.lock();
		m_mpMarket = mv;
		m_lock.unlock();
	}
};

struct FuncData 
{
	DataType flag;						/**< �������ͱ�־����������'F' */
	struct Func
	{
		char Code[8];
		char Name[44];
	};
	
	typedef multimap<UINT, Func> MapFunc;	/**< ����汾��Ϣ������� */
	MapFunc m_mpFunc;					/**< �����ݿ���ȡ�õİ汾��Ϣ����ڴ� */
	LockSingle m_lock;					/**< �г���Ϣ������ */
	
	void Update(const MapFunc &mv)
	{
		m_lock.lock();
		m_mpFunc = mv;
		m_lock.unlock();
	}
};

struct SubMarketData 
{
	DataType flag;						/**< �������ͱ�־����������'S' */
	struct SubMkt
	{
		int nMktCode;					/**< ���г����� */
		char Name[44];					/**< ���г����� */
	};
	typedef map<UINT, SubMkt> MapSubMarket;	/**< ����汾��Ϣ������� */
	MapSubMarket m_mpSubMarket;			/**< �����ݿ���ȡ�õİ汾��Ϣ����ڴ� */
	LockSingle m_lock;					/**< �г���Ϣ������ */
	
	void Update(const MapSubMarket &mv)
	{
		m_lock.lock();
		m_mpSubMarket = mv;
		m_lock.unlock();
	}
};

struct HqData 
{
	DataType flag;						/**< �������ͱ�־����������'Q' */
	typedef map<UINT, std::string> MapHq;	/**< ����汾��Ϣ������� */
	MapHq m_mpHq;						/**< �����ݿ���ȡ�õİ汾��Ϣ����ڴ� */
	LockSingle m_lock;					/**< �г���Ϣ������ */
	
	void Update(const MapHq &mv)
	{
		m_lock.lock();
		m_mpHq = mv;
		m_lock.unlock();
	}
};

#pragma pack()

#endif
