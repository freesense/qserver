
#ifndef __WORKTHREAD_FARM_ADDIN_INVESTODAY_H__
#define __WORKTHREAD_FARM_ADDIN_INVESTODAY_H__

#include "../../public/commx/xthread.h"
#include "../../public/messagequeue.h"

class CWorkThread : public XThread
{
public:
	virtual void close();
	virtual unsigned int __stdcall svc(void* lpParam);

	void AddSql(char *lpSql);

private:
	CMessageQueue<char*> m_queue;		/** sql事件队列 [7/1/2009 xinl] */
	HANDLE m_hQuit;						/** 推出线程信号 [7/1/2009 xinl] */
	CDblib m_dbsql;
};

#endif
