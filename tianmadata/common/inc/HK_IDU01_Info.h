//HK_IDU01_Info.h
#ifndef _HK_IDU01_Info_H_
#define _HK_IDU01_Info_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CHK_IDU01_Info : public CBaseFile
{
public:
	CHK_IDU01_Info();
	virtual~CHK_IDU01_Info();

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