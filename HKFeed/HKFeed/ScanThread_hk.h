//ScanThread.hͷ�ļ�
//

#pragma once

#include "../../public/QuoteFeed/ThreadEx.h"
#include "../../public/QuoteFeed/DBFile.h"
#include <map> 
#include "../../public/QuoteFeed/CommonStruct.h"

using namespace std; 

struct tagHKRecord
{
	tagHKRecord()
	{
		byScanTimes = 0;
		pHQRecBuf	= NULL;
		bNameChanged=false;
	}

	unsigned char byScanTimes;	//ɨ����� 0��1��2...
	char*         pHQRecBuf;	//��������һ����¼�Ļ�����
	bool          bNameChanged;	//��Ʊ���Ʒ����仯�����ȷ���ɾ����Ʊ���ݣ��ٷ�����ӹ�Ʊ����
};

typedef pair <int, tagHKRecord*> HKRecord_Pair;
/////////////////////////////////////////////////////
class CScanThread : public CThreadEx
{
public:
	CScanThread();
	virtual ~CScanThread();

	virtual void Run();
	virtual void Stop();
	
	BOOL SetDBFPath(const char *pHKPath);

	unsigned short   m_nStartTime;			  //ֹͣ����ɨ�����ݿ�ʼʱ��
	unsigned short   m_nStopTime;			  //ֹͣ����ɨ�����ݽ���ʱ��
private:
	char      m_szDbfPath_HK[MAX_PATH];       //�����������·��
	CDBFile   m_dbfHK;						  //������������

	int       m_nHKPackSize;                  //��۱��ĵĴ�С
	char*     m_pHKSendBuf;                   //��۴����ͻ�����
	int       m_nHKSendBufSize;				  //��۴����ͻ�������С
	char*     m_pHKCurSendBufPos;			  //��۵�ǰָ��m_pHKSendBuf��λ��

	tagPackHeader  m_packHdr;				  //��ͷ

	map<int, tagHKRecord*>                    m_HKMap;  //����������
	map<int, tagHKRecord*>::iterator          m_HKiterator;

	unsigned short   m_wHKMinute;  
	DWORD     m_dwPreTick;                    //tickcount

	BOOL ScanHKDBF();   //ɨ���Ϻ��������ݿ�
	void ScanHKDeleteRecord();//ɨ�������ɾ����Ʊ 

	unsigned short TimeToMinute(int nTime); //��ʱ��ת�����Է���Ϊ��λ������
	void FreeMapMemory();  //�ͷ�MAP�е�����
	
	//����ֶ�ֵ��������
	void FillValueToBuf(unsigned short wFieldType, const char* pFieldName,
		unsigned int nDBFType, BOOL bIndex);
	
	//���Ͳ����ڵĹ�Ʊ�������ֶ�
	void HKStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);
	

	//�Դ��ڵĹ�Ʊ���бȽϣ����ͱ仯���ֶ�
	void HKStockExist(unsigned short wMinute, BOOL bIndex);

	//���仯���ֶε�������
	BOOL FillChangedValueToBuf(unsigned short wFieldType, const char* pFieldName,
		const char* pOldRecord, int& nFieldCount, unsigned int nDBFType, BOOL bIndex);

	void SetStockType(unsigned char  byScanTimes, unsigned char byMapType); // ����MAP�й�Ʊɨ��Ĵ���

	BOOL AllocateMemoryForSendBuf(); //����ۻ����������ڴ�
	void CheckHKSendBufForSend();   //����Ϻ����黺�����Ƿ�����һ����¼���������ͻ�����
	void ChangeStockCode(char* pStockCode, BOOL bIndex); //�ѹ�Ʊ�����Ϊ��׼����
	void Trim(char* pData);  //ȥ���ַ�ǰ��Ŀո�

	BOOL IsValidStockCode(char* pCode); //����Ʊ�����Ƿ���Ч��dbf��ʱ�����������󣬷��ʹ�����־���������
	bool IsStockNameChanged(const char* pStockCode, const char* pOldRecord, unsigned int nDBFType);
};
