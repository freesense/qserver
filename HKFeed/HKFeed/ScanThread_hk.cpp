//ScanThread.cpp
//

#include "stdafx.h"
#include "ScanThread_hk.h"
#include "../../public/CharTranslate.h"
#include "../../public/feedinterface.h"
#include "../../public/commx/highperformancecounter.h"
#include "constdef_hk.h"
#include "../../public/commx/mery.h"

extern IFeedOwner*    g_pOwner;
extern int g_nTimeout;

#define  MAP_TYPE_HK	      2
#define  HK_INDEX_CODE_MAX	 100000	  //��۽����� ָ����Χ����Сֵ
///////////////////////////////////////////////////////////////
CScanThread::CScanThread()
{
	m_szDbfPath_HK[0]   = 0;

	m_nHKPackSize       = 0;
	m_pHKSendBuf        = NULL;
	m_nHKSendBufSize    = 0;
	m_pHKCurSendBufPos  = 0;

	m_wHKMinute = 0; 

	m_nStartTime = 0;
	m_nStopTime  = 0;
	m_dwPreTick  = 0;
}

CScanThread::~CScanThread()
{
	Stop();
}

void CScanThread::Stop()
{
	CThreadEx::Stop();
	FreeMapMemory();

	if(m_pHKSendBuf)
	{
		delete []m_pHKSendBuf;
		m_pHKSendBuf     = NULL;
	}
}

BOOL CScanThread::SetDBFPath(const char *pHKPath)
{
	strcpy_s(m_szDbfPath_HK, MAX_PATH, pHKPath);

	char szMsg[MSG_BUF_LEN];	

	//����������
	if(!m_dbfHK.Open(m_szDbfPath_HK))
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s failed!\n", m_szDbfPath_HK);
		REPORT_Ex(szMsg, RPT_ERROR);
		return FALSE;
	}
	else
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s successed!\n", m_szDbfPath_HK);
		REPORT_Ex(szMsg, RPT_INFO);
	}

	return TRUE;
}

unsigned short CScanThread::TimeToMinute(int nTime)
{
	int nHour = nTime / 10000;
	int nMinute = nTime % 10000 / 100;
	return (nHour*60 + nMinute);
}

void CScanThread::FreeMapMemory()
{
	//�ͷ���������MAP
	for( m_HKiterator = m_HKMap.begin(); m_HKiterator != m_HKMap.end(); m_HKiterator++)
	{
		tagHKRecord* pRec = m_HKiterator->second;
		if(pRec)
		{
			if(pRec->pHQRecBuf)
				delete[] pRec->pHQRecBuf;

			delete pRec;
			pRec = NULL;
		}
	}

	m_HKMap.clear();
}

