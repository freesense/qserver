
#ifndef __WATCH_DOG_CONFIG_H__
#define __WATCH_DOG_CONFIG_H__

#include <string>
#include "../../public/commx/synch.h"
#include "../../public/xml/xmlparser.h"

class CConfig : public TiXmlParser
{
public:
	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	std::string m_strServiceAddress;
	std::string m_strCCAddress;
	unsigned short m_usCCPort;

private:
	LockRW m_lock;
};

extern CConfig cfg;

#endif
