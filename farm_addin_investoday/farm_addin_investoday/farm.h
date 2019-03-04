
#ifndef __QUOTE_FARM_DATA_FARM_H__
#define __QUOTE_FARM_DATA_FARM_H__

#include "config.h"
#include "../../public/mmapfile.h"
#include "../../public/hashtable.h"
#include "../../public/data_struct.h"
#include <string>

#define FILE_INDEX	"index.dat"			//�����ļ�
#define FILE_LEVEL0 "quote.dat"			//���龵���ļ�
#define FILE_TICK	"tick.dat"			//�ֱʳɽ��ļ�
#define FILE_MINK	"mink.dat"			//����K���ļ�
#define HISK_DAY	"day"				//��ʷ�ļ�dayĿ¼
#define HISK_WEK	"wek"				//��ʷ�ļ�wekĿ¼
#define HISK_MON	"mon"				//��ʷ�ļ�monĿ¼
#define HIS_DETAIL	"detail"			//��ʷ��ϸĿ¼

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
		char symbol[STOCK_CODE_LEN];	//symbol����
	};

	struct _findpy
	{
		std::string symbols;
		void OnBucket(std::string pydm, PYNODE pyNode);
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

	HASHIDX m_hashIdx;					/** Ʒ��������ϣ�� [5/22/2009 xinl] */
	HASHPY m_hashPy;					/** ƴ�������ϣ�� [5/22/2009 xinl] */
	MarketStatus *m_pMarketStatus;		/** �г������̽ṹ [5/22/2009 xinl] */

private:
	std::string converthztopy(char *as_hzstring);
	std::string GetHzpy(const char *lpSymbol);

	inline unsigned int GetNextQuote();
	inline unsigned int GetNextTick();
	inline unsigned int GetNextMink();

	IndexHead *m_pIndexHead;			/** �ļ�ͷ�ṹ�����ڹ����ڴ��У���Ϊ����tick�����Ϳ���mink������ʱ�仯����Ҫ��ʱ��Ӧ���ļ��� [5/22/2009 xinl] */

	CMMapFile<Quote> m_tbLevel0;		/** ����ʵʱ���� [5/22/2009 xinl] */
	CMMapFile<TickBlock> m_tbTick;		/** ����ֱʳɽ� [5/22/2009 xinl] */
	CMMapFile<MinBlock> m_tbMinK;		/** �������K�� [5/22/2009 xinl] */

#ifdef _WIN32
	HANDLE m_hFile;						/** map�ļ���� [5/22/2009 xinl] */
	HANDLE m_hMappingFile;				/** ӳ���� [5/22/2009 xinl] */
#elif defined _POSIX_C_SOURCE
	int m_fd;							/** �ļ������� [5/22/2009 xinl] */
#endif

	char* m_lpData;						/** ӳ����ڴ��׵�ַ [5/22/2009 xinl] */

	unsigned int m_nFarmOpenDate;		/** todo: �����յĵ�һ�ο������� [6/8/2009 xinl] */
};

#endif
