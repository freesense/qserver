// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include "../../public/commx/commxapi.h"
#include "../../public/data_struct.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "../debug/kvisitor")
#pragma comment(lib, "../../public/lib/commxd")
#else
#pragma comment(lib, "../release/kvisitor")
#pragma comment(lib, "../../public/lib/commxr")
#endif

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
