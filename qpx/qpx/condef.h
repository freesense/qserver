//condef.h
#ifndef _CONDEF_H_
#define _CONDEF_H_
#include "../../public/protocol.h"

#pragma warning(disable:4819)
#pragma warning(disable:4267)
#pragma warning(disable:4311)
#pragma warning(disable:4312)
////////////////////////////////////////////////////////////////////////////////////////

//报文类型
#define PT_REAL_MINS         0x0004		//分时走势
#define PT_QUOTEPRICE        0x0003		//报价表
#define PT_HKDATA_DAY		 0x0005		//日线
#define PT_HKDATA_WEEK		 0x0006		//周线
#define PT_HKDATA_MONTH		 0x0007		//月线
#define PT_SORTDATA		     0x1770		//排行榜
#define PT_INIT  		     0x0001		//初始化

#define CODE_LEN             6
#define NAME_LEN             12
#define NEWREALMIN           0x0700     //新分时协议
#define REALMINSDATA         0x0400 //测试
#define REPORTDATA           0x0200     //自选股报价
#define STATDATA             0x0300     //个股统计信息
#define HISKDATA             0x0600     //历史K线数据
#define SORTDATA             0x0800     //排行榜
#define REALMINSTAT          0x0B00     //新分时统计协议
#define REALMINOVERLAP       0x0F00     //叠加分时协议
#define INITMARKET           0x1000     //市场初始化


const	unsigned short	LISTCODE=0x0A00;//代码列举
const	unsigned short	HK_Stock		=0x2004;
const	unsigned int	MULTIPLE=10000;//数据的放大倍数
const	float			ROUND=(float)0.00005;//用以取值时四舍五入

const	unsigned char	CH_CHAR=0x40;//为字符字段
const	unsigned char	CH_PRICE=0x10;//为价格字段
const	unsigned char	CH_VOLUME=0x20;//为成交量字段
const	unsigned char	CH_FLOAT=0x04;//为小数字段
const	unsigned char	CH_TIME=0x08;//为时间字段
const	unsigned char	CH_BARGIN=0x02;//为成交字段
const	unsigned char	CH_NONE=0;//该字段不用

const	unsigned short	DAY2REALMIN=0x0D00;//两日分时走势
const	unsigned int	MAX_STRUCTSIZE=102400;//结构体所占用的最大空间
//排行榜子类定义
const	unsigned char	CODE_SORT=0;//代码顺序排行榜
const	unsigned char	RISE_SORT=1;//涨幅排行榜
const	unsigned char	SUM_SORT=4;//成交金额排行榜
const	unsigned char	VOLUME_SORT=6;//成交量排行榜
const	unsigned char	SWING_SORT=2;//振幅排行榜
const	unsigned char	VOLRATIO_SORT=3;//量比排行榜
const	unsigned char	BUYSELL_SORT=5;//委比排行榜
const	unsigned char	PRICE_SORT=7;//成交价排行榜
const	unsigned char	PRICEASSET_SORT=8;//市净率排行榜
const	unsigned char	VOLSHARE_SORT=9;//换手率排行榜
const	unsigned char	PRICEYIELD_SORT=10;//市盈率排行榜

const	unsigned char	DAY_HISK=61;//日K线
const	unsigned char	WEEK_HISK=62;//周K线
const	unsigned char	MONTH_HISK=63;//月K线

const	unsigned short	ST_Index		=0x01;//指数
const	unsigned short	ST_Bond			=0x02;//债券
const	unsigned short	ST_Fund			=0x03;//基金
const	unsigned short	ST_AStock		=0x04;//A股
const	unsigned short	ST_BStock		=0x05;//B股
const	unsigned short	ST_Other		=0x06;//其它

const	unsigned short	SH_Index		=0x1001;
const	unsigned short	SH_Bond			=0x1002;
const	unsigned short	SH_Fund			=0x1003;
const	unsigned short	SH_AStock		=0x1004;
const	unsigned short	SH_BStock		=0x1005;
const	unsigned short	SH_Other		=0x1006;

const	unsigned short	SZ_Index		=0x1011;
const	unsigned short	SZ_Bond			=0x1012;
const	unsigned short	SZ_Fund			=0x1013;
const	unsigned short	SZ_AStock		=0x1014;
const	unsigned short	SZ_BStock		=0x1015;
const	unsigned short	SZ_Other		=0x1016;
////////////////////////////////////////////////////////////////////
#pragma pack(push)
#pragma pack(1)

static unsigned short	reverse_s(unsigned short	usData)
{
	return	(usData>>8)+((usData & 0xff)<<8);
}

static unsigned int	reverse_i(unsigned int	uiData)
{
	unsigned char	aucData[4];

	int i;
	for( i=0;i<4;i++)
		aucData[i]=uiData>>((3-i)*8);
	uiData=0;

	for(i=0;i<4;i++)
		uiData+=(aucData[i]<<(i*8));
	return	uiData;
}

struct SMHeader
{
	unsigned int nSize;		//网络字节顺序
	unsigned int nReserve;
	unsigned short HostToNet(unsigned char*	pucData)
	{
		//转换成网络字节顺序
		unsigned short	usSize=0;
		((unsigned	int*)(pucData+usSize))[0]=reverse_i(nSize);
		usSize+=sizeof(unsigned	int);

		((unsigned	int*)(pucData+usSize))[0]=reverse_i(nReserve);
		usSize+=sizeof(unsigned	int);

		return usSize;
	}
};

