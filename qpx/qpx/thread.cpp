
#include "stdafx.h"
#include "thread.h"

void CWorkThread::close()
{}

unsigned int CWorkThread::svc(void* lpParam)
{
	char *lpData = NULL;
	if (GetThreadID() == GetThreadID(0))
	{//客户请求
		while (1)
		{
			int iq = m_queue_request.Pop(lpData);
		}
	}
	else
	{//行情服务器的应答
		while (1)
		{
			int iq = m_queue_answer.Pop(lpData);
		}
	}
	return 0;
}
