// MessageQueue.h: interface for the CMessageQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEQUEUE_H__D87E2FA1_82E2_43E0_BD80_7E7F5DC91778__INCLUDED_)
#define AFX_MESSAGEQUEUE_H__D87E2FA1_82E2_43E0_BD80_7E7F5DC91778__INCLUDED_

#pragma warning(disable : 4786)

#include <list>

template <class T>
struct _message_queue_clear
{
	_message_queue_clear(T&)
	{};
};

/*******************************************************************
Description: 消息队列
Link:        
History:
--------------------------------------------------------------------
Date:        2004-02-21
Version:     1.0.0.1
Author:      xinliang
Description: 初版
2004-11-01 增加模板支持
*******************************************************************/
template <class T, class Destory = _message_queue_clear<T> >
class CMessageQueue 
{
public:
	CMessageQueue()
	{
		InitializeCriticalSection(&m_csLock);
		m_hIsEmpty = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!m_hIsEmpty)
			throw 0;
	}

	~CMessageQueue()
	{
		Clear();
		DeleteCriticalSection(&m_csLock);
		CloseHandle(m_hIsEmpty);
	}

	void Push(const T &block)			//向消息队列增加一个包
	{
		EnterCriticalSection(&m_csLock);
		m_queue.push_back(block);
		SetEvent(m_hIsEmpty);
		LeaveCriticalSection(&m_csLock);
	}

	bool Pop(T &block)					//从消息队列弹出一个包
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hIsEmpty, INFINITE))
		{
			//读取并删除
			EnterCriticalSection(&m_csLock);
			block = m_queue.front();
			m_queue.pop_front();
			if (!m_queue.empty())
				SetEvent(m_hIsEmpty);
			LeaveCriticalSection(&m_csLock);
		}
		else
		{
			return false;
		}
		return true;
	}

	void Clear()					//清空队列
	{
		ResetEvent(m_hIsEmpty);
		EnterCriticalSection(&m_csLock);
		while (m_queue.size())
		{
			T block = m_queue.front();
			Destory vv(block);
			m_queue.pop_front();
		}
		LeaveCriticalSection(&m_csLock);
	}

private:
	std::list<T> m_queue;

	//占用锁
	CRITICAL_SECTION m_csLock;
	HANDLE m_hIsEmpty;

	CMessageQueue(const CMessageQueue&);
	CMessageQueue& operator=(const CMessageQueue&);
};

#endif // !defined(AFX_MESSAGEQUEUE_H__D87E2FA1_82E2_43E0_BD80_7E7F5DC91778__INCLUDED_)
