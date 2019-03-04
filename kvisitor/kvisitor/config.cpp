
#include "stdafx.h"
#include "config.h"
#include <direct.h>
#include "../../public/sqlite/sqlite3.h"

CConfig g_cfg;

CConfig::CConfig()
	: m_nWeightMode(0)
{
}

CConfig::~CConfig()
{}

int CConfig::makepath(const char *path)
{
	string p, oldpath = path;
	char *pToken = strtok((char*)oldpath.c_str(), "\\");
	while (pToken && !strstr(pToken, ".k"))
	{
		p += pToken;
		p += "\\";
		mkdir(p.c_str());
		pToken = strtok(NULL, "\\");
	}
	return 0;
}

bool CConfig::kdb_create(const char *mkt)
{
	string scd_name = m_strPath + "\\" + mkt + ".k";
	WIN32_FIND_DATA wfd;
	HANDLE hf = FindFirstFile(scd_name.c_str(), &wfd);
	if (hf == INVALID_HANDLE_VALUE)
	{
		DWORD ec = GetLastError();
		if (ec == ERROR_PATH_NOT_FOUND)
		{
			CHECK_RUN(0 != makepath(scd_name.c_str()), MN, T("创建k线目录失败\n"), RPT_ERROR, return false);
		}
		else
		{
			CHECK_RUN(ec != ERROR_FILE_NOT_FOUND, MN, T("创建 %s 失败[%d]\n", scd_name.c_str(), ec),
				RPT_ERROR, return false);
		}

		sqlite3* psql = NULL;
		int sqln = sqlite3_open(scd_name.c_str(), &psql);
		CHECK_RUN(sqln != SQLITE_OK, MN,
			T("{{ 打开 %s 失败：\nErrCode: %d\n%s }}\n", scd_name.c_str(), sqln, sqlite3_errmsg(psql)),
			RPT_ERROR, return false);

		string sql = "create table dayk\
					 (\
					 symbol varchar(50) not null,\
					 open unsigned int not null,\
					 high unsigned int not null,\
					 low unsigned int not null,\
					 close unsigned int not null,\
					 amount unsigned int not null,\
					 sum unsigned int not null,\
					 scale float,\
					 day unsigned int not null\
					 );\
					 create index dayk_idx on dayk(symbol, day);";

		char *pErrMsg = NULL;
		sqln = sqlite3_exec(psql, sql.c_str(), NULL, NULL, &pErrMsg);
		CHECK_RUN(sqln != SQLITE_OK, MN,
			T("{{ %s 建表失败：\nErrcode: %d\n%s }}\n", scd_name.c_str(), sqln, pErrMsg),
			RPT_ERROR, {sqlite3_free(pErrMsg); return false;});
		sqlite3_close(psql);
		REPORT(MN, T("建立 %s 成功\n", scd_name.c_str()), RPT_INFO);
	}
	FindClose(hf);
	return true;
}

void CConfig::prebuilddb()
{
	for (unsigned int i = 0; i < m_vecMarket.size(); i++)
		kdb_create(m_vecMarket[i].mkt.c_str());
}

int CConfig::GetTz(const char *lpMarket)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	__try
	{
		for (unsigned int i = 0; i < m_vecMarket.size(); i++)
		{
			if (m_vecMarket[i].mkt == lpTmp)
				return m_vecMarket[i].tz;
		}
		return 0;
	}
	__finally
	{
		free(lpTmp);
	}
}

unsigned int CConfig::GetMarketYear(time_t now, const char *lpMarket)
{
	int tz = GetTz(lpMarket);
	time_t tzNow = now + tz * 3600;
	struct tm *tztm = gmtime(&tzNow);
	return tztm->tm_year + 1900;
}

void CConfig::OnNode(TiXmlElement *pElm, NODEWAY way)
{
	if (!strcmp(pElm->Value(), "市场") && way == TiXmlParser::ENTER)
		m_nWeightMode = 1;
	else if (!strcmp(pElm->Value(), "市场") && way == TiXmlParser::LEAVE)
	{
		m_vecMarket.push_back(m_tmpMkt);
		m_nWeightMode = 0;
	}
	else if (!strcmp(pElm->Value(), "还权") && way == TiXmlParser::ENTER)
		m_nWeightMode = 2;
	else if (!strcmp(pElm->Value(), "还权") && way == TiXmlParser::LEAVE)
		m_nWeightMode = 0;
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (NodeName == "路径")
		m_strPath = NodeText;
	else if (m_nWeightMode == 1)
	{
		if (NodeName == "代码")
			m_tmpMkt.mkt = NodeText;
		else if (NodeName == "时差")
			m_tmpMkt.tz = atoi(NodeText.c_str());
	}
	else if (m_nWeightMode == 2)
	{
		if (NodeName == "server")
			server = NodeText;
		else if (NodeName == "database")
			database = NodeText;
		else if (NodeName == "uid")
			uid = NodeText;
		else if (NodeName == "pwd")
			pwd = NodeText;
	}
}
