
#include "stdafx.h"
#include "config.h"

CConfig cfg;

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	WGUARD(LockRW, m_lock, grd);
	if (NodeName == "控制中心地址")
		m_strCCAddress = NodeText;
	else if (NodeName == "控制中心端口")
		m_usCCPort = atoi(NodeText.c_str());
	else if (NodeName == "服务地址")
		m_strServiceAddress = NodeText;
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
}