void CScanThread::FillValueToBuf(unsigned short wFieldType, const char* pFieldName,
								 unsigned int nDBFType, BOOL bIndex)
{
	tagFieldInfo   fieldInfo;
	char           szField[30];
	int            nFieldSize = 0;

	fieldInfo.wFieldType  = wFieldType;
	if (nDBFType == HK_HQ_DBF)
		nFieldSize = m_dbfHK.GetFieldValue(pFieldName, szField);

	switch(wFieldType)
	{
		//��������
	case FD_ZRSP: case FD_JRKP: case FD_ZJCJ: case FD_ZGCJ: case FD_ZDCJ:
	case FD_SJW5: case FD_SJW4: case FD_SJW3:
	case FD_SJW2: case FD_SJW1: case FD_BJW1: case FD_BJW2: case FD_BJW3:
	case FD_BJW4: case FD_BJW5:
		//������Ϣ
	case FD_MGMZ: case FD_JSFL: case FD_YHSL: case FD_GHFL:	case FD_JGDW:	
	case FD_JHCS: case FD_LXCS: case FD_ZTJG: case FD_DTJG:	case FD_ZHBL:
		{
			fieldInfo.nFieldValue = CCharTranslate::PriceToDigital(szField, nFieldSize);
			break;
		}
		//����
	case FD_CJSL: 
		{
			if (!bIndex)
				fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital100(szField, nFieldSize);
			else
				fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital(szField, nFieldSize);
			break;
		}
		//����
	case FD_CJJE:
		{
			fieldInfo.nFieldValue = CCharTranslate::SumPriceToDigital1000(szField, nFieldSize);
			break;
		}
		//����
	case FD_CJBS: case FD_HYCC:	case FD_SSL5: case FD_SSL4: case FD_SSL3: case FD_SSL2:
	case FD_SSL1: case FD_BSL1: case FD_BSL2: case FD_BSL3: case FD_BSL4:
	case FD_BSL5:
		//������Ϣ
	case FD_JYDW: case FD_MBXL: case FD_BLDW: case FD_SLDW: case FD_XJXZ:
		{
			fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital(szField, nFieldSize);
			break;
		}
		//����
	case FD_JSD1: case FD_JSD2: case FD_SYL1: case FD_SYL2:// �۸�������֮�֣������͵��ֶ������޷�������
		//������Ϣ
	case FD_SNLR: case FD_BNLR:
		{
			fieldInfo.nFieldValue = CCharTranslate::SignedPriceToDigital(szField, nFieldSize);
			break;
		}
		//������Ϣ
	case FD_ZFXL:	case FD_LTGS:
		{
			fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital10000(szField, nFieldSize);
			break;
		}
	}

	if (nDBFType == HK_HQ_DBF)
	{
		memcpy(m_pHKCurSendBufPos, &fieldInfo, sizeof(tagFieldInfo));
		m_nHKPackSize          += sizeof(tagFieldInfo);
		m_pHKCurSendBufPos     += sizeof(tagFieldInfo);
	}
}


/**********************************************
����ֶ�ֵ��ͬ����TRUE, nFieldCount��1��
**********************************************/
BOOL CScanThread::FillChangedValueToBuf(unsigned short wFieldType, const char* pFieldName,
						const char* pOldRecord,	int& nFieldCount, unsigned int nDBFType, BOOL bIndex)
{
	char* pNewField = NULL;

	if (nDBFType == HK_HQ_DBF)
		pNewField = m_dbfHK.CompFieldBuffer(pFieldName, pOldRecord);	

	if(pNewField != NULL)
	{
		nFieldCount++;	
		FillValueToBuf(wFieldType, pFieldName, nDBFType, bIndex);
		return TRUE;
	}

	return FALSE;
}

void CScanThread::SetStockType(unsigned char  byScanTimes, unsigned char byMapType)
{
	//�������MAP��Ʊɨ��Ĵ���
	if(byMapType == MAP_TYPE_HK)
	{
		map<int, tagHKRecord*>::iterator     HKiterator;

		for(HKiterator = m_HKMap.begin(); HKiterator != m_HKMap.end(); HKiterator++)
		{
			tagHKRecord* pRec = HKiterator->second;
			if(pRec)
			{
				pRec->byScanTimes = byScanTimes;
				pRec->bNameChanged = false;
			}
		}
	}
}


BOOL CScanThread::AllocateMemoryForSendBuf()
{
	int nHKSize = (m_dbfHK.GetRecordCount() - 1) *
		( sizeof(tagNewPackInfo) + HK_MAX_FIELD * sizeof(tagFieldInfo) );  //��෢��28���ֶ�

	nHKSize   += PACK_HEADER_LEN; //�����Ҫ���ڴ���  8���ֽڵı���ͷ

	m_pHKSendBuf      = new char[nHKSize];
	if(!m_pHKSendBuf)
	{
		REPORT_Ex("Allocate memory failed for HK SendBuf\n", RPT_ERROR);
		return FALSE;
	}
	m_nHKSendBufSize  = nHKSize;

	return TRUE;
}

