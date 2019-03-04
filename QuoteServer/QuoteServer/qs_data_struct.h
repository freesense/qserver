#ifndef _QUOTE_SERVER_DATA_STRUCT_
#define _QUOTE_SERVER_DATA_STRUCT_
#include "..\\protocol.h"
#pragma pack(1)
const	unsigned short	FARMHEART=0;		//QuoteFarm的心跳
const	unsigned short	FARMNOTIFY=65000;	//行情变化通知
const	unsigned short	INITDATA=0x0001;		//市场初始化
const	unsigned short	TRACEDATA=0x0002;	//成交明细
const	unsigned short	REPORTDATA=0x0003;	//自选股报价
const	unsigned short	REALMINSDATA=0x0004;	//分钟走势
const	unsigned short	HISKDAYDATA=0x0005;		//历史K线日
const	unsigned short	HISKWEEKDATA=0x0006;		//历史K线周
const	unsigned short	HISKMONTHDATA=0x0007;		//历史K线月
const	unsigned short	REALMINSDATA2=0x0008;			//两日分时

const	unsigned short	LINETESTREQ=0x0000;//线路测试请求包
const	unsigned short	LINETESTREP=0x0100;//线路测试应答包


const	unsigned short	REALPUSHREQ=0X1000;	//推送请求
const	unsigned int BUFFER_SIZE=102400;

typedef struct _PACKETHEAD
{
	CommxHead		m_commxHead;
	unsigned short	m_usFuncNo;//功能编号
	unsigned short	m_usReqCount;//请求个数
}PACKETHEAD,*PPACKETHEAD;

typedef struct _REQ_ITEM//请求项目
{
	unsigned short	m_unType;//类型
	unsigned short	m_usStockCount;//证券个数
	char		*	m_pStocks;//列表
}REQ_ITEM,* PREQ_ITEM;

typedef struct _K_LINE
{
			unsigned int day;
			unsigned int open;
			unsigned int high;
			unsigned int low;
			unsigned int close;
			unsigned int volume;
			unsigned int amount;
}KLINE,*PKLINE;


//
//自定义枚举数据类型，用来标识套接字IO动作类型
//
typedef enum _IO_OPERATION 
{
        IoAccept, //AcceptEx/accept
		IoRead,   //WSARecv/recv/ReadFile
		IoWrite,   //WSASend/send/WriteFile
		IoEnd,
		IoExtraSend,
		IoExtraSendEnd
}IO_OPERATION, *PIO_OPERATION;

#pragma pack()
#endif
