#include "StdAfx.h"
#include "DataPool.h"

CDataPool::CDataPool(void)
{
}

CDataPool::~CDataPool(void)
{
	m_rwlock.WaitWrite();
	RemoveAll();
	m_rwlock.EndWrite();
}
SDataItem * CDataPool::ReadData(string c_strName)
{
	if (m_mapItems.find(c_strName) == m_mapItems.end())
		return NULL;
	else
		return m_mapItems[c_strName];
}
SDataItem * CDataPool::PutData(string c_strName,char * c_pDataBuf,int c_nBufLen,int c_nDataLen)
{
	//return NULL;
	SDataItem * pItem;
	if (m_mapItems.find(c_strName) == m_mapItems.end())
	{
		pItem = new SDataItem();
		pItem->m_nBufLen = c_nBufLen;
		pItem->m_nDataLen = c_nDataLen;
		pItem->m_unUsedCount = 0;
		pItem->m_pBuf = (char *)malloc(c_nBufLen);
		memcpy(pItem->m_pBuf,c_pDataBuf,c_nDataLen);
		m_mapItems[c_strName] = pItem;
	}
	else
	{
		pItem = m_mapItems[c_strName];
		if (pItem->m_nBufLen < c_nDataLen)
		{
			free(pItem->m_pBuf);
			pItem->m_pBuf = (char *)malloc(c_nBufLen);
		}
		memcpy(pItem->m_pBuf,c_pDataBuf,c_nDataLen);
	}
	return pItem;
}

void CDataPool::RemoveAll()
{
	map<string,SDataItem *>::iterator iter;
	while(m_mapItems.size() > 0)
	{
		iter = m_mapItems.begin();
		free((*iter).second->m_pBuf);
		delete (*iter).second;
		m_mapItems.erase(iter);
	}
}

void CDataPool::RemoveItem(string c_strName)
{
	SDataItem * pItem;
	if (m_mapItems.find(c_strName) != m_mapItems.end())
	{
		pItem = m_mapItems[c_strName];
		free(m_mapItems[c_strName]->m_pBuf);
		m_mapItems.erase(c_strName);
		delete pItem;
	}
	
}