void CScanThread::Run()
{
	Sleep(5000);  //��QuoteFarm���Ӻ��ٷ������ݣ��������ݶ�ʧ

	//��ȡ����
	if(!m_dbfHK.Requery())
	{
		REPORT_Ex("Read HK dbf record failed!\n", RPT_ERROR);
		return;
	}

	if(!AllocateMemoryForSendBuf())
	{
		REPORT_Ex("Allocate memory failed, scan thread exit!\n", RPT_ERROR);
		return;
	}
		
	REPORT_Ex("Start scan dbf\n", RPT_INFO);

	m_dwPreTick  = GetTickCount();
	while(m_bRun)
	{
		//�������й�Ʊ��ɨ�����Ϊ0������ʱ��������ҵĹ�Ʊ��ɨ�����Ϊ1����ѹ�Ʊ��Ϊ��ͨ״̬��
		//ɨ�����Ϊ0 �����Ѿ�ɾ���Ĺ�Ʊ
		SetStockType(0, MAP_TYPE_HK);  
		BOOL bRet = ScanHKDBF();    //ɨ����������
		if(!m_bRun)
			break;

		if (bRet)
			ScanHKDeleteRecord(); //ɨ����ɾ���ļ�¼

		if(!m_bRun)
			break;

		//�ж��Ƿ�ʱ
		DWORD dwCurTick = GetTickCount();
		if (dwCurTick - m_dwPreTick > (DWORD)g_nTimeout*1000)
		{
			REPORT_Ex("HeartBeat\n", RPT_HEARTBEAT|RPT_IGNORE);
			m_dwPreTick = dwCurTick;
		}

		Sleep(2000);
	}

	REPORT_Ex("Stop scan dbf\n", RPT_INFO);
}

void CScanThread::ScanHKDeleteRecord()
{
	//���¿�ʼɨ��ǰ����
	memset(m_pHKSendBuf, 0, m_nHKSendBufSize);
	m_nHKPackSize      = 0;
	m_pHKCurSendBufPos = m_pHKSendBuf;

	tagPackHeader packHdr;
	m_pHKCurSendBufPos += sizeof(tagPackHeader); //����ͷ����λ��
	//****************************************************************
	map<int, tagHKRecord*>::iterator     HKiterator;
	
	for(HKiterator = m_HKMap.begin(); HKiterator != m_HKMap.end();)
	{
		tagHKRecord* pRec = HKiterator->second;
		if(!pRec)
		{
			HKiterator++;
			continue;
		}
		if(pRec->byScanTimes != 0 && !pRec->bNameChanged) //ɨ�����Ϊ0���Ʊ���Ʊ仯���Ϊɾ��
		{
			HKiterator++;
			continue;
		}
		//STOCK_TYPE_DELETE ��ɾ���Ĺ�Ʊ
		
		//��Ʊ��Ϣ����Ϣͷ��
		tagPackInfo packInfo;
		packInfo.wStockType  = STOCK_TYPE_DELETE;
		packInfo.wTime       = m_wHKMinute;
		packInfo.wFieldCount = 0;

		BOOL bIndex = FALSE;
		if(HKiterator->first > HK_INDEX_CODE_MAX)     //ָ��
		{
			bIndex = TRUE;
			strcpy_s(packInfo.szMarketType, 6, "HK");
		}
		else
		{
			bIndex = FALSE;
			strcpy_s(packInfo.szMarketType, 6, "hk");
		}
		
		if (pRec->pHQRecBuf != NULL)
			m_dbfHK.GetFieldValue("S1", pRec->pHQRecBuf, packInfo.szStockCode);

		ChangeStockCode(packInfo.szStockCode, bIndex);
		//�ڴ��С���*****************************************
		CheckHKSendBufForSend();
		//*****************************************************

		memcpy(m_pHKCurSendBufPos, &packInfo, sizeof(tagPackInfo));
		m_nHKPackSize += sizeof(tagPackInfo);
		m_pHKCurSendBufPos += sizeof(tagPackInfo);	
		
		//��MAP��ɾ��
		if(pRec->pHQRecBuf != NULL)
		{
			delete[] pRec->pHQRecBuf;
			pRec->pHQRecBuf = NULL;
		}

		if (pRec != NULL)
		{
			delete pRec;
			pRec = NULL;
			m_HKMap.erase(HKiterator++);
		}
	}

	//********************************************************
	packHdr.ulPackSize = m_nHKPackSize;
	memcpy(m_pHKSendBuf, &packHdr, sizeof(tagPackHeader));

	if(m_nHKPackSize > 0)
	{
		//Send   m_pHKSendBuf
		int nLen = m_nHKPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //����4���ֽڸ�������ʹ��
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for HKSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pHKSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pHKSendBuf, PACK_HEADER_LEN);
			
			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pHKSendBuf+PACK_HEADER_LEN, m_nHKPackSize);
			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8���ֽڵı���ͷ
		}
	}
}


