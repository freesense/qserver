//CN_IDU02_Qind.h
#ifndef _CN_IDU02_Qind_H_
#define _CN_IDU02_Qind_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CCN_IDU02_Qind : public CBaseFile
{
public:
	CCN_IDU02_Qind();
	virtual~CCN_IDU02_Qind();

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