//HttpDataFile.h
//

#ifndef _HTTPDATAFILE_H_
#define _HTTPDATAFILE_H_

#include "BaseFile.h"
#include "LockFile.h"
////////////////////////////////////////////////////////////////////////////////
struct tagHttpKFileInfo
{
	unsigned int  nDate;              //日期
	unsigned int  nNewPrice;	      //最新价，收盘后为收盘价
	unsigned int  nOpenPrice;	      //开盘价格
	unsigned int  nPreClosePrice;	  //前收盘价格
	unsigned int  nMaxPrice;	      //最高价格
	unsigned int  nMinPrice;	      //最低价格
	unsigned int  nVolume;		      //成交数量
	unsigned int  nSum;			      //成交金额
};

class CHttpDataFile : public CBaseFile
{
public:
	CHttpDataFile();
	virtual ~CHttpDataFile();

/***********************************************************************
	函数功能: 打开文件
	dwDesiredAccess: 参数可为以下一个或几个
	    GENERIC_READ     GENERIC_WRITE     GENERIC_EXECUTE    GENERIC_ALL         
    dwShareMode: 参数可为以下一个或几个
	    FILE_SHARE_READ  FILE_SHARE_WRITE  FILE_SHARE_DELETE               
	dwCreationDisposition: 参数可为以下一个或几个
	    CREATE_NEW       CREATE_ALWAYS     OPEN_EXISTING       OPEN_ALWAYS  
		TRUNCATE_EXISTING   
	***********************************************************************/
	BOOL Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                      DWORD dwCreationDisposition);
	void Close();
	BOOL Requery();
	BOOL WriteData();
	BOOL WriteDataToEnd();
	BOOL AddRecord(tagHttpKFileInfo* pHttpInfo);

protected:

private:
	HANDLE          m_hFile;               //文件句柄
	CLockFile       m_hLockFile;           //用于锁定dbf文件

};













////////////////////////////////////////////////////////////////////////////////
#endif