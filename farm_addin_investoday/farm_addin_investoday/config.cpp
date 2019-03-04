
#include "stdafx.h"
#include "config.h"
#include "../../public/commx/code_convert.h"

#pragma warning(disable:4267)

CConfig cfg;

CConfig::CConfig()
{
	m_bValid = false;
}

void CConfig::OnNode(TiXmlElement *pElm, NODEWAY way)
{
	TiXmlAttribute *pAttr = pElm->FirstAttribute();
	if (pAttr && !strcmp(pAttr->Name(), "Package") && !stricmp(pAttr->Value(), "afit.dll"))
	{
		if (way == ENTER)
			m_bValid = true;
		else
			m_bValid = false;
	}
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (!m_bValid)
		return;

	if (NodeName == "���ݿ�")
		m_strDbName = NodeText;
	else if (NodeName == "�û�")
		m_strUser = NodeText;
	else if (NodeName == "����")
		m_strPassword = NodeText;
	else if (NodeName == "SQL")
		m_vSql.push_back(NodeText);
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
	if (m_bValid && NodeName == "MSSQL" && AttrName == "��ַ")
		m_strDbAddress = AttrText;
}
