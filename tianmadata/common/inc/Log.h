//Log.h
//
#ifndef _LOG_H_
#define _LOG_H_
#include "stdio.h"
/////////////////////////////////////////////////////////////////////////////////

#define _SH_DENYRW      0x10    /* deny read/write mode */
#define _SH_DENYWR      0x20    /* deny write mode */
#define _SH_DENYRD      0x30    /* deny read mode */
#define _SH_DENYNO      0x40    /* deny none mode */
#define _SH_SECURE      0x80    /* secure mode */

#define MN_BUF_SIZE                 60   //如果不是8的整数倍会出错，
#define OPENMODE_BUF_SIZE           16

class CLog
{
public:
	CLog();
	~CLog();

	BOOL AddLog(const char* pData, int nLen);
	
	BOOL Open(const char* pLogPath, const char* pModuleName, 
		const char* pOpenMode = "a+", int nShareFlag = _SH_DENYNO);
	void Close();
	
	BOOL IsOpen();
	BOOL Reopen();

	void GetWorkPath(char* pPath, int nMaxBuf);
	BOOL IsPathExist(LPCTSTR lpPath);
	BOOL CreatePath(LPCTSTR lpPath);
protected:
	unsigned int GetCurrentDate(BOOL& bNewDate);
	void GetCurTime(char* pCurTime, int nBufLen);  
	  
	FILE*              m_pFile;
	char               m_chLogPath[MAX_PATH];
	char               m_chModuleName[MN_BUF_SIZE];
	unsigned int       m_nOldDate;
	char               m_chOpenMode[OPENMODE_BUF_SIZE];	
	int                m_nShareFlag;
};






















/////////////////////////////////////////////////////////////////////////////////
#endif