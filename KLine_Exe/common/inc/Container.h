/************************************************************************************
*
* 文件名  : Container.h
*
* 文件描述: 容器类，用于保存数据
*
* 创建人  : dingjing, 2009-01-23
*
* 版本号  : 1.0
*
* 修改记录:
*
************************************************************************************/
#ifndef _CONTAINER_H_
#define _CONTAINER_H_
#include <deque>
#include <map>
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////
//队列
template<class T>
class CDeque
{
public:	//构造
	CDeque(){::InitializeCriticalSection(&m_csQueue);}
	virtual ~CDeque(){::DeleteCriticalSection(&m_csQueue);}

public: //属性
	UINT Size(){return (UINT)m_deque.size();}
	deque<T>* GetDeque(){return &m_deque;}

public: //操作
	T PopFront()
	{
		T pData = NULL;
		::EnterCriticalSection(&m_csQueue);
		if (!m_deque.empty())
		{
			pData = m_deque.front();
			m_deque.pop_front();
		}
		::LeaveCriticalSection(&m_csQueue);
		return pData;
	}

	void PushBack(T pData)
	{
		::EnterCriticalSection(&m_csQueue);
		m_deque.push_back(pData);
		::LeaveCriticalSection(&m_csQueue);
	}

	void Lock(){::EnterCriticalSection(&m_csQueue);}
	void Unlock(){::LeaveCriticalSection(&m_csQueue);}

private:
	deque<T>           m_deque;	  //存放数据的队列
	CRITICAL_SECTION   m_csQueue; //队列锁

};

//###################################################################################
template<class T>
class CMapEx
{
public:	//构造
	CMapEx(){::InitializeCriticalSection(&m_csMap);}
	virtual ~CMapEx(){::DeleteCriticalSection(&m_csMap);}

public: //属性
	UINT Size(){return m_csMap.size();}
	map<T, T>* GetMap(){return &m_map;}
public:	//操作
	void Insert(T pData)
	{
		::EnterCriticalSection(&m_csMap);
		m_map.insert(std::make_pair(pData, pData));
		::LeaveCriticalSection(&m_csMap);
	}

	BOOL Remove(T pData)
	{
		::EnterCriticalSection(&m_csMap);
		map<T, T>::iterator find = m_map.find(pData);
		BOOL bRet = FALSE;
		if (find != m_map.end())
		{
			m_map.erase(find);
			bRet = TRUE;
		}
		::LeaveCriticalSection(&m_csMap);

		return bRet;
	}

	void Lock(){::EnterCriticalSection(&m_csMap);}
	void Unlock(){::LeaveCriticalSection(&m_csMap);}

private:
	map<T, T>          m_map;   
	CRITICAL_SECTION   m_csMap; 
};
/////////////////////////////////////////////////////////////////////////////////////
#endif