//供客户端使用
struct	SMCodeInfo{
	unsigned	short	m_usMarketType;//市场标识
	char				m_acCode[6];//股票代码或者拼音简称
	char				m_acName[12];//名称
	char				m_ggpj[12];//个股评级

	void NetToHost()//把结构体中数据转换成主机顺序
	{
		m_usMarketType = reverse_s(m_usMarketType);
	}

	unsigned	short	NetToHost(unsigned char*	pucData)
	{
		//转换成主机字节顺序
		unsigned short	usSize=0;
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketType);
		usSize+=sizeof(unsigned	short);

		int i;
		for( i=0;i<CODE_LEN;i++)
			(pucData+usSize)[i]=m_acCode[i];

		usSize+=CODE_LEN;

		for(i=0;i<NAME_LEN;i++)
			(pucData+usSize)[i]=m_acName[i];

		usSize += NAME_LEN;
		for(i=0;i<sizeof(m_ggpj);i++)
			(pucData+usSize)[i]=m_ggpj[i];

		return	usSize+/*NAME_LEN+*/sizeof(m_ggpj);
	}

	unsigned	short	HostToNet(unsigned char*	pucData)
	{
		//转换成网络字节顺序
		unsigned short	usSize=0;
		//		unsigned char	ucLen=6;

		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketType);
		usSize+=sizeof(unsigned	short);

		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		//		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize += NAME_LEN;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		return	usSize+/*NAME_LEN+*/sizeof(m_ggpj);
	}
};

//客户端请求数据结构
struct	SMRequestData{
	unsigned	short	m_usType;//请求类型
	unsigned	short	m_usMarketType;//市场标识
	unsigned	short	m_usIndex;//请求索引
	unsigned	short	m_usMobileType;//手机型号
	char				m_acRight[32];//权限描述符
	//请求证券总数,即结构CodeInfo的个数,在请求K线和成交明细数据时则表示K线和成交明细的个数
	unsigned	short	m_usSize;
	SMCodeInfo*		m_pstCode;//在请求K线和成交明细时将请求起始位置值赋给它

	unsigned	short	NetToHost(unsigned char*	pucData)
	{
		//转换成主机字节顺序
		unsigned short	usSize=0;
		unsigned short* pusData=&m_usType;

		int i;
		for( i=0;i<4;i++)
		{
			((unsigned short*)pucData)[i]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		memcpy(pucData+usSize,m_acRight,32);
		usSize+=32;
		((unsigned short*)(pucData+usSize))[0]=reverse_s(m_usSize);
		usSize+=sizeof(unsigned	short);
/*
		//如果这里使用m_pstCode的话会莫名其妙地向后偏移2字节
		//因此我们直接访问请求缓冲区，求得正确的SMCodeInfo数据
		char *pTemp = ((char*)&m_usSize) + sizeof(unsigned short);
//		((unsigned	int*)(pucData+usSize))[0]=reverse_i((unsigned int)&pTemp);
//		SMCodeInfo*	pstMCodeInfo=(SMCodeInfo*)(((char*)&m_pstCode)+sizeof(SMCodeInfo*));
		pTemp += sizeof(void*);
		SMCodeInfo*	pstMCodeInfo=(SMCodeInfo*)pTemp;
*/
		((unsigned	int*)(pucData+usSize))[0]=reverse_i((unsigned int)m_pstCode);
		SMCodeInfo*	pstMCodeInfo=(SMCodeInfo*)(((unsigned int)&m_pstCode)+sizeof(SMCodeInfo*));

		usSize=sizeof(SMRequestData);
		unsigned short	usUnits=reverse_s(m_usSize);

		if(0xffffffff != (unsigned int)m_pstCode)
			usUnits=1;

		for(i=0;i<usUnits;i++)
			usSize+=pstMCodeInfo[i].NetToHost(pucData+usSize);

		return	usSize;
	}

	unsigned	short	HostToNet(unsigned char*	pucData)
	{
		//转换成网络字节顺序
		unsigned short	usSize=0;
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usType);
		usSize+=sizeof(unsigned	short);
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketType);
		usSize+=sizeof(unsigned	short);
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usIndex);
		usSize+=sizeof(unsigned	short);
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMobileType);
		usSize+=sizeof(unsigned	short);
		memset(pucData+usSize, 0, 32);
		usSize+=32;
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usSize);
		usSize+=sizeof(unsigned	short);
		usSize+=4;

		return	usSize;
	}

	void NetToHost()//把结构体中数据转换成主机顺序
	{
		m_usType       = reverse_s(m_usType);
		m_usMarketType = reverse_s(m_usMarketType);
		m_usIndex      = reverse_s(m_usIndex);
		m_usMobileType = reverse_s(m_usMobileType);
		m_usSize       = reverse_s(m_usSize);
	}
};

