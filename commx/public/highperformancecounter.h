
#ifndef __HIGH_PERFORMANCE_TIMER_H__
#define __HIGH_PERFORMANCE_TIMER_H__

#include "commxapi.h"

class COMMXAPI CHighPerformanceCounter
{
public:
	CHighPerformanceCounter();
	double count();
	unsigned int fpscount(double fps);	// 根据每秒帧数计算两帧之间需要sleep的毫秒数

private:
	double m_dfFreq;			/**< CPU时钟频率 */
	LONGLONG m_llCount;
	LARGE_INTEGER m_liTemp;
};

#endif
