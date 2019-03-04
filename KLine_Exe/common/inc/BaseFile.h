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
	//给记录缓冲区m_pRecordBuf分配内存并初始化
	BOOL AllocateMemory();
	BOOL AllocateMemoryForSave(unsigned int nRecordCount);
protected:    //protected型数据需要在派生类中初始化
	char*	        m_pRecordBuf;	       //存储记录的缓冲区
	unsigned int    m_nRecordBufLen;       //存储记录的缓冲区长度

	int             m_nRecordPos;	       //当前记录的下标
	char*           m_pCurRcdBuf;          //当前记录的缓冲区

	unsigned int    m_nRecordCount;        //记录总数
	unsigned int    m_nRecordLen;		   //一条记录的长度
	BOOL            m_bIsOpen;             //文件是否打开

	unsigned int    m_nFieldCount;		   //字段总数
	tagFieldInfo*   m_pFieldInfo;		   //字段信息数组指针
private:

};


//////////////////////////////////////////////////////////////////////////////////
#endif