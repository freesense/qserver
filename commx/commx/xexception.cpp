
#include "stdafx.h"
#include "../public/xexception.h"
#include "../public/report.h"

#pragma warning(disable: 4311 4312)

SEH* SEH::m_instance = NULL;
TCHAR SehReport::szModuleName_[MAX_PATH] = {0};

////////////////////////////////////////////////////////////////////////////////////////////////////
SEH* SEH::Instance()
{
	return m_instance;
}

int SEH::Initialize(SehReportBase* pReport, SehThrowBase *pThrow)
{
	if (!m_instance)
	{
		m_instance = new SEH;
		CHECK_RUN(!m_instance, MN, T("分配内存失败\n"), RPT_CRITICAL, return -1);
		m_instance->m_pReport = pReport;
		m_instance->m_pThrow = pThrow;
		return 0;
	}
	return 1;
}

void SEH::Close()
{
	delete m_instance;
	m_instance = NULL;
}

LPCTSTR SEH::Code2Str(DWORD uCode)
{
	switch(uCode)
	{
	case EXCEPTION_ACCESS_VIOLATION            : return _T("ACCESS_VIOLATION|无效地址访问");
	case EXCEPTION_DATATYPE_MISALIGNMENT       : return _T("DATATYPE_MISALIGNMENT|数据类型对齐错误");
	case EXCEPTION_BREAKPOINT                  : return _T("BREAKPOINT|断点");
	case EXCEPTION_SINGLE_STEP                 : return _T("SINGLE_STEP|单步运行");
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED       : return _T("ARRAY_BOUNDS_EXCEEDED|数组越界");
	case EXCEPTION_FLT_DENORMAL_OPERAND        : return _T("FLT_DENORMAL_OPERAND|非正常的浮点操作");
	case EXCEPTION_FLT_DIVIDE_BY_ZERO          : return _T("FLT_DIVIDE_BY_ZERO|浮点数除零错误");
	case EXCEPTION_FLT_INEXACT_RESULT          : return _T("FLT_INEXACT_RESULT|浮点运算结果失去精度");
	case EXCEPTION_FLT_INVALID_OPERATION       : return _T("FLT_INVALID_OPERATION|无效浮点数操作");
	case EXCEPTION_FLT_OVERFLOW                : return _T("FLT_OVERFLOW|浮点数溢出");
	case EXCEPTION_FLT_STACK_CHECK             : return _T("FLT_STACK_CHECK|浮点数处理堆栈溢出或浮点下溢");
	case EXCEPTION_FLT_UNDERFLOW               : return _T("FLT_UNDERFLOW|浮点下溢");
	case EXCEPTION_INT_DIVIDE_BY_ZERO          : return _T("INT_DIVIDE_BY_ZERO|整数除零错误");
	case EXCEPTION_INT_OVERFLOW                : return _T("INT_OVERFLOW|整数数溢出");
	case EXCEPTION_PRIV_INSTRUCTION            : return _T("PRIV_INSTRUCTION|当前处理器模式不允许的指令");
	case EXCEPTION_IN_PAGE_ERROR               : return _T("IN_PAGE_ERROR|访问不存在的页面");
	case EXCEPTION_ILLEGAL_INSTRUCTION         : return _T("ILLEGAL_INSTRUCTION|无效指令");
	case EXCEPTION_NONCONTINUABLE_EXCEPTION    : return _T("NONCONTINUABLE_EXCEPTION|发生无法继续运行的结构性异常");
	case EXCEPTION_STACK_OVERFLOW              : return _T("STACK_OVERFLOW|堆栈溢出");
	case EXCEPTION_INVALID_DISPOSITION         : return _T("INVALID_DISPOSITION|无效的异常处理配置");
	case EXCEPTION_GUARD_PAGE                  : return _T("GUARD_PAGE|页面访问出错");
	case EXCEPTION_INVALID_HANDLE              : return _T("INVALID_HANDLE|无效句柄");
	}

	return _T("UNKNOWN");
}

void SEH::DoCatch(bool bCatch)
{
	static LPTOP_LEVEL_EXCEPTION_FILTER old_filter = NULL;

	if (bCatch)
		old_filter = SetUnhandledExceptionFilter(UserTopExceptionFilter);
	else if (old_filter)
		SetUnhandledExceptionFilter(old_filter);
}

