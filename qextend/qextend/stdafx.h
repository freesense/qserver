// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifdef _DEBUG
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../../public/lib/commxr")
#endif
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../../public/lib/ntwdblib")

#define MN "HQEXTEND"
#define WIN32_LEAN_AND_MEAN

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#endif

// ������뽫λ������ָ��ƽ̨֮ǰ��ƽ̨��ΪĿ�꣬���޸����ж��塣
// �йز�ͬƽ̨��Ӧֵ��������Ϣ����ο� MSDN��
#ifndef WINVER				// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define WINVER 0x0501		// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINDOWS		// ����ʹ���ض��� Windows 98 ����߰汾�Ĺ��ܡ�
#define _WIN32_WINDOWS 0x0410 // ����ֵ����Ϊ�ʵ���ֵ����ָ���� Windows Me ����߰汾��ΪĿ�ꡣ
#endif

#ifndef _WIN32_IE			// ����ʹ���ض��� IE 6.0 ����߰汾�Ĺ��ܡ�
#define _WIN32_IE 0x0600	// ����ֵ����Ϊ��Ӧ��ֵ���������� IE �������汾��
#endif

#include <Windows.h>
#include "../../public/protocol.h"
#include "../../public/data_struct.h"
#include "config.h"
#include "workthread.h"
#include "../../public/messagequeue.h"
#include "../../public/commx/synch.h"
#include "../../public/commx/report.h"
#include "../../public/commx/mery.h"
#include "../../public/commx/code_convert.h"
#include "../../public/commx/highperformancecounter.h"

const char gVer[] = "$qextend.dll#2.1.26$";

extern CWorkThread gThread;
extern CMessageQueue<CommxHead*> gQueue;
