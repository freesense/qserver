
#include "stdafx.h"
#include "thread.h"

void CWorkThread::close()
{}

unsigned int CWorkThread::svc(void* lpParam)
{
	char *lpData = NULL;
	if (GetThreadID() == GetThreadID(0))
	{//�ͻ�����
		while (1)
		{
			int iq = m_queue_request.Pop(lpData);
		}
	}
	else
	{//�����������Ӧ��
		while (1)
		{
			int iq = m_queue_answer.Pop(lpData);
		}
	}
	return 0;
}
