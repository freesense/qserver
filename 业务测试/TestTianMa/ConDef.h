//ConDef.h
#ifndef _CONDEF_H_
#define _CONDEF_H_

////////////////////////////////////////////////////////////////////////////
#define INIT_INFO         0x0001  //初始化   获取所有股票信息、股票代码、简称、拼音、昨收盘等信息
#define BARGAIN_BILL      0x0002  //成交明细 获取股票的实时成交数据
#define QUOTE_BILL        0x0003  //报价表   获取股票的实时行情
#define REAL_MIN          0x0004  //分时走势
#define DAY_KLINE		  0x0005  //日线
#define WEK_KLINE		  0x0006  //周线
#define MON_KLINE		  0x0007  //月线
#define TWO_DAY_REAL_MIN  0x0008  //两日分时走势

#define SORT_INFO		  6000	  //排行榜数据
#define IDX_NUM           6001	  //取指数的上涨，下跌和平盘家数

#define INVALID_DATA      0xDEAD  //乱包数据测试

#define SYMBOL_LEN        16  

/////////////////////////////////////////////////////////////////////////////
#endif