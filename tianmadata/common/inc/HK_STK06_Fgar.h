//HK_STK06_Fgar.h
#ifndef _HK_STK06_Fgar_H_
#define _HK_STK06_Fgar_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CHK_STK06_Fgar : public CBaseFile
{
public:
	CHK_STK06_Fgar();
	virtual~CHK_STK06_Fgar();

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