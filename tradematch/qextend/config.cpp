
#include "stdafx.h"
#include "config.h"
#include "../../public/commx/code_convert.h"

#pragma warning(disable:4267)

CConfig cfg;

void CConfig::OnNode(TiXmlElement *pElm, NODEWAY way)
{
	std::string NodeName = pElm->Value();
	if (NodeName == "�г�")
		m_nMarketCount++;
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (NodeName == "����ʱ��")
	{
		char *p;
		char buf[256];
		memset(buf,0,sizeof(buf));
		strcpy(buf,NodeText.c_str());
		p = strstr(buf,"-");
		*p = 0;
		cfg.m_nStopSvcBegin = atoi(buf);
		cfg.m_nStopSvcEnd = atoi(p+1);		
	}
	else
	if (NodeName == "������")
	{
		m_strServerIP = NodeText;
	}
	else
	if (NodeName == "���ݿ�")
	{
		m_strDataBase = NodeText;
	}
	else
	if (NodeName == "�û�")
	{
		m_strUser = NodeText;
	}
	else
	if (NodeName == "����")
	{
		m_strPass = NodeText;
	}
	else
	if (NodeName == "Ʒ��")
	{
		m_tmpPattern.RegexPattern = NodeText;
		m_vPattern.push_back(m_tmpPattern);
	}
	else if (NodeName == "����")
	{
		char szGroup[256];
		const char *fmt = "group_name = '%s'";
		sprintf(szGroup, fmt, NodeText.c_str());
		if (m_tmpKind.length() == 0)
			m_tmpKind = "where ";
		else
			m_tmpKind += " or ";
		m_tmpKind += szGroup;
		m_mpStatistics[m_tmpSymbol] = m_tmpKind;
	}
	else if (NodeName == "Symbol")
		m_nSymbolCount = atoi(NodeText.c_str());
	else if (NodeName == "Min")
		m_nMinkCount = atoi(NodeText.c_str());
	else if (NodeName == "Tick")
		m_nTickCount = atoi(NodeText.c_str());
	else if (NodeName == "��ʱʱ��")
		m_nDevTimeout = atoi(NodeText.c_str());
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
	if (NodeName == "Ʒ��" && AttrName == "code")
	{
		m_tmpPattern.PatternCode = AttrText;
	}
	else if (NodeName == "����ͳ��" && AttrName == "code")
	{
		m_tmpSymbol = AttrText;
		m_tmpKind = "";
	}
}

const char* CConfig::PrepareStatistics(const char *symbol)
{
	std::map<std::string, std::string>::iterator iter = m_mpStatistics.find(symbol);
	if (iter == m_mpStatistics.end())
		return NULL;
	return iter->second.c_str();
}
