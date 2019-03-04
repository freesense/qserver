//HttpDataFile.h
//

#ifndef _HTTPDATAFILE_H_
#define _HTTPDATAFILE_H_

#include "BaseFile.h"
#include "LockFile.h"
////////////////////////////////////////////////////////////////////////////////
struct tagHttpKFileInfo
{
	unsigned int  nDate;              //����
	unsigned int  nNewPrice;	      //���¼ۣ����̺�Ϊ���̼�
	unsigned int  nOpenPrice;	      //���̼۸�
	unsigned int  nPreClosePrice;	  //ǰ���̼۸�
	unsigned int  nMaxPrice;	      //��߼۸�
	unsigned int  nMinPrice;	      //��ͼ۸�
	unsigned int  nVolume;		      //�ɽ�����
	unsigned int  nSum;			      //�ɽ����
};

class CHttpDataFile : public CBaseFile
{
public:
	CHttpDataFile();
	virtual ~CHttpDataFile();

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
	BOOL AddRecord(tagHttpKFileInfo* pHttpInfo);

protected:

private:
	HANDLE          m_hFile;               //�ļ����
	CLockFile       m_hLockFile;           //��������dbf�ļ�

};













////////////////////////////////////////////////////////////////////////////////
#endif