//HK_MKT01_Idx.h
#ifndef _HK_MKT01_Idx_H_
#define _HK_MKT01_Idx_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CHK_MKT01_Idx : public CBaseFile
{
public:
	CHK_MKT01_Idx();
	virtual~CHK_MKT01_Idx();

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