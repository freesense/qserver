// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif
//#pragma comment(lib, "../../public/lib/sqlite3")

#define MN "TRADEMATCH"

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#endif

// ������뽫λ������ָ��ƽ̨֮ǰ��ƽ̨��ΪĿ�꣬���޸����ж��塣
// �йز�ͬƽ̨��Ӧֵ��������Ϣ����ο� MSDN��
#ifndef WINVER				// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define WINVER 0x0501		// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINDOWS		// ����ʹ���ض��� Windows 98 ����߰汾�Ĺ��ܡ�
#define _WIN32_WINDOWS 0x0410 // ����ֵ����Ϊ�ʵ���ֵ����ָ���� Windows Me ����߰汾��ΪĿ�ꡣ
#endif

#ifndef _WIN32_IE			// ����ʹ���ض��� IE 6.0 ����߰汾�Ĺ��ܡ�
#define _WIN32_IE 0x0600	// ����ֵ����Ϊ��Ӧ��ֵ���������� IE �������汾��
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩ CString ���캯��������ʽ��

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE ��
#include <afxodlgs.h>       // MFC OLE �Ի�����
#include <afxdisp.h>        // MFC �Զ�����
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC ���ݿ���
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
//#include <afxdao.h>			// MFC DAO ���ݿ���
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>
#include <afxoledb.h>
#include <atlplus.h>
#import "c:\program files\common files\system\ado\msado15.dll" no_namespace rename("EOF","adoEOF") 

#include "../../public/protocol.h"
#include "../../public/data_struct.h"
#include "config.h"
#include "workthread.h"
#include "DataBaseEx.h"
#include "DataStore.h"
#include "../../public/messagequeue.h"
#include "../../public/commx/synch.h"
#include "../../public/commx/report.h"
#include "../../public/commx/mery.h"
#include "../../public/commx/code_convert.h"
#include "../../public/commx/highperformancecounter.h"

const char gVer[] = "$tradematch.dll#2.1.3$";

extern CWorkThread gWorkThread;
extern CWriteThread gWriteThread;
extern CInfoThread gInfoThread;
extern CDataBaseEx gDBRead;
extern CDataBaseEx gDBWrite;
extern CMessageQueue<CommxHead*> gQueue;
extern CDataStore gDataStore;
extern CDataFarm gFarm;
extern int gnTradeTime;//����ʱ��
extern char gacTradeTime[4];//����ʱ��,������ָ֤���Ľ���ʱ��
extern bool gbIsTradeTime;//�Ƿ��ǽ���ʱ��,��ָ֤��9:30��11:30������13:00��15:00Ϊ����ʱ��.�������гɽ����仯����д��
extern bool gbIsWorkTime;//�Ƿ��ǹ���ʱ��,����3�㵽3���Ϊ�ǹ���ʱ��,�������ύί��
extern CMessageQueue<Consign*> gConsignQueue;//ί�е���,�ȴ�д��ί�гɽ���
extern CMessageQueue<Consign*> gConsignQueueWait;//ί�е���,�ȴ�д��ί�гɽ���
extern bool gbExitFlag;//�˳�
extern SumInfo gSumInfo;//ͳ����Ϣ
extern bool gbWorkDay;//�Ƿ��ǹ�����
extern bool gbTradeTime;//����ʱ��