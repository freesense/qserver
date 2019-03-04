
#ifndef __QUOTEFEED_CONFIG_H__
#define __QUOTEFEED_CONFIG_H__

#include "stdafx.h"
#include "../../public/xml/xmlparser.h"
#include <vector>
#include <string>

class CConfig : public TiXmlParser
{
public:
	CConfig();
	~CConfig();

	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	std::string        m_strDevServIP; //设备服务器IP
	unsigned int       m_nDevServPort; //设备服务器端口
	std::string        m_strAppName;   //AppName

	std::string        m_strQuoteAddress;  //行情服务器IP:端口
	std::string        m_strHostAddress;   //本地监听端口

	//main.cfg
	std::string m_strKjavaVer;
	std::string m_strPpcVer;
	std::string m_strKjavaUpgradeXml;
	std::string m_strPpcUpgradeXml;
	std::string m_strKjavaContent;
	std::string m_strPpcContent;

	std::string m_strOther;

	std::string m_strhkurl;
	std::string m_strhsurl;
	std::string m_strinfourl;
	std::string m_strtmdb;
	std::string m_strtmdbuser;
	std::string m_strtmdbpwd;
	std::string m_strtmtable;
};

extern CConfig g_cfg;

#endif
