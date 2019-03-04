
#include "stdafx.h"
//#include "Exception.h"
#include "../public/highperformancecounter.h"

#pragma warning(disable:4244)

CHighPerformanceCounter::CHighPerformanceCounter()
{
	BOOL b = QueryPerformanceFrequency(&m_liTemp);
//	if (!b)
//		THROWEXCEPTION(EXCEPTION_SYSTEM_NOT_SUPPORT);
	m_dfFreq = (double)m_liTemp.QuadPart;

	QueryPerformanceCounter(&m_liTemp);
	m_llCount = (double)m_liTemp.QuadPart;
}

double CHighPerformanceCounter::count()
{
	QueryPerformanceCounter(&m_liTemp);
	LONGLONG llNow = (double)m_liTemp.QuadPart;
	double df = ((double)(llNow - m_llCount)) / m_dfFreq;
	m_llCount = llNow;
	return df;
}

unsigned int CHighPerformanceCounter::fpscount(double fps)
{
	double df = count();
	df = fps - df;
	if (df <= 0)
		return 0;
	return int(df * 1000);
}
