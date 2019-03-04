
#ifndef __TIXML_PARSER_H__
#define __TIXML_PARSER_H__

#include "tinyxml.h"

class TiXmlParser : public TiXmlDocument
{
protected:
	enum NODEWAY {ENTER, LEAVE};

public:
	void Read(const char *lpXmlText)
	{
		Parse(lpXmlText);
		TiXmlElement *pRootElm = RootElement();
		ParseNode(pRootElm);
	}

	void Load(const char *lpFile)
	{
		LoadFile(lpFile);
		TiXmlElement *pRootElm = RootElement();
		ParseNode(pRootElm);
	}

	virtual void OnNode(TiXmlElement *pElm, NODEWAY way) {};
	virtual void OnText(std::string NodeName, std::string NodeText) = 0;
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText) = 0;

private:
	void ParseNode(TiXmlElement *pElm)
	{
		if (!pElm)
			return;

		OnNode(pElm, ENTER);

		TiXmlAttribute *pAttr = pElm->FirstAttribute();
		while (pAttr)
		{
			OnAttr(pElm->Value(), pAttr->Name(), pAttr->Value());
			pAttr = pAttr->Next();
		}

		if (pElm->GetText())
			OnText(pElm->Value(), pElm->GetText());

		TiXmlElement *pNextElm = pElm->FirstChildElement();
		ParseNode(pNextElm);

		OnNode(pElm, LEAVE);

		pNextElm = pElm->NextSiblingElement();
		ParseNode(pNextElm);
	}
};

#endif