void CScanThread::CheckHKSendBufForSend()
{
	int nNeedSize = PACK_HEADER_LEN + m_nHKPackSize + sizeof(tagNewPackInfo) + 
		sizeof(tagFieldInfo) * HK_MAX_FIELD; //����һ����¼�Ļ�����

	if(nNeedSize > m_nHKSendBufSize)
	{
		m_packHdr.ulPackSize = m_nHKPackSize;
		memcpy(m_pHKSendBuf, &m_packHdr, sizeof(tagPackHeader));

		//Send   m_pHKSendBuf
		int nLen = m_nHKPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //����4���ֽڸ�������ʹ��
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for HKSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pHKSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pHKSendBuf, PACK_HEADER_LEN);
			
			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pHKSendBuf+PACK_HEADER_LEN, m_nHKPackSize);
			
			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8���ֽڵı���ͷ
		}

		memset(m_pHKSendBuf, 0, m_nHKSendBufSize);
		m_nHKPackSize      = 0;
		m_pHKCurSendBufPos = m_pHKSendBuf;

		m_pHKCurSendBufPos += sizeof(tagPackHeader); //����ͷ����λ��
	}
}



BOOL CScanThread::ScanHKDBF()
{
	//���¿�ʼɨ��ǰ����
	memset(m_pHKSendBuf, 0, m_nHKSendBufSize);
	m_nHKPackSize      = 0;
	m_pHKCurSendBufPos = m_pHKSendBuf;

	tagPackHeader packHdr;
	m_pHKCurSendBufPos += sizeof(tagPackHeader); //����ͷ����λ��
	//****************************************************************
	if (!m_dbfHK.Requery())         //��ȡ���µ����ݵ�������
	{
		REPORT_Ex("Requery() failed\n", RPT_ERROR);
		return FALSE;
	}
	//�۹���ÿ���¼����ȫ��ɾ����Ȼ����ȫ����ӣ������ʱ���ڲ�����ɾ���������	
	time_t  timeCur;
	time(&timeCur);
	tm tmCur;
	localtime_s(&tmCur, &timeCur);
	unsigned short nCurTime = tmCur.tm_hour*100 + tmCur.tm_min;
	if (nCurTime >= m_nStartTime && nCurTime <= m_nStopTime)
		return FALSE;

	m_dbfHK.MoveFirst();	   //�ƶ�����һ����¼

	//��ȡʱ������Ϊ��λ
	int nTime = 0;
    m_dbfHK.GetFieldValue("S2", nTime);
	unsigned short wMinute = TimeToMinute(nTime);
	if (wMinute > m_wHKMinute)
		m_wHKMinute = wMinute;

	m_dbfHK.MoveNext();       //�ƶ�ָ�뵽�ڶ�������
	int nCode = 0;
	char szCode[50];
	while(!m_dbfHK.IsEOF())
	{
		//����Ʊ�������Ч��
		memset(szCode, 0, 50);
		m_dbfHK.GetFieldValue("S1", szCode);
		if (!IsValidStockCode(szCode))
		{
			REPORT_Ex((T("����Ĺ�Ʊ����:%s������dbf\n", szCode)).c_str(), RPT_CRITICAL);
			return FALSE;
		}
		//end

		m_dbfHK.GetFieldValue("S1", nCode);
		if (nCode == 100013) //�����¼��ɨ��
		{
			m_dbfHK.MoveNext();
			continue;
		}
		m_HKiterator = m_HKMap.find(nCode);

		BOOL bIndex = FALSE;
		if(nCode > HK_INDEX_CODE_MAX)
			bIndex = TRUE;
		
		if( m_HKiterator == m_HKMap.end() )//������
		{
			HKStockNotExist(wMinute, nCode, bIndex);
		}
		else
		{
			tagHKRecord* pRec = m_HKiterator->second;
			if (pRec->pHQRecBuf == NULL)  //��Ʊ������ڣ�tagSZRecordָ�����,���������ݲ�����
			{
				HKStockNotExist(wMinute, nCode, bIndex, TRUE);
			}
			else
			{
				HKStockExist(wMinute, bIndex);
			}
		}

		m_dbfHK.MoveNext();
	}

	//********************************************************
	packHdr.ulPackSize = m_nHKPackSize;
	memcpy(m_pHKSendBuf, &packHdr, sizeof(tagPackHeader));

	if(m_nHKPackSize > 0)
	{
		//Send   m_pHKSendBuf
		int nLen = m_nHKPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //����4���ֽڸ�������ʹ��
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for HKSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pHKSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pHKSendBuf, PACK_HEADER_LEN);
			
			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pHKSendBuf+PACK_HEADER_LEN, m_nHKPackSize);
			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8���ֽڵı���ͷ
		}
	}

	return TRUE;
}

