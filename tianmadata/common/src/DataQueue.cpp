//DataQueue.cpp
#include "stdafx.h"
#include "DataQueue.h"

/////////////////////////////////////////////////////////////////////
CDataQueue::CDataQueue()
{
	m_hEmptyEvent = NULL;
}

CDataQueue::~CDataQueue()
{
}

BOOL CDataQueue::Init()
{
	::InitializeCriticalSection(&m_csQueue);
	m_hEmptyEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hEmptyEvent == NULL)
		return false;

	return true;
}

void CDataQueue::Release()
{
	m_bRun = FALSE;
	::SetEvent(m_hEmptyEvent);
	::EnterCriticalSection(&m_csQueue);
	while(m_deque.size())
	{
		tagReturnMsgID* pData = m_deque.front();
		//free memory
		{
			delete[]pData->pBuf;
			delete pData;
		}
		m_deque.pop_front();
	}
	::LeaveCriticalSection(&m_csQueue);

	CloseHandle(m_hEmptyEvent);
	::DeleteCriticalSection(&m_csQueue);
}

void CDataQueue::PushBack(tagReturnMsgID* pData)
{
	::EnterCriticalSection(&m_csQueue);
	m_deque.push_back(pData);
	SetEvent(m_hEmptyEvent);
	::LeaveCriticalSection(&m_csQueue);
}

tagReturnMsgID* CDataQueue::PopFront()
{
	m_bRun = TRUE;
	tagReturnMsgID* pData = NULL;
	if (WAIT_OBJECT_0 == WaitForSingleObject(m_hEmptyEvent, 30000))
	{
		if (!m_bRun)
			return NULL;
		::EnterCriticalSection(&m_csQueue);
		pData = m_deque.front();
		m_deque.pop_front();
		if (!m_deque.empty())
			SetEvent(m_hEmptyEvent);
		::LeaveCriticalSection(&m_csQueue);
		return pData;
	}
	else
	{
		return NULL;
	}
}