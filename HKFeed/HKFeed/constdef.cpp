// constdef.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "constdef_hk.h"
#include <stdio.h>
#include <time.h>

extern IFeedOwner*    g_pOwner;
//////////////////////////////////////////////////////////////////////////////////////////////
void Report(const char* pMsg, int nLevel, UINT nLine, char *pFile)
{
	g_pOwner->Report(MODULE_NAME, GetCurrentThreadId(), pFile, nLine, pMsg, nLevel);
}
