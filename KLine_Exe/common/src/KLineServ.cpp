//KLineServ.cpp
//
#include "stdafx.h"
#include "KLineServ.h"
#include "MainDlg.h"
#include "KLine_Exe.h"

/////////////////////////////////////////////////////////////////////////////////////
CKLineServ::CKLineServ()
{
}

CKLineServ::~CKLineServ()
{
}

void CKLineServ::Run()
{
	CMainDlg dlg;
	theApp.m_pMainWnd = &dlg;
	theApp.m_pcheckThread = &dlg.m_checkThread;
	dlg.DoModal();
	theApp.m_pMainWnd = NULL;
}

void CKLineServ::Stop()
{
	CService::Stop();
}