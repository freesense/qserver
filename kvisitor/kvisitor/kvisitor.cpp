// kvisitor.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "config.h"
#include "dblib.h"
#include <string>
#include <direct.h>
#include <algorithm>
#include "../../public/kvisitor.h"
#include "../../public/sqlite/sqlite3.h"

using namespace std;

#pragma warning(disable: 4267)

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{/** ��xml�ļ���ȡ���� [2/25/2010 xinl] */
		string sPath = GetExecutePath();
		sPath = sPath.substr(0, sPath.rfind('\\') + 1) + "QuoteFarm.xml";
		g_cfg.Load(sPath.c_str());
		g_cfg.prebuilddb();
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

/**@brief ������Ӧ���г�k�����ݿ�

�г���xml�����ļ��л��
@return
 - true �������ݿ�ɹ�
 - false �������ݿ�ʧ��
 */
void kdb_create()
{
	for (unsigned int i = 0; i < g_cfg.m_vecMarket.size(); i++)
		g_cfg.kdb_create(g_cfg.m_vecMarket[i].mkt.c_str());
}

/**@brief ����һ��k������

��û���������ӵ����ݿ��У���Ϊ���������ݿ����Ч�ʺܵͣ�ֻ����һ��sql���
@param kline �����򴫵ݹ�����k������
@return sql���
 */
string kdb_addkline(dayk &kline)
{
	char szSql[1024];
	sprintf(szSql, "insert into dayk values('%s', %d, %d, %d, %d, %d, %d, 1, %d);", kline.szStockCode,
		kline.open, kline.high, kline.low, kline.close, kline.volume, kline.amount, kline.day);
	return szSql;
}

/**@brief ��k���ļ�������k������

�˴���k�������ύ��k�����ݿ�
@param mkt Ҫͳһ�����ύk�����ݵ��г�
@param sql ����һ��kdb_addkline�������ص��ִ�ƴ�Ӷ��ɵ��ַ������ڴ˿����������ύ
@return
- true ����k�߳ɹ�
- false ����k��ʧ��
 */
bool kdb_addayk(const char *mkt, string &sql)
{
	sql.insert(0, "begin transaction;");
	sql += "commit;";
	sqlite3* psql = NULL;
	char *pErrMsg = NULL;
	string path = g_cfg.m_strPath + "\\" + mkt + ".k";

	int sqln = sqlite3_open(path.c_str(), &psql);
	CHECK_RUN(sqln != SQLITE_OK, MN,
		T("{{ �� %s ʧ�ܣ�\nErrCode: %d\n%s }}\n", path.c_str(), sqln, sqlite3_errmsg(psql)),
		RPT_ERROR, return false);

	sqln = sqlite3_exec(psql, sql.c_str(), NULL, NULL, &pErrMsg);
	CHECK_RUN(sqln != SQLITE_OK, MN,
		T("{{ [%s]����k��ʧ�ܣ�\nErrcode: %d\n%s }}\n", mkt, sqln, pErrMsg),
		RPT_ERROR, {sqlite3_free(pErrMsg); sqlite3_close(psql); return false;});
	sqlite3_close(psql);
	REPORT(MN, T("[%s]����k�߳ɹ�\n", mkt), RPT_INFO);
	return true;
}

/**@brief ά��k����ʷ���ݣ�k�߿���ת��

��ʷk����֯��ʽ��
1.ÿ���г����ݿ�ı���ȵ�k�����ݱ���Ϊmkt.k
2.��ʷ���ݿ��ļ�����Ϊmkt.year
3.��������ݿ��������ݣ������տ⣨ֻ�п�ṹ�������ݣ�
@param lpmkt �г�����
@return ��ʱ������
 */
bool kdb_update_history(const char *lpmkt)
{
	time_t now;
	time(&now);
	int maxyear = kdb_getlastestday(lpmkt)/10000;
	if (maxyear != g_cfg.GetMarketYear(now, lpmkt))
	{
		char szOldName[MAX_PATH], szNewName[MAX_PATH];
		sprintf(szOldName, "%s\\%s.k", g_cfg.m_strPath.c_str(), lpmkt);
		sprintf(szNewName, "%s\\%s.%d", g_cfg.m_strPath.c_str(), lpmkt, maxyear);
		rename(szOldName, szNewName);
		g_cfg.kdb_create(lpmkt);
	}
	return true;
}

bool _update_weight(int year, string &sqlsh, string &sqlsz)
{
	time_t now;
	time(&now);
	tm *lt = localtime(&now);
	int thisyear = lt->tm_year+1900;
	char szFileName[MAX_PATH], shFileName[MAX_PATH];

	if (year == thisyear)
	{
		sprintf(szFileName, "%s\\sz.k", g_cfg.m_strPath.c_str());
		sprintf(shFileName, "%s\\sh.k", g_cfg.m_strPath.c_str());
	}
	else
	{
		sprintf(szFileName, "%s\\sz.%d", g_cfg.m_strPath.c_str(), year);
		sprintf(shFileName, "%s\\sh.%d", g_cfg.m_strPath.c_str(), year);
	}

	sqlite3* psql = NULL;
	char *pErrMsg = NULL;
	int sqln = sqlite3_open_v2(shFileName, &psql, SQLITE_OPEN_READWRITE, NULL);
	if (sqln == SQLITE_OK && sqlsh.length())
	{
		sqlite3_exec(psql, "begin transaction;", NULL, NULL, &pErrMsg);
		sqlsh.insert(0, "update dayk set scale = 1;");
//		sqlsh.insert(0, "begin transaction;update dayk set scale = 1;");
//		sqlsh += "commit;";
		sqln = sqlite3_exec(psql, sqlsh.c_str(), NULL, NULL, &pErrMsg);
		CHECK_RUN(sqln != SQLITE_OK, MN,
			T("{{ �����Ϻ���Ȩ����ʧ�ܣ�\nErrcode: %d\n%s }}\n", sqln, pErrMsg),
			RPT_ERROR, {sqlite3_free(pErrMsg); sqlite3_close(psql); return false;});
		sqlite3_exec(psql, "commit transaction;", NULL, NULL, &pErrMsg);
	}
	sqlite3_close(psql);
	REPORT(MN, T("�Ѹ���%s��Ȩ����\n", shFileName), RPT_INFO);

	sqln = sqlite3_open_v2(szFileName, &psql, SQLITE_OPEN_READWRITE, NULL);
	if (sqln == SQLITE_OK && sqlsz.length())
	{
		sqlite3_exec(psql, "begin transaction;", NULL, NULL, &pErrMsg);
		sqlsz.insert(0, "update dayk set scale = 1;");
// 		sqlsz.insert(0, "begin transaction;update dayk set scale = 1;");
// 		sqlsz += "commit;";
		sqln = sqlite3_exec(psql, sqlsz.c_str(), NULL, NULL, &pErrMsg);
		CHECK_RUN(sqln != SQLITE_OK, MN,
			T("{{ �������ڻ�Ȩ����ʧ�ܣ�\nErrcode: %d\n%s }}\n", sqln, pErrMsg),
			RPT_ERROR, {sqlite3_free(pErrMsg); sqlite3_close(psql); return false;});
		sqlite3_exec(psql, "commit transaction;", NULL, NULL, &pErrMsg);
	}
	sqlite3_close(psql);
	REPORT(MN, T("�Ѹ���%s��Ȩ����\n", szFileName), RPT_INFO);

	return true;
}

bool kdb_update_weight()
{
	CDblib dblib;
	bool bdb = dblib.Open(g_cfg.server.c_str(), g_cfg.uid.c_str(), g_cfg.pwd.c_str());
	CHECK_RUN(!bdb, MN, T("Open dblib[%s:%s] failed: %s\n", g_cfg.server.c_str(), g_cfg.uid.c_str(), dblib.GetErrorMsg()), RPT_ERROR, return false);
	bdb = dblib.Run("select sec_cd, f0010, f0020 from ind_s_right where f0020 is not null and sec_cd is not null and f0010 is not null order by f0010 asc");
	CHECK_RUN(!bdb, MN, T("Execute sql failed: %s\n", dblib.GetErrorMsg()), RPT_ERROR, return false);

	string sqlsz, sqlsh;
	int year = 1900;
	char sql[1024];
	while (dblib.MoveNext())
	{
		string zqdm = dblib.GetColData(0);
		int hqrq = atoi(dblib.GetColData(1));
		double f0020 = atof(dblib.GetColData(2));
		if (hqrq / 10000 != year)
		{
			_update_weight(year, sqlsh, sqlsz);
			year = hqrq / 10000;
			sqlsz = "";
			sqlsh = "";
		}

		int code = atoi(zqdm.c_str());
		if (code > 599999 && code < 700000 || code > 899999)
		{
			sprintf(sql, "update dayk set scale=%f where symbol='%s.sh' and day=%d;", f0020, zqdm.c_str(), hqrq);
			sqlsh += sql;
		}
		else
		{
			sprintf(sql, "update dayk set scale=%f where symbol='%s.sz' and day=%d;", f0020, zqdm.c_str(), hqrq);
			sqlsz += sql;
		}
	}

	_update_weight(year, sqlsh, sqlsz);
	dblib.Close();

	return true;
}

bool _less(const string &f1, const string &f2)
{
	int idx1 = f1.find('.');
	int idx2 = f2.find('.');
	idx1++;
	idx2++;
	if (f1[idx1] == 'k')
		return false;
	if (f2[idx2] == 'k')
		return true;
	char *lp1 = (char*)(f1.c_str()+idx1);
	char *lp2 = (char*)(f2.c_str()+idx2);
	return atoi(lp1) < atoi(lp2);
}

int kdb_getfirstday(const char *lpSymbol)
{
	vector<string> vfn;

	char sql[1024], path[MAX_PATH];
	char *lpmkt = (char*)strrchr(lpSymbol, '.');
	lpmkt += 1;
	string sym = lpSymbol;
	sym = sym.substr(0, sym.rfind('.'));
	sprintf(sql, "select min(day) from dayk where symbol='%s'", lpSymbol);

	BOOL bFind = TRUE;
	WIN32_FIND_DATA wfd;
	sprintf(path, "%s\\%s.*", g_cfg.m_strPath.c_str(), lpmkt);
	HANDLE hFind = FindFirstFile(path, &wfd);
	while (hFind != INVALID_HANDLE_VALUE && bFind)
	{
		vfn.push_back(wfd.cFileName);
		bFind = FindNextFile(hFind, &wfd);
	}

	sqlite3* psql = NULL;
	int minday = 0;
	sort(vfn.begin(), vfn.end(), _less);

	for (unsigned int i = 0; !minday && i < vfn.size(); i++)
	{
		sprintf(path, "%s\\%s", g_cfg.m_strPath.c_str(), vfn[i].c_str());
		int sqln = sqlite3_open_v2(path, &psql, SQLITE_OPEN_READONLY, NULL);
		if (sqln != SQLITE_OK)
			return -1;

		char *pErrMsg = NULL, **dbResult;
		int nRow, nColumn;
		sqln = sqlite3_get_table(psql, sql, &dbResult, &nRow, &nColumn, &pErrMsg);
		CHECK_RUN(sqln != SQLITE_OK, MN,
			T("{{ %s��\n%s }}\n", sql, pErrMsg),
			RPT_ERROR, {sqlite3_free(pErrMsg); sqlite3_close(psql); return -1;});
		if (nRow == 1 && dbResult[1])
			minday = atoi(dbResult[1]);

		sqlite3_free_table(dbResult);
		sqlite3_close(psql);
	}
	return minday;
}

int kdb_getlastestday(const char *lpSymbol)
{
	char sql[1024], path[MAX_PATH];
	char *lpmkt = (char*)strrchr(lpSymbol, '.');
	if (!lpmkt)
	{
		sprintf(sql, "select max(day) from dayk");
		sprintf(path, "%s\\%s.k", g_cfg.m_strPath.c_str(), lpSymbol);
	}
	else
	{
		lpmkt += 1;
		string sym = lpSymbol;
		sym = sym.substr(0, sym.rfind('.'));
		sprintf(sql, "select max(day) from dayk where symbol='%s'", lpSymbol);
		sprintf(path, "%s\\%s.k", g_cfg.m_strPath.c_str(), lpmkt);
	}

	sqlite3* psql = NULL;
	int maxday = 0, year = 0;

	while (maxday == 0)
	{
		int sqln = sqlite3_open_v2(path, &psql, SQLITE_OPEN_READONLY, NULL);
		if (sqln != SQLITE_OK)
			return -1;

		char *pErrMsg = NULL, **dbResult;
		int nRow, nColumn;
		sqln = sqlite3_get_table(psql, sql, &dbResult, &nRow, &nColumn, &pErrMsg);
		CHECK_RUN(sqln != SQLITE_OK, MN,
			T("{{ %s��\n%s }}\n", sql, pErrMsg),
			RPT_ERROR, {sqlite3_free(pErrMsg); sqlite3_close(psql); return -1;});
		if (nRow == 1 && dbResult[1])
			maxday = atoi(dbResult[1]);
		else if (year == 0)
		{
			time_t now;
			time(&now);
			struct tm *lt = localtime(&now);
			year = lt->tm_year + 1900;
		}
		year--;
		sprintf(path, "%s\\%s.%d", g_cfg.m_strPath.c_str(), lpmkt, year);

		sqlite3_free_table(dbResult);
		sqlite3_close(psql);
	}
	return maxday;
}

unsigned int _getdayk(dayk *pdk, unsigned int num, const char *path, const char *sql)
{
	sqlite3* psql = NULL;
	int sqln = sqlite3_open_v2(path, &psql, SQLITE_OPEN_READONLY, NULL);
	if (sqln != SQLITE_OK)
		return 0;

	char *pErrMsg = NULL, **dbResult;
	int nRow, nColumn;
	sqln = sqlite3_get_table(psql, sql, &dbResult, &nRow, &nColumn, &pErrMsg);
	CHECK_RUN(sqln != SQLITE_OK, MN,
		T("{{ %s��\n%s }}\n", sql, pErrMsg),
		RPT_ERROR, {sqlite3_free(pErrMsg); sqlite3_close(psql); return 0;});

	for (int i = 0, j = 0; i < nRow; i++)
	{
		strcpy(pdk[i].szStockCode, dbResult[(i+1)*nColumn]);
		pdk[i].open = dbResult[(i+1)*nColumn+1] ? atoi(dbResult[(i+1)*nColumn+1]) : 0;
		if (pdk[i].open == 0)	// ���̼�Ϊ0ֱ�Ӷ�������k��
			continue;
		pdk[i].high = dbResult[(i+1)*nColumn+2] ? atoi(dbResult[(i+1)*nColumn+2]) : 0;
		pdk[i].low = dbResult[(i+1)*nColumn+3] ? atoi(dbResult[(i+1)*nColumn+3]) : 0;
		pdk[i].close = dbResult[(i+1)*nColumn+4] ? atoi(dbResult[(i+1)*nColumn+4]) : 0;
		pdk[i].volume = dbResult[(i+1)*nColumn+5] ? atoi(dbResult[(i+1)*nColumn+5]) : 0;
		pdk[i].amount = dbResult[(i+1)*nColumn+6] ? atoi(dbResult[(i+1)*nColumn+6]) : 0;
		pdk[i].scale = dbResult[(i+1)*nColumn+7] ? atof(dbResult[(i+1)*nColumn+7]) : 0;
		pdk[i].day = dbResult[(i+1)*nColumn+8] ? atoi(dbResult[(i+1)*nColumn+8]) : 0;
		j++;
	}

	sqlite3_free_table(dbResult);
	sqlite3_close(psql);
	return nRow;
}

/**@brief ȡ��k������

@param lpSymbol Ʒ�ִ���
@param pdk �����߷����k������ָ�룬Ӧȷ�����㹻�Ŀռ�
@param num size of pdk
@param dayfrom �Ӻ�ʱ��ʼ��ǰȡk��(20070822)
@return ����ȡ����k������
 */
unsigned int kdb_getdayk(dayk *pdk, unsigned int num, const char *lpSymbol, unsigned int dayfrom)
{
	char *lpmkt = (char*)strrchr(lpSymbol, '.');
	CHECK_RUN(!lpmkt, MN, T("[%s]symbol�Ƿ�\n", lpSymbol), RPT_WARNING, return 0);
	lpmkt += 1;

	char sql[1024], path[MAX_PATH];
	sprintf(path, "%s\\%s.k", g_cfg.m_strPath.c_str(), lpmkt);

	int count = 0, year = 0, minday = 0, maxday = 0;
	while (count < (int)num)
	{
		sprintf(sql, "select a.symbol, case a.open when 0 then (select b.close from dayk b where b.day<a.day and b.symbol=a.symbol and b.close<>0 order by b.day desc limit 1) else a.open end as open, a.high, a.low, a.close, a.amount, a.sum, a.scale, a.day from dayk a where a.symbol='%s' and a.day<=%d order by a.day desc limit %d", lpSymbol, dayfrom, num - count);
		unsigned int sqln = _getdayk(pdk+count, num-count, path, sql);
		if (sqln == 0)
		{
			if (!minday)
			{
				minday = kdb_getfirstday(lpSymbol);
				maxday = kdb_getlastestday(lpSymbol);
			}
			if (minday == 0 || maxday <= 0 || minday == pdk[count-1].day)
				break;
			if (!year)
				year = maxday/10000+1;
		}
		else if (sqln != -1)
			count += sqln;

		if (year == 0)
		{
			time_t now;
			time(&now);
			struct tm *lt = localtime(&now);
			year = lt->tm_year + 1900;
		}
		year--;
		sprintf(path, "%s\\%s.%d", g_cfg.m_strPath.c_str(), lpmkt, year);
	}

	for (int i = 0; i < count; i++)
	{
		if (pdk[i].close == 0)
			pdk[i].close = pdk[i].open;
		if (pdk[i].high == 0)
			pdk[i].high = max(pdk[i].open, pdk[i].close);
		if (pdk[i].low == 0)
			pdk[i].low = min(pdk[i].open, pdk[i].close);
	}
	return count;
}

/**@brief ȡ5��ƽ������

@param lpSymbol Ʒ�ִ���
@return 5��ƽ������
 */
unsigned int kdb_get5dayvol(const char *lpSymbol, unsigned int today)
{
	dayk dk[5];
	unsigned int cnt = kdb_getdayk(dk, 5, lpSymbol, today);
	CHECK_RUN(cnt == 0, MN, T("[%s]K������Ϊ0\n", lpSymbol), RPT_WARNING, return 0);
	unsigned int total = 0;
	for (unsigned int i = 0; i < cnt; i++)
		total += dk[i].volume;
	return total/cnt;
}
