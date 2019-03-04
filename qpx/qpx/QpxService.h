//QpxService.h
#ifndef _QpxService_H_
#define _QpxService_H_

#include "NTService.h"
////////////////////////////////////////////////////////////////////////////////////
class CQpxService : public CNTService
{
public:
	CQpxService();
	virtual ~CQpxService();

protected:
	virtual void Run();
	virtual void Stop();

private:

};
////////////////////////////////////////////////////////////////////////////////////
#endif