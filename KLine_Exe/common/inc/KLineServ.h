//KLineServ.h
/*
Author:dingjing
Time:2008-11-6
 */
#ifndef _KLINESERV_H_
#define _KLINESERV_H_

#include "Service.h"

//////////////////////////////////////////////////////////////////////////////////
class CKLineServ : public CService
{
public:
	CKLineServ();
	virtual ~CKLineServ();

protected:
	virtual void Run();
	virtual void Stop();

private:


};

//////////////////////////////////////////////////////////////////////////////////
#endif