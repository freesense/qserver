// qextend.h : qextend DLL ����ͷ�ļ�
//

#pragma once

#include "resource.h"		// ������

//Server���������仯֪ͨ�ص�����ԭ��
typedef void(*pfnOnQuoteChanged)(const char *symbol, const int *notify);
//Server�����ҵ������ص�����
typedef void(*pfnOnBusiness)(unsigned int msgid, CommxHead *pHead);

class CqextendApp
{
public:
	CqextendApp();

// ��д
public:
	int m_nMode;	// ����ģʽ,0-������(��Farm�ҽ�),1-������(��Server�ҽ�)
	pfnOnBusiness m_onBusiness;			// ҵ����ɵ�֪ͨ�ص�����
};

extern CqextendApp theApp;
