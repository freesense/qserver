// TestTianMa.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestTianMaApp:
// �йش����ʵ�֣������ TestTianMa.cpp
//

class CTestTianMaApp : public CWinApp
{
public:
	CTestTianMaApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()

private:
	CReportFile*   m_pRptFile;
	virtual int ExitInstance();
};

extern CTestTianMaApp theApp;