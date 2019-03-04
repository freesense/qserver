//CN_STK06_Fgar.h
#ifndef _CN_STK06_Fgar_H_
#define _CN_STK06_Fgar_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CCN_STK06_Fgar : public CBaseFile
{
public:
	CCN_STK06_Fgar();
	virtual~CCN_STK06_Fgar();

	BOOL Open(const char* pFileName = NULL, DWORD dwDesiredAccess = 0, DWORD dwShareMode = 0,
                      DWORD dwCreationDisposition = 0);
	void Close();
	BOOL Requery();
	virtual BOOL WriteData();

	//virtual void LogEvent(LPCTSTR pFormat, ...);
	int GetStockRecordCount(char* pStock);
protected:

private:

};
//////////////////////////////////////////////////////////////////////
#endif