#pragma once
#include <map>
#include <windows.h>

#include <string>
#include "rwlock.h"
using namespace std; 
struct SDataItem{
	char * m_pBuf;//缓冲区
	int  m_nBufLen;//缓冲区长度
	int	 m_nDataLen;//数据区使用长度
	unsigned int	 m_unUsedCount;//累计访问次数
};
class CDataPool
{
public:
	CDataPool(void);
	~CDataPool(void);
public:
	SDataItem * ReadData(string c_strName);
	SDataItem * PutData(string c_strName,char * c_pDataBuf,int c_nBufLen,int c_nDataLen);
	void RemoveItem(string c_strName);
	void RemoveAll();
public:
	map<string,SDataItem *>	  m_mapItems;
	RWLock m_rwlock;
};
