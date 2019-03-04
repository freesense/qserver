//QuoteFeedService.h
#ifndef _QUOTEFEEDSERVICE_H_
#define _QUOTEFEEDSERVICE_H_

#include "NTService.h"
////////////////////////////////////////////////////////////////////////////////////
class CQuoteFeedService : public CNTService
{
public:
	CQuoteFeedService();
	virtual ~CQuoteFeedService();

protected:
	virtual void Run();
	virtual void Stop();

private:

};
////////////////////////////////////////////////////////////////////////////////////
#endif