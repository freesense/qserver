//DBFile.h ͷ�ļ�
//

#pragma once
#include "LockFile.h"

////////////////////////////////////////////////////////////////////////

typedef struct tagFILEHEADER      //32Byte,file top
{
	unsigned char  flag;          //0x03
	unsigned char  date[3];       //Record date
	long  		   RecordSum;     //Total record sum
	unsigned short dbfHeadSize;   //The size of *.dbf file's head
	unsigned short RecordLen;     //A single record's size
	unsigned char  Reserve[20];   //Reserved
}FILEHEADER;

typedef struct tagFIELD			//ÿ32�ֽ�����һ���ֶ�
{
	char          FieName[11]; // field  name
	unsigned char FieType;     // field type  example:N;C;D etc
	unsigned long BegPos;      // The first field is 1 ,the others is BegPos + Seglen
	unsigned char FieLen;      // field length
	unsigned char DotLen;      //float dot length
	unsigned char reserve[14];
}FIELD;

class CDBFile
{
public:
	CDBFile();
	virtual ~CDBFile();

	BOOL Open(const char* dbfName);
	BOOL Reopen();
	BOOL Requery();	//����dbf�ļ����ݵ�������
	void Close();
	BOOL IsOpen();
	long GetRecordCount();
	long GetRecordSize();
	long GetAllRecordSize();
	BOOL GetRecordToBuffer(char *pDesBuf);
	BOOL GetAllRecordToBuffer(char *pDesBuf);
	
	void MoveFirst(){ m_nRecordPos = 0; m_pCurRcdBuf = m_pRecordBuf;}
	void MoveLast(){m_nRecordPos = m_fHdr.RecordSum - 1; m_pCurRcdBuf = m_pRecordBuf + m_fHdr.RecordLen * (m_fHdr.RecordSum - 1);}
	void MoveNext(){m_nRecordPos++;m_pCurRcdBuf += m_fHdr.RecordLen;}
	void MovePrev(){m_nRecordPos--; m_pCurRcdBuf -= m_fHdr.RecordLen;}
	void Move(long nRows)
	{
		if(nRows < 0)
		{
			m_nRecordPos -= nRows;
			m_pCurRcdBuf -= m_fHdr.RecordLen * nRows;
		}
		else if(nRows > 0)
		{
			m_nRecordPos += nRows;
			m_pCurRcdBuf += m_fHdr.RecordLen * nRows;
		}
	}
	BOOL IsBOF()
	{
		if(m_nRecordPos < 0 || m_pCurRcdBuf < m_pRecordBuf) 
			return TRUE;
		return FALSE;
	}
	BOOL IsEOF()
	{
		if(m_nRecordPos > m_fHdr.RecordSum - 1 || m_pCurRcdBuf > m_pRecordBuf + m_fHdr.RecordLen * (m_fHdr.RecordSum - 1))
			return TRUE;

		return FALSE;
	}

	int GetFieldCount()
	{
		return (m_fHdr.dbfHeadSize - 1 - sizeof(FILEHEADER))/sizeof(FIELD);
	}

	int GetFieldValue(short nIndex, char* pValue);
	int GetFieldValue(const char* pFieldName, char* pValue);
	int GetFieldValue(const char* pFieldName, int& nValue);
	char* CompFieldBuffer(const char* pFieldName, const char* pRecord);
	int GetFieldValue(const char* pFieldName, const char* pRecord, char* pValue);
protected:
	BOOL AllocateMemory();
	
protected:
	HANDLE          m_hFile;               //�ļ����
	//CLockFile       m_hLockFile;           //��������dbf�ļ�	  bydj2008-12-4
	FILEHEADER      m_fHdr;

	char*	        m_pFileHead;           //�洢�ļ�ͷ�Ļ�����
	unsigned int	m_nFileHeadLen;	       //�洢�ļ�ͷ�Ļ���������
	char*	        m_pRecordBuf;	       //�洢��¼�Ļ�����
	unsigned int    m_nRecordBufLen;       //�洢��¼�Ļ���������

	int             m_nRecordPos;	       //��ǰ��¼���±�
	char*           m_pCurRcdBuf;          //��ǰ��¼�Ļ����� 

	std::string     m_strFilePath;
};