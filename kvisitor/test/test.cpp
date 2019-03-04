// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <vld.h>
#include "../../public/kvisitor.h"
#include "../../public/commx/xdll.h"
#include "../../public/commx/report.h"
#include "../../public/commx/highperformancecounter.h"

map<string, int> mapLastday;

struct _ql
{
	long m_tTime;  //日期(t_time)
	long m_fOpen;  //开
	long m_fMaxPrice; //高
	long m_fMinPrice; //低
	long m_fPrevClose; //收
	long m_lTotal;  //成交额
	long m_lVolume;  //成交量
	int nouse1;
	int nouse2;
	int nouse3;
};

bool operator<(const dayk &k1, const dayk &k2)
{
	if (k1.day < k2.day)
		return true;
	return false;
}

void addayk()
{
	string line;
	vector<dayk> vdayksh, vdayksz;
	int mktflag;
	dayk dk;

	ifstream f1("D:\\Source\\QServer\\kconv\\zs");
	while (1)
	{
		getline(f1, line);
		if (line.length() == 0)
			break;

		for (string::size_type i = line.find(",,"); i != string::npos; i = line.find(",,"))
			line = line.replace(i, 2, ",0,");
		for (string::size_type i = line.find("\"\""); i != string::npos; i = line.find("\"\""))
			line = line.replace(i, 2, "\" \"");

		char *token = strtok((char*)line.c_str(), ",\"");
		char *tmp = strchr(token, '-');
		tmp[0] = 0x00;
		int year = atoi(token);
		token = tmp + 1;
		tmp = strchr(token, '-');
		tmp[0] = 0x00;
		int mon = atoi(token);
		token = tmp + 1;
		tmp = strchr(token, ' ');
		tmp[0] = 0x00;
		int day = atoi(token);
		dk.day = year * 10000 + mon * 100 + day;

		token = strtok(NULL, ",\"");
		int ncode = atoi(token);
		strcpy(dk.szStockCode, token);

		token = strtok(NULL, ",\"");
		token = strtok(NULL, ",\"");
		token = strtok(NULL, ",\"");
		if (!strcmp(token, "上交所"))
		{
			mktflag = 0;
			strcat(dk.szStockCode, ".SH");
		}
		else
		{
			mktflag = 1;
			strcat(dk.szStockCode, ".SZ");
		}

		token = strtok(NULL, ",\"");
		token = strtok(NULL, ",\"");
		dk.open = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.high = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.low = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.close = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.volume = (int)(atof(token));

		token = strtok(NULL, ",\"");
		if (token)
			dk.amount = (int)(atof(token) / 1000);
		else
			dk.amount = 0;

		if (ncode < 399000 && ncode != 300 && mktflag == 1)
			continue;

		mapLastday[dk.szStockCode] = dk.day;
		if (mktflag == 0)
			vdayksh.push_back(dk);
		else
			vdayksz.push_back(dk);
	}
	f1.close();

	ifstream f2("D:\\Source\\QServer\\kconv\\zqrhq");
	while (1)
	{
		getline(f2, line);
		if (line.length() == 0)
			break;

		for (string::size_type i = line.find(",,"); i != string::npos; i = line.find(",,"))
			line = line.replace(i, 2, ",0,");
		for (string::size_type i = line.find("\"\""); i != string::npos; i = line.find("\"\""))
			line = line.replace(i, 2, "\" \"");

		char *token = strtok((char*)line.c_str(), ",\"");
		strcpy(dk.szStockCode, token);

		token = strtok(NULL, ",\"");
		token = strtok(NULL, ",\"");		
		char *tmp = strchr(token, '-');
		tmp[0] = 0x00;
		int year = atoi(token);
		token = tmp + 1;
		tmp = strchr(token, '-');
		tmp[0] = 0x00;
		int mon = atoi(token);
		token = tmp + 1;
		tmp = strchr(token, ' ');
		tmp[0] = 0x00;
		int day = atoi(token);
		dk.day = year * 10000 + mon * 100 + day;

		token = strtok(NULL, ",\"");
		token = strtok(NULL, ",\"");
		if (!strcmp(token, "上交所"))
		{
			mktflag = 0;
			strcat(dk.szStockCode, ".sh");
		}
		else
		{
			mktflag = 1;
			strcat(dk.szStockCode, ".sz");
		}

		token = strtok(NULL, ",\"");
		token = strtok(NULL, ",\"");
		dk.open = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.high = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.low = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.close = (int)(atof(token) * 1000);

		token = strtok(NULL, ",\"");
		dk.volume = (int)(atof(token) / 100);

		token = strtok(NULL, ",\"");
		if (token)
			dk.amount = (int)(atof(token) / 1000);
		else
			dk.amount = 0;

		if (dk.close == 0 || dk.volume == 0)
			continue;

		mapLastday[dk.szStockCode] = dk.day;
		if (mktflag == 0)
			vdayksh.push_back(dk);
		else
			vdayksz.push_back(dk);
	}
	f2.close();

	_ql ql;
	int maxday = 0;
	string code;
	string path = "D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SHASE\\day\\*.day";
	WIN32_FIND_DATA wfd;
	HANDLE hf = FindFirstFile(path.c_str(), &wfd);
	BOOL bb = TRUE;
	while (bb)
	{
		code = wfd.cFileName;
		code = code.substr(0, 6);
		if (atoi(code.c_str()) < 1000)
			code += ".SH";
		else
			code += ".sh";

		if (mapLastday.find(code) != mapLastday.end())
			maxday = mapLastday[code];
		else
			maxday = 0;
		REPORT("KLV", T("[%s]maxday=%d\n", code.c_str(), maxday), RPT_INFO);

		line = "";
		strcpy(dk.szStockCode, code.c_str());
		path = path.substr(0, path.rfind("\\")+1) + wfd.cFileName;
		FILE *fp = fopen(path.c_str(), "rb");
		while (fread(&ql, 1, sizeof(_ql), fp) == sizeof(_ql))
		{
			if (ql.m_tTime > maxday)
			{
				dk.day = ql.m_tTime;
				dk.open = ql.m_fOpen;
				dk.high = ql.m_fMaxPrice;
				dk.low = ql.m_fMinPrice;
				dk.close = ql.m_fPrevClose;
				dk.volume = ql.m_lVolume;
				dk.amount = ql.m_lTotal;
				maxday = ql.m_tTime;
				vdayksh.push_back(dk);
			}
		}
		fclose(fp);
		bb = FindNextFile(hf, &wfd);
	}

	path = "D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SZNSE\\day\\*.day";
	hf = FindFirstFile(path.c_str(), &wfd);
	bb = TRUE;
	while (bb)
	{
		code = wfd.cFileName;
		code = code.substr(0, 6);
		if (atoi(code.c_str()) < 400000 && atoi(code.c_str()) >= 399000)
			code += ".SZ";
		else
			code += ".sz";

		if (mapLastday.find(code) != mapLastday.end())
			maxday = mapLastday[code];
		else
			maxday = 0;
		REPORT("KLV", T("[%s]maxday=%d\n", code.c_str(), maxday), RPT_INFO);

		line = "";
		strcpy(dk.szStockCode, code.c_str());
		path = path.substr(0, path.rfind("\\")+1) + wfd.cFileName;
		FILE *fp = fopen(path.c_str(), "rb");
		while (fread(&ql, 1, sizeof(_ql), fp) == sizeof(_ql))
		{
			if (ql.m_tTime > maxday)
			{
				dk.day = ql.m_tTime;
				dk.open = ql.m_fOpen;
				dk.high = ql.m_fMaxPrice;
				dk.low = ql.m_fMinPrice;
				dk.close = ql.m_fPrevClose;
				dk.volume = ql.m_lVolume;
				dk.amount = ql.m_lTotal;
				maxday = ql.m_tTime;
				vdayksz.push_back(dk);
			}
		}
		fclose(fp);
		bb = FindNextFile(hf, &wfd);
	}

	sort(vdayksh.begin(), vdayksh.end());
	sort(vdayksz.begin(), vdayksz.end());

	unsigned int year = vdayksh[0].day/10000;
	for (unsigned int i = 0; i < vdayksh.size(); i++)
	{
		if (vdayksh[i].day/10000 != year)
		{
			kdb_addayk("sh", line);
			kdb_update_history("sh");
			line = "";
			REPORT("KLV", T("sh: %d\n", year), RPT_INFO);
			year = vdayksh[i].day/10000;
		}
		line += kdb_addkline(vdayksh[i]);
	}

	kdb_addayk("sh", line);
	line = "";

	year = vdayksz[0].day/10000;
	for (unsigned int i = 0; i < vdayksz.size(); i++)
	{
		if (vdayksz[i].day/10000 != year)
		{
			kdb_addayk("sz", line);
			kdb_update_history("sz");
			line = "";
			REPORT("KLV", T("sz: %d\n", year), RPT_INFO);
			year = vdayksz[i].day/10000;
		}
		line += kdb_addkline(vdayksz[i]);
	}

	kdb_addayk("sz", line);
	line = "";
}

