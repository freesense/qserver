
#ifndef __QUOTE_DATA_STRUCT_H__
#define __QUOTE_DATA_STRUCT_H__

#define STOCK_CODE_LEN	16
#define STOCK_NAME_LEN	16
#define STOCK_KIND_LEN	16
#define STOCK_PYDM_LEN	6
#define TICK_PERBLOCK	120
#define MINK_PERBLOCK	60
#define QUOTEHEAD		(STOCK_CODE_LEN+STOCK_NAME_LEN)
#define QUOTETAIL		(STOCK_KIND_LEN+sizeof(unsigned int)*4)

/// 定义行情源发送过来的数据的列标志
#define  FD_ZRSP                 1
#define  FD_JRKP                 2
#define  FD_ZJCJ                 3
#define  FD_CJSL                 4
#define  FD_CJJE                 5
#define  FD_CJBS                 6
#define  FD_ZGCJ                 7
#define  FD_ZDCJ                 8
#define  FD_SYL1                 9
#define  FD_SYL2                10
#define  FD_JSD1                11
#define  FD_JSD2                12
#define  FD_HYCC                13
#define  FD_SJW5                14
#define  FD_SSL5                15
#define  FD_SJW4                16
#define  FD_SSL4                17
#define  FD_SJW3                18
#define  FD_SSL3                19
#define  FD_SJW2                20
#define  FD_SSL2                21
#define  FD_SJW1                22
#define  FD_SSL1                23
#define  FD_BJW1                24
#define  FD_BSL1                25
#define  FD_BJW2                26
#define  FD_BSL2                27
#define  FD_BJW3                28
#define  FD_BSL3                29
#define  FD_BJW4                30
#define  FD_BSL4                31
#define  FD_BJW5                32
#define  FD_BSL5                33
#define  FD_XXJYDW				34
#define  FD_XXMGMZ				35
#define  FD_XXZFXL				36
#define  FD_XXLTGS				37
#define  FD_XXSNLR				38
#define  FD_XXBNLR				39
#define  FD_XXJSFL				40
#define  FD_XXYHSL				41
#define  FD_XXGHFL				42
#define  FD_XXMBXL				43
#define  FD_XXBLDW				44
#define  FD_XXSLDW				45
#define  FD_XXJGDW				46
#define  FD_XXJHCS				47
#define  FD_XXLXCS				48
#define  FD_XXXJXZ				49
#define  FD_XXZTJG				50
#define  FD_XXDTJG				51
#define  FD_XXZHBL				52
#define  FD_AVERAGE				53

#pragma pack(1)

//索引文件头结构
struct IndexHead
{
	unsigned char Version;				//版本号
	unsigned short nBucketCount;		//桶的数量
	unsigned short nSymbolCount;		//symbol数量
	unsigned int nTickCount;			//tick结构数量
	unsigned int nMinkCount;			//分钟k线数量

	unsigned int nQuoteIndex;			//可用Quote索引
	unsigned int nTickIndex;			//可用tick索引
	unsigned int nMinkIndex;			//可用mink索引

	unsigned int nIndexOffset;			//索引表偏移
	unsigned int nPyOffset;				//拼音索引偏移
};

//市场开收盘日期
struct MarketStatus
{
	char szMarket[STOCK_CODE_LEN];		//市场代码，sz,sh,hk...
	unsigned int dateOpen;				//开盘日期
	unsigned int dateClose;				//收盘日期
};

//行情结构
struct Quote
{
	char szStockCode[STOCK_CODE_LEN];	//品种代码
	char szStockName[STOCK_NAME_LEN];	//股票简称
	unsigned int zrsp;					//昨收
	unsigned int jrkp;					//今开
	unsigned int ztjg;					//涨停价
	unsigned int dtjg;					//跌停价
	unsigned int syl1;					//市盈率1
	unsigned int syl2;					//市盈率2
	unsigned int zgjg;					//最高价格
	unsigned int zdjg;					//最低价格
	unsigned int zjjg;					//最近成交价
	unsigned int zjcj;					//最近成交量
	unsigned int cjsl;					//总成交量
	unsigned int cjje;					//成交金额
	unsigned int cjbs;					//成交笔数
	unsigned int BP1;
	unsigned int BM1;
	unsigned int BP2;
	unsigned int BM2;
	unsigned int BP3;
	unsigned int BM3;
	unsigned int BP4;
	unsigned int BM4;
	unsigned int BP5;
	unsigned int BM5;
	unsigned int SP1;
	unsigned int SM1;
	unsigned int SP2;
	unsigned int SM2;
	unsigned int SP3;
	unsigned int SM3;
	unsigned int SP4;
	unsigned int SM4;
	unsigned int SP5;
	unsigned int SM5;
	unsigned int day5pjzs;				//5日平均总手
	unsigned int pjjg;					//均价
	int wb;								//委比
	unsigned int lb;					//量比
	unsigned int np;					//内盘
	unsigned int wp;					//外盘
	int zdf;							//涨跌幅
	int zf;								//振幅

	char szStockKind[STOCK_KIND_LEN];	//品种类别，类似于md_szag,......
	unsigned int flag[4];				//行情变化标志
};

//分笔成交结构
struct TickUnit
{
	unsigned int Time;					//时间，格式：HHMM
	unsigned int Price;					//成交价
	unsigned int Volume;				//成交量
	unsigned char Way;					//成交方向，0-未知，1-内盘，2-外盘
};

struct TickBlock
{
	TickUnit unit[TICK_PERBLOCK];
	unsigned int next;
};

//内存分钟K线结构
struct MinUnit
{
	unsigned int Time;					//时间，格式：HHMM
	unsigned int OpenPrice;
	unsigned int MaxPrice;
	unsigned int MinPrice;
	unsigned int NewPrice;
	unsigned int Volume;
	unsigned int AvgPrice;				//均价
};

struct MinBlock
{
	MinUnit unit[MINK_PERBLOCK];
	unsigned int next;
};

struct RINDEX
{
	int idxQuote;				//行情索引
	int idxTick;				//分笔成交索引
	int cntTick;				//分笔成交个数
	int idxMinK;				//分钟K线索引
	int cntMinK;				//分钟K线个数
};

/**@brief 收盘结构

由于k线历史文件交给专门的dll来做，因此定义本结构作为dll与主程序之间数据传递的基本单位
 */
struct dayk
{
	char szStockCode[STOCK_CODE_LEN];	//品种代码
	unsigned int day;					/** 收盘日期 [2/25/2010 xinl] */
	unsigned int open;					//开盘价格
	unsigned int high;					//最高价
	unsigned int low;					//最低价
	unsigned int close;					//当日收盘价
	unsigned int volume;				//当日成交量
	unsigned int amount;				//当日成交金额
	double scale;						/** 当日该股还权因子 [2/25/2010 xinl] */
};

#pragma pack()

#endif
