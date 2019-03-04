
#ifndef __QUOTE_FARM_CONFIG_H__
#define __QUOTE_FARM_CONFIG_H__

#include "../../public/xml/xmlparser.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

class CConfig : public TiXmlParser
{
	struct _mkt
	{
		string mkt;
		short tz;
	};

public:
	CConfig();
	~CConfig();

	virtual void OnNode(TiXmlElement *pElm, NODEWAY way);
	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText) {};

	bool kdb_create(const char *mkt);
	void prebuilddb();
	unsigned int GetMarketYear(time_t now, const char *lpMarket);

	string server, database, uid, pwd;
	string m_strPath;
	vector<_mkt> m_vecMarket;

private:
	int makepath(const char *path);
	int GetTz(const char *lpMarket);

	int m_nWeightMode;
	_mkt m_tmpMkt;
};

extern CConfig g_cfg;

#endif
