#pragma once
#include <map>
#include <windows.h>

#include <string>
#include "rwlock.h"
using namespace std; 
struct SDataItem{
	char * m_pBuf;//������
	int  m_nBufLen;//����������
	int	 m_nDataLen;//������ʹ�ó���
	unsigned int	 m_unUsedCount;//�ۼƷ��ʴ���
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
