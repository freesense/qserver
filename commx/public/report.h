
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

/**@defgroup logcommon ��־����
@ingroup log
ȡֵ��Χ0~255�������ֵ�����û��Զ���
@{
*/
#define RPT_DEBUG		0x10
#define RPT_INFO		0x20
#define RPT_WARNING		0x40
#define RPT_ERROR		0x80
#define RPT_CRITICAL	0xf0
/** @} */

/**@defgroup logspecial ��־�����־λ
@ingroup log
�ⲿ��ʹ�û���㣬�����ֵ�����û��Զ���
@{
*/
#define RPT_IGNORE		0x00000100		/** ����ʾ���κ�����ն��� [6/25/2009 xinl] */
#define RPT_NOPREFIX	0x00000200		/** ������־ǰ��ĸ�����Ϣ [6/25/2009 xinl] */
#define RPT_IMPORTANT	0x00000400		/** ��Ҫ��Ϣ [6/25/2009 xinl] */
/** @} */

///////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief ��ʹ�õĸ�ʽ���������
@ingroup log
ֱ����REPORT��غ���ʹ��
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

/**@brief ������־���
@ingroup log
��stdout���
 */
class COMMXAPI CReportBase
{
public:
	/**@brief ���һ����־
	
	@param tn ��־ʱ��
	@return
		- true ��������־������������־
		- false ֹͣ����������־
	@see CReportBox::AddReport
	 */
	virtual bool AddReport(string &tn, string &mn, int tid,	string &fn, int ln, string &sr, int lv = RPT_IGNORE);
	LockSingle m_lock;	/** ����д��־��ͻ���� [6/25/2009 xinl] */
};

/**@brief ��־�ļ����
@ingroup log
д����־�ļ���ÿ��һ��
 */
class COMMXAPI CReportFile : public CReportBase
{
public:
	/**@brief 
	
	@param lpszFileName �ļ�����־��ϵͳ����������Ļ����ϻ�Ҫ����ǰ׺�ͺ�׺�������γ��������ļ���
	 */
	CReportFile(const char *lpszFileName, unsigned int nDay = 30);

	/**@brief 
	
	@see CReportBase::AddReport
	 */
	virtual bool AddReport(string &tn, string &mn, int tid,	string &fn, int ln, string &sr, int lv = RPT_IGNORE);

	/**@brief �����ʷ��־�ļ�
	
	 */
	void Clean();

private:
	/**@brief �����µ���־�ļ�
	@param tn �µ���־��ʱ��
	�ú��������־ʱ�䣬������ֹ���һ�죬�������µ���־�ļ�
	 */
	void NewFileName(string &tn);

	ofstream m_of;					/** Ҫд����ļ����� [6/25/2009 xinl] */
	string m_FileNamePrefix;		/** �ļ���ǰ׺ [6/25/2009 xinl] */
	string m_FileNameDate;			/** ��־�ļ������� [6/25/2009 xinl] */
	unsigned int m_nDay;			/** �������������־�ļ� [1/8/2010 xinl] */
};

/**@brief ��־������
@ingroup log
�������е���־������������־����־�����еĴ���
 */
class COMMXAPI CReportBox
{
public:
	/**@brief 
	@return ��������
	�������ǵ���������һ��������ֻ����һ��
	 */
	static CReportBox* Instance();

	/**@brief ������һ���Ѵ��ڵ���־������
	@param prb 
	
	 */
	static void Instance(CReportBox *prb);

	/**@brief �ر���־������
	@param pr Ҫ�رյĶ���ָ�룬���ΪNULL���ر�����
	
	 */
	static void Close(CReportBase *pr = NULL);

	/**@brief ���16��������
	@param szmn ģ������
	@param tid �߳�ID
	@param szFile ���Դ�ļ���
	@param ln ���Դ�ļ��к�
	@param lpData ������ݿ�ָ��
	@param nLength ������ݿ鳤��
	@param sr ��Ҫ˵��
	@param lv �������
	 */
	void HexDump(const char *szmn, int tid, const char *szFile, int ln,	const void *lpData,
		unsigned int nLength, string &sr, int lv = RPT_DEBUG);

	/**@brief ��logϵͳ���һ����־

	@param szmn ģ������
	@param tid �߳�ID
	@param szFile �ļ�����
	@param ln �к�
	@param sr ��־
	@param lv ����
	 */
	void AddReport(const char *szmn, int tid, const char *szFile, int ln, string &sr, int lv = RPT_IGNORE);

	/**@brief ��logϵͳ����һ����������
	@param pUnit ����������ָ��
	
	 */
	void AddReportUnit(CReportBase *pUnit);

private:
	CReportBox();
	~CReportBox();

	LockSingle m_lock;						/** �������� [6/25/2009 xinl] */
	deque<CReportBase*> m_vReportUnit;		/** ��־�������� [6/25/2009 xinl] */
	static CReportBox *m_pInstance;			/** �������� [6/25/2009 xinl] */
};

/**@defgroup logmarco ��־��
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
