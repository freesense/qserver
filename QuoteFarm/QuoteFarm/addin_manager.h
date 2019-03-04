
#ifndef __QUOTE_FARM_ADDIN_MANAGER_H__
#define __QUOTE_FARM_ADDIN_MANAGER_H__

#include "../../public/commx/xdll.h"
#include "farm.h"
#include "config.h"
#include <vector>

//�������
typedef int(*pfnInitialize)(int iMode, void *pFarm);
//�˳����
typedef void(*pfnRelease)();
//����仯֪ͨ��Ϣ��������ԭ��
typedef void(*pfnSym_Begin)();
typedef void(*pfnOnQuote)(int imode, const char *symbol, int *pFlag, Quote *pq, TickUnit *ptick);
typedef void(*pfnSym_Commit)();
//��ʱ��֪ͨ��Ϣԭ�ͣ����붨ʱ������ʱ��
typedef void(*pfnOnTimer)(short timer);
//���ҵ��������ԭ��
typedef bool(*pfnOnRequest)(unsigned int msgid, CommxHead *pRequest);
//������������仯֪ͨ�ص�����ԭ��
typedef void(*pfnOnQuoteChanged)(const char *symbol, const int *notify);
//��������ҵ������ص�����
typedef void(*pfnOnBusiness)(unsigned int msgid, CommxHead *pHead);
//��������ע�ắ��ԭ��
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