void addql(int nday)
{
	_ql ql;
	int maxday = 0;
	string code, line;
	string path = "D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SHASE\\day\\*.day";
	WIN32_FIND_DATA wfd;
	HANDLE hf = FindFirstFile(path.c_str(), &wfd);
	BOOL bb = TRUE;
	dayk dk;
	vector<dayk> vdayksh, vdayksz;

	while (bb)
	{
		code = wfd.cFileName;
		code = code.substr(0, 6);
		if (atoi(code.c_str()) < 1000)
			code += ".SH";
		else
			code += ".sh";

		int maxday = kdb_getlastestday(code.c_str());

		line = "";
		strcpy(dk.szStockCode, code.c_str());
		path = path.substr(0, path.rfind("\\")+1) + wfd.cFileName;
		FILE *fp = fopen(path.c_str(), "rb");
		while (fread(&ql, 1, sizeof(_ql), fp) == sizeof(_ql))
		{
			if (ql.m_tTime > maxday && ql.m_tTime >= nday)
			{
				dk.day = ql.m_tTime;
				dk.open = ql.m_fOpen;
				dk.high = ql.m_fMaxPrice;
				dk.low = ql.m_fMinPrice;
				dk.close = ql.m_fPrevClose;
				dk.volume = ql.m_lVolume;
				dk.amount = ql.m_lTotal;
				maxday = ql.m_tTime;
				vdayksh.push_back(dk);
			}
		}
		fclose(fp);
		bb = FindNextFile(hf, &wfd);
	}

	path = "D:\\Programs\\Qianlong\\qijian\\QLDATA\\history\\SZNSE\\day\\*.day";
	hf = FindFirstFile(path.c_str(), &wfd);
	bb = TRUE;
	while (bb)
	{
		code = wfd.cFileName;
		code = code.substr(0, 6);
		if (atoi(code.c_str()) < 400000 && atoi(code.c_str()) >= 399000)
			code += ".SZ";
		else
			code += ".sz";

		int maxday = kdb_getlastestday(code.c_str());

		line = "";
		strcpy(dk.szStockCode, code.c_str());
		path = path.substr(0, path.rfind("\\")+1) + wfd.cFileName;
		FILE *fp = fopen(path.c_str(), "rb");
		while (fread(&ql, 1, sizeof(_ql), fp) == sizeof(_ql))
		{
			if (ql.m_tTime > maxday && ql.m_tTime >= nday)
			{
				dk.day = ql.m_tTime;
				dk.open = ql.m_fOpen;
				dk.high = ql.m_fMaxPrice;
				dk.low = ql.m_fMinPrice;
				dk.close = ql.m_fPrevClose;
				dk.volume = ql.m_lVolume;
				dk.amount = ql.m_lTotal;
				maxday = ql.m_tTime;
				vdayksz.push_back(dk);
			}
		}
		fclose(fp);
		bb = FindNextFile(hf, &wfd);
	}

	sort(vdayksh.begin(), vdayksh.end());
	sort(vdayksz.begin(), vdayksz.end());

	unsigned int year = vdayksh[0].day/10000;
	for (unsigned int i = 0; i < vdayksh.size(); i++)
	{
		if (vdayksh[i].day/10000 != year)
		{
			kdb_addayk("sh", line);
			kdb_update_history("sh");
			line = "";
			REPORT("KLV", T("sh: %d\n", year), RPT_INFO);
			year = vdayksh[i].day/10000;
		}
		line += kdb_addkline(vdayksh[i]);
	}

	kdb_addayk("sh", line);
	line = "";

	year = vdayksz[0].day/10000;
	for (unsigned int i = 0; i < vdayksz.size(); i++)
	{
		if (vdayksz[i].day/10000 != year)
		{
			kdb_addayk("sz", line);
			kdb_update_history("sz");
			line = "";
			REPORT("KLV", T("sz: %d\n", year), RPT_INFO);
			year = vdayksz[i].day/10000;
		}
		line += kdb_addkline(vdayksz[i]);
	}

	kdb_addayk("sz", line);
	line = "";
}

