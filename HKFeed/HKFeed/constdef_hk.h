//constdef.h 头文件
//

#pragma once
#include "../../public/feedinterface.h"
#include "../../public/commx/report.h"
////////////////////////////////////////////////////////////////////////////////

#define MODULE_NAME       "HKFEED"   //本模块的名称
#define MSG_BUF_LEN       200 
//#define _TEST_SCAN_TIME				 //发送扫描DBF时间   供测试使用

#define HK_HQ_DBF  0x03













///////////////////////////////////////////////////////////////////////////////

void Report(const char* pMsg, int nLevel, UINT nLine, char *pFile);
#define REPORT_Ex(Msg, nLevel)\
	Report(Msg, nLevel, __LINE__, __FILE__)