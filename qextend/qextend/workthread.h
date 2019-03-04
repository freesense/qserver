
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

#define	RPT_HEARTBEAT  0xAA	//������־����������߳�

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
	int reserved1;					// ��������֮����ֶ�
	double reserved2;				// ��������֮����ֶ�
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

	std::string sort_colname;		// �����������
	int nAscending;					// ����or����
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
	// symbol��Ѷ
	struct _symbol_zixun
	{
		double fMarketValue;			//����ֵ
		double fFlowValue;				//��ͨ��ֵ
		double fFlowShare;				//��ͨ��
	};

	// ��ҵ�б�
	struct _vocation_list
	{
		std::string strVocationCode;	//��ҵ����
		std::string strVocationName;	//��ҵ����
		double fMarketValue;			//��ҵ��ֵ
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
	
	/**@brief ���뷵�ذ��ڴ�
	
	CommxHead��ͷ��2H���Լ����岿�ֵĵ�һ��H��������nSize���棬���ǻᱻ������������ص�����ָ��Ҳ������
	ǰ��Ĺ̶���ʽ������
	@param nSize ���ذ������ݲ��ֳ��ȣ��������̶��İ�ͷ�Ȳ���
	@param pReqHead ������İ�ͷָ�룬���ذ�Ҫ����������İ�ͷ
	@param func ��������
	@return ���������ָ�룬NULL��ʾ�ڴ����ʧ��
	 */
	char* Malloc(unsigned int nSize, CommxHead *pReqHead);
	char* do_sort(CommxHead *pHead);
	char* do_6001(CommxHead *pHead);
	char* do_6002(CommxHead *pHead);
	char* do_6003(CommxHead *pHead);
	char* do_6004(CommxHead *pHead);

	char* MakeSort(CommxHead *pHead, std::vector<_symbol_data> &vSymbol, std::vector<std::string> &vCol);
	char* MakeError(CommxHead *pHead, const char *lpMsg);

	void UpdateDatabase(bool bForce);			/// �������ݿ�
	bool VisitComponent();						/// ��ѯָ���ɷݹ�
	bool VisitMarketValue();					/// ��ѯ���ɵ�����ֵ����ͨ��ֵ
	bool VisitVocation();						/// ��ѯ��ҵ���ƣ���ҵ���룬��ҵ������Ʊ(�����ļ���ҵ)��������ҵ��ͨ��ֵ

	void CalcVocation(const char *group, std::vector<_symbol_data> &vSymbol);
	void SelGxd(const char *group, std::vector<_symbol_data> &vSymbol);

	void SelSymbols(const char *group, unsigned int nLength, std::vector<_symbol_data> &vSymbol);
	void OnTimer(DWORD dwNow);

	LockRW m_lock_zixun, m_lock_vocation, m_lock_vocsymbol;
	MAPZIXUN m_zixun;
	MAPVOCATION m_vocation;
	MAPVOCATIONSYMBOL m_vocsymbol;

	CommxHead m_head;					/// רΪ�˳��߳�Ԥ��
	CDataFarm m_farm;

	DWORD m_dwPreTick;
	unsigned int m_nUpdateDbDate;		/// ���ݿ��������
};

#endif