//当日证券成交数据，用于画价格线和成交量线
struct	SRealMinsUnit{//以分钟为单位生成
	unsigned	short	m_usTime;//距离第一次开盘时间
	unsigned	int		m_uiNewPrice;
	unsigned	int		m_uiVolume;
	unsigned	int		m_uiAverage;//平均价格
	/*unsigned	char	m_ucStructSize;//结构长度
	unsigned	int		m_usTime;//距离第一次开盘时间
	unsigned	int		m_uiNewPrice;
	unsigned	int		m_uiUpMargin;//涨跌
	unsigned	int		m_uiVolume;
	unsigned	int		m_uiAverage;//平均价格
	unsigned	int		m_uiVolRatio;//量比值
	unsigned	int		m_uiSellVol;//委卖手数
	unsigned	int		m_uiBuyVol;//委买手数
	unsigned	int		m_uiKeepVol;//持仓量,适用于期货市场
	*/
	unsigned	short	HostToNet(unsigned char*	pucData, unsigned short usType)
	{//转换成网络字节顺序
		if (usType == REALMINSDATA || usType == DAY2REALMIN)
		{			
			char* pBufPos = (char*)pucData;
			memcpy(pBufPos, &m_usTime, sizeof(unsigned	short));
			pBufPos += sizeof(unsigned	short);
			memcpy(pBufPos, &m_uiNewPrice, sizeof(unsigned	int));
			pBufPos += sizeof(unsigned	int);
			memcpy(pBufPos, &m_uiVolume, sizeof(unsigned	int));
			pBufPos += sizeof(unsigned	int);
			memcpy(pBufPos, &m_uiAverage, sizeof(unsigned	int));
			pBufPos += sizeof(unsigned	int);

			unsigned short	usSize = sizeof(unsigned	short) + sizeof(unsigned int)*3;
			return	usSize;
			/*pucData[0]=sizeof(SRealMinsUnit);
			unsigned short	usSize=sizeof(unsigned	char);
			unsigned int*	puiData=&m_usTime;
			// ???
			if(m_uiVolume < 0)
				m_uiVolume = 0;
			// ???
			
			int i;
			for( i=0;i<9;i++)
			{
				((unsigned	int*)(pucData+usSize))[0]=reverse_i(puiData[i]);
				usSize+=sizeof(unsigned	int);
			}
			return	usSize;*/
		}
		else if (usType == REALMINOVERLAP)
		{
			unsigned short usTime = (unsigned short)m_usTime;
			((unsigned short*)pucData)[0] = reverse_s(usTime);
			((unsigned int*)(pucData+2))[0] = reverse_i(m_uiNewPrice);
			return sizeof(unsigned int) + 2;
		}
		else
		{
			unsigned short usTime = (unsigned short)m_usTime;
			((unsigned short*)pucData)[0] = reverse_s(usTime);

			((unsigned int*)(pucData+2))[0] = reverse_i(m_uiNewPrice);
			((unsigned int*)(pucData+2))[1] = reverse_i(m_uiVolume);
			((unsigned int*)(pucData+2))[2] = reverse_i(m_uiAverage);
			return sizeof(unsigned int) * 3 + 2;
		}
	}
	static unsigned short	GetBufNeedSize(unsigned short usSize, unsigned short usType)
	{
		if (usType == NEWREALMIN || usType == DAY2REALMIN)
			return usSize * 37;  //37是SRealMinsUnit的大小
		return 0;
	}
};

