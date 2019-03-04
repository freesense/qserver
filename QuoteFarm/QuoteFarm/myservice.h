// myservice.h

#include "ntservice.h"

class CMyService : public CNTService
{
public:
	CMyService();
	virtual BOOL OnInit();
    virtual int Run();
	virtual void OnStop();
	virtual void OnShutdown();
    virtual BOOL OnUserControl(DWORD dwOpcode);

private:
};
