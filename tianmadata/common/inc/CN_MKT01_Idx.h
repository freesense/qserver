//CN_MKT01_Idx.h
#ifndef _CN_MKT01_Idx_H_
#define _CN_MKT01_Idx_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////

class CCN_MKT01_Idx : public CBaseFile
{
public:
	CCN_MKT01_Idx();
	virtual~CCN_MKT01_Idx();

	BOOL Open(const char* pFileName = NULL, DWORD dwDesiredAccess = 0, DWORD dwShareMode = 0,
                      DWORD dwCreationDisposition = 0);
	void Close();
	BOOL Requery();
	virtual BOOL WriteData();
	int GetDataSize();

	//virtual void LogEvent(LPCTSTR pFormat, ...);
protected:

private:
	void FreeMemory();

	int     m_nDataSize;  //需要缓冲区大小

};
//////////////////////////////////////////////////////////////////////
#endif