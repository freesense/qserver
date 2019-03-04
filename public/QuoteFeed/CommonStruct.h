//CommonStruct.h
//
#pragma once
//////////////////////////////////////
#define  VERSION              0x03
#define  ENCODING             0x00
#define  BYTEORDER            0x00
#define  ROUTETARGET          0x00

#define  STOCK_TYPE_NORMAL    0x00
#define  STOCK_TYPE_NEW       0x01
#define  STOCK_TYPE_DELETE    0x02

//深行情库
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
#define  FD_JYDW				34
#define  FD_MGMZ				35

#define  FD_ZFXL				36
#define  FD_LTGS				37
#define  FD_SNLR				38
#define  FD_BNLR				39
#define  FD_JSFL				40

#define  FD_YHSL				41
#define  FD_GHFL				42
#define  FD_MBXL				43
#define  FD_BLDW				44
#define  FD_SLDW				45

#define  FD_JGDW				46
#define  FD_JHCS				47
#define  FD_LXCS				48
#define  FD_XJXZ				49
#define  FD_ZTJG				50

#define  FD_DTJG				51
#define  FD_ZHBL				52

#define  FD_AVERAGE             53

//上海行情库
//注意：没有  S12 S14 S20
#define  FD_S3                  FD_ZRSP
#define  FD_S4                  FD_JRKP
#define  FD_S5                  FD_CJJE
#define  FD_S6                  FD_ZGCJ
#define  FD_S7                  FD_ZDCJ

#define  FD_S8                  FD_ZJCJ
//新增
#define  FD_S9                  FD_BJW1
#define  FD_S10                 FD_SJW1
//
#define  FD_S11                 FD_CJSL
#define  FD_S13                 FD_SYL1
#define  FD_S15                 FD_BSL1
#define  FD_S16                 FD_BJW2

#define  FD_S17                 FD_BSL2
#define  FD_S18                 FD_BJW3
#define  FD_S19                 FD_BSL3
#define  FD_S21                 FD_SSL1
#define  FD_S22                 FD_SJW2

#define  FD_S23                 FD_SSL2
#define  FD_S24                 FD_SJW3
#define  FD_S25                 FD_SSL3
#define  FD_S26                 FD_BJW4
#define  FD_S27                 FD_BSL4

#define  FD_S28                 FD_BJW5
#define  FD_S29                 FD_BSL5
#define  FD_S30                 FD_SJW4
#define  FD_S31                 FD_SSL4
#define  FD_S32                 FD_SJW5

#define  FD_S33                 FD_SSL5

#define  PACK_HEADER_LEN          8
#define  RESERVE_LEN			  4
#define  SZ_MAX_FIELD            33
#define  SH_MAX_FIELD            28
#define  SZXX_MAX_FIELD          19
#define  HK_MAX_FIELD            28

#define  STOCK_NAME_LEN          17

#define  CRC_FIELD_LEN            sizeof(unsigned long)	//CRC检测码 int型
/*************************************************************************
1.报文头固定为8个字节
2.报文属性算法：Prop = ver << 4 | (encoding << 2) & 0x0e | byteorder & 0x01
  Ver：      协议版本号，目前暂定为0x03，修改协议时要相应的修改这个字段。
  Encoding： 字符串编码方式，0-ANSI/GBK，1-UTF8，2-UTF16LE。
  Byteorder：字节序。0-主机序，1-网络序。
  报文头和报文正文都必须遵循encoding和byteorder定义的格式。
3.路由目标目前暂定为0x00。
4.报文编号由请求方设置，行情平台在应答中设置该数值为请求方的编号。从0开始递增
5.报文长度不包括报文头长度。
*************************************************************************/
#pragma pack(push)
#pragma pack(1)
typedef struct tagPackHeader
{
	tagPackHeader()
	{
		byProp        = VERSION << 4 | (ENCODING << 2) & 0x0e | BYTEORDER & 0x01;
		byRouteTarget = ROUTETARGET;
		wPackNum      = 0; 
		ulPackSize    = 0;
	}

	unsigned char    byProp;          //报文属性
	unsigned char    byRouteTarget;   //路由目标
	unsigned short   wPackNum;		  //报文编号
	unsigned long    ulPackSize;      //报文大小，不包括报文头
}PACKHEADER;
	  
struct tagPackInfo     //普通股票需发送的信息
{
	unsigned short   wStockType;      //股票类型(0x00普通， 0x01新增， 0x02删除)      
	unsigned short   wTime;			  //时间（从0点开始计算，以分为单位	）
	unsigned short   wFieldCount;	  //行情数据个数,多少个字段需发送
	char             szMarketType[6]; //市场代码(sz sh)
	char             szStockCode[8];  //股票代码
};

struct tagNewPackInfo  //新增股票需发送的信息
{
	tagPackInfo      packinfo;
	char             szStockName[STOCK_NAME_LEN]; //股票名称
};

struct tagFieldInfo
{
	unsigned short   wFieldType;      //字段类型，按顺序编号 1-昨收 2-今开...
	unsigned int     nFieldValue;	  //字段值
};
#pragma pack(pop)