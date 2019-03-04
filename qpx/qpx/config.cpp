
#include "stdafx.h"
#include "config.h"

CConfig g_cfg;

CConfig::CConfig()
{
}

CConfig::~CConfig()
{
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (NodeName == "�豸������IP")
		m_strDevServIP = NodeText;
	else if (NodeName == "�豸�������˿�")
		m_nDevServPort = atoi(NodeText.c_str());
	else if (NodeName == "AppName")
		m_strAppName = NodeText;

	else if (NodeName == "�����������ַ")
		m_strQuoteAddress = NodeText;
	else if (NodeName == "Э��ת���˿�")
		m_strHostAddress = NodeText;

	else if (NodeName == "kjava")
		m_strKjavaVer = NodeText;
	else if (NodeName == "ppc")
		m_strPpcVer = NodeText;
	else if (NodeName == "kjavaupgradeurl")
		m_strKjavaUpgradeXml = NodeText;
	else if (NodeName == "ppcupgradeurl")
		m_strPpcUpgradeXml = NodeText;
	else if (NodeName == "kjavaupgradecontent")
		m_strKjavaContent = NodeText;
	else if (NodeName == "ppcupgradecontent")
		m_strPpcContent = NodeText;

	else if (NodeName == "hkurl")
		m_strhkurl = NodeText;
	else if (NodeName == "hsurl")
		m_strhsurl = NodeText;
	else if (NodeName == "infourl")
		m_strinfourl = NodeText;
	else if (NodeName == "tmdb")
		m_strtmdb = NodeText;
	else if (NodeName == "tmdbuser")
		m_strtmdbuser = NodeText;
	else if (NodeName == "tmdbpwd")
		m_strtmdbpwd = NodeText;
	else if (NodeName == "tmtable")
		m_strtmtable = NodeText;
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
}
