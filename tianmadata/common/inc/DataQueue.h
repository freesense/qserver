//DataQueue.h
#ifndef _DATAQUEUE_H_
#define _DATAQUEUE_H_
#include <deque>
#include "TMDdef.h"
////////////////////////////////////////////////////////////////
class CDataQueue
{
public:
	CDataQueue();
	virtual ~CDataQueue();

	BOOL Init();
	void Release();

	void PushBack(tagReturnMsgID* pData);
	tagReturnMsgID* PopFront();

protected:

private:
	HANDLE             m_hEmptyEvent;    //判断队列是否为空
	CRITICAL_SECTION   m_csQueue;		 //队列锁

	std::deque<tagReturnMsgID*> m_deque;
	BOOL   m_bRun;  //运行标志
};

////////////////////////////////////////////////////////////////
#endif