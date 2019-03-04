// qextend.h : qextend DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

//Server���������仯֪ͨ�ص�����ԭ��
typedef void(*pfnOnQuoteChanged)(const char *symbol, const int *notify);
//Server�����ҵ������ص�����
typedef void(*pfnOnBusiness)(unsigned int msgid, CommxHead *pHead);

class CqextendApp : public CWinApp
{
public:
	CqextendApp();

// ��д
public:
	virtual BOOL InitInstance();

	int m_nMode;	// ����ģʽ,0-������(��Farm�ҽ�),1-������(��Server�ҽ�)
	pfnOnQuoteChanged m_onQuoteChanged;	// ����仯֪ͨ�Ļص�����
	pfnOnBusiness m_onBusiness;			// ҵ����ɵ�֪ͨ�ص�����

	DECLARE_MESSAGE_MAP()
};

extern CqextendApp theApp;
