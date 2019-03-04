
#ifndef __QUOTE_EXTEND_H__
#define __QUOTE_EXTEND_H__

#include "../../public/xml/xmlparser.h"
#include <vector>
#include <string>
#include <map>

class CConfig : public TiXmlParser
{
public:
	CConfig();

	virtual void OnNode(TiXmlElement *pElm, NODEWAY way);
	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	std::string m_strDbAddress;				/** 数据库ip [6/30/2009 xinl] */
	std::string m_strDbName;				/** 数据库名称 [6/30/2009 xinl] */
	std::string m_strUser;					/** 数据库用户 [6/30/2009 xinl] */
	std::string m_strPassword;				/** 访问密码 [6/30/2009 xinl] */
	std::vector<std::string> m_vSql;		/** 业务sql代码 [6/30/2009 xinl] */

private:
	bool m_bValid;
};

extern CConfig cfg;
#endif
