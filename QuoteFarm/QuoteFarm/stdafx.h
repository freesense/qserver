// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once
#pragma warning(disable:4311 4312)

#define MN "Farm"

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif
#pragma comment(lib, "../../public/lib/kvisitor")

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#include <stdio.h>
#include <set>
#include <tchar.h>
#include <Windows.h>
#include "../../public/commx/synch.h"
#include "../../public/commx/report.h"
#include "../../public/commx/commxapi.h"
#include "../../public/commx/encryption.h"
#include "../../public/data_struct.h"
#include "../../public/kvisitor.h"
#include "communication.h"

const char gVer[] = "$Quotefarm.exe#2.0.62$";

extern HANDLE gQuit;
typedef std::map<std::string, std::vector<RINDEX*> > MAPSPECIAL;
typedef std::map<std::string, std::vector<RINDEX*> >::iterator SPECIALITER;
extern MAPSPECIAL g_mapCloseSpecial;

//������ͳ������
extern atomint g_nFeedRecvLength;		/** ��Feed�յ������ݳ���(ÿ����ʱ����) [5/22/2009 xinl] */
extern atomint g_nTotalRecv;			/** ��Feed�յ������ݳ���(�ܳ���) [5/22/2009 xinl] */
extern atomint g_nFeedPacketCount;		/** ��Feed�յ������ݰ����� [5/22/2009 xinl] */

extern void OnAnswer(unsigned int msgid, CommxHead *pHead);	//���qextend������ص�����
extern void* ConnectFeed(const char *lpFeedName, const char *lpFeedAddress, CFeedSocket *pSocket = NULL);
extern int _internal_main();

//�汾����
typedef void(*pfnGetModuleVer)(char **lpVer);

struct _find_close_special
{
	int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);
	std::string symbol;
};
