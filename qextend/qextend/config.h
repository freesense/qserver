
#ifndef __QUOTE_EXTEND_H__
#define __QUOTE_EXTEND_H__

#include "../../public/xml/xmlparser.h"
#include <vector>
#include <string>
#include <map>

class CConfig : public TiXmlParser
{
	struct _mssql_business
	{
		bool bEnabled;
		std::string strDbAddress;
		std::string strDbName;
		std::string strUser;
		std::string strPassword;
		std::vector<std::string> vSql;
	};

public:
	CConfig();

	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	const char* PrepareStatistics(const char *symbol);

	std::map<std::string, std::string> m_mpStatistics;
	std::map<unsigned short, bool> m_mpBusinessNo;			/// 保存插件支持的业务代码状态，是否启用这个业务代码

	unsigned int m_nSymbolCount;
	unsigned int m_nMinkCount;
	unsigned int m_nTickCount;
	unsigned int m_nDevTimeout;
	unsigned int m_nMarketCount;

	_mssql_business m_marketvalue_sql;
	_mssql_business m_component_sql;
	_mssql_business m_vocation_sql;

private:
	_mssql_business *m_pTmpSql;
	std::string m_tmpSymbol, m_tmpKind;
	bool m_tmpBusinessNoEnabled;
};

extern CConfig cfg;
#endif
