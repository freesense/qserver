
#ifndef __WATCH_DOG_CONFIG_H__
#define __WATCH_DOG_CONFIG_H__

#include <string>
#include "../../public/commx/synch.h"
#include "../../public/xml/xmlparser.h"

class CConfig : public TiXmlParser
{
public:
	CConfig();
	virtual void OnNode(TiXmlElement *pElm, NODEWAY way);
	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	unsigned int m_nMaketCount;
};

#endif
