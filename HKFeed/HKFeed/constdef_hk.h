//constdef.h ͷ�ļ�
//

#pragma once
#include "../../public/feedinterface.h"
#include "../../public/commx/report.h"
////////////////////////////////////////////////////////////////////////////////

#define MODULE_NAME       "HKFEED"   //��ģ�������
#define MSG_BUF_LEN       200 
//#define _TEST_SCAN_TIME				 //����ɨ��DBFʱ��   ������ʹ��

#define HK_HQ_DBF  0x03













///////////////////////////////////////////////////////////////////////////////

void Report(const char* pMsg, int nLevel, UINT nLine, char *pFile);
#define REPORT_Ex(Msg, nLevel)\
	Report(Msg, nLevel, __LINE__, __FILE__)