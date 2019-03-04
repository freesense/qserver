
#include "stdafx.h"
#include "config.h"
#include "../../public/commx/code_convert.h"

#pragma warning(disable:4267)

CConfig cfg;

CConfig::CConfig()
{
	m_nMarketCount = 0;
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (NodeName == "����")
	{
		m_tmpKind += (NodeText + "|");
		m_mpStatistics[m_tmpSymbol] = m_tmpKind;
	}
	else if (NodeName == "����")
		m_nMarketCount++;
	else if (NodeName == "Symbol")
		m_nSymbolCount = atoi(NodeText.c_str());
	else if (NodeName == "Min")
		m_nMinkCount = atoi(NodeText.c_str());
	else if (NodeName == "Tick")
		m_nTickCount = atoi(NodeText.c_str());
	else if (NodeName == "��ʱʱ��")
		m_nDevTimeout = atoi(NodeText.c_str());
	else if (NodeName == "ҵ�����")
		m_mpBusinessNo[atoi(NodeText.c_str())] = m_tmpBusinessNoEnabled;
	else if (NodeName == "IP")
		m_pTmpSql->strDbAddress = NodeText;
	else if (NodeName == "DATABASE")
		m_pTmpSql->strDbName = NodeText;
	else if (NodeName == "USER")
		m_pTmpSql->strUser = NodeText;
	else if (NodeName == "PASSWORD")
		m_pTmpSql->strPassword = NodeText;
	else if (NodeName == "SQL")
		m_pTmpSql->vSql.push_back(NodeText);
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
	if (NodeName == "����ͳ��" && AttrName == "code")
	{
		m_tmpSymbol = AttrText;
		m_tmpKind = "";
	}
	else if (NodeName == "ҵ�����" && AttrName == "Enable")
		m_tmpBusinessNoEnabled = (AttrText == "True" ? true : false);
	else if (NodeName == "��ֵ" && AttrName == "״̬")
	{
		m_pTmpSql = &m_marketvalue_sql;
		m_pTmpSql->bEnabled = (AttrText == "����" ? true : false);
	}
	else if (NodeName == "�ɷݹ�" && AttrName == "״̬")
	{
		m_pTmpSql = &m_component_sql;
		m_pTmpSql->bEnabled = (AttrText == "����" ? true : false);
	}
	else if (NodeName == "��ҵ" && AttrName == "״̬")
	{
		m_pTmpSql = &m_vocation_sql;
		m_pTmpSql->bEnabled = (AttrText == "����" ? true : false);
	}
}

const char* CConfig::PrepareStatistics(const char *symbol)
{
	std::map<std::string, std::string>::iterator iter = m_mpStatistics.find(symbol);
	if (iter == m_mpStatistics.end())
		return NULL;
	return iter->second.c_str();
}
