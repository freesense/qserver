// qextend.h : qextend DLL 的主头文件
//

#pragma once

#include "resource.h"		// 主符号

//Server定义的行情变化通知回调函数原型
typedef void(*pfnOnQuoteChanged)(const char *symbol, const int *notify);
//Server定义的业务请求回调函数
typedef void(*pfnOnBusiness)(unsigned int msgid, CommxHead *pHead);

class CqextendApp
{
public:
	CqextendApp();

// 重写
public:
	int m_nMode;	// 运行模式,0-生产者(被Farm挂接),1-消费者(被Server挂接)
	pfnOnBusiness m_onBusiness;			// 业务完成的通知回调函数
};

extern CqextendApp theApp;
