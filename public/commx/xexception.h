/**@file  xexception.cpp
 * @brief 异常接口
 */

#ifndef __XEXCEPTION_COMMX_H__
#define __XEXCEPTION_COMMX_H__

#include "commxapi.h"
#include "synch.h"
#include <cassert>
#include <ctime>
#include <stdexcept>
#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>

/**@brief 报告基类


 */
class COMMXAPI SehReportBase
{
public:
	virtual ~SehReportBase() {};
	virtual void Report(_EXCEPTION_POINTERS* pException) = 0;
};

/**@brief 异常处理基类


 */
class COMMXAPI SehThrowBase
{
public:
	virtual ~SehThrowBase() {};
	virtual void Throw(_EXCEPTION_POINTERS* pException) = 0;
};

/**@brief 异常处理类


 */
 class COMMXAPI SEH
 {
 public:
	 static SEH* Instance();
	 static int Initialize(SehReportBase* pReport, SehThrowBase *pThrow);
	 static void Close();
	 static LPCTSTR Code2Str(DWORD uCode);

	 //! 捕捉结构性异常
	 void DoCatch(bool bCatch = true);

	 //! 将结构性异常转换抛出C++异常
	 void DoCatchCpp(bool bCatch = true);

	 //! 将结构性异常转换抛出C++异常入口
	 static void _cdecl ThrowCppException(unsigned int uCode, _EXCEPTION_POINTERS* pException);

	 //! C模式异常捕捉入口
	 static LONG WINAPI UserTopExceptionFilter(_EXCEPTION_POINTERS *pException);

 private:
	 static SEH* m_instance;
	 SehReportBase *m_pReport;
	 SehThrowBase *m_pThrow;
 };

