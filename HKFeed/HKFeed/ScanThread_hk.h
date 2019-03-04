//ScanThread.h头文件
//

#pragma once

#include "../../public/QuoteFeed/ThreadEx.h"
#include "../../public/QuoteFeed/DBFile.h"
#include <map> 
#include "../../public/QuoteFeed/CommonStruct.h"

using namespace std; 

struct tagHKRecord
{
	tagHKRecord()
	{
		byScanTimes = 0;
		pHQRecBuf	= NULL;
		bNameChanged=false;
	}

	unsigned char byScanTimes;	//扫描次数 0，1，2...
	char*         pHQRecBuf;	//香港行情库一条记录的缓冲区
	bool          bNameChanged;	//股票名称发生变化，则先发送删除股票数据，再发送添加股票数据
};

typedef pair <int, tagHKRecord*> HKRecord_Pair;
/////////////////////////////////////////////////////
class CScanThread : public CThreadEx
{
public:
	CScanThread();
	virtual ~CScanThread();

	virtual void Run();
	virtual void Stop();
	
	BOOL SetDBFPath(const char *pHKPath);

	unsigned short   m_nStartTime;			  //停止发送扫描数据开始时间
	unsigned short   m_nStopTime;			  //停止发送扫描数据结束时间
private:
	char      m_szDbfPath_HK[MAX_PATH];       //存放香港行情库路径
	CDBFile   m_dbfHK;						  //操作香港行情库

	int       m_nHKPackSize;                  //香港报文的大小
	char*     m_pHKSendBuf;                   //香港待发送缓冲区
	int       m_nHKSendBufSize;				  //香港待发送缓冲区大小
	char*     m_pHKCurSendBufPos;			  //香港当前指向m_pHKSendBuf的位置

	tagPackHeader  m_packHdr;				  //包头

	map<int, tagHKRecord*>                    m_HKMap;  //存放香港行情
	map<int, tagHKRecord*>::iterator          m_HKiterator;

	unsigned short   m_wHKMinute;  
	DWORD     m_dwPreTick;                    //tickcount

	BOOL ScanHKDBF();   //扫描上海行情数据库
	void ScanHKDeleteRecord();//扫描香港已删除股票 

	unsigned short TimeToMinute(int nTime); //把时间转换成以分钟为单位的数据
	void FreeMapMemory();  //释放MAP中的数据
	
	//填充字段值到缓冲区
	void FillValueToBuf(unsigned short wFieldType, const char* pFieldName,
		unsigned int nDBFType, BOOL bIndex);
	
	//发送不存在的股票的所有字段
	void HKStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);
	

	//对存在的股票进行比较，发送变化的字段
	void HKStockExist(unsigned short wMinute, BOOL bIndex);

	//填充变化的字段到缓冲区
	BOOL FillChangedValueToBuf(unsigned short wFieldType, const char* pFieldName,
		const char* pOldRecord, int& nFieldCount, unsigned int nDBFType, BOOL bIndex);

	void SetStockType(unsigned char  byScanTimes, unsigned char byMapType); // 设置MAP中股票扫描的次数

	BOOL AllocateMemoryForSendBuf(); //给香港缓冲区分配内存
	void CheckHKSendBufForSend();   //检测上海行情缓冲区是否不足存放一条记录，不足则发送缓冲区
	void ChangeStockCode(char* pStockCode, BOOL bIndex); //把股票代码改为标准代码
	void Trim(char* pData);  //去掉字符前后的空格

	BOOL IsValidStockCode(char* pCode); //检测股票代码是否有效，dbf有时出现乱码现象，发送错误日志到监控中心
	bool IsStockNameChanged(const char* pStockCode, const char* pOldRecord, unsigned int nDBFType);
};
