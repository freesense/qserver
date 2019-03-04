//HK_STK02_Qind.h
#ifndef _HK_STK02_Qind_H_
#define _HK_STK02_Qind_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CHK_STK02_Qind : public CBaseFile
{
public:
	CHK_STK02_Qind();
	virtual~CHK_STK02_Qind();

	BOOL Open(const char* pFileName = NULL, DWORD dwDesiredAccess = 0, DWORD dwShareMode = 0,
                      DWORD dwCreationDisposition = 0);
	void Close();
	BOOL Requery();
	virtual BOOL WriteData();

	//virtual void LogEvent(LPCTSTR pFormat, ...);
protected:

private:

};
//////////////////////////////////////////////////////////////////////
#endif