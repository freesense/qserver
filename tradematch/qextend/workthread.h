
#ifndef __WORK_THREAD_H__
#define __WORK_THREAD_H__

#include "farm.h"
#include "../../public/commx/xthread.h"

#define	RPT_HEARTBEAT  0xAA	//心跳日志，用来监控线程
extern int CheckSymbol(char * c_acSymbol);
extern int GetWorkState();

///////////////////////////////////////////////////////////////////////////////////////////
struct _find_symbol
{
	long cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);
	bool find(std::string *lpSymbol);
	TickUnit * getlasttickunit();
	RINDEX *m_pIdx;
};
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
class CWorkThread : public XThread
{
public:
	virtual void close();
	virtual unsigned int __stdcall svc(void* lpParam);

private:
	DWORD m_dwPreTick;
	int		m_nLastConsignId;
};

///////////////////////////////////////////////////////////////////////////////////////////
class CWriteThread : public XThread
{
public:
	virtual void close();
	virtual unsigned int __stdcall svc(void* lpParam);

private:
	DWORD m_dwPreTick;
	int		m_nLastConsignId;
};

///////////////////////////////////////////////////////////////////////////////////////////
class CInfoThread : public XThread
{
public:
	virtual void close();
	virtual unsigned int __stdcall svc(void* lpParam);

private:
	DWORD m_dwPreTick;
};

#endif