#define PSEH SEH::Instance()

 /**@brief 向日志链输出异常记录


 */
 class COMMXAPI SehReport : public SehReportBase
 {
 public:
	 SehReport();
	 virtual ~SehReport();
	 virtual void Report(_EXCEPTION_POINTERS* pException);						//!< 报告

 protected:
	 void Dump(_EXCEPTION_POINTERS* pException);				//!< 向日志链输出相关信息
	 void ReportContext(_EXCEPTION_POINTERS *ExceptionInfo);	//!< 寄存器内容
	 void ReportStackWalk(_EXCEPTION_POINTERS *ExceptionInfo);	//!< 堆栈迭代
	 void ReportModules();										//!< 模块列表
	 void ReportSymbols(_EXCEPTION_POINTERS *ExceptionInfo);	//!< 符号列表
	 void MiniDump(LPCTSTR szFile, _EXCEPTION_POINTERS* pException, MINIDUMP_TYPE Type = MiniDumpNormal);		//!< 内存拷贝
	 static LPCTSTR SymType2Str(SYM_TYPE st);							//!< 符号输出类型
	 static LPCTSTR Code2Str(DWORD uCode);								//!< 异常代码类型
	 static BOOL CALLBACK EnumSymbolsProc(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);
	 static BOOL CALLBACK EnumerateModulesProc(PSTR ModuleName, ULONG BaseOfDll, PVOID UserContext);

	 typedef BOOL (WINAPI *MiniDumpWriteDumpFuncType)(
		 IN HANDLE hProcess,
		 IN DWORD ProcessId,
		 IN HANDLE hFile,
		 IN MINIDUMP_TYPE DumpType,
		 IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
		 IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
		 IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
		 );//dbghelp.dll
	 typedef BOOL (WINAPI * SymInitializeFuncType)(
		 IN HANDLE   hProcess,
		 IN LPSTR    UserSearchPath,
		 IN BOOL     fInvadeProcess
		 );//dbghelp.dll
	 typedef BOOL (WINAPI *SymCleanupFuncType)(
		 IN HANDLE hProcess
		 );//dbghelp.dll
	 typedef DWORD (WINAPI *SymSetOptionsFuncType)(
		 DWORD SymOptions  
		 );//dbghelp.dll
	 typedef BOOL (WINAPI * StackWalkFuncType)(
		 DWORD                             MachineType,
		 HANDLE                            hProcess,
		 HANDLE                            hThread,
		 LPSTACKFRAME                      StackFrame,
		 LPVOID                            ContextRecord,
		 PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
		 PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
		 PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
		 PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
		 );//dbghelp.dll
	 typedef LPVOID (WINAPI *SymFunctionTableAccessFuncType)(
		 HANDLE  hProcess,
		 DWORD   AddrBase
		 );//dbghelp.dll
	 typedef DWORD (WINAPI *SymGetModuleBaseFuncType)(
		 IN  HANDLE              hProcess,
		 IN  DWORD               dwAddr
		 );//dbghelp.dll
	 typedef DWORD (WINAPI *UnDecorateSymbolNameFuncType)(
		 PCSTR DecoratedName,
		 PSTR UnDecoratedName,
		 DWORD UndecoratedLength,
		 DWORD Flags
		 );//dbghelp.dll
	 typedef BOOL (WINAPI *SymGetSymFromAddrFuncType)(
		 IN  HANDLE              hProcess,
		 IN  DWORD               dwAddr,
		 OUT PDWORD              pdwDisplacement,
		 OUT PIMAGEHLP_SYMBOL    Symbol
		 );//dbghelp.dll
	 typedef BOOL (WINAPI *SymGetLineFromAddrFuncType)(
		 HANDLE hProcess,
		 DWORD dwAddr,
		 PDWORD pdwDisplacement,
		 PIMAGEHLP_LINE Line
		 );//dbghelp.dll
	 typedef BOOL (WINAPI *SymEnumerateModulesFuncType)(
		 HANDLE hProcess,                                
		 PSYM_ENUMMODULES_CALLBACK EnumModulesCallback,  
		 PVOID UserContext                               
		 );//dbghelp.dll
	 typedef BOOL (WINAPI *SymGetModuleInfoFuncType)(
		 HANDLE hProcess,              
		 DWORD dwAddr,                 
		 PIMAGEHLP_MODULE ModuleInfo  
		 );//dbghelp.dll
	 typedef BOOL (WINAPI *SymEnumSymbolsFuncType)(
		 HANDLE  hProcess,
		 ULONG64  BaseOfDll,
		 PCSTR  Mask,
		 PSYM_ENUMERATESYMBOLS_CALLBACK  EnumSymbolsCallback,
		 PVOID  UserContext
		 );//dbghelp.dll

	 MiniDumpWriteDumpFuncType			MiniDumpWriteDump_;
	 SymInitializeFuncType				SymInitialize_;
	 SymCleanupFuncType					SymCleanup_;
	 SymSetOptionsFuncType				SymSetOptions_;
	 UnDecorateSymbolNameFuncType		UnDecorateSymbolName_;
	 StackWalkFuncType					StackWalk_;
	 SymFunctionTableAccessFuncType		SymFunctionTableAccess_;
	 SymGetModuleBaseFuncType			SymGetModuleBase_;
	 SymGetSymFromAddrFuncType			SymGetSymFromAddr_;
	 SymGetLineFromAddrFuncType			SymGetLineFromAddr_;
	 SymEnumerateModulesFuncType		SymEnumerateModules_;
	 SymGetModuleInfoFuncType			SymGetModuleInfo_;
	 SymEnumSymbolsFuncType				SymEnumSymbols_;

	 HINSTANCE							hDll_;
	 LockSingle							m_lock;
	 TCHAR								szComputerName_[128];
	 TCHAR								szUserName_[64];
	 LPCTSTR							szModuleBaseName_;
	 SYSTEM_INFO						SystemInfo_;
	 OSVERSIONINFOEX					OsVersionInfo_;

	 static TCHAR						szModuleName_[MAX_PATH];
 };

 /**@brief 抛出标准C++异常
 
 
  */
 class COMMXAPI SehThrowStd : public SehThrowBase
 {
 public:
	 virtual void Throw(_EXCEPTION_POINTERS *pException);
 };

#endif
