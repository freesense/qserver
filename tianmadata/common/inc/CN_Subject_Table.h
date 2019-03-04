//CN_Subject_Table.h
#ifndef _CN_Subject_Table_H_
#define _CN_Subject_Table_H_

#include "BaseFile.h"
//////////////////////////////////////////////////////////////////////
class CCN_Subject_Table : public CBaseFile
{
public:
	CCN_Subject_Table();
	virtual~CCN_Subject_Table();

	BOOL Open(const char* pFileName = NULL, DWORD dwDesiredAccess = 0, DWORD dwShareMode = 0,
                      DWORD dwCreationDisposition = 0);
	void Close();
	BOOL Requery();
	virtual BOOL WriteData();
	int MovePos(unsigned short nTableNum);

	//virtual void LogEvent(LPCTSTR pFormat, ...);
protected:
	BOOL GetDataFromTable(LPCTSTR lpTableName, unsigned short nTableNum);

private:
	int   m_nCount16;  //CN_STK16_Fbsc_R   ±í¼ÇÂ¼Êý
	int   m_nCount17;  //CN_STK17_FHind_R
	int   m_nCount18;  //CN_STK18_ASSET1_R
	int   m_nCount19;  //CN_STK19_ASSET2_R
	int   m_nCount20;  //CN_STK20_PROFIT1_R 
	int   m_nCount21;  //CN_STK21_PROFIT2_R 
	int   m_nCount22;  //CN_STK22_CASH1_R
	int   m_nCount23;  //CN_STK23_CASH2_R

	int   m_n16Pos;
	int   m_n17Pos;
	int   m_n18Pos;
	int   m_n19Pos;
	int   m_n20Pos;
	int   m_n21Pos;
	int   m_n22Pos;
	int   m_n23Pos;
};
//////////////////////////////////////////////////////////////////////
#endif