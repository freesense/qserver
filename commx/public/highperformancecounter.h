
#ifndef __HIGH_PERFORMANCE_TIMER_H__
#define __HIGH_PERFORMANCE_TIMER_H__

#include "commxapi.h"

class COMMXAPI CHighPerformanceCounter
{
public:
	CHighPerformanceCounter();
	double count();
	unsigned int fpscount(double fps);	// ����ÿ��֡��������֮֡����Ҫsleep�ĺ�����

private:
	double m_dfFreq;			/**< CPUʱ��Ƶ�� */
	LONGLONG m_llCount;
	LARGE_INTEGER m_liTemp;
};

#endif