//分钟成交走势
struct	SRealMinsData{
	unsigned	char	m_ucStructSize;//结构长度
	unsigned	short	m_usType;//响应类型，与请求数据包类型一致
	unsigned	short	m_usIndex;//请求索引，与请求数据包类型一致
	unsigned	short	m_usMarketType;//市场标识
	char				m_acName[12];//名称
	char				m_acCode[6];//代码
	char				m_ggpj[12];
	unsigned	int		m_uiPrevClose;//昨收价
	unsigned	short	m_usFirstOpen;//开市时间;
	unsigned	short	m_usFirstClose;//闭市时间;
	unsigned	short	m_usSecondOpen;//开市时间;
	unsigned	short	m_usSecondClose;//闭市时间;
	unsigned	short	m_usMinsUnits;//结构RealDealUnit的个数
	unsigned	char	m_ucUnits;//成员m_pcTitle的元素个数
	unsigned	char*	m_pcTitle;
	SRealMinsUnit*		m_pstMinsUnit;
	unsigned	short	HostToNet(unsigned char*	pucData, unsigned short usType)
	{//转换成网络字节顺序
		char*	pcTitle[]={"时间","最新","涨跌","均值","手数","","","","","",};
		unsigned char	aucDescribe[]={CH_TIME,CH_PRICE,CH_PRICE,CH_PRICE,CH_VOLUME,
			CH_PRICE,CH_FLOAT,CH_VOLUME,CH_VOLUME,CH_NONE};
		pucData[0]=sizeof(SRealMinsData);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;
		int i;
		for( i=0;i<3;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize+=NAME_LEN;
		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		usSize+=sizeof(m_ggpj);
		((unsigned	int*)(pucData+usSize))[0]=reverse_i(m_uiPrevClose);
		usSize+=sizeof(unsigned	int);
		pusData=&m_usFirstOpen;

		for(i=0;i<5;i++)
		{	((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
		usSize+=sizeof(unsigned	short);
		}

		m_ucUnits=10;
		(pucData+usSize)[0]=m_ucUnits;
		usSize+=sizeof(unsigned	char);

		for(i=0;i<m_ucUnits;i++)
		{
			ucLen=(unsigned char)strlen(pcTitle[i]);
			//			code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
				ucLen,(LPWSTR)(pucData+usSize),ucLen);
			usSize+=ucLen;
			pucData[usSize]='|';
			pucData[usSize+1]=aucDescribe[i];
			usSize+=2;
		}

		for(i=0;i<m_usMinsUnits;i++)
		{
			usSize+=m_pstMinsUnit[i].HostToNet(pucData+usSize, usType);
		}

		return	usSize;
	}
	unsigned short	GetSize(unsigned short	usSize)
	{
		return	40+sizeof(SRealMinsData)+usSize*sizeof(SRealMinsUnit);
	}

	static unsigned short	GetBufNeedSize(unsigned short usSize, unsigned short usType)
	{
		if (usType == NEWREALMIN  || usType == DAY2REALMIN)
			return 98 + usSize * sizeof(SRealMinsUnit);  //98是SRealMinsData的大小 	 */
		return 0;  
	}
};

//交易品种K线数据结构
struct	SHisKUnit{
	unsigned	int		m_uiDate;
	unsigned	int		m_uiOpenPrice;
	unsigned	int		m_uiMaxPrice;
	unsigned	int		m_uiMinPrice;
	unsigned	int		m_uiClosePrice;
	unsigned	int		m_uiVolume;
	unsigned	int		m_uiSum;//成交金额(持仓量)

	unsigned	short	HostToNet(unsigned char*	pucData)
	{//转换成网络字节顺序
		pucData[0]=sizeof(SHisKUnit)-sizeof(unsigned int)+sizeof(unsigned char);
		unsigned short	usSize=sizeof(unsigned char);
		unsigned int*	puiData=&m_uiDate;

		int i;
		for( i=0;i<6;i++)
		{
			((unsigned	int*)(pucData+usSize))[0]=reverse_i(puiData[i]);
			usSize+=sizeof(unsigned	int);
		}

		return	usSize;
	}
};

//历史K线数据
struct	SHisKData{
	unsigned	char	m_ucStructSize;//结构长度
	unsigned	short	m_usType;//响应类型，与请求数据包类型一致
	unsigned	short	m_usIndex;//请求索引，与请求数据包类型一致
	unsigned	short	m_usMarketType;//市场标识
	char				m_acName[12];//名称
	char				m_acCode[6];//代码
	char				m_ggpj[12];
	unsigned	char	m_ucUnits;//成员m_pcTitle的字符长度
	unsigned	char*	m_pcTitle;
	unsigned	short	m_usSize;//结构HisKUnit的个数
	SHisKUnit*			m_pstHisKUnit;

	unsigned	short	HostToNet(unsigned char*	pucData)
	{//转换成网络字节顺序
		char*	pcTitle[]={"时间","开盘","最高","最低","收盘","成交量",};
		unsigned char	aucDescribe[]={CH_TIME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,CH_VOLUME,};
		pucData[0]=sizeof(SHisKData);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i=0;
		for(;i<3;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize+=NAME_LEN;
		ucLen=CODE_LEN;
		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		usSize+=sizeof(m_ggpj);
		m_ucUnits=6;
		pucData[usSize]=m_ucUnits;
		usSize+=sizeof(unsigned char);

		for(i=0;i<m_ucUnits;i++)
		{
			ucLen=(unsigned char)strlen(pcTitle[i]);
			//			code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
				ucLen,(LPWSTR)(pucData+usSize),ucLen);
			usSize+=ucLen;
			pucData[usSize]='|';
			pucData[usSize+1]=aucDescribe[i];
			usSize+=2;
		}

		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usSize);
		usSize+=sizeof(unsigned	short);

		for(i=0;i<m_usSize;i++)
			usSize+=m_pstHisKUnit[i].HostToNet(pucData+usSize);

		return	usSize;
	}
	unsigned short	GetSize(unsigned short	usSize)
	{
		return	26+sizeof(SHisKData)+usSize*sizeof(SHisKUnit);
	}

	unsigned short	GetBufNeedSize(unsigned short	usSize)
	{
		return 84 + usSize * 25;  //84是SHisKData的大小 25是SHisKUnit的大小
	}
};

// 排行榜数据
struct	SSortUnit{
	unsigned	char	m_ucStructSize;//结构长度
	unsigned short m_usMarketCode;
	char				m_acName[12];//名称
	char				m_acCode[6];//代码
	char				m_ggpj[12];
	unsigned	int		m_uiPrevClose;//昨收价
	unsigned	int		m_uiNewPrice;//最新价
	unsigned	int		m_uiUpMargin;//涨幅
	unsigned	int		m_uiVolume;//成交量总手
	unsigned	int		m_uiCurVol;//成交量现手
	unsigned	int		m_uiSum;//成交金额(持仓量)
	unsigned	int		m_uiBuyPrice;//委买价
	unsigned	int		m_uiSellPrice;//委卖价
	unsigned	int		m_uiOpenPrice;//今开盘
	unsigned	int		m_uiMaxPrice;//最高价
	unsigned	int		m_uiMinPrice;//最低价
	unsigned	int		m_uiVolRatio;//量比(开仓)
	int					m_iEntrustRatio;//委比(平仓)
	unsigned	int		m_uiPriceYield;//市应率
	unsigned	int		m_uiPriceAsset;//市净率
	unsigned	int		m_uiVolShare;//换手绿
	unsigned	short	HostToNet(unsigned char*	pucData,unsigned char	ucType)
	{//转换成网络字节顺序
		unsigned short	usSize=sizeof(m_ucStructSize);

		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketCode);
		usSize += sizeof(unsigned short);

