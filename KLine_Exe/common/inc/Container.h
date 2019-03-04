/************************************************************************************
*
* �ļ���  : Container.h
*
* �ļ�����: �����࣬���ڱ�������
*
* ������  : dingjing, 2009-01-23
*
* �汾��  : 1.0
*
* �޸ļ�¼:
*
************************************************************************************/
#ifndef _CONTAINER_H_
#define _CONTAINER_H_
#include <deque>
#include <map>
using namespace std;
/////////////////////////////////////////////////////////////////////////////////////
//����
template<class T>
class CDeque
{
public:	//����
	CDeque(){::InitializeCriticalSection(&m_csQueue);}
	virtual ~CDeque(){::DeleteCriticalSection(&m_csQueue);}

public: //����
	UINT Size(){return (UINT)m_deque.size();}
	deque<T>* GetDeque(){return &m_deque;}

public: //����
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
	deque<T>           m_deque;	  //������ݵĶ���
	CRITICAL_SECTION   m_csQueue; //������

};

//###################################################################################
template<class T>
class CMapEx
{
public:	//����
	CMapEx(){::InitializeCriticalSection(&m_csMap);}
	virtual ~CMapEx(){::DeleteCriticalSection(&m_csMap);}

public: //����
	UINT Size(){return m_csMap.size();}
	map<T, T>* GetMap(){return &m_map;}
public:	//����
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