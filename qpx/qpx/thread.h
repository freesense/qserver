
#ifndef __THREAD_QPX_H__
#define __THREAD_QPX_H__

#include "../../public/commx/xthread.h"
#include "../../public/messagequeue.h"

class CWorkThread : public XThread
{
public:
	virtual void close();
	virtual unsigned int __stdcall svc(void* lpParam);

private:
	CMessageQueue<char*> m_queue_request;
	CMessageQueue<char*> m_queue_answer;
};

#endif
