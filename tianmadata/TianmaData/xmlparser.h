
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
	int          m_nUpdateTime;       //�����ݿ��������ʱ�� ��ʽ��HHMM

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
		if (NodeName == "������IP")	
			m_strSerIP = NodeText;
		else if (NodeName == "���ݿ���")
			m_strDBName = NodeText;
		else if (NodeName == "�û���")
			m_strUserName = NodeText;
		else if (NodeName == "����")
			m_strPassword = NodeText;
		else if (NodeName == "��������ʱ��")	
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
