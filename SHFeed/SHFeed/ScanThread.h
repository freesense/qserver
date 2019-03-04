//ScanThread.h头文件
//

#pragma once

#include "../../public/QuoteFeed/ThreadEx.h"
#include "../../public/QuoteFeed/DBFile.h"
#include <map> 
#include "../../public/QuoteFeed/CommonStruct.h"
#include <list>
#include "regexpr2.h"

using namespace std; 
using namespace regex;

struct tagSHRecord
{
	tagSHRecord()
	{
		byScanTimes = 0;
		pHQRecBuf	= NULL;
		bNameChanged=false;
	}
	unsigned char byScanTimes;	//扫描次数 0，1，2...
	char*         pHQRecBuf;	//上海行情库一条记录的缓冲区
	bool          bNameChanged;	//股票名称发生变化，则先发送删除股票数据，再发送添加股票数据
};

struct tagSZRecord
{
	tagSZRecord()
	{
		byScanTimes = 0;
		pHQRecBuf	= NULL;
		pXXRecBuf	= NULL;
		bNameChanged=false;
	}
	unsigned char byScanTimes;	//扫描次数 0，1，2...
	char*         pHQRecBuf;	//深圳行情库一条记录的缓冲区
	char*         pXXRecBuf;	//深圳信息库一条记录的缓冲区
	bool          bNameChanged;	//股票名称发生变化，则先发送删除股票数据，再发送添加股票数据
};

#define SZ_SCAN_TIME        2	//需要扫描的数据库数

typedef pair <int, tagSZRecord*> SZRecord_Pair;
typedef pair <int, tagSHRecord*> SHRecord_Pair;
/////////////////////////////////////////////////////
class CScanThread : public CThreadEx
{
public:
	CScanThread();
	virtual ~CScanThread();

	virtual void Run();
	virtual void Stop();
	
	BOOL SetDBFPath(const char *pSZPath, const char *pSHPath);
private:
	char      m_szDbfPath_SZ[MAX_PATH];       //存放深圳行情库路径
	char      m_szDbfPath_SH[MAX_PATH];		  //存放上海行情库路径
	char      m_szDbfPath_SZXX[MAX_PATH];     //存放深圳信息库路径

	CDBFile   m_dbfSZ;						  //操作深圳行情库
	CDBFile   m_dbfSH;						  //操作上海行情库
	CDBFile   m_dbfSZXX;					  //操作深圳信息库

	int       m_nSZPackSize;                  //深圳报文的大小
	char*     m_pSZSendBuf;                   //深圳待发送缓冲区
	int       m_nSZSendBufSize;				  //深圳待发送缓冲区大小
	char*     m_pSZCurSendBufPos;			  //深圳当前指向m_pSZSendBuf的位置

	int       m_nSHPackSize;                  //上海报文的大小
	char*     m_pSHSendBuf;                   //上海待发送缓冲区
	int       m_nSHSendBufSize;				  //上海待发送缓冲区大小
	char*     m_pSHCurSendBufPos;			  //上海当前指向m_pSHSendBuf的位置

	int       m_nSZXXPackSize;                  //深圳报文的大小
	char*     m_pSZXXSendBuf;                   //深圳待发送缓冲区
	int       m_nSZXXSendBufSize;			    //深圳待发送缓冲区大小
	char*     m_pSZXXCurSendBufPos;			    //深圳当前指向m_pSZSendBuf的位置

	tagPackHeader  m_packHdr;				  //包头
	DWORD     m_dwPreTick;                    //tickcount

	map<int, tagSZRecord*>                    m_SZMap;  //存放深圳行情
	map<int, tagSZRecord*>::iterator          m_SZiterator;

	map<int, tagSHRecord*>                    m_SHMap;  //存放上海行情
	map<int, tagSHRecord*>::iterator          m_SHiterator;

	unsigned short   m_wSZMinute;  //深圳dbf最新的时间
	unsigned short   m_wSHMinute;  

	BOOL ScanSZDBF();   //扫描深圳行情数据库
	BOOL ScanSHDBF();   //扫描上海行情数据库
	BOOL ScanSZXXDBF();	//扫描深圳信息数据库

	void ScanSZDeleteRecord(); //扫描深圳已删除股票
	void ScanSHDeleteRecord(); 

	unsigned short TimeToMinute(int nTime); //把时间转换成以分钟为单位的数据
	void FreeMapMemory();  //释放MAP中的数据
	
	//填充字段值到缓冲区
	void FillValueToBuf(unsigned short wFieldType, const char* pFieldName,
		unsigned int nDBFType, BOOL bIndex);
	
	//发送不存在的股票的所有字段
	void SZStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);   //bIndex 是否为指数
	void SHStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);
	void SZXXStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);

	//对存在的股票进行比较，发送变化的字段
	void SZStockExist(unsigned short wMinute, BOOL bIndex);
	void SHStockExist(unsigned short wMinute, BOOL bIndex);
	void SZXXStockExist(unsigned short wMinute, BOOL bIndex);

	//填充变化的字段到缓冲区
	BOOL FillChangedValueToBuf(unsigned short wFieldType, const char* pFieldName,
		const char* pOldRecord, int& nFieldCount, unsigned int nDBFType, BOOL bIndex);

	void SetStockType(unsigned char  byScanTimes, unsigned char byMapType); // 设置MAP中股票扫描的次数

	BOOL AllocateMemoryForSendBuf(); //给深圳和上海缓冲区分配内存

	void CheckSZSendBufForSend();   //检测深圳行情缓冲区是否不足存放一条记录，不足则发送缓冲区
	void CheckSHSendBufForSend();   //检测上海行情缓冲区是否不足存放一条记录，不足则发送缓冲区
	void CheckSZXXSendBufForSend(); //检测深圳信息缓冲区是否不足存放一条记录，不足则发送缓冲区
	void Trim(char* pData);  //去掉字符前后的空格

	BOOL FindSHRecord(int nStockCode); //查找上海记录是否存在
	BOOL IsValidStockCode(char* pCode); //检测股票代码是否有效，dbf有时出现乱码现象，发送错误日志到监控中心
	bool IsStockNameChanged(const char* pStockCode, const char* pOldRecord, unsigned int nDBFType);
	BOOL IsGuoZaiHuiGou(unsigned int nDBFType, char* pStockCode);
};
