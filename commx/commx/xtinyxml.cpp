
#include "stdafx.h"
#include "../public/xtinyxml.h"
#include "../public/commxapi.h"
#include "../public/report.h"
#include <string>

#pragma warning(disable: 4996)

XTinyXmlWrapper::XTinyXmlWrapper()
{
	m_pRootElement = NULL;
}

void XTinyXmlWrapper::Save()
{
	m_doc.SaveFile();
}

bool XTinyXmlWrapper::Load(const char *lpFileName)
{
	if (!m_doc.LoadFile(lpFileName))
		return false;
	m_pRootElement = m_doc.RootElement();
	return true;
}

bool XTinyXmlWrapper::Read(const char *lpXmlString)
{
	if (!m_doc.Parse(lpXmlString))
		return false;
	m_pRootElement = m_doc.RootElement();
	return true;
}

const char* XTinyXmlWrapper::GetEncoding()
{
	TiXmlDeclaration *pDec = m_doc.FirstChild()->ToDeclaration();
	return pDec->Encoding();
}

int XTinyXmlWrapper::CheckNode(TiXmlElement *pElm, const char *tag, char **ppAttr)
{
	if (!pElm)		// �ڵ�Ϊ��
		return 1;

	// ��������
	std::string strtag = tag;
	char *lpAttr = (char*)strchr(strtag.c_str(), '?'), *attr = NULL;
	if (lpAttr)
	{
		lpAttr[0] = 0x00;
		lpAttr += sizeof(char);
		CHECK_RUN(lpAttr >= strtag.c_str() + strtag.length(), MN, T("xml���ʽ����%s\n", tag), RPT_ERROR, return -1);
	}

	if (stricmp(pElm->Value(), strtag.c_str()))		// tagname��ƥ��
		return 1;

	if (lpAttr)
	{// �������ֵ
		attr = strtok(lpAttr, "&");
		while (attr)
		{
			char *value = strchr(attr, '=');
			CHECK_RUN(!value, MN, T("xml���ʽ����%s\n", tag), RPT_ERROR, return -1);

			value[0] = 0x00;
			value += sizeof(char);

			if (value >= strtag.c_str()+strtag.length())
			{
				CHECK_RUN(!ppAttr, MN, T("xml���ʽ����%s\n", tag), RPT_ERROR, return -1);
				CHECK_RUN(*ppAttr, MN, T("xml���ʽ����%s\n", tag), RPT_ERROR, return -1);
				*ppAttr = attr;
			}
			else
			{
				attr = (char*)Trim(attr);
				value = (char*)Trim(value);
				if (stricmp(value, pElm->Attribute(attr)))		//�������ֵ�Ƿ�ƥ��
					return 1;
			}
			attr = strtok(NULL, "&");
		}
	}

	if (ppAttr && *ppAttr)
	{
		char *pAttrName = (char*)tag + (*ppAttr - strtag.c_str());
		pAttrName[strlen(*ppAttr)] = 0x00;
		*ppAttr = pAttrName;
	}
	return 0;
}

void XTinyXmlWrapper::FindTag(char *exp, TiXmlElement &rs, TiXmlElement *pStartElm)
{
	if (pStartElm == ROOTELEMENT)
		pStartElm = m_pRootElement;
	if (!pStartElm)
		return;

	std::string strexp = exp;
	char *tag = (char*)strexp.c_str();
	exp = strchr(tag, '/');
	if (exp)
	{
		exp[0] = 0x00;
		exp += sizeof(char);
	}

	while (pStartElm && tag)
	{
		int iCheck = CheckNode(pStartElm, tag);
		if (-1 == iCheck)
			return;
		if (0 == iCheck)
		{
			if (exp)
				FindTag(exp, rs, pStartElm->FirstChildElement());
			else	// �Ѿ��ѱ��ʽȫ���������ˣ��Ƿ��ϱ��ʽ�Ľڵ㣬���Ҫ����������
				rs.InsertEndChild(*pStartElm);
		}

		pStartElm = pStartElm->NextSiblingElement();
	}
}

void XTinyXmlWrapper::SetTag(char *exp, const char *text, TiXmlElement *pStartElm)
{
	if (pStartElm == ROOTELEMENT)
		pStartElm = m_pRootElement;
	if (!pStartElm)
		return;

	std::string strexp = exp;
	char *tag = (char*)strexp.c_str();
	exp = strchr(tag, '/');
	if (exp)
	{
		exp[0] = 0x00;
		exp += sizeof(char);
	}

	char *pAttrName = NULL;
	while (pStartElm && tag)
	{
		int iCheck = CheckNode(pStartElm, tag, &pAttrName);
		if (-1 == iCheck)
			return;
		if (0 == iCheck)
		{
			if (exp)
			{
				CHECK_RUN(pAttrName, MN, T("xml���ʽ����:%s\n", exp), RPT_ERROR, return);
				SetTag(exp, text, pStartElm->FirstChildElement());
			}
			else
			{// �Ѿ��ѱ��ʽȫ���������ˣ��Ƿ��ϱ��ʽ�Ľڵ㣬�޸Ľڵ�����
				if (pAttrName)
					pStartElm->SetAttribute(pAttrName, text);
				else
				{
					TiXmlNode* pTextNode = pStartElm->FirstChild();
					CHECK_RUN(pTextNode->Type() != TiXmlNode::TINYXML_TEXT, MN,
						T("Ŀ��ڵ��TEXT�ڵ�:\n%s", PrintNode(pStartElm).CStr()), RPT_ERROR, {return;});
					pTextNode->SetValue(text);
				}
			}
		}

		pStartElm = pStartElm->NextSiblingElement();
	}
}

TiXmlPrinter XTinyXmlWrapper::PrintNode(TiXmlElement *pElm)
{
	TiXmlDocument doc;
	doc.InsertEndChild(TiXmlDeclaration("1.0", GetEncoding(), "yes"));
	doc.InsertEndChild(*pElm);
	TiXmlPrinter printer;
	doc.Accept(&printer);
	return printer;
}
