//BaseFile.h
//

#ifndef _BASEFILE_H_
#define _BASEFILE_H_

///////////////////////////////////////////////////////////////////////////////////

struct tagFieldInfo
{
	unsigned long BegPos;       // The first field is 1 ,the others is BegPos + Seglen
	unsigned char FieLen;       // field length
	unsigned char reserve[3];
};

class CBaseFile
{
public:
	CBaseFile();
	virtual ~CBaseFile();
								
	virtual BOOL Open(const char* pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                      DWORD dwCreationDisposition) = 0;
	virtual void Close();
	virtual BOOL Requery() = 0;
	virtual BOOL WriteData() = 0;
	
	BOOL IsOpen(){return m_bIsOpen;}

	unsigned int GetFieldCount(){return m_nFieldCount;}
	unsigned int GetRecordCount(){return m_nRecordCount;}
	unsigned int GetRecordSize(){return m_nRecordLen;}
	unsigned int GetAllRecordSize(){return m_nRecordLen*m_nRecordCount;}

	BOOL GetRecordToBuffer(char *pDesBuf);
	BOOL GetAllRecordToBuffer(char *pDesBuf);
	BOOL SetAllRecordToBuffer(const char *pDesBuf);
	
	BOOL SetRecordBuffer(const char *pDesBuf);

	int GetFieldValue(unsigned int nIndex, char* pValue);
	int GetFieldValue(unsigned int nIndex, unsigned int& nValue);

	void MoveFirst();
	void MoveLast();
	void MoveNext();
	void MovePrev();
	void Move(long nRows);
	BOOL IsBOF();
	BOOL IsEOF();

	//virtual void LogEvent(WORD nLogType, LPCTSTR pFormat, ...);
	//����¼������m_pRecordBuf�����ڴ沢��ʼ��
	BOOL AllocateMemory();
	BOOL AllocateMemoryForSave(unsigned int nRecordCount);
protected:    //protected��������Ҫ���������г�ʼ��
	char*	        m_pRecordBuf;	       //�洢��¼�Ļ�����
	unsigned int    m_nRecordBufLen;       //�洢��¼�Ļ���������

	int             m_nRecordPos;	       //��ǰ��¼���±�
	char*           m_pCurRcdBuf;          //��ǰ��¼�Ļ�����

	unsigned int    m_nRecordCount;        //��¼����
	unsigned int    m_nRecordLen;		   //һ����¼�ĳ���
	BOOL            m_bIsOpen;             //�ļ��Ƿ��

	unsigned int    m_nFieldCount;		   //�ֶ�����
	tagFieldInfo*   m_pFieldInfo;		   //�ֶ���Ϣ����ָ��
private:

};


//////////////////////////////////////////////////////////////////////////////////
#endif