void CScanThread::HKStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist)
{
	if (bExist)  //��Ʊ������ڣ�tagSZRecordָ����ڣ����������ݲ�����
	{
		tagHKRecord* pOldRec = m_HKiterator->second;
		int nRecordLen = m_dbfHK.GetRecordSize();
		
		pOldRec->byScanTimes++;
		pOldRec->pHQRecBuf = new char[nRecordLen];
		if (pOldRec->pHQRecBuf == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec->pRecordBuf, function return\n", RPT_ERROR);
			return;
		}

		m_dbfHK.GetRecordToBuffer(pOldRec->pHQRecBuf);
	}
	else
	{
		int nRecordLen = m_dbfHK.GetRecordSize();
		tagHKRecord* pRec = new tagHKRecord;
		if(!pRec)
		{
			REPORT_Ex("Allocate memory failed for pRec, function return\n", RPT_ERROR);
			return;
		}

		pRec->byScanTimes++;
		pRec->pHQRecBuf = new char[nRecordLen];
		if (pRec->pHQRecBuf == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec->pRecordBuf, function return\n", RPT_ERROR);
			return;
		}

		m_dbfHK.GetRecordToBuffer(pRec->pHQRecBuf);
		m_HKMap.insert(HKRecord_Pair(nCode, pRec));  //����MAP��
	}
	//��Ʊ��Ϣ����Ϣͷ��
	
	tagNewPackInfo newPackInfo;
	
	if (bExist)
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NORMAL;
	else
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NEW;

	newPackInfo.packinfo.wTime       = wMinute;
	newPackInfo.packinfo.wFieldCount = HK_MAX_FIELD;  //28���ֶ���Ҫ����(�۹ɹ�28���ֶ�)
	
	if(bIndex)
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "HK");
	else
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "hk");
	
	m_dbfHK.GetFieldValue("S1", newPackInfo.packinfo.szStockCode);
	ChangeStockCode(newPackInfo.packinfo.szStockCode, bIndex);

	if (!bExist) //dbf������̫�� 16�ֽ� newPackInfo.szStockNameֻ��12�ֽ�
	{
		m_dbfHK.GetFieldValue("S2", newPackInfo.szStockName);
		Trim(newPackInfo.szStockName);
	}
	
	//�ڴ��С���*****************************************
	CheckHKSendBufForSend();
	//*****************************************************

	if (bExist)	//ȥ����Ʊ����
	{
		memcpy(m_pHKCurSendBufPos, &newPackInfo, sizeof(tagPackInfo));
		m_nHKPackSize += sizeof(tagPackInfo);
		m_pHKCurSendBufPos += sizeof(tagPackInfo);
	}
	else
	{
		memcpy(m_pHKCurSendBufPos, &newPackInfo, sizeof(tagNewPackInfo));
		m_nHKPackSize += sizeof(tagNewPackInfo);
		m_pHKCurSendBufPos += sizeof(tagNewPackInfo);
	}

	//�ֶ�ֵ
	FillValueToBuf(FD_S3, "S3", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S4, "S4", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S5, "S5", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S6, "S6", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S7, "S7", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S8, "S8", HK_HQ_DBF, bIndex);
	//����2008-10-17
	FillValueToBuf(FD_S9, "S9", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S10, "S10", HK_HQ_DBF, bIndex);
	//
	FillValueToBuf(FD_S11, "S11", HK_HQ_DBF, bIndex);

	FillValueToBuf(FD_S13, "S13", HK_HQ_DBF, bIndex);

	FillValueToBuf(FD_S15, "S15", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S16, "S16", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S17, "S17", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S18, "S18", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S19, "S19", HK_HQ_DBF, bIndex);

	FillValueToBuf(FD_S21, "S21", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S22, "S22", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S23, "S23", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S24, "S24", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S25, "S25", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S26, "S26", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S27, "S27", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S28, "S28", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S29, "S29", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S30, "S30", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S31, "S31", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S32, "S32", HK_HQ_DBF, bIndex);
	FillValueToBuf(FD_S33, "S33", HK_HQ_DBF, bIndex);
}

