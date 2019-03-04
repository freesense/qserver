//TianmaDataExport.h
#ifndef _TIANMADATAEXPORT_H_
#define _TIANMADATAEXPORT_H_

#ifdef _DLL
#define TMDAPI __declspec(dllexport)
#else
#define TMDAPI __declspec(dllimport)
#endif
///////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct SMsgID
{
	unsigned int m_unID;
	unsigned int m_unSocket;
};

//数据请求结构
#define SYMBOL_LEN     16
#define STOCK_CODE_LEN 16
#define STOCK_NAME_LEN 16

#define REQ_QUOTE   	1		//行情数据
#define REQ_MINK	    2		//分钟走势
#define REQ_TICK		3		//分笔成交
#define REQ_HISK		4		//历史K线

struct SMinKReq
{
	unsigned int m_unDate;
};

struct STickReq
{
	unsigned int m_unDate;
};

struct SHisKReq
{
	unsigned int   m_unBeginDate;
	unsigned int   m_unEndDate;
	unsigned short m_usRight;   //1向前还权，向后还权
};

struct SPlugReq
{
	unsigned int	m_unType;
	char			m_acSymbol[SYMBOL_LEN];
	union
	{
		SMinKReq	m_sMinKReq;
		STickReq	m_sTickReq;
		SHisKReq	m_sHisKReq;
	};
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
	unsigned int wb;					//委比
	unsigned int lb;					//量比
	unsigned int np;					//内盘
	unsigned int wp;					//外盘
};

//分笔成交结构单元
struct TickUnit
{
	unsigned int Time;					//时间，格式：HHMM
	unsigned int Price;					//成交价
	unsigned int Volume;				//成交量
	unsigned char Way;					//成交方向，-未知，-内盘，-外盘
};

//内存分钟K线结构单元
struct MinUnit
{
	unsigned int Time;					//时间，格式：HHMM
	unsigned int OpenPrice;
	unsigned int MaxPrice;
	unsigned int MinPrice;
	unsigned int NewPrice;
	unsigned int Volume;
};

//历史K线单元
struct	SHisKUnit
{
		unsigned	int		m_uiDate;
		unsigned	int		m_uiOpenPrice;
		unsigned	int		m_uiMaxPrice;
		unsigned	int		m_uiMinPrice;
		unsigned	int		m_uiClosePrice;
		unsigned	int		m_uiVolume;
		unsigned	int		m_uiSum;//成交金额
		//unsigned	int		m_uEx;//招商个unsigned int
};

typedef int (*_SendReply)( SMsgID* c_pMsgID, char* c_pBuf, int c_iLen);
typedef Quote* (*_GetSymbolQuote)(char* c_pSymbol);
typedef int (*_GetSymbolData)(char* c_pReq, char* c_pBuf, int c_iMaxLen, int c_iDataType);
typedef void (*_PlugQuit)(unsigned int c_unPlugID);

/**********************************************************************
Function  : 初始化
c_unPlugID:	平台分配给此插件的ID号
return    : 0表示成功，否则失败
***********************************************************************/
extern "C" TMDAPI int _OnInit(unsigned int c_unPlugID,const char * c_pParam);

/**********************************************************************
Function  : 清理操作
c_unPlugID:	
return    : 
***********************************************************************/
extern "C" TMDAPI void _OnRelease();

/**********************************************************************
Function  : 响应数据更新处理(要求异步处理，插件在得到数据后，立即返回)
c_pBuf    :	本次数据更新的项目
c_iLen    :	c_pBuf的有效长度
return    : 0表示成功，否则失败
***********************************************************************/
extern "C" TMDAPI int _OnUpdateData(char * c_pBuf,int c_iLen);

/**********************************************************************
Function  : 响应外部请求(异步处理，立即返回)
c_pMsgID  :	请求标识，插件需要复制此结构内容，不允许保留指针，
            在处理完成后在应答时需要将此内容送回
c_pBuf    :	请求内容
c_iLen    :	c_pBuf的有效长度
return    : 0表示成功，否则失败
***********************************************************************/
extern "C" TMDAPI int _OnRequest(SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);

/**********************************************************************
Function  : 发送应答
c_pFunc   :	函数指针
            typedef int (*_SendReply)( SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);
			c_pMsgID: 请求标识，在OnRequest时候复制下来的
            c_pBuf  : 返回内容，指向一个行情数据单元
            c_iLen  : 返回数据长度
return    :
***********************************************************************/
extern "C" TMDAPI void _Set_SendReplay(_SendReply c_pFunc);

/**********************************************************************
Function  : 获取行情数据
c_pFunc   :	函数指针
            typedef Quote* (*_GetSymbolQuote)(char* c_pSymbol);
			c_pSymbol: 请求的股票代码
			return   : 返回NULL表示取失败，否则是一个有效的Quote指针
return    :
***********************************************************************/
extern "C" TMDAPI void _Set_GetSymbolQuote(_GetSymbolQuote c_pFunc);

/**********************************************************************
Function  : 获取数据
c_pFunc   :	函数指针
            typedef int (*_GetSymbolData)(char* c_pReq, char* c_pBuf,
			int c_iMaxLen, int c_iDataType);
			c_pReq     : 数据请求结构，声明数据类型、股票代码及必须的参数，
			             此结构根据实际情况进行补充，文档最后说明此结构
			c_pBuf     : 返回数据缓冲区，格式采用I[结果单元]…，I表示有多少
			             个结果单元，已知的行情单元、分时走势单元、分时成交
					     单元、K线单元，未知等待补充
			c_iMaxLen  : c_pBuf的最大长度
			c_iDataType: 数据类型
return     : 0表示成功，否则失败
***********************************************************************/
extern "C" TMDAPI void _Set_GetSymbolData(_GetSymbolData c_pFunc);

/**********************************************************************
Function  : 通知退出
c_pFunc   :	函数指针
            typedef void (*_PlugQuit)(unsigned int c_unPlugID);
			c_unPlugID: 平台分配给此插件的ID号
return    :
***********************************************************************/
extern "C" TMDAPI void _Set_PlugQuit(_PlugQuit c_pFunc);

extern "C" TMDAPI void GetModuleVer(char** pBuf);
///////////////////////////////////////////////////////////////////////
#endif