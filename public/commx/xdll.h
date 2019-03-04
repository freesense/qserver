
#ifndef __COMMON_SHARED_LIBRARY_H__
#define __COMMON_SHARED_LIBRARY_H__

#include "commxapi.h"

class COMMXAPI XDLL
{
public:
	XDLL();
	XDLL(const char *lpDllPath);
	virtual ~XDLL();

	bool Open(const char *lpDllPath);
	void* Symbol(const char *lpSymbolName);

private:
	unsigned int m_nDllHandler;
};

#endif
