//StockInfo.h
#ifndef _STOCKINFO_H_
#define _STOCKINFO_H_
#include "socketx.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CStockInfo
{
public:
	CStockInfo();
	virtual~CStockInfo();

	void Answer(char *lpData, unsigned int nLength);
	void UpdateStockInfo();
	char* GetStockCode(char *pSpell, int& nStockCount);
	unsigned short GetMktType(char* pStockCode);

protected:
	
	void SendInitRequest();
	void GetCurDateTime(unsigned int& nCurDate, unsigned int& nCurTime);

private:
	LockSingle      m_lockStockInfo;        //访问股票信息锁
    char*           m_pStockInfo;           //存放股票信息的缓冲区指针
	unsigned int    m_nBufsize;				//缓冲区大小
	unsigned int    m_nStockCount;			//股票个数

	unsigned int    m_nUpdateDate;          //更新股票信息日期，明天开盘时更新一次
};

extern CStockInfo g_StockInfo;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif