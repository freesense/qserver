// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#include <stdio.h>
#include <tchar.h>

#define SHAREMEM_NO
#define HTTPREQ_NO

#define MN "QS"

#include "iomodel.h"
#include "farm.h"
#include "Realdata.h"
#include "DataPool.h"
#include "..\\plug.h"
#include "PlugManager.h"
#include "..\\..\\public\\commx\\report.h"

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
extern CDataFarm * g_pDataFarm;
extern CompletionPortModel  * g_pIo;
extern CPlugManager * g_pPlugs;
extern CIndexSummary g_IndexSummary;
extern CDataPool g_DataPool;
extern CReportFile * g_pReportFile;
extern CStockKindManager g_StockKindManager;
extern std::string g_strNoLineTest;
