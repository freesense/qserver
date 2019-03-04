//CN_MKT02_Qind.h
#ifndef _CN_MKT02_Qind_H_
#define _CN_MKT02_Qind_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CCN_MKT02_Qind : public CBaseFile
{
public:
	CCN_MKT02_Qind();
	virtual~CCN_MKT02_Qind();

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