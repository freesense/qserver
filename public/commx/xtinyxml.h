
#ifndef __TINYXML_WRAPPER_H__
#define __TINYXML_WRAPPER_H__

#include "commxapi.h"
#include "tinyxml.h"

#define ROOTELEMENT (TiXmlElement*)(-1)

class COMMXAPI XTinyXmlWrapper
{
public:
	XTinyXmlWrapper();

	bool Load(const char *lpFileName);
	bool Read(const char *lpXmlString);

	const char* GetEncoding();

	// 表达式定义: Tag/Tag/Tag?Attr=*&Attr=*/Tag...
	void FindTag(char *exp, TiXmlElement &rs, TiXmlElement *pStartElm = ROOTELEMENT);
	void SetTag(char *exp, const char *text, TiXmlElement *pStartElm = ROOTELEMENT);

	TiXmlPrinter PrintNode(TiXmlElement *pElm);

	void Save();

protected:
	int CheckNode(TiXmlElement *pElm, const char *tag, char **ppAttrName = NULL);

	TiXmlDocument m_doc;
	TiXmlElement *m_pRootElement;
};

#endif
