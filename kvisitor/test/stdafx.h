// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
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

// TODO: 在此处引用程序需要的其他头文件
