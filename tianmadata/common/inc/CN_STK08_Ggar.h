//CN_STK08_Ggar.h
#ifndef _CN_STK08_Ggar_H_
#define _CN_STK08_Ggar_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CCN_STK08_Ggar : public CBaseFile
{
public:
	CCN_STK08_Ggar();
	virtual~CCN_STK08_Ggar();

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