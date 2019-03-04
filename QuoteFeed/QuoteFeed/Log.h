//Log.h
//

#ifndef _LOG_H_
#define _LOG_H_

//#define VERIFY_DATA
#include "CommonStruct.h"
/////////////////////////////////////////////////////////////////////////////////

#define _SH_DENYRW      0x10    /* deny read/write mode */
#define _SH_DENYWR      0x20    /* deny write mode */
#define _SH_DENYRD      0x30    /* deny read mode */
#define _SH_DENYNO      0x40    /* deny none mode */
#define _SH_SECURE      0x80    /* secure mode */

#define MN_BUF_SIZE                 64   
#define OPENMODE_BUF_SIZE           16

class CLog
{
public:
	CLog();
	~CLog();

	BOOL AddLog(const char* pData, int nLen);
	
	void Close();
	BOOL IsOpen();
	BOOL Reopen();

	void ParseDataToLog(const char *pData, int nLen);
	BOOL IsPathExist(LPCTSTR lpPath);
	BOOL CreatePath(LPCTSTR lpPath);

protected:
	unsigned int GetCurrentDate(BOOL& bNewDate);
	void GetCurTime(char* pCurTime, int nBufLen);

#ifdef VERIFY_DATA
	void CheckPackHdr(tagNewPackInfo* pNewPackInfo, BOOL bSZXX);
	void CheckFieldInfo(tagFieldInfo* pFieldInfo, char* pStockCode, UINT nMarketType);
#endif

	FILE*              m_pFile;
	unsigned int       m_nOldDate;
	char               m_chOpenMode[OPENMODE_BUF_SIZE];
	int                m_nShareFlag;
	char			   m_cMode;
};






















/////////////////////////////////////////////////////////////////////////////////
#endif