//KillProcess.cpp
#include "stdafx.h"
#include "KillProcess.h"

////////////////////////////////////////////////////////////////////////////////////
CKillProcess::CKillProcess()
{
}

CKillProcess::~CKillProcess()
{
}
	  
BOOL CKillProcess::KillProc(DWORD dwProcessID, LPCSTR lpExeFile)
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32 = {0};

	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
	{
		REPORT(MODULE_NAME, T("CreateToolhelp32Snapshot() failed with error:%d\n", GetLastError()), RPT_ERROR);
		return FALSE;
	}

	pe32.dwSize = sizeof( PROCESSENTRY32 );
	
	if (!Process32First(hProcessSnap, &pe32))
	{
		REPORT( MODULE_NAME, T("Process32First() failed with error:%d\n", GetLastError()), RPT_ERROR);
		CloseHandle( hProcessSnap ); 
		return FALSE;
	}

	do
	{
		if (dwProcessID != 0)
		{
			if (pe32.th32ProcessID != dwProcessID)
				continue;

			if (lpExeFile != NULL)
			{
				if (_stricmp(lpExeFile, pe32.szExeFile) != 0)
					continue;
			}
		}
		else
		{
			if (lpExeFile != NULL)
			{
				if (_stricmp(lpExeFile, pe32.szExeFile) != 0)
					continue;
			}
		}

		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if(hProcess == NULL)
		{
			REPORT(MODULE_NAME, T("OpenProcess() failed with error:%d\n", GetLastError()), RPT_ERROR);
			return FALSE;
		}
		else
		{
			if (!TerminateProcess(hProcess, 0))
			{
				REPORT(MODULE_NAME, T("TerminateProcess() failed with error:%d\n", GetLastError()), RPT_ERROR);
				CloseHandle(hProcess);
				CloseHandle(hProcessSnap);
				return FALSE;
			}
			else
			{
				CloseHandle(hProcess);
				CloseHandle(hProcessSnap);
				return TRUE;
			}
		}
	}
	while(Process32Next(hProcessSnap, &pe32));
	REPORT(MODULE_NAME, T("Don't find Process ID:%d\n", dwProcessID), RPT_WARNING);

	CloseHandle(hProcessSnap);
	return FALSE;
}	  
 
void CKillProcess::LogEvent(WORD nLogType, LPCSTR pFormat, ...)
{
/*	char szMsg[DB_MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(szMsg, DB_MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);  */
	////»’÷æ
}  