void CScanThread::HKStockExist(unsigned short wMinute, BOOL bIndex)
{
	tagHKRecord* pRec = m_HKiterator->second;
	pRec->byScanTimes++;

	//��Ʊ��Ϣ����Ϣͷ��
	tagPackInfo packInfo;
	packInfo.wStockType  = STOCK_TYPE_NORMAL;
	packInfo.wTime       = wMinute;
	//packInfo.wFieldCount//���ڲ���ȷ��
	
	if(bIndex)
		strcpy_s(packInfo.szMarketType, 6, "HK");
	else
		strcpy_s(packInfo.szMarketType, 6, "hk");	
	
	m_dbfHK.GetFieldValue("S1", packInfo.szStockCode);
	
	ChangeStockCode(packInfo.szStockCode, bIndex);
	//�ڴ��С���*****************************************
	CheckHKSendBufForSend();
	//*****************************************************

	//������Ϣͷ�Ŀռ䣬���û�б仯���ֶΣ��ƶ�ָ�뵽ԭʼ��m_pCurSendBufPos=pSavePackInfoBuf
	char* pSavePackInfoBuf = m_pHKCurSendBufPos;
	m_nHKPackSize += sizeof(tagPackInfo);
	m_pHKCurSendBufPos += sizeof(tagPackInfo);	

	//�Ƚ��ֶ�
	BOOL  bFieldChanged = FALSE;
	int   nFieldCount   = 0;
	pRec->bNameChanged = IsStockNameChanged(packInfo.szStockCode, pRec->pHQRecBuf, HK_HQ_DBF);
	//****************************************
	BOOL  bCjsl = FALSE;
	BOOL  bCjje = FALSE;

	bFieldChanged |= FillChangedValueToBuf(FD_S3, "S3", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S4, "S4", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);

	bCjje = FillChangedValueToBuf(FD_S5, "S5", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= bCjje;

	bFieldChanged |= FillChangedValueToBuf(FD_S6, "S6", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S7, "S7", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S8,  "S8", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	//����2008-10-27
	bFieldChanged |= FillChangedValueToBuf(FD_S9,  "S9", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S10,  "S10", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	//
	bCjsl = FillChangedValueToBuf(FD_S11, "S11", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= bCjsl;
	bFieldChanged |= FillChangedValueToBuf(FD_S13, "S13", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S15, "S15", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S16, "S16", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S17, "S17", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S18, "S18", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S19, "S19", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S21, "S21", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S22, "S22", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S23, "S23", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S24, "S24", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S25, "S25", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S26, "S26", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S27, "S27", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S28, "S28", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S29, "S29", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S30, "S30", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S31, "S31", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S32, "S32", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S33, "S33", pRec->pHQRecBuf, nFieldCount, HK_HQ_DBF, bIndex);

	if (bCjsl && bCjje && !bIndex) //�������
	{					  		
		char  szField[30] = {0};
		int   nFieldSize = m_dbfHK.GetFieldValue("S11", szField);
		unsigned int nCjsl = CCharTranslate::VolumeToDigital100(szField, nFieldSize);

		nFieldSize = m_dbfHK.GetFieldValue("S5", szField);
		unsigned int nCjje = CCharTranslate::SumPriceToDigital1000(szField, nFieldSize);

		if (nCjsl != 0 && nCjje != 0)
		{
			nFieldCount++;
			unsigned int nAverage = (unsigned int)( ((double)nCjje / (double)nCjsl) * 10000 );  // �ɽ���� / �ɽ����� * 1000

			tagFieldInfo* pFieldInfo = (tagFieldInfo*)m_pHKCurSendBufPos;
			pFieldInfo->wFieldType  = FD_AVERAGE;
			pFieldInfo->nFieldValue = nAverage;
			m_nHKPackSize          += sizeof(tagFieldInfo);
			m_pHKCurSendBufPos     += sizeof(tagFieldInfo);
		}
	}
	//****************************************
	if(bFieldChanged)
	{
		packInfo.wFieldCount = nFieldCount; //�仯�˵��ֶ�
		memcpy(pSavePackInfoBuf, &packInfo, sizeof(tagPackInfo));
		m_dbfHK.GetRecordToBuffer(pRec->pHQRecBuf); //�����µ����ݴ���MAP
	}
	else  //�ƶ�ָ�뵽ԭʼ��m_pCurSendBufPos=pSavePackInfoBuf
	{
		m_nHKPackSize     -= sizeof(tagPackInfo);
		m_pHKCurSendBufPos = pSavePackInfoBuf;
	}
}

void CScanThread::ChangeStockCode(char* pStockCode, BOOL bIndex)
{
	const unsigned short STOCKCODE_LEN = 8;
	if (!bIndex)
	{
		char szStockCode[STOCKCODE_LEN];
		strcpy_s(szStockCode, STOCKCODE_LEN, pStockCode);
		memset(pStockCode, 0, STOCKCODE_LEN);
		memcpy(pStockCode, szStockCode+1, STOCKCODE_LEN-1);
		return;
	}
	else
	{
		if (_strcmpi(pStockCode, "100001") == 0) //����ָ��
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSI");
			return;
		}
		if (_strcmpi(pStockCode, "100002") == 0) //�����ۺ�
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSCI");
			return;
		}
		if (_strcmpi(pStockCode, "100003") == 0) //����ָ
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSCCI");
			return;
		}
		if (_strcmpi(pStockCode, "100004") == 0) //H��ָ��
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSCFI");
			return;
		}
		if (_strcmpi(pStockCode, "100005") == 0) //����ۺ�00CIHK
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSHKCI");
			return;
		}

		if (_strcmpi(pStockCode, "100006") == 0) //���ڷ���
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSIFIN");
			return;
		}
		if (_strcmpi(pStockCode, "100007") == 0) //���÷���
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSIUTL");
			return;
		}
		if (_strcmpi(pStockCode, "100008") == 0) //�ز�����
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSIPRO");
			return;
		}
		if (_strcmpi(pStockCode, "100009") == 0) //���̷���
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSICI");
			return;
		}
		if (_strcmpi(pStockCode, "100010") == 0) //�ڵ��ۺ�
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HSMLCI");
			return;
		}

		if (_strcmpi(pStockCode, "100011") == 0) //��ҵָ��
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "GEM");
			return;
		}
		if (_strcmpi(pStockCode, "100012") == 0) //���͹�ָ
		{
			memset(pStockCode, 0, STOCKCODE_LEN);
			strcpy_s(pStockCode, STOCKCODE_LEN, "HKL");
			return;
		}
	}
}

void CScanThread::Trim(char* pData)
{
	int nLen = (int)strlen(pData);
	for (int i= nLen-1; i>=0; i--)
	{
		if (pData[i] == ' ')
			pData[i] = 0;
		else
			break;
	}
}

BOOL CScanThread::IsValidStockCode(char* pCode)
{
	for (int i=0; i<6; i++)  //�۹�dbf�еĹ�Ʊ������6λ
	{
		if(pCode[i] < '0' || pCode[i] > '9')
			return FALSE;
	}

	return TRUE;
}

bool CScanThread::IsStockNameChanged(const char* pStockCode, const char* pOldRecord, unsigned int nDBFType)
{
	char* pNewField = NULL;

	if (nDBFType == HK_HQ_DBF)
		pNewField = m_dbfHK.CompFieldBuffer("S2", pOldRecord);

	if(pNewField != NULL)
	{
		char szName[17] = {0};
		memcpy(szName, pNewField, 16);
		REPORT_Ex((T("��Ʊ����[%s]�����Ʊ仯Ϊ[%s]\n", pStockCode, szName)).c_str(), RPT_INFO);
		return true;
	}
	
	return false;
}