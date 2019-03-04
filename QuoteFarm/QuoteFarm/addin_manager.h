
#ifndef __QUOTE_FARM_ADDIN_MANAGER_H__
#define __QUOTE_FARM_ADDIN_MANAGER_H__

#include "../../public/commx/xdll.h"
#include "farm.h"
#include "config.h"
#include <vector>

//启动插件
typedef int(*pfnInitialize)(int iMode, void *pFarm);
//退出插件
typedef void(*pfnRelease)();
//行情变化通知消息引出函数原型
typedef void(*pfnSym_Begin)();
typedef void(*pfnOnQuote)(int imode, const char *symbol, int *pFlag, Quote *pq, TickUnit *ptick);
typedef void(*pfnSym_Commit)();
//定时器通知消息原型，传入定时器触发时间
typedef void(*pfnOnTimer)(short timer);
//插件业务请求函数原型
typedef bool(*pfnOnRequest)(unsigned int msgid, CommxHead *pRequest);
//插件定义的行情变化通知回调函数原型
typedef void(*pfnOnQuoteChanged)(const char *symbol, const int *notify);
//插件定义的业务请求回调函数
typedef void(*pfnOnBusiness)(unsigned int msgid, CommxHead *pHead);
//插件定义的注册函数原型
typedef void(*pfnRegisterCallback)(pfnOnQuoteChanged funcQuote, pfnOnBusiness funcBusiness);

class CAddinManager
{
	struct _export_func
	{
		pfnSym_Begin _begin;
		pfnOnQuote _onQuote;
		pfnSym_Commit _commit;
	};

public:
	CAddinManager();

	bool LoadAddin(CConfig::_addin &param, char **ppVer);
	void ReleaseAddin();
	void SetFarm(CDataFarm *pFarm);

	std::vector<_export_func> m_vOnQuote;

private:
	std::vector<XDLL*> m_vDll;
	CDataFarm *m_pFarm;
};

extern CAddinManager gAddin;

#endif
