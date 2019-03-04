//HK_STK08_Ggar.h
#ifndef _HK_STK08_Ggar_H_
#define _HK_STK08_Ggar_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CHK_STK08_Ggar : public CBaseFile
{
public:
	CHK_STK08_Ggar();
	virtual~CHK_STK08_Ggar();

	BOOL Open(const char* pFileName = NULL, DWORD dwDesiredAccess = 0, DWORD dwShareMode = 0,
                      DWORD dwCreationDisposition = 0);
	void Close();
	BOOL Requery();
	virtual BOOL WriteData();
	int GetStockRecordCount(const char* pStock);

	//virtual void LogEvent(LPCTSTR pFormat, ...);
protected:
	

private:

};
//////////////////////////////////////////////////////////////////////
#endif