
#include "stdafx.h"
#include "config.h"

CConfig cfg;

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	WGUARD(LockRW, m_lock, grd);
	if (NodeName == "�������ĵ�ַ")
		m_strCCAddress = NodeText;
	else if (NodeName == "�������Ķ˿�")
		m_usCCPort = atoi(NodeText.c_str());
	else if (NodeName == "�����ַ")
		m_strServiceAddress = NodeText;
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
}
