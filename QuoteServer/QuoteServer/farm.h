
#ifndef __QUOTE_FARM_DATA_FARM_H__
#define __QUOTE_FARM_DATA_FARM_H__

#include "config.h"
#include "../../public/mmapfile.h"
#include "../../public/hashtable.h"
#include "../../public/data_struct.h"
#include <string>

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
		struct k_line
		{
			unsigned int day;
			unsigned int open;
			unsigned int high;
			unsigned int low;
			unsigned int close;
			unsigned int volume;
			unsigned int amount;
		};

		CDataFarm *pFarm;
		CConfig::CHKMKT *pChkmkt;
		time_t now;

		inline bool IsSameWeek(unsigned int uiNow, unsigned int uiPrev);
		inline bool IsSameMonth(unsigned int uiNow, unsigned int uiPrev);
		inline unsigned int SeekEnd(FILE *fp, unsigned int nBlock);

		int OnElement(std::string *pSymbol, RINDEX *pIdx);
		unsigned int Get5DayVol(const char *lpSymbol);
		void OpenSymbol(const char *lpSymbol, RINDEX *pIndex);

		void CloseSymbol(const char *lpSymbol, RINDEX *pIndex);
		/// 某些特殊symbol收盘时采用的特殊函数
		void CloseSymbol(std::vector<RINDEX*> vri, unsigned int &cjsl, unsigned int &cjje);
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
	void AddMink(RINDEX *pIdx, unsigned short now, unsigned int price, unsigned int volumn);

	inline Quote* GetQuote(unsigned int idx);
	inline TickBlock* GetTick(unsigned int idx);
	inline MinBlock* GetMinK(unsigned int idx);
	unsigned int GetHashSize();

	void OnTask(CConfig::CHKMKT &chkmkt, time_t now);

	MarketStatus * GetMarketStatus(char * c_pSymbol)//Add by wuqing
	{
		unsigned int un;
		for(un=0; un<m_unMarketCount; un++)
		{
			if (stricmp(c_pSymbol,(m_pMarketStatus + un)->szMarket) == 0)
				return (m_pMarketStatus + un);
		}
		return NULL;
	}
	HASHIDX m_hashIdx;					//品种索引哈希表
	HASHPY m_hashPy;					//拼音代码哈希表
	unsigned int m_unMarketCount;		//Add by wuqing 
	MarketStatus *m_pMarketStatus;		//市场开收盘结构
public:
	std::string GetHzpy(const char *lpSymbol);
private:
	std::string converthztopy(char *as_hzstring);
	//std::string GetHzpy(const char *lpSymbol);
	void OnCloseSpecial();

	inline unsigned int GetNextQuote();
	inline unsigned int GetNextTick();
	inline unsigned int GetNextMink();

	IndexHead *m_pIndexHead;			//文件头结构，置于共享内存中，因为可用tick索引和可用mink索引随时变化，需要即时反应在文件中

	CMMapFile<Quote> m_tbLevel0;		//保存实时行情
	CMMapFile<TickBlock> m_tbTick;		//保存分笔成交
	CMMapFile<MinBlock> m_tbMinK;		//保存分钟K线

#ifdef _WIN32
	HANDLE m_hFile;						//文件句柄
	HANDLE m_hMappingFile;				//映射句柄
#elif defined _POSIX_C_SOURCE
	int m_fd;							//文件描述符
#endif

	char* m_lpData;						//映射的内存首地址
};

#endif
