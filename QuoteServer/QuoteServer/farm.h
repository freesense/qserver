
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
		/// ĳЩ����symbol����ʱ���õ����⺯��
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
	HASHIDX m_hashIdx;					//Ʒ��������ϣ��
	HASHPY m_hashPy;					//ƴ�������ϣ��
	unsigned int m_unMarketCount;		//Add by wuqing 
	MarketStatus *m_pMarketStatus;		//�г������̽ṹ
public:
	std::string GetHzpy(const char *lpSymbol);
private:
	std::string converthztopy(char *as_hzstring);
	//std::string GetHzpy(const char *lpSymbol);
	void OnCloseSpecial();

	inline unsigned int GetNextQuote();
	inline unsigned int GetNextTick();
	inline unsigned int GetNextMink();

	IndexHead *m_pIndexHead;			//�ļ�ͷ�ṹ�����ڹ����ڴ��У���Ϊ����tick�����Ϳ���mink������ʱ�仯����Ҫ��ʱ��Ӧ���ļ���

	CMMapFile<Quote> m_tbLevel0;		//����ʵʱ����
	CMMapFile<TickBlock> m_tbTick;		//����ֱʳɽ�
	CMMapFile<MinBlock> m_tbMinK;		//�������K��

#ifdef _WIN32
	HANDLE m_hFile;						//�ļ����
	HANDLE m_hMappingFile;				//ӳ����
#elif defined _POSIX_C_SOURCE
	int m_fd;							//�ļ�������
#endif

	char* m_lpData;						//ӳ����ڴ��׵�ַ
};

#endif
