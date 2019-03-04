//ScanThread.hͷ�ļ�
//

#pragma once

#include "../../public/QuoteFeed/ThreadEx.h"
#include "../../public/QuoteFeed/DBFile.h"
#include <map> 
#include "../../public/QuoteFeed/CommonStruct.h"
#include <list>
#include "regexpr2.h"

using namespace std; 
using namespace regex;

struct tagSHRecord
{
	tagSHRecord()
	{
		byScanTimes = 0;
		pHQRecBuf	= NULL;
		bNameChanged=false;
	}
	unsigned char byScanTimes;	//ɨ����� 0��1��2...
	char*         pHQRecBuf;	//�Ϻ������һ����¼�Ļ�����
	bool          bNameChanged;	//��Ʊ���Ʒ����仯�����ȷ���ɾ����Ʊ���ݣ��ٷ�����ӹ�Ʊ����
};

struct tagSZRecord
{
	tagSZRecord()
	{
		byScanTimes = 0;
		pHQRecBuf	= NULL;
		pXXRecBuf	= NULL;
		bNameChanged=false;
	}
	unsigned char byScanTimes;	//ɨ����� 0��1��2...
	char*         pHQRecBuf;	//���������һ����¼�Ļ�����
	char*         pXXRecBuf;	//������Ϣ��һ����¼�Ļ�����
	bool          bNameChanged;	//��Ʊ���Ʒ����仯�����ȷ���ɾ����Ʊ���ݣ��ٷ�����ӹ�Ʊ����
};

#define SZ_SCAN_TIME        2	//��Ҫɨ������ݿ���

typedef pair <int, tagSZRecord*> SZRecord_Pair;
typedef pair <int, tagSHRecord*> SHRecord_Pair;
/////////////////////////////////////////////////////
class CScanThread : public CThreadEx
{
public:
	CScanThread();
	virtual ~CScanThread();

	virtual void Run();
	virtual void Stop();
	
	BOOL SetDBFPath(const char *pSZPath, const char *pSHPath);
private:
	char      m_szDbfPath_SZ[MAX_PATH];       //������������·��
	char      m_szDbfPath_SH[MAX_PATH];		  //����Ϻ������·��
	char      m_szDbfPath_SZXX[MAX_PATH];     //���������Ϣ��·��

	CDBFile   m_dbfSZ;						  //�������������
	CDBFile   m_dbfSH;						  //�����Ϻ������
	CDBFile   m_dbfSZXX;					  //����������Ϣ��

	int       m_nSZPackSize;                  //���ڱ��ĵĴ�С
	char*     m_pSZSendBuf;                   //���ڴ����ͻ�����
	int       m_nSZSendBufSize;				  //���ڴ����ͻ�������С
	char*     m_pSZCurSendBufPos;			  //���ڵ�ǰָ��m_pSZSendBuf��λ��

	int       m_nSHPackSize;                  //�Ϻ����ĵĴ�С
	char*     m_pSHSendBuf;                   //�Ϻ������ͻ�����
	int       m_nSHSendBufSize;				  //�Ϻ������ͻ�������С
	char*     m_pSHCurSendBufPos;			  //�Ϻ���ǰָ��m_pSHSendBuf��λ��

	int       m_nSZXXPackSize;                  //���ڱ��ĵĴ�С
	char*     m_pSZXXSendBuf;                   //���ڴ����ͻ�����
	int       m_nSZXXSendBufSize;			    //���ڴ����ͻ�������С
	char*     m_pSZXXCurSendBufPos;			    //���ڵ�ǰָ��m_pSZSendBuf��λ��

	tagPackHeader  m_packHdr;				  //��ͷ
	DWORD     m_dwPreTick;                    //tickcount

	map<int, tagSZRecord*>                    m_SZMap;  //�����������
	map<int, tagSZRecord*>::iterator          m_SZiterator;

	map<int, tagSHRecord*>                    m_SHMap;  //����Ϻ�����
	map<int, tagSHRecord*>::iterator          m_SHiterator;

	unsigned short   m_wSZMinute;  //����dbf���µ�ʱ��
	unsigned short   m_wSHMinute;  

	BOOL ScanSZDBF();   //ɨ�������������ݿ�
	BOOL ScanSHDBF();   //ɨ���Ϻ��������ݿ�
	BOOL ScanSZXXDBF();	//ɨ��������Ϣ���ݿ�

	void ScanSZDeleteRecord(); //ɨ��������ɾ����Ʊ
	void ScanSHDeleteRecord(); 

	unsigned short TimeToMinute(int nTime); //��ʱ��ת�����Է���Ϊ��λ������
	void FreeMapMemory();  //�ͷ�MAP�е�����
	
	//����ֶ�ֵ��������
	void FillValueToBuf(unsigned short wFieldType, const char* pFieldName,
		unsigned int nDBFType, BOOL bIndex);
	
	//���Ͳ����ڵĹ�Ʊ�������ֶ�
	void SZStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);   //bIndex �Ƿ�Ϊָ��
	void SHStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);
	void SZXXStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist = FALSE);

	//�Դ��ڵĹ�Ʊ���бȽϣ����ͱ仯���ֶ�
	void SZStockExist(unsigned short wMinute, BOOL bIndex);
	void SHStockExist(unsigned short wMinute, BOOL bIndex);
	void SZXXStockExist(unsigned short wMinute, BOOL bIndex);

	//���仯���ֶε�������
	BOOL FillChangedValueToBuf(unsigned short wFieldType, const char* pFieldName,
		const char* pOldRecord, int& nFieldCount, unsigned int nDBFType, BOOL bIndex);

	void SetStockType(unsigned char  byScanTimes, unsigned char byMapType); // ����MAP�й�Ʊɨ��Ĵ���

	BOOL AllocateMemoryForSendBuf(); //�����ں��Ϻ������������ڴ�

	void CheckSZSendBufForSend();   //����������黺�����Ƿ�����һ����¼���������ͻ�����
	void CheckSHSendBufForSend();   //����Ϻ����黺�����Ƿ�����һ����¼���������ͻ�����
	void CheckSZXXSendBufForSend(); //���������Ϣ�������Ƿ�����һ����¼���������ͻ�����
	void Trim(char* pData);  //ȥ���ַ�ǰ��Ŀո�

	BOOL FindSHRecord(int nStockCode); //�����Ϻ���¼�Ƿ����
	BOOL IsValidStockCode(char* pCode); //����Ʊ�����Ƿ���Ч��dbf��ʱ�����������󣬷��ʹ�����־���������
	bool IsStockNameChanged(const char* pStockCode, const char* pOldRecord, unsigned int nDBFType);
	BOOL IsGuoZaiHuiGou(unsigned int nDBFType, char* pStockCode);
};
