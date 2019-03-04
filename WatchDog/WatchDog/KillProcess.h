/************************************************************************************
*
* 文件名  : KillProcess.h
*
* 文件描述: 结束指定进程
*
* 创建人  : dingjing, 2009-02-10
*
* 版本号  : 1.0
*
* 修改记录:
*
************************************************************************************/
#ifndef _KILLPROCESS_H_
#define _KILLPROCESS_H_
#include <windows.h>
#include <tlhelp32.h>


//定义日志的级别
#ifndef	LT_DEBUG
#define LT_DEBUG		0x10
#endif

#ifndef	LT_INFO
#define LT_INFO		    0x20
#endif

#ifndef	LT_WARNING
#define LT_WARNING		0x30
#endif

#ifndef	LT_ERROR
#define LT_ERROR		0x40
#endif
/////////////////////////////////////////////////////////////////////////////////////
class CKillProcess
{
public:
	CKillProcess();
	virtual ~CKillProcess();

	/********************************************************************************
	* 函数名: KillProc
	* 参  数:
	*		  DWORD dwProcessID [IN] : 进程ID
	*         LPCSTR lpExeFile  [IN] : 程序的名称，包括.exe， 例如test.exe
	* 功  能: 结束指定进程
	* 返回值: 连接成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL KillProc(DWORD dwProcessID, LPCSTR lpExeFile = NULL);

protected:
	/********************************************************************************
	* 函数名: LogEvent
	* 参  数:
	*		  WORD nLogType [IN] : 日志的类型
	*                              LT_DEBUG   : 表示是调试信息
	*						       LT_INFO    : 表示是提示信息
	*							   LT_WARNING : 表示是警告信息
	*                              LT_ERROR   : 表示是错误信息
	* 功  能: 处理日志
	********************************************************************************/
	virtual void LogEvent(WORD nLogType, LPCSTR pFormat, ...);

private:
};

/////////////////////////////////////////////////////////////////////////////////////
#endif