		//		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize+=NAME_LEN;
		//		ucLen=CODE_LEN;
		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		usSize+= sizeof(m_ggpj);
		unsigned int*	puiData=(unsigned	int*)(pucData+usSize);
		int	nIndex=0;
		puiData[nIndex++]=m_uiPrevClose;
		switch(ucType)
		{
			// 所有排行榜中的数据 从 title  中的第二项中开始对应
		case CODE_SORT: // 代码顺序排行榜
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiUpMargin;//涨幅
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//振幅
			break;
			// 相对应的 title
			//"名称"
			//"开盘"
			//"成交"
			//"涨幅"
			//"总手"
			//"现手"
			//"最高"
			//"最低"
			//"买入"
			//"卖出"
			//"振幅"
		case RISE_SORT: // 涨幅排行榜
			//puiData[nIndex++]=(int)((((float)m_uiNewPrice-m_uiPrevClose)/m_uiPrevClose+ROUND)*MULTIPLE);//涨幅
			puiData[nIndex++]=m_uiUpMargin;
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//涨跌
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			break;
		case SUM_SORT: // 成交金额排行榜
			puiData[nIndex++]=m_uiSum;//金额
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//涨跌
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			break;
		case VOLUME_SORT: // 成交量排行榜
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//涨跌
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//振幅
			break;
		case SWING_SORT: // 振幅排行榜
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//振幅
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//涨跌
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			break;
		case VOLRATIO_SORT: // 量比排行榜
			puiData[nIndex++]=m_uiVolRatio;//量比
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//涨跌
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			break;
		case BUYSELL_SORT: // 委比排行榜
			puiData[nIndex++]=m_iEntrustRatio;//委比
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//涨跌
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			break;
		case PRICE_SORT: // 成交价排行榜
			puiData[nIndex++]=m_uiNewPrice;//成交
			puiData[nIndex++]=m_uiOpenPrice;//开盘
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//振幅
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//涨跌
			puiData[nIndex++]=m_uiVolume;//总手
			puiData[nIndex++]=m_uiCurVol;//现手
			puiData[nIndex++]=m_uiMaxPrice;//最高
			puiData[nIndex++]=m_uiMinPrice;//最低
			puiData[nIndex++]=m_uiBuyPrice;//买入
			puiData[nIndex++]=m_uiSellPrice;//卖出
			break;
		}

		int i;
		for( i=0;i<nIndex;i++)
			puiData[i]=reverse_i(puiData[i]);

		usSize+=(unsigned short)sizeof(unsigned	int)*nIndex;
		pucData[0]=(unsigned char)(sizeof(unsigned char)+nIndex*sizeof(unsigned int)+CODE_LEN*2+NAME_LEN+sizeof(m_ggpj));
		return	usSize;
	}

	static unsigned short GetBufNeedSize(unsigned char	ucType)
	{
		if(CODE_SORT == ucType)
			return 83;  // 83是SSortUnit的大小
		else
			return 0;
	}
};

