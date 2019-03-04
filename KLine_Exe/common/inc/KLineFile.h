//KLineFile.h
//
#ifndef _KLINEFILE_H_
#define _KLINEFILE_H_
#include "BaseFile.h"
#include "LockFile.h"

///////////////////////////////////////////////////////////////////////////////////
struct tagKLineInfo
{
	unsigned int  nDate;          //����
	unsigned int  nOpenPrice;	  //���̼۸�
	unsigned int  nMaxPrice;	  //��߼۸�
	unsigned int  nMinPrice;	  //��ͼ۸�
	unsigned int  nClosePrice;	  //���̼۸�
	unsigned int  nVolume;		  //�ɽ�����
	unsigned int  nSum;			  //�ɽ����
};

class CKLineFile : public CBaseFile
{
public:
	CKLineFile();
	virtual ~CKLineFile();

	/***********************************************************************
	��������: ���ļ�
	dwDesiredAccess: ������Ϊ����һ���򼸸�
	    GENERIC_READ     GENERIC_WRITE     GENERIC_EXECUTE    GENERIC_ALL         
    dwShareMode: ������Ϊ����һ���򼸸�
	    FILE_SHARE_READ  FILE_SHARE_WRITE  FILE_SHARE_DELETE               
	dwCreationDisposition: ������Ϊ����һ���򼸸�
	    CREATE_NEW       CREATE_ALWAYS     OPEN_EXISTING       OPEN_ALWAYS  
		TRUNCATE_EXISTING   
	***********************************************************************/
	BOOL Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                      DWORD dwCreationDisposition);
	void Close();
	BOOL Requery();
	BOOL WriteData();
	BOOL WriteDataToEnd();
	
protected:

private:
	HANDLE          m_hFile;               //�ļ����
	CLockFile       m_hLockFile;           //��������dbf�ļ�

};




//////////////////////////////////////////////////////////////////////////////////
#endif