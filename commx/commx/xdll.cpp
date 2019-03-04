
#include "stdafx.h"
#include "../public/xdll.h"

#pragma warning(disable:4311 4312)

XDLL::XDLL()
: m_nDllHandler(0)
{
}

XDLL::XDLL(const char *lpDllPath)
{
	Open(lpDllPath);
}


XDLL::~XDLL()
{
	if (m_nDllHandler)
	{
#ifdef _WIN32
		::FreeLibrary((HMODULE)m_nDllHandler);
#elif defined _POSIX_C_SOURCE
		dlclose(m_nDllHandler);
#endif
	}
}

bool XDLL::Open(const char *lpDllPath)
{
#ifdef _WIN32
	m_nDllHandler = (unsigned int)::LoadLibrary(lpDllPath);
#elif defined _POSIX_C_SOURCE
	m_nDllHandler = dlopen(lpDllPath, RTLD_LAZY);
#endif
	if (m_nDllHandler == 0)
		return false;
	return true;
}

void* XDLL::Symbol(const char *lpSymbolName)
{
#ifdef _WIN32
	return GetProcAddress((HMODULE)m_nDllHandler, lpSymbolName);
#elif defined _POSIX_C_SOURCE
	return dlsym(m_nDllHandler, lpSymbolName);
#endif
}
