
#ifndef __QUOTE_FARM_DATA_FARM_H__
#define __QUOTE_FARM_DATA_FARM_H__

#include "config.h"
#include "../../public/mmapfile.h"
#include "../../public/hashtable.h"
#include "../../public/data_struct.h"
#include <string>
#include <map>

#define FILE_INDEX	"index.dat"			//索引文件
#define FILE_LEVEL0 "quote.dat"			//行情镜像文件
#define FILE_TICK	"tick.dat"			//分笔成交文件
#define FILE_MINK	"mink.dat"			//分钟K线文件
#define HISK_DAY	"day"				//历史文件day目录
#define HISK_WEK	"wek"				//历史文件wek目录
#define HISK_MON	"mon"				//历史文件mon目录
#define HIS_DETAIL	"detail"			//历史明细目录

template <>
struct Hash<char[STOCK_CODE_LEN]>
{
	static inline unsigned long GetHash(const char value[STOCK_CODE_LEN])
	{
		return from_blizzard::HashString(0, (char*)value);
	}
};

class CDataFarm
{
	struct PYNODE
	{
		char symbol[STOCK_CODE_LEN];	//symbol名称
	};

	struct _findpy
	{
		std::string symbols;
		void OnBucket(std::string pydm, PYNODE pyNode);
	};

	struct _maintain
	{
		_maintain();

		CDataFarm *pFarm;
		CConfig::CHKMKT *pChkmkt;
		time_t now;

		inline bool IsSameWeek(unsigned int uiNow, unsigned int uiPrev);
		inline bool IsSameMonth(unsigned int uiNow, unsigned int uiPrev);

		int OnElement(std::string *pSymbol, RINDEX *pIdx);

		void OpenSymbol(const char *lpSymbol, RINDEX *pIndex);
		void CloseSymbol(const char *lpSymbol, RINDEX *pIndex);
		void CloseSymbol(std::vector<RINDEX*> vri, unsigned int &cjsl, unsigned int &cjje);	/// 某些特殊symbol收盘时采用的特殊函数

		int CheckSymbol(const char *lpSymbol, RINDEX *pIndex);
	};

	typedef CHashTable<std::string, RINDEX, LockProcess, HashAllocateMMapFile<std::string, RINDEX> > HASHIDX;
	typedef CHashTable<std::string, PYNODE, LockProcess, HashAllocateMMapFile<std::string, PYNODE> > HASHPY;

public:
	CDataFarm();
	~CDataFarm();

	bool mapfile(const char* lpszFileName, unsigned int nMarketCount, unsigned int nBuckets, unsigned int nNodes, unsigned int nTickCount, unsigned int nMinkCount);
	void AddSymbolIndex(const char *lpSymbol, RINDEX *pIndex);
	void AddPydm(const char *lpSymbol, const char *lpStockName);
	void DeleteSymbolIndex(const char *lpSymbol, RINDEX *pIndex);
	std::string GetPySymbol(const char *lpPy);

	void AddTick(RINDEX *pIdx, TickUnit *ptick);
	void AddMink(RINDEX *pIdx, unsigned short now, unsigned int price, unsigned int volumn, unsigned int avg_price);

	inline Quote* GetQuote(unsigned int idx);
	inline TickBlock* GetTick(unsigned int idx);
	inline MinBlock* GetMinK(unsigned int idx);
	unsigned int GetHashSize();
	unsigned int Get5DayVol(const char *lpSymbol);
	unsigned int SeekEnd(FILE *fp, unsigned int nBlock);

	void OnTask(CConfig::CHKMKT &chkmkt, time_t now);

	/**@brief Farm自检

	Quote::zrsp,Quote::szStockName
	@param pMkts 需要自检的市场代码，多市场之间以'|'分隔
	- NULL 所有symbol都不参与自检，只检查最外层的结构，如哈希表等等
	@return
	- true 自检成功
	- false 自检出错误，函数通过REPORT报告错误信息
	*/
	bool SelfCheck(const char *pMkts = NULL);

	HASHIDX m_hashIdx;					/** 品种索引哈希表 [5/22/2009 xinl] */
	HASHPY m_hashPy;					/** 拼音代码哈希表 [5/22/2009 xinl] */
	MarketStatus *m_pMarketStatus;		/** 市场开收盘结构 [5/22/2009 xinl] */

private:
	std::string converthztopy(char *as_hzstring);
	std::string GetHzpy(const char *lpSymbol);
	void dumpHashIndex();
	void OnCloseSpecial();

	inline unsigned int GetNextQuote();
	inline unsigned int GetNextTick();
	inline unsigned int GetNextMink();

	void _rebuildQuoteIndex();

	IndexHead *m_pIndexHead;			/** 文件头结构，置于共享内存中，因为可用tick索引和可用mink索引随时变化，需要即时反应在文件中 [5/22/2009 xinl] */

	CMMapFile<Quote> m_tbLevel0;		/** 保存实时行情 [5/22/2009 xinl] */
	CMMapFile<TickBlock> m_tbTick;		/** 保存分笔成交 [5/22/2009 xinl] */
	CMMapFile<MinBlock> m_tbMinK;		/** 保存分钟K线 [5/22/2009 xinl] */

#ifdef _WIN32
	HANDLE m_hFile;						/** map文件句柄 [5/22/2009 xinl] */
	HANDLE m_hMappingFile;				/** 映射句柄 [5/22/2009 xinl] */
#elif defined _POSIX_C_SOURCE
	int m_fd;							/** 文件描述符 [5/22/2009 xinl] */
#endif

	char* m_lpData;						/** 映射的内存首地址 [5/22/2009 xinl] */

	unsigned int m_nFarmOpenDate;		/** todo: 交易日的第一次开盘日期 [6/8/2009 xinl] */
};

#endif
