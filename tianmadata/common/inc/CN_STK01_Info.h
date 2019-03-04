//CN_STK01_Info.h
#ifndef _CN_STK01_INFO_H_
#define _CN_STK01_INFO_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CCn_Stk01_Info : public CBaseFile
{
public:
	CCn_Stk01_Info();
	virtual~CCn_Stk01_Info();

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