// void weight(int nday)
// {
// 	time_t now;
// 	time(&now);
// 	tm *lt = localtime(&now);
// 	int day = (lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday;
// 	while (day > nday)
// 	{
// 		kdb_update_weight("sz|sh", day);
// 		now -= 24*3600;
// 		lt = localtime(&now);
// 		day = (lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday;
// 	}
// }

void test_get_dayk(const char *symbol, unsigned int day, unsigned int num)
{
	CHighPerformanceCounter hpc;
	unsigned int daynum;
	time_t now;
	time(&now);
	tm *lt = localtime(&now);
	daynum = kdb_get5dayvol(symbol, (lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday);
// 	dayk *pdk = new dayk[num];
// 	daynum = kdb_getdayk(pdk, num, symbol, day);
// 	for (unsigned int i = 0; i < daynum; i++)
// 		cout << pdk[i].day << ":" << pdk[i].open << "," << pdk[i].high << "," << pdk[i].low << "," << pdk[i].close << endl;
// 	delete []pdk;
	cout << hpc.count() <<endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	CReportBase *prb = new CReportBase;
	CReportBox::Instance()->AddReportUnit(prb);

// 	addayk();
	test_get_dayk("300111.sz", 20100707, 10000);
// 	test_get_dayk("523008.sh", 20100505, 100);
// 	addql(20100101);

	CReportBox::Instance()->Close();
	return 0;
}
