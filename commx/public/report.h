
#ifndef __REPORT_BOX_H__
#define __REPORT_BOX_H__

#pragma warning(disable:4251 4996)

#include "synch.h"
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <deque>
#include <fstream>
#include <iostream>
#include <sys/timeb.h>
#include <time.h>
#include <ctype.h>
#include "commxapi.h"
#ifdef _POSIX_C_SOURCE
#include <pthread.h>
#endif

using std::string;
using std::deque;
using std::ofstream;
using std::cout;
using std::endl;
using std::ios;

/**@defgroup log log
*/

/**@defgroup logcommon 日志级别
@ingroup log
取值范围0~255，多余的值留给用户自定义
@{
*/
#define RPT_DEBUG		0x10
#define RPT_INFO		0x20
#define RPT_WARNING		0x40
#define RPT_ERROR		0x80
#define RPT_CRITICAL	0xf0
/** @} */

/**@defgroup logspecial 日志特殊标志位
@ingroup log
这部分使用或计算，多余的值留给用户自定义
@{
*/
#define RPT_IGNORE		0x00000100		/** 不显示在任何输出终端中 [6/25/2009 xinl] */
#define RPT_NOPREFIX	0x00000200		/** 忽略日志前面的辅助信息 [6/25/2009 xinl] */
#define RPT_IMPORTANT	0x00000400		/** 重要信息 [6/25/2009 xinl] */
/** @} */

///////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief 宏使用的格式化输出函数
@ingroup log
直接在REPORT相关宏中使用
 */
inline string T(const char *fmt, ...)
{
	char szMsg[1024];
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	vsnprintf(szMsg, sizeof(szMsg), fmt, arg_ptr);
	va_end(arg_ptr);
	return szMsg;
}

/**@brief 基础日志输出
@ingroup log
向stdout输出
 */
class COMMXAPI CReportBase
{
public:
	/**@brief 输出一条日志
	
	@param tn 日志时间
	@return
		- true 继续沿日志容器链传递日志
		- false 停止继续传递日志
	@see CReportBox::AddReport
	 */
	virtual bool AddReport(string &tn, string &mn, int tid,	string &fn, int ln, string &sr, int lv = RPT_IGNORE);
	LockSingle m_lock;	/** 避免写日志冲突的锁 [6/25/2009 xinl] */
};

/**@brief 日志文件输出
@ingroup log
写入日志文件，每天一个
 */
class COMMXAPI CReportFile : public CReportBase
{
public:
	/**@brief 
	
	@param lpszFileName 文件名标志，系统在这个参数的基础上还要加上前缀和后缀，才能形成真正的文件名
	 */
	CReportFile(const char *lpszFileName, unsigned int nDay = 30);

	/**@brief 
	
	@see CReportBase::AddReport
	 */
	virtual bool AddReport(string &tn, string &mn, int tid,	string &fn, int ln, string &sr, int lv = RPT_IGNORE);

	/**@brief 清除历史日志文件
	
	 */
	void Clean();

private:
	/**@brief 建立新的日志文件
	@param tn 新的日志的时间
	该函数检查日志时间，如果发现过了一天，将建立新的日志文件
	 */
	void NewFileName(string &tn);

	ofstream m_of;					/** 要写入的文件对象 [6/25/2009 xinl] */
	string m_FileNamePrefix;		/** 文件名前缀 [6/25/2009 xinl] */
	string m_FileNameDate;			/** 日志文件的日期 [6/25/2009 xinl] */
	unsigned int m_nDay;			/** 保留多少天的日志文件 [1/8/2010 xinl] */
};

/**@brief 日志管理器
@ingroup log
管理所有的日志容器，处理日志在日志容器中的传递
 */
class COMMXAPI CReportBox
{
public:
	/**@brief 
	@return 单例对象
	本对象是单例对象，在一个进程中只存在一个
	 */
	static CReportBox* Instance();

	/**@brief 连接另一个已存在的日志管理器
	@param prb 
	
	 */
	static void Instance(CReportBox *prb);

	/**@brief 关闭日志管理器
	@param pr 要关闭的对象指针，如果为NULL，关闭自身
	
	 */
	static void Close(CReportBase *pr = NULL);

	/**@brief 输出16进制数据
	@param szmn 模块名称
	@param tid 线程ID
	@param szFile 输出源文件名
	@param ln 输出源文件行号
	@param lpData 输出数据块指针
	@param nLength 输出数据块长度
	@param sr 简要说明
	@param lv 输出级别
	 */
	void HexDump(const char *szmn, int tid, const char *szFile, int ln,	const void *lpData,
		unsigned int nLength, string &sr, int lv = RPT_DEBUG);

	/**@brief 向log系统输出一条日志

	@param szmn 模块名称
	@param tid 线程ID
	@param szFile 文件名称
	@param ln 行号
	@param sr 日志
	@param lv 级别
	 */
	void AddReport(const char *szmn, int tid, const char *szFile, int ln, string &sr, int lv = RPT_IGNORE);

	/**@brief 向log系统增加一个处理容器
	@param pUnit 新增的容器指针
	
	 */
	void AddReportUnit(CReportBase *pUnit);

private:
	CReportBox();
	~CReportBox();

	LockSingle m_lock;						/** 容器链锁 [6/25/2009 xinl] */
	deque<CReportBase*> m_vReportUnit;		/** 日志处理器链 [6/25/2009 xinl] */
	static CReportBox *m_pInstance;			/** 单例对象 [6/25/2009 xinl] */
};

/**@defgroup logmarco 日志宏
@ingroup log
@{
*/
#ifdef _WIN32
#define REPORT(M, R, Level) \
	CReportBox::Instance()->AddReport(M, GetCurrentThreadId(), __FILE__, __LINE__, R, Level)
#define HEXDUMP(M, D, Len, R, Level) \
	CReportBox::Instance()->HexDump(M, GetCurrentThreadId(), __FILE__, __LINE__, D, Len, R, Level)
#elif defined _POSIX_C_SOURCE
#define REPORT(M, R, Level) \
	CReportBox::Instance()->AddReport(M, (int)pthread_self(), __FILE__, __LINE__, R, Level)
#define HEXDUMP(M, D, Len, R, Level) \
	CReportBox::Instance()->HexDump(M, (int)pthread_self(), __FILE__, __LINE__, D, Len, R, Level)
#endif

#ifdef _DEBUG
#define DEBUG_REPORT(M, R, Level) REPORT(M, R, Level)
#define DEBUG_DUMP(M, D, Len, R, Level) HEXDUMP(M, D, Len, R, Level)
#else
#define DEBUG_REPORT(M, R, Level) {}
#define DEBUG_DUMP(M, D, Len, R, Level) {}
#endif

#define REPORT_RUN(M, R, Level, V) {REPORT(M, R, Level); V;}
#define HEXDUMP_RUN(M, D, Len, R, Level, V) {HEXDUMP(M, D, Len, R, Level); V;}
#define CHECK(X, M, R, Level) {if (X) REPORT(M, R, Level);}
#define CHECKDUMP(X, M, D, Len, R, Level) {if (X) HEXDUMP(M, D, Len, R, Level);}
#define CHECK_RUN(X, M, R, Level, V) {if (X) REPORT_RUN(M, R, Level, V);}
#define CHECKDUMP_RUN(X, M, D, Len, R, Level, V) {if (X) HEXDUMP_RUN(M, D, Len, R, Level, V);}
/** @} */

#endif
