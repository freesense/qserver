
#include "stdafx.h"
#include "config.h"
#include "addin_manager.h"
#include "../../public/commx/report.h"

#pragma warning(disable:4311 4312)

CAddinManager gAddin;

//////////////////////////////////////////////////////////////////////////////
CAddinManager::CAddinManager()
{}

void CAddinManager::SetFarm(CDataFarm *pFarm)
{
	m_pFarm = pFarm;
}

void CAddinManager::ReleaseAddin()
{
	for (unsigned int i = 0; i < m_vDll.size(); i++)
	{
		XDLL *pDll = m_vDll[i];
		pfnRelease pfn = (pfnRelease)pDll->Symbol("Release");
		CHECK_RUN(!pfn, MN, T("缺少Release接口\n"), RPT_ERROR, continue);
		pfn();
		delete pDll;
	}
}

bool CAddinManager::LoadAddin(CConfig::_addin &param, char **ppVer)
{
	XDLL *pAddin = new XDLL(param.strPath.c_str());
	CHECK_RUN(!pAddin, MN, T("初始化扩展插件失败\n"), RPT_ERROR, return false);
	m_vDll.push_back(pAddin);

	if (param.bQuoteNotify)
	{
		pfnOnQuote quote = (pfnOnQuote)pAddin->Symbol("OnQuote");
		pfnSym_Begin begin = (pfnSym_Begin)pAddin->Symbol("Sym_Begin");
		pfnSym_Commit commit = (pfnSym_Commit)pAddin->Symbol("Sym_Commit");
		if (quote && begin && commit)
		{
			_export_func ef;
			ef._begin = begin;
			ef._onQuote = quote;
			ef._commit = commit;
			m_vOnQuote.push_back(ef);
		}
		else
			REPORT(MN, T("插件接口不全\n"), RPT_ERROR);
	}
	if (param.mpTimer.size())
	{
		pfnOnTimer pfn = (pfnOnTimer)pAddin->Symbol("OnTimer");
		if (pfn)
			param.pfnTimer = reinterpret_cast<unsigned int>(pfn);
		else
			REPORT(MN, T("插件无OnTimer接口\n"), RPT_ERROR);
	}

	pfnInitialize pfnInit = (pfnInitialize)pAddin->Symbol("Initialize");
	CHECK_RUN(!pfnInit, MN, T("插件无Initialize接口\n"), RPT_ERROR, return false);
	pfnInit(0, m_pFarm);

	pfnGetModuleVer pfnVer = (pfnGetModuleVer)pAddin->Symbol("GetModuleVer");
	CHECK_RUN(!pfnVer, MN, T("插件无GetModuleVer接口\n"), RPT_ERROR, return false);
	pfnVer(ppVer);
	return true;
}