struct	SSortData{
	unsigned	char	m_ucStructSize;//结构长度
	unsigned	short	m_usType;//响应类型，与请求数据包类型一致
	unsigned	short	m_usIndex;//请求索引，与请求数据包类型一致
	unsigned	short	m_usSize;//结构SortUnit的个数
	unsigned	short	m_usTotal;//排序股票的总数目
	unsigned	char	m_ucUnits;//成员m_pcTitle的元素个数
	unsigned	char*	m_pcTitle;
	SSortUnit*			m_pstSortUnit;
	unsigned	short	HostToNet(unsigned char*	pucData,unsigned char	ucType)
	{//转换成网络字节顺序
		pucData[0]=sizeof(SHisKData)-sizeof(SSortUnit*);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i;
		for( i=0;i<4;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		m_ucUnits=11;
		(pucData+usSize)[0]=m_ucUnits;
		usSize+=sizeof(unsigned	char);

		unsigned char	ucLen;

		for(i=0;i<m_ucUnits;i++)
		{
			switch(ucType)
			{
			case CODE_SORT://代码顺序排行榜
				{	char*	pcTitle[]={"名称","开盘","成交","涨幅","总手",
					"现手","最高","最低","买入","卖出","振幅",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_PRICE,CH_PRICE,CH_FLOAT,CH_VOLUME,
					CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,CH_FLOAT,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case RISE_SORT://涨幅排行榜
				{	char*	pcTitle[]={"名称","涨幅","开盘","成交","涨跌",
					"总手","现手","最高","最低","买入","卖出",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case SUM_SORT://成交金额排行榜
				{	char*	pcTitle[]={"名称","金额","开盘","成交","涨跌",
					"总手","现手","最高","最低","买入","卖出",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case VOLUME_SORT://成交量排行榜
				{	char*	pcTitle[]={"名称","总手","开盘","成交","涨跌",
					"现手","最高","最低","买入","卖出","振幅",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_FLOAT,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case SWING_SORT://振幅排行榜
				{	char*	pcTitle[]={"名称","振幅","开盘","成交","涨跌",
					"总手","现手","最高","最低","买入","卖出",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case VOLRATIO_SORT://量比排行榜
				{	char*	pcTitle[]={"名称","量比","开盘","成交","涨跌",
					"总手","现手","最高","最低","买入","卖出",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case BUYSELL_SORT://委比排行榜
				{	char*	pcTitle[]={"名称","委比","开盘","成交","涨跌",
					"总手","现手","最高","最低","买入","卖出",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case PRICE_SORT://成交价排行榜
				{	char*	pcTitle[]={"名称","成交","开盘","振幅","涨跌",
					"总手","现手","最高","最低","买入","卖出",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_PRICE,CH_PRICE,CH_FLOAT,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			}

			usSize+=2;
		}

		for(i=0;i<m_usSize;i++)
			usSize+=m_pstSortUnit[i].HostToNet(pucData+usSize,ucType);

		return	usSize;
	}

	unsigned short	GetSize(unsigned char	ucSize)
	{
		return	sizeof(SSortData)+ucSize*sizeof(SSortUnit);
	}

	static unsigned short GetBufNeedSize(unsigned char	ucSize, unsigned char	ucType)
	{
		if(CODE_SORT == ucType)
			return 76 + ucSize*83;  //76是结构体SSortData的大小 83是SSortUnit的大小
		else
			return 0;
	}
};

//服务端内部使用
struct	SRequestSort{
	unsigned	short	m_usType;//请求类型
	unsigned	short	m_usMarketType;//市场标识
	unsigned	short	m_usIndex;//请求索引
	unsigned	short	m_usSize;//请求的证券总数
	unsigned	short	m_usFirst;//排名的起始序号
};

//客户端请求排行榜数据
struct	SMRequestSort{
	unsigned	short	m_usType;//请求类型
	unsigned	short	m_usMarketType;//市场标识
	unsigned	short	m_usIndex;//请求索引
	unsigned	short	m_usMobileType;//手机型号
	char				m_acRight[32];//权限描述符
	unsigned	short	m_usSize;//请求的证券总数
	unsigned	short	m_usFirst;//排名的起始序号
	unsigned	short	NetToHost(unsigned char*	pucData)
	{//转换成主机字节顺序
		unsigned short	usSize=0;
		unsigned short*	pusData=&m_usType;

		int i;
		for( i=0;i<4;i++)
		{
			pusData[i]=reverse_s(((unsigned	short*)(pucData+usSize))[0]);
			usSize+=sizeof(unsigned	short);
		}

		memcpy(m_acRight,pucData+usSize,32);
		usSize+=32;
		m_usSize=reverse_s(((unsigned	short*)(pucData+usSize))[0]);
		usSize+=sizeof(unsigned	short);
		m_usFirst=reverse_s(((unsigned	short*)(pucData+usSize))[0]);
		usSize+=sizeof(unsigned	short);
		return	usSize;
	}
};

//服务端内部使用
struct	SCodeInfo{
	unsigned	short	m_usMarketType;//市场标识
	unsigned	int		m_uiCode;//证券代码
	char				m_acCode[6];//股票代码
	char				m_acName[12];//名称
	unsigned	int		m_uiPrevClose;//昨收价
	unsigned	int 	m_ui5DayVol;//五日平均总手
	unsigned	int 	m_uiUpLimit;//涨停板限制,对于深圳国债表示应记利息
	unsigned	int		m_uiDownLimit;//跌停板限制
	unsigned	int		m_uiShares;//流通股数
	short				m_sAsset;//每股净资产
	short				m_sYield;//每股收益
	char				m_ggpj[12];//个股评级
//	unsigned	int		m_uiExRightsPrevClose;//昨收价
};

//服务端内部使用
struct	SRequestData{
	unsigned	short	m_usType;//请求类型
	unsigned	short	m_usMarketType;//市场标识
	unsigned	short	m_usIndex;//请求索引
	unsigned	short	m_usSize;//请求证券总数,即结构CodeInfo的个数
	unsigned	int		m_uiFirst;//请求数据结构的的起始
	SCodeInfo*			m_pstCode;
};

//用于向手机客户端推送的实时行情
struct	SRealUnit{
	unsigned	char	m_ucStructSize;//结构长度
	unsigned	int*	m_puiData;
	unsigned	short	HostToNet(unsigned char*	pucData,unsigned short	usUnits)
	{
		//转换成网络字节顺序
		unsigned short	usSize=sizeof(unsigned char);
		pucData[0]=usUnits*sizeof(unsigned int)+sizeof(unsigned char);

		for(int i=0;i<usUnits;i++)
		{	((unsigned	int*)(pucData+usSize))[0]=reverse_i(m_puiData[i]);
		usSize+=sizeof(unsigned	int);
		}

		return	usSize;
	}

	unsigned short	GetSize(unsigned char	ucSize)
	{
		return	sizeof(unsigned char)+ucSize*sizeof(unsigned int);
	}
};

//包括证券成交统计信息
struct	SStatData{
	unsigned	char	m_ucStructSize;//结构长度
	unsigned	short	m_usType;//响应类型，与请求数据包类型一致
	unsigned	short	m_usIndex;//请求索引，与请求数据包类型一致
	unsigned	short	m_usMarketType;//市场标识
	char				m_acName[12];//名称
	char				m_acCode[6];//代码
	char				m_ggpj[12];
	unsigned	int		m_uiPrevClose;//昨收价
	unsigned	char	m_ucUnits;//成员m_pcTitle的字符长度
	unsigned	char*	m_pcTitle;
	SRealUnit			m_stRealUnit;
	unsigned	short	HostToNet(unsigned char*	pucData)
	{//转换成网络字节顺序
		pucData[0]=sizeof(SStatData);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i,j;
		for( i=0;i<3;i++)
		{	((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
		usSize+=sizeof(unsigned	short);
		}

		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);
		usSize+=NAME_LEN;
//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);
		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));
		usSize+=sizeof(m_ggpj);
		((unsigned	int*)(pucData+usSize))[0]=reverse_i(m_uiPrevClose);
		usSize+=sizeof(unsigned	int);

		if(ST_Index != (m_usMarketType & 0x0f))
		{
			char*	pcTitle[]={"涨跌","涨跌幅","昨收","开盘","最高","最低",
				"涨停","跌停","均价","总手","委比","量比","金额","主买","主卖",
				"市盈率","换手率","卖五","","卖四","","卖三","","卖二","",
				"卖一","","成交","","买一","","买二","","买三","","买四","","买五","",};

			//对于成员对象m_stNowData从其第三个成员变量开始的特征描述，包括颜色，价格，成交量和小数
			unsigned char	aucDescribe[]={CH_PRICE,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,
				CH_PRICE,CH_PRICE,CH_PRICE,CH_VOLUME,CH_FLOAT,CH_FLOAT,CH_PRICE,CH_VOLUME,CH_VOLUME,
				CH_FLOAT,CH_FLOAT,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,
				CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_BARGIN,CH_VOLUME,
				CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,
				CH_PRICE,CH_VOLUME,};

			m_ucUnits=39;
			(pucData+usSize)[0]=m_ucUnits;
			usSize+=sizeof(unsigned	char);

			for( i=0;i<m_ucUnits;i++)
			{
				ucLen=strlen(pcTitle[i]);
//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
				ucLen,(LPWSTR)(pucData+usSize),ucLen);
			usSize+=ucLen;
			pucData[usSize]='|';
			pucData[usSize+1]=aucDescribe[i];
			usSize+=2;
			}
		}
		else
		{
			char*	pcTitle[]={"A股成交","B股成交","基金成交","指数","涨跌","幅度",
				"总成交额","总成交量","最高指数","最低指数","上涨家数","下跌家数",};

			//对于成员对象m_stNowData从其第三个成员变量开始的特征描述，包括颜色，价格，成交量和小数
			unsigned char	aucDescribe[]={CH_VOLUME,CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_FLOAT,
				CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_VOLUME,CH_VOLUME,};
			m_ucUnits=12;
			(pucData+usSize)[0]=m_ucUnits;
			usSize+=sizeof(unsigned	char);

			unsigned char	ucInvert;
			for( i=0;i<m_ucUnits;i++)
			{
				ucLen=strlen(pcTitle[i]);
				ucInvert=ucLen;
				for( j=0;j<ucLen;j++)
				{
					if(pcTitle[i][j]>0)
						ucInvert++;
				}

//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucInvert);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
					ucLen,(LPWSTR)(pucData+usSize),ucInvert);
				usSize+=ucInvert;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];
				usSize+=2;
			}
		}

		usSize+=m_stRealUnit.HostToNet(pucData+usSize,m_ucUnits);
		return	usSize;
	}

	unsigned short	GetSize(unsigned char	ucType)
	{//转换成网络字节顺序
		if(ST_Index != ucType)
			return	202+sizeof(SStatData)+m_stRealUnit.GetSize(39);
		else
			return	108+sizeof(SStatData)+m_stRealUnit.GetSize(12);

		return	m_stRealUnit.GetSize(ucType);
	}
};

// 量价结构，显示买价、买量或者卖价、卖量
struct	SVolPrice
{
	unsigned	int		m_uiPrice;//价格
	unsigned	int		m_uiVolume;//量
};

struct SINDEXOVERLAP
{
	unsigned int m_uiClose;
	unsigned char m_ucUnits;
	SRealMinsUnit *m_pstMinsUnit;

	unsigned short HostToNet(unsigned char*	pucData, unsigned short usType)
	{
		unsigned short usLen = 0;
		((unsigned int*)pucData)[0] = reverse_i(m_uiClose);
		usLen += 4;
		pucData[usLen] = m_ucUnits;
		usLen += 1;
		SRealMinsUnit *prmu = (SRealMinsUnit*)((char*)&m_ucUnits + 1);
		for (int i = 0; i < m_ucUnits; i++)
			usLen += prmu[i].HostToNet(pucData+usLen, usType);
		return usLen;
	};

	unsigned short GetSize(unsigned short usSize)
	{
		return sizeof(int)+sizeof(char)+usSize*sizeof(SRealMinsUnit);
	};
};

//用于验证股票代码是否合法或者返回多个股票代码
struct	SCodeList{
	unsigned	char	m_ucStructSize;//结构长度
	unsigned	short	m_usType;//响应类型，与请求数据包类型一致
	unsigned	short	m_usIndex;//请求索引，与请求数据包类型一致
	unsigned	char	m_ucSize;//结构SCodeInfo的个数
	SMCodeInfo*			m_pstCodeInfo;
	unsigned	short	HostToNet(unsigned char*	pucData)
	{//转换成网络字节顺序
		pucData[0]=sizeof(SHisKData)-sizeof(SSortUnit*);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i;
		for( i=0;i<2;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		(pucData+usSize)[0]=m_ucSize;
		usSize+=sizeof(m_ucSize);
		char *pTemp = ((char*)&m_usType) + 2 * sizeof(unsigned short)  + sizeof(unsigned char) + sizeof(SMCodeInfo*);
		SMCodeInfo* pstCodeInfo= (SMCodeInfo*)pTemp;
//			(SMCodeInfo*)(&m_ucSize+sizeof(m_ucSize)+sizeof(SMCodeInfo*));

		for(i=0;i<m_ucSize;i++)
			usSize+=pstCodeInfo[i].HostToNet(pucData+usSize);

		return	usSize;
	}

	unsigned	short	GetSize(unsigned char	ucSize)
	{
		return	sizeof(SHisKData)+ucSize*sizeof(SMCodeInfo);
	}
};
////////////////////////////////////////////////////////////////////////////////////////
//分时走势
struct tagRealMins
{
	unsigned int	 nTime;           //时间，以分为单位
	unsigned int	 nOpenPrice;	  //开盘价格
	unsigned int	 nMaxPrice;		  //最高价格
	unsigned int	 nMinPrice;		  //最低价格
	unsigned int	 nNewPrice;       //最新价格
	unsigned int	 nVolume;		  //成交数量
	unsigned int     AvgPrice;				//均价
};

//个股统计
struct tagQuotedPrice
{
	char szCode[16];				  //证券代码
	char szName[16];                  //证券名称
	unsigned int	nZrsp;            //昨日收盘
	unsigned int	nJrkp;			  //今开盘
	unsigned int	nZtjg;            //涨停价格

	unsigned int	nDtjg;            //跌停价格
	unsigned int	nSyl1;            //市盈率1
	unsigned int	nSyl2;            //市盈率2
	unsigned int	nZgjg;            //最高价格
	unsigned int	nZdjg;            //最低价格

	unsigned int	nZjjg;            //最近价格
	unsigned int	nZjcj;            //最近成交
	unsigned int	nCjsl;            //成交数量
	unsigned int	nCjje;            //成交金额
	unsigned int	nCjbs;            //成交笔数

	unsigned int	nBjw1;            //买价一
	unsigned int	nBsl1;            //买量一
	unsigned int	nBjw2;            //买价二
	unsigned int	nBsl2;            //买量二
	unsigned int	nBjw3;            //买价三

	unsigned int	nBsl3;            //买量三
	unsigned int	nBjw4;            //买价四
	unsigned int	nBsl4;            //买量四
	unsigned int	nBjw5;            //买价五
	unsigned int	nBsl5;            //买量五

	unsigned int	nSjw1;            //卖价一
	unsigned int	nSsl1;            //卖量一
	unsigned int	nSjw2;            //卖价二
	unsigned int	nSsl2;            //卖量二
	unsigned int	nSjw3;            //卖价三

	unsigned int	nSsl3;            //卖量三
	unsigned int	nSjw4;            //卖价四
	unsigned int	nSsl4;            //卖量四
	unsigned int	nSjw5;            //卖价五
	unsigned int	nSsl5;            //卖量五

	unsigned int	n5rpjzs;          //五日平均总手
	unsigned int	nPjjg;            //均价
	unsigned int	nWb;			  //委比
	unsigned int    nLb;			  //量比
	unsigned int	nWp;			  //内盘
	unsigned int	nNp;			  //外盘
	
	unsigned int	nZd;              //涨跌
	unsigned int	nZdf;             //涨跌幅
	unsigned int	nZf;              //振幅
};

struct tagKLineInfo
{
	unsigned int     nDate;			  //日期
	unsigned int     nOpenPrice;	  //开盘价格
	unsigned int     nMaxPrice;		  //最高价格
	unsigned int     nMinPrice;		  //最低价格
	unsigned int     nClosePrice;	  //收盘价格
	unsigned int     nVolume;		  //成交数量
	unsigned int     nSum;			  //成交金额
};

struct tagSaveData   //保存序号和数据指针
{
	unsigned short   SerialNo;
	char*            pData;
};

struct tagStockInfo
{
	char             szCode[16];	 //证券代码
	char             szName[16];	 //证券简称
	char             szSpell[4];	 //拼音
	unsigned int     nZrsp;			 //昨日收盘
};
////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)
#endif