void SEH::DoCatchCpp(bool bCatch)
{
	static DWORD dwTlsIndex = 0;
	if (!dwTlsIndex)
		dwTlsIndex = TlsAlloc();

	assert(0xFFFFFFFF != dwTlsIndex);

	if (bCatch)
		TlsSetValue(dwTlsIndex, _set_se_translator(ThrowCppException));
	else
		_set_se_translator((_se_translator_function)TlsGetValue(dwTlsIndex));
}

void SEH::ThrowCppException(unsigned int uCode, _EXCEPTION_POINTERS* pException)
{
	PSEH->m_pReport->Report(pException);
	PSEH->m_pThrow->Throw(pException);
}

LONG SEH::UserTopExceptionFilter(_EXCEPTION_POINTERS *pException)
{
	PSEH->m_pReport->Report(pException);
	return EXCEPTION_EXECUTE_HANDLER;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SehThrowStd::Throw(_EXCEPTION_POINTERS *pException)
{
	TCHAR szMsg[128];
	_stprintf(szMsg, _T("SEH: Thread(0x%08X) Address(0x%p) Code(0x%08X) Type(%s)."),
		GetCurrentThreadId(),
		pException->ExceptionRecord->ExceptionAddress,
		pException->ExceptionRecord->ExceptionCode,
		SEH::Code2Str(pException->ExceptionRecord->ExceptionCode));

#ifdef _UNICODE
	char szTemp[512] = {0};
	WideCharToMultiByte(CP_ACP, 0, szMsg, -1, szTemp, (unsigned int)(wcslen(szMsg)*sizeof(wchar_t)), NULL, NULL);
	throw std::runtime_error(szTemp);
#else
	throw std::runtime_error(szMsg);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SehReport::SehReport()
{
	::GetModuleFileName(NULL, szModuleName_, MAX_PATH);
	szModuleBaseName_ = _tcsrchr(szModuleName_, _T('\\')) + 1;

	GetSystemInfo(&SystemInfo_);

	DWORD dwRet = 128;
	GetComputerName(szComputerName_, &dwRet);
	szComputerName_[dwRet] = _T('\0');
	dwRet = 64;
	GetUserName(szUserName_, &dwRet);
	szUserName_[dwRet] = _T('\0');

	OsVersionInfo_.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx((LPOSVERSIONINFO)&OsVersionInfo_))
	{
		OsVersionInfo_.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((LPOSVERSIONINFO)&OsVersionInfo_);
	}

	hDll_ = LoadLibrary(_T("dbghelp.dll"));
	if (!hDll_)
		hDll_ = LoadLibrary(_T("imagehlp.dll"));
	assert(hDll_);

	if (hDll_)
	{
		MiniDumpWriteDump_		= (MiniDumpWriteDumpFuncType)GetProcAddress(hDll_, "MiniDumpWriteDump");
		SymInitialize_			= (SymInitializeFuncType)GetProcAddress(hDll_, "SymInitialize");
		SymCleanup_				= (SymCleanupFuncType)GetProcAddress(hDll_, "SymCleanup");
		SymSetOptions_			= (SymSetOptionsFuncType)GetProcAddress(hDll_, "SymSetOptions");
		UnDecorateSymbolName_	= (UnDecorateSymbolNameFuncType)GetProcAddress(hDll_, "UnDecorateSymbolName");
		StackWalk_				= (StackWalkFuncType)GetProcAddress(hDll_, "StackWalk");
		SymFunctionTableAccess_	= (SymFunctionTableAccessFuncType)GetProcAddress(hDll_, "SymFunctionTableAccess");
		SymGetModuleBase_		= (SymGetModuleBaseFuncType)GetProcAddress(hDll_, "SymGetModuleBase");
		SymGetSymFromAddr_		= (SymGetSymFromAddrFuncType)GetProcAddress(hDll_, "SymGetSymFromAddr");
		SymGetLineFromAddr_		= (SymGetLineFromAddrFuncType)GetProcAddress(hDll_, "SymGetLineFromAddr");
		SymEnumerateModules_	= (SymEnumerateModulesFuncType)GetProcAddress(hDll_, "SymEnumerateModules");
		SymGetModuleInfo_		= (SymGetModuleInfoFuncType)GetProcAddress(hDll_, "SymGetModuleInfo");
		SymEnumSymbols_			= (SymEnumSymbolsFuncType)GetProcAddress(hDll_, "SymEnumSymbols");
	}
}

SehReport::~SehReport()
{
	if (hDll_)
		FreeLibrary(hDll_);
}

void SehReport::Report(_EXCEPTION_POINTERS* pException)
{
	m_lock.lock();

// 	SYSTEMTIME st;
// 	GetLocalTime(&st);
// 	TCHAR szFile[128];
// 
// 	_stprintf(szFile, _T(".\\SEH\\SE_%04d%02d%02dT%02d%02d%02d_%03d_%p.dmp"), 
// 		st.wYear,
// 		st.wMonth,
// 		st.wDay,
// 		st.wHour,
// 		st.wMinute,
// 		st.wSecond,
// 		st.wMilliseconds,
// 		pException->ExceptionRecord->ExceptionAddress);
// 
// 	if (MiniDumpWriteDump_)
// 	{
// 		__try
// 		{
// 			MiniDump(szFile, pException);
// 		}
// 		__except(EXCEPTION_EXECUTE_HANDLER)
// 		{
// 			assert(0);
// 		}
// 	}

	__try
	{
// 		_tcscpy(szFile+_tcslen(szFile)-3, _T("txt"));
// 		FILE *fp = _tfopen(szFile, _T("a+"));
// 		assert(fp);
// 		if (fp)
// 			Report(fp, pException);
// 		fclose(fp);
// 
// #if defined(_CONSOLE) //&& defined(_DEBUG)
// 		Report(stdout, pException);
// 		fflush(stdout);
// #endif
		Dump(pException);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		assert(0);
	}

	m_lock.unlock();
}

void SehReport::MiniDump(LPCTSTR szFile, _EXCEPTION_POINTERS* pException, MINIDUMP_TYPE Type)
{
	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = pException;
	eInfo.ClientPointers = FALSE;

	HANDLE hFile = ::CreateFile(
		szFile,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, 
		NULL
		);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MiniDumpWriteDump_(GetCurrentProcess(), GetCurrentProcessId(), hFile, Type, pException ? &eInfo : NULL, NULL, NULL);
		CloseHandle(hFile) ;
	}
}

void SehReport::Dump(_EXCEPTION_POINTERS* pException)
{
	REPORT(szModuleName_, T("\nThread(0x%08X) Address(0x%p) Code(0x%08X) Type(%s)\n",
		GetCurrentThreadId(),
		pException->ExceptionRecord->ExceptionAddress,
		pException->ExceptionRecord->ExceptionCode,
		SEH::Code2Str(pException->ExceptionRecord->ExceptionCode)), RPT_NOPREFIX|RPT_ERROR);

	if (!SymSetOptions_ || !SymInitialize_ || !SymCleanup_)
	{
		REPORT(szModuleName_, T("\n-- SymSetOptions load fail --\n"), RPT_NOPREFIX|RPT_ERROR);
		return;
	}

	SymSetOptions_(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME );//| SYMOPT_LOAD_ANYTHING

	if (!SymInitialize_(GetCurrentProcess(), 0, TRUE))
	{
		REPORT(szModuleName_, T("SymInitialize fail. GetLastError()=%d", GetLastError()), RPT_NOPREFIX|RPT_ERROR);
		return;
	}

	ReportContext(pException);
	ReportStackWalk(pException);
	ReportModules();
	ReportSymbols(pException);
	REPORT(szModuleName_, T("\n-- End --\n"), RPT_NOPREFIX|RPT_ERROR);

	SymCleanup_(GetCurrentProcess());
}

void SehReport::ReportContext(_EXCEPTION_POINTERS *ExceptionInfo)
{
	PCONTEXT pContext = ExceptionInfo->ContextRecord;
	REPORT(szModuleName_, T("\nContext:\n"), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  EAX:%08X  EBX:%08X\n", pContext->Eax, pContext->Ebx), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  ECX:%08X  EDX:%08X\n", pContext->Ecx, pContext->Edx), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  ESI:%08X  EDI:%08X\n", pContext->Esi, pContext->Edi), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  ECX:%08X  EDX:%08X\n", pContext->Ecx, pContext->Edx), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  ESI:%08X  EDI:%08X\n", pContext->Esi, pContext->Edi), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  EIP:%08X  ESP:%08X\n", pContext->Eip, pContext->Esp), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  EBP:%08X  EFL:%08X\n", pContext->Ebp, pContext->EFlags), RPT_NOPREFIX|RPT_ERROR);
	REPORT(szModuleName_, T("  CS:%04X  DS:%04X  ES:%04X  FS:%04X  GS:%04X\n",
		pContext->SegCs, pContext->SegDs, pContext->SegEs, pContext->SegFs, pContext->SegGs), RPT_NOPREFIX|RPT_ERROR);
}

void SehReport::ReportStackWalk(_EXCEPTION_POINTERS *ExceptionInfo)
{
	REPORT(szModuleName_, T("\nStack Walk:\n"), RPT_NOPREFIX|RPT_ERROR);

	STACKFRAME sf;
	memset( &sf, 0, sizeof(sf) );

	sf.AddrPC.Offset       = ExceptionInfo->ContextRecord->Eip;
	sf.AddrPC.Mode         = AddrModeFlat;
	sf.AddrStack.Offset    = ExceptionInfo->ContextRecord->Esp;
	sf.AddrStack.Mode      = AddrModeFlat;
	sf.AddrFrame.Offset    = ExceptionInfo->ContextRecord->Ebp;
	sf.AddrFrame.Mode      = AddrModeFlat;

	CONTEXT Context		   = *ExceptionInfo->ContextRecord;

	MEMORY_BASIC_INFORMATION mbi;
	WORD wMachine = IMAGE_FILE_MACHINE_I386;

	if (VirtualQuery((LPCVOID)ExceptionInfo->ContextRecord->Eip, &mbi, sizeof(mbi)))
	{
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)mbi.AllocationBase;
		PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(pDosHeader + pDosHeader->e_lfanew);
		wMachine = pNtHeader->FileHeader.Machine;
	}

	for (int level = 0, i, j; ; level++)
	{
		if (!StackWalk_(
			IMAGE_FILE_MACHINE_I386,
			GetCurrentProcess(),
			GetCurrentThread(),
			&sf,
			&Context,
			NULL,
			SymFunctionTableAccess_,
			SymGetModuleBase_,
			NULL 
			) 
			)
			break;

		if (!sf.AddrFrame.Offset) 
			break;   

		REPORT(szModuleName_, T("\n  Call queue (%d):\n  Address:%08X\n  Stack Info: Frame(EBP) %08X Stack(ESP) %08X\n",
			level, sf.AddrPC.Offset, sf.AddrFrame.Offset, sf.AddrStack.Offset), RPT_NOPREFIX|RPT_ERROR);

		char *pStackPtr = (char *)(sf.AddrStack.Offset);
		for (i = 0, j; i<4; i++)
		{
			if (IsBadReadPtr(pStackPtr, 16))
				break;

			REPORT(szModuleName_, T("  %p:", pStackPtr), RPT_NOPREFIX|RPT_ERROR);
			for (j = 0; j<16; j++, pStackPtr++)
				REPORT(szModuleName_, T(" %02X", (BYTE)*pStackPtr), RPT_NOPREFIX|RPT_ERROR);

			REPORT(szModuleName_, T(" "), RPT_NOPREFIX|RPT_ERROR);

			for (j = 0, pStackPtr-=16; j<16; j++, pStackPtr++)
				REPORT(szModuleName_, T("%c", ((BYTE)(*pStackPtr)>0x80)||((BYTE)(*pStackPtr)<0x20) ? '.' : *pStackPtr), RPT_NOPREFIX|RPT_ERROR);
			REPORT(szModuleName_, T("\n"), RPT_NOPREFIX|RPT_ERROR);
		}

		TCHAR szModuleName[MAX_PATH];
		MEMORY_BASIC_INFORMATION mbi;

		if (VirtualQuery((LPCVOID)sf.AddrPC.Offset, &mbi, sizeof(mbi)))//
		{
			if (GetModuleFileName((HMODULE)mbi.AllocationBase, szModuleName, MAX_PATH))
				REPORT(szModuleName_, T("  Module:%s\n", szModuleName), RPT_NOPREFIX|RPT_ERROR);
		}

		BYTE btBuffer[sizeof(IMAGEHLP_SYMBOL) + 1024];
		PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)btBuffer;
		pSymbol->SizeOfStruct = sizeof(btBuffer);
		pSymbol->MaxNameLength = 1024;
		DWORD dwDisplacement = 0;  

		if (SymGetSymFromAddr_(GetCurrentProcess(), sf.AddrPC.Offset, &dwDisplacement, pSymbol))
			REPORT(szModuleName_, T("  Function:%s(%08X+%04X)\n", pSymbol->Name, sf.AddrPC.Offset-dwDisplacement, dwDisplacement), RPT_NOPREFIX|RPT_ERROR);

		IMAGEHLP_LINE line;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

		dwDisplacement = 0;
		if (SymGetLineFromAddr_(GetCurrentProcess(), sf.AddrPC.Offset, &dwDisplacement, &line))
			REPORT(szModuleName_, T("  Source:%s Line:%d\n", line.FileName, line.LineNumber), RPT_NOPREFIX|RPT_ERROR);

		for (i = 0; i<4; i++)
			REPORT(szModuleName_, T("  Parameter[%d]=%08X\n", i, sf.Params[i]), RPT_NOPREFIX|RPT_ERROR);
	}
}

void SehReport::ReportModules()
{
	if (SymEnumerateModules_ && SymGetModuleInfo_)
	{
		struct EC{
			int   id_;
			FILE* fp_;
			SymGetModuleInfoFuncType f_;
		}ec;
		ec.id_ = 1;
		ec.fp_ = 0;
		ec.f_ = SymGetModuleInfo_;
		REPORT(szModuleName_, T("\nModules: \n"), RPT_NOPREFIX|RPT_ERROR);

		BOOL b = SymEnumerateModules_(GetCurrentProcess(), EnumerateModulesProc, &ec);
	}
}

BOOL SehReport::EnumerateModulesProc(PSTR ModuleName, ULONG BaseOfDll, PVOID UserContext)
{
	struct EC{
		int   id_;
		FILE* fp_;
		SymGetModuleInfoFuncType f_;
	}* pec = (EC*)UserContext;

	IMAGEHLP_MODULE im;
	im.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
	TCHAR szTime[64] = _T("");

	if ((*pec->f_)(GetCurrentProcess(), BaseOfDll, &im))
	{
// 		_tcsftime(szTime, 64, _T("%Y-%m-%d %H:%M:%S %A"), localtime((time_t*)&im.TimeDateStamp));
		REPORT(szModuleName_, T("  Module %02d:  File:%s\n", pec->id_++, im.LoadedImageName), RPT_NOPREFIX|RPT_ERROR);
		REPORT(szModuleName_, T("    Timestamp: FileSize:%d\n    Address:%08X CheckSum:%08X SymbolNumber:%d Export:%s\n"
// 			, szTime
			, im.ImageSize
			, im.BaseOfImage
			, im.CheckSum
			, im.NumSyms
			, SymType2Str(im.SymType)
			), RPT_NOPREFIX|RPT_ERROR);
	}
	else
		REPORT(szModuleName_, T("Module %03d: %s SymGetModuleInfo Fail:%d\n", pec->id_++, ModuleName, GetLastError()), RPT_NOPREFIX|RPT_ERROR);

	return TRUE;
}

void SehReport::ReportSymbols(_EXCEPTION_POINTERS *ExceptionInfo)
{
	MEMORY_BASIC_INFORMATION mbi;
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	if (SymEnumSymbols_ && VirtualQuery((LPCVOID)ExceptionInfo->ContextRecord->Eip, &mbi, sizeof(mbi)))
	{
		struct EC{
			int   id_;
			FILE* fp_;
		}ec;
		ec.id_ = 1;
		ec.fp_ = 0;

		REPORT(szModuleName_, T("\nAddress:%08X Symbol: \n", (DWORD)mbi.AllocationBase), RPT_NOPREFIX|RPT_ERROR);
		SymEnumSymbols_(GetCurrentProcess(), (DWORD)mbi.AllocationBase, "", EnumSymbolsProc, &ec);
	}
}

BOOL SehReport::EnumSymbolsProc(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
	struct EC{
		int   id_;
		FILE* fp_;
	}* pec = (EC*)UserContext;

	if (!IsBadWritePtr((LPVOID)pSymInfo->Address, 4) && SYMFLAG_LOCAL&pSymInfo->Flags )
		REPORT(szModuleName_, T("  Symbol %03d: %p Value:%08X Flag:%08X Name:%s \n",
		pec->id_++, (LPVOID)pSymInfo->Address, *(LPDWORD)(LPVOID)pSymInfo->Address, pSymInfo->Flags, pSymInfo->Name),
		RPT_NOPREFIX|RPT_ERROR);
	return TRUE;
}

LPCTSTR SehReport::SymType2Str(SYM_TYPE st)
{
	switch(st) {
	case SymCoff		: return _T("COFF symbols");        
	case SymCv			: return _T("CodeView symbols");        
	case SymDeferred	: return _T("loading deferred");        
	case SymDia			: return _T("DIA symbols");        
	case SymExport		: return _T("DLL export table");        
	case SymNone		: return _T("No symbols");        
	case SymPdb			: return _T("PDB symbols");        
	case SymSym			: return _T(".SYM file");   
	}
	return _T("Unknown");   
}
