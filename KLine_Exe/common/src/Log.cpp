//Log.cpp
//
#include "stdafx.h"
#include "Log.h"

///////////////////////////////////////
#define LOG_BUF_LEN      1000
#define FIELD_BUF_LEN    50
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CLog::CLog()
{
	m_pFile           = NULL;
	m_nOldDate        = 0;
	
	m_chLogPath[0]    = 0;
	m_chModuleName[0] = 0;
	m_chOpenMode[0]   = 0;
	m_nShareFlag      = _SH_DENYNO;
}

CLog::~CLog()
{
}

BOOL CLog::Open(const char *pLogPath, const char *pModuleName, 
				const char *pOpenMode, int nShareFlag)
{
	if(IsOpen())
	{
		Close();
	}

	if(pModuleName == NULL)
		return FALSE;

	if(pLogPath == NULL)
	{
		char chLogPath[MAX_PATH];
		GetWorkPath(chLogPath, MAX_PATH);
		strcpy_s(m_chLogPath, MAX_PATH, chLogPath);
	}
	else
	{
		if (!CValidFunction::IsPathExist(pLogPath)) //目录不存在则创建
		{
			if (!CValidFunction::CreatePath(pLogPath))
				return FALSE;
		}
		strcpy_s(m_chLogPath, MAX_PATH, pLogPath);
	}

	strcpy_s(m_chModuleName, MN_BUF_SIZE, pModuleName);

	BOOL bNewDate = FALSE;
	unsigned int nCurDate = GetCurrentDate(bNewDate);

	char chLogName[MAX_PATH];
	sprintf_s(chLogName, MAX_PATH, "%s\\%s%d.log", m_chLogPath, m_chModuleName, nCurDate);

	strcpy_s(m_chOpenMode, OPENMODE_BUF_SIZE, pOpenMode);
	m_nShareFlag = nShareFlag;
	m_pFile = _fsopen(chLogName, pOpenMode, nShareFlag);
	if(m_pFile == NULL)
		return FALSE;
	else
		return TRUE;
}

void CLog::Close()
{
	if(m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

BOOL CLog::IsOpen()
{
	if(m_pFile != NULL)
		return TRUE;
	else
		return FALSE;
}

void CLog::GetCurTime(char* pCurTime, int nBufLen)
{
	time_t  timeCur;
	time(&timeCur);

	tm tmCur;
	localtime_s(&tmCur, &timeCur);

	if(pCurTime != NULL && nBufLen > 0)
	{
		sprintf_s(pCurTime, nBufLen, "%04d-%02d-%02d %02d:%02d:%02d ", tmCur.tm_year+1900, tmCur.tm_mon+1, 
			tmCur.tm_mday, tmCur.tm_hour, tmCur.tm_min, tmCur.tm_sec);
	}
}

unsigned int CLog::GetCurrentDate(BOOL& bNewDate)
{
	time_t  timeCur;
	time(&timeCur);

	tm tmCur;
	localtime_s(&tmCur, &timeCur);

	unsigned int nCurDate = (tmCur.tm_year+1900)*10000 + (tmCur.tm_mon+1)*100 + tmCur.tm_mday;
	if(nCurDate != m_nOldDate)
	{
		m_nOldDate = nCurDate;
		bNewDate = TRUE;
	}

	return nCurDate;
}

BOOL CLog::Reopen()
{
	Close();

	char chLogName[MAX_PATH];
	sprintf_s(chLogName, MAX_PATH, "%s\\%s%d.log", m_chLogPath, m_chModuleName, m_nOldDate);

	m_pFile = _fsopen(chLogName, m_chOpenMode, m_nShareFlag);
	if(m_pFile == NULL)
		return FALSE;
	else
		return TRUE;
}

BOOL CLog::AddLog(const char *pData, int nLen)
{
	BOOL bNewDate = FALSE;
	unsigned int nCurDate = GetCurrentDate(bNewDate);

	if(bNewDate)
	{
		if(!Reopen())
			return FALSE;
	}

	//char chCurTime[50];
	//GetCurTime(chCurTime, 50);

	//fwrite(chCurTime, sizeof(char), strlen(chCurTime), m_pFile);
	fwrite(pData, sizeof(char), nLen, m_pFile);
	fflush(m_pFile);
	//fwrite("\n", sizeof(char), 1, m_pFile);

	return TRUE;
}

void CLog::GetWorkPath(char* pPath, int nMaxBuf)
{
	char szExePath[MAX_PATH];
	::GetModuleFileName(NULL, szExePath, nMaxBuf);

	char *pRet = strrchr(szExePath, '\\');
	pRet[0] = 0;

	strcpy_s(pPath, nMaxBuf, szExePath);
}
