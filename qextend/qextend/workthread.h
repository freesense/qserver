
#ifndef __WORK_THREAD_H__
#define __WORK_THREAD_H__

#include "farm.h"
#include "dblib.h"
#include <map>
#include <string>
#include "../../public/commx/xthread.h"
#include "../../public/commx/synch.h"

using std::map;
using std::multimap;
using std::string;

#define	RPT_HEARTBEAT  0xAA	//心跳日志，用来监控线程

///////////////////////////////////////////////////////////////////////////////////////////
struct _symbol_data
{
	_symbol_data()
	{
		memset(&pq, 0x00, sizeof(Quote));
	}

	_symbol_data(const char *code)
		: symbol(code)
	{
		memset(&pq, 0x00, sizeof(Quote));
	};

	std::string symbol;
	Quote pq;
	int reserved1;					// 基础行情之外的字段
	double reserved2;				// 基础行情之外的字段
};

struct _symbol_zjjg
{
	int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);
	int zdf;
	CDataFarm *m_pFarm;
};

struct _find_symbol
{
	_find_symbol(CDataFarm *pFarm);
	int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);
	int OnElement(std::string *pSymbol, RINDEX *pIdx);
	void operator()(_symbol_data &sd);
	CDataFarm *m_pFarm;
	std::vector<_symbol_data> *m_pvSymbol;
	std::string m_strGroup;
	_symbol_data *m_symbol_data;
};

struct less_symbol
{
	less_symbol(std::string &sortcol, int ascend, CDataFarm *pFarm);
	bool operator()(_symbol_data &s1, _symbol_data &s2);

	std::string sort_colname;		// 参与排序的列
	int nAscending;					// 正序or逆序
	CDataFarm *m_pFarm;
};

struct index_stat
{
	index_stat(CDataFarm *pFarm, unsigned int *u, unsigned int *d, unsigned int *e);
	void operator()(_symbol_data &s);

	unsigned int *up, *dn, *eq;
	CDataFarm *m_pFarm;
};

struct _remove_symbol
{
	_remove_symbol(const char *lpColName);
	bool operator()(_symbol_data &sd);
	std::string sort_col;
};

///////////////////////////////////////////////////////////////////////////////////////////
class CWorkThread : public XThread
{
	// symbol资讯
	struct _symbol_zixun
	{
		double fMarketValue;			//总市值
		double fFlowValue;				//流通市值
		double fFlowShare;				//流通股
	};

	// 行业列表
	struct _vocation_list
	{
		std::string strVocationCode;	//行业代码
		std::string strVocationName;	//行业名称
		double fMarketValue;			//行业市值
	};

	typedef map<string, _symbol_zixun> MAPZIXUN;
	typedef multimap<unsigned short, _vocation_list> MAPVOCATION;
	typedef multimap<string, string> MAPVOCATIONSYMBOL;

public:
	CWorkThread();

	virtual void close();
	virtual unsigned int __stdcall svc(void* lpParam);

private:
	unsigned int GetPacketSize(std::vector<std::string> &vCol);
	
	/**@brief 申请返回包内存
	
	CommxHead包头，2H，以及包体部分的第一个H不包括在nSize里面，但是会被分配出来，返回的数据指针也不包括
	前面的固定格式的数据
	@param nSize 返回包的数据部分长度，不包括固定的包头等部分
	@param pReqHead 请求包的包头指针，返回包要复制请求包的包头
	@param func 请求类型
	@return 申请的数据指针，NULL表示内存分配失败
	 */
	char* Malloc(unsigned int nSize, CommxHead *pReqHead);
	char* do_sort(CommxHead *pHead);
	char* do_6001(CommxHead *pHead);
	char* do_6002(CommxHead *pHead);
	char* do_6003(CommxHead *pHead);
	char* do_6004(CommxHead *pHead);

	char* MakeSort(CommxHead *pHead, std::vector<_symbol_data> &vSymbol, std::vector<std::string> &vCol);
	char* MakeError(CommxHead *pHead, const char *lpMsg);

	void UpdateDatabase(bool bForce);			/// 更新数据库
	bool VisitComponent();						/// 查询指数成份股
	bool VisitMarketValue();					/// 查询个股的总市值和流通市值
	bool VisitVocation();						/// 查询行业名称，行业代码，行业所属股票(三级四级行业)，计算行业流通市值

	void CalcVocation(const char *group, std::vector<_symbol_data> &vSymbol);
	void SelGxd(const char *group, std::vector<_symbol_data> &vSymbol);

	void SelSymbols(const char *group, unsigned int nLength, std::vector<_symbol_data> &vSymbol);
	void OnTimer(DWORD dwNow);

	LockRW m_lock_zixun, m_lock_vocation, m_lock_vocsymbol;
	MAPZIXUN m_zixun;
	MAPVOCATION m_vocation;
	MAPVOCATIONSYMBOL m_vocsymbol;

	CommxHead m_head;					/// 专为退出线程预留
	CDataFarm m_farm;

	DWORD m_dwPreTick;
	unsigned int m_nUpdateDbDate;		/// 数据库更新日期
};

#endif
