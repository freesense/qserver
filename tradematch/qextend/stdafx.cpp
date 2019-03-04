// stdafx.cpp : 只包括标准包含文件的源文件
// qextend.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

CWorkThread gWorkThread;
CWriteThread gWriteThread;
CInfoThread gInfoThread;
CMessageQueue<CommxHead*> gQueue;
CDataBaseEx gDBRead;
CDataBaseEx gDBWrite;
CDataStore gDataStore;
CDataFarm gFarm;
int gnTradeTime = 0;//交易时间
char gacTradeTime[4];//交易时间,跟踪上证指数的交易时间
bool gbIsTradeTime;//是否是交易时间
bool gbIsWorkTime;//是否是工作时间
CMessageQueue<Consign*> gConsignQueue;//委托单库,等待写入委托成交表
CMessageQueue<Consign*> gConsignQueueWait;//委托单库,等待写入委托成交表
bool gbExitFlag;//退出
SumInfo gSumInfo;//统计信息
bool	gbWorkDay;//是否是工作日，根据是否有OnNewTick，并且在OnNewTick中生成撮合成功
bool	gbTradeTime;//交易时间