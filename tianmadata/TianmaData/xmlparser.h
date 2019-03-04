
#ifndef __TIXML_PARSER_H__
#define __TIXML_PARSER_H__

#include "..\..\public\xml\tinyxml.h"

class TiXmlParser : public TiXmlDocument
{
public:
	void Read(const char *lpXmlText)
	{
		Parse(lpXmlText);
		TiXmlElement *pRootElm = RootElement();
		ParseNode(pRootElm, NULL);
	}

	void Load(const char *lpFile)
	{
		LoadFile(lpFile);
		TiXmlElement *pRootElm = RootElement();
		ParseNode(pRootElm, NULL);
	}

	std::string  m_strSerIP;
	std::string  m_strDBName;
	std::string  m_strUserName;
	std::string  m_strPassword;
	int          m_nUpdateTime;       //从数据库更新数据时间 格式：HHMM

	//virtual void* OnNode(std::string NodeName, void *lParam);
	//virtual void OnText(std::string NodeName, std::string NodeText, void *lParam);
	//virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText, void *lParam);

private:
	void* OnNode(std::string NodeName, void *lParam)
	{
		return lParam;
	}
	void OnText(std::string NodeName, std::string NodeText, void *lParam)
	{
		if (NodeName == "服务器IP")	
			m_strSerIP = NodeText;
		else if (NodeName == "数据库名")
			m_strDBName = NodeText;
		else if (NodeName == "用户名")
			m_strUserName = NodeText;
		else if (NodeName == "密码")
			m_strPassword = NodeText;
		else if (NodeName == "更新数据时间")	
			m_nUpdateTime = atoi(NodeText.c_str());
	}
	void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText, void *lParam)
	{
	}
	void ParseNode(TiXmlElement *pElm, void *lParam)
	{
		if (!pElm)
			return;

		TiXmlAttribute *pAttr = pElm->FirstAttribute();
		while (pAttr)
		{
			OnAttr(pElm->Value(), pAttr->Name(), pAttr->Value(), lParam);
			pAttr = pAttr->Next();
		}

		void *lpSubParam = NULL;
		TiXmlElement *pNextElm = NULL;
		lpSubParam = OnNode(pElm->Value(), lParam);
		if (pElm->GetText())
			OnText(pElm->Value(), pElm->GetText(), lParam);

		pNextElm = pElm->FirstChildElement();
		ParseNode(pNextElm, lpSubParam);
		pNextElm = pElm->NextSiblingElement();
		ParseNode(pNextElm, lParam);
	}
};

#endif
