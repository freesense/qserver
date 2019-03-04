
#ifndef __QUOTE_EXTEND_H__
#define __QUOTE_EXTEND_H__

#include "../../public/xml/xmlparser.h"
#include <vector>
#include <string>
#include <map>

class CConfig : public TiXmlParser
{
	struct _pattern
	{
		std::string PatternCode;		/*分类代码*/
		std::string RegexPattern;		/*分类的正则表达式*/
	};
	
public:
	CConfig(){m_nMarketCount=0;};

	virtual void OnNode(TiXmlElement *pElm, NODEWAY way);
	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	const char* PrepareStatistics(const char *symbol);

	std::vector<_pattern> m_vPattern;
	std::map<std::string, std::string> m_mpStatistics;

	unsigned int m_nSymbolCount;
	unsigned int m_nMinkCount;
	unsigned int m_nTickCount;
	unsigned int m_nDevTimeout;
	unsigned int m_nMarketCount;

	std::string m_strServerIP;
	std::string m_strDataBase;
	std::string m_strUser;
	std::string m_strPass;

	int m_nStopSvcBegin;//结束服务开始时间
	int m_nStopSvcEnd;//结束服务结束时间
private:
	_pattern m_tmpPattern;
	std::string m_tmpSymbol, m_tmpKind;
};

extern CConfig cfg;
#endif
