
#include "stdafx.h"
#include "config.h"

CConfig::CConfig()
{
	m_nMaketCount = 0;
}

void CConfig::OnNode(TiXmlElement *pElm, NODEWAY way)
{
	std::string NodeName = pElm->Value();
	if (NodeName == "ÊÐ³¡")
		m_nMaketCount++;
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
}


