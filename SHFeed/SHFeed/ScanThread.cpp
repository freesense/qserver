//ScanThread.cpp
//

#include "stdafx.h"
#include "ScanThread.h"
#include "../../public/CharTranslate.h"
#include "../../public/feedinterface.h"
#include "../../public/commx/highperformancecounter.h"
#include "constdef.h"
#include "../../public/commx/mery.h"

extern IFeedOwner*    g_pOwner;
extern int g_nTimeout;
extern std::string    g_strSZRegex;        //深圳国债回购正则表达式
extern std::string    g_strSHRegex;


#define  MAP_TYPE_SZ	      1
#define  MAP_TYPE_SH	      2
#define  SUBSLEN              10 

#define  SZ_INDEX_CODE_MIN   399000	  //深圳交易所 指数范围的最小值
#define  SH_INDEX_CODE_MAX	 2000	  //上海交易所 指数范围的最大值
///////////////////////////////////////////////////////////////
CScanThread::CScanThread()
{
	m_szDbfPath_SZ[0]   = 0;
	m_szDbfPath_SH[0]   = 0;
	m_szDbfPath_SZXX[0] = 0;

	m_nSZPackSize       = 0;
	m_pSZSendBuf        = NULL;
	m_nSZSendBufSize    = 0;
	m_pSZCurSendBufPos  = 0;

	m_nSHPackSize       = 0;
	m_pSHSendBuf        = NULL;
	m_nSHSendBufSize    = 0;
	m_pSHCurSendBufPos  = 0;

	m_nSZXXPackSize       = 0;
	m_pSZXXSendBuf        = NULL;
	m_nSZXXSendBufSize    = 0;
	m_pSZXXCurSendBufPos  = 0;

	m_wSZMinute = 0;  //深圳dbf最新的时间
	m_wSHMinute = 0; 

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
	if(m_pSZSendBuf)
	{
		delete []m_pSZSendBuf;
		m_pSZSendBuf     = NULL;
	}

	if(m_pSHSendBuf)
	{
		delete []m_pSHSendBuf;
		m_pSHSendBuf     = NULL;
	}

	if(m_pSZXXSendBuf)
	{
		delete []m_pSZXXSendBuf;
		m_pSZXXSendBuf     = NULL;
	}
}

BOOL CScanThread::SetDBFPath(const char *pSZPath, const char *pSHPath)
{
	strcpy_s(m_szDbfPath_SZ, MAX_PATH, pSZPath);
	strcpy_s(m_szDbfPath_SH, MAX_PATH, pSHPath);

	char szMsg[MSG_BUF_LEN];
	//打开深圳行情库
	if(!m_dbfSZ.Open(m_szDbfPath_SZ))
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s failed!\n", m_szDbfPath_SZ);
		REPORT_Ex(szMsg, RPT_ERROR);
		return FALSE;
	}
	else
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s successed!\n", m_szDbfPath_SZ);
		REPORT_Ex(szMsg, RPT_INFO);
	}

	//打开上海行情库
	if(!m_dbfSH.Open(m_szDbfPath_SH))
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s failed!\n", m_szDbfPath_SH);
		REPORT_Ex(szMsg, RPT_ERROR);
		return FALSE;
	}
	else
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s successed!\n", m_szDbfPath_SH);
		REPORT_Ex(szMsg, RPT_INFO);
	}

	//打开深圳信息库
	strcpy_s(m_szDbfPath_SZXX, MAX_PATH, m_szDbfPath_SZ);
	char* pPath = strrchr(m_szDbfPath_SZXX, '\\');
	pPath[0] = 0;
	strcat_s(m_szDbfPath_SZXX, MAX_PATH, "\\SJSXX.DBF");
	if(!m_dbfSZXX.Open(m_szDbfPath_SZXX))
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s failed!\n", m_szDbfPath_SZXX);
		REPORT_Ex(szMsg, RPT_ERROR);
		return FALSE;
	}
	else
	{
		sprintf_s(szMsg, MSG_BUF_LEN, "Open %s successed!\n", m_szDbfPath_SZXX);
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
	//释放深圳MAP
	for( m_SZiterator = m_SZMap.begin(); m_SZiterator != m_SZMap.end(); m_SZiterator++)
	{
		tagSZRecord* pRec = m_SZiterator->second;
		if(pRec)
		{
			if(pRec->pHQRecBuf)
				delete[] pRec->pHQRecBuf;

			if(pRec->pXXRecBuf)
				delete[] pRec->pXXRecBuf;

			delete pRec;
			pRec = NULL;
		}
	}

	m_SZMap.clear();

	//释放上海行情库MAP
	for( m_SHiterator = m_SHMap.begin(); m_SHiterator != m_SHMap.end(); m_SHiterator++)
	{
		tagSHRecord* pRec = m_SHiterator->second;
		if(pRec)
		{
			if(pRec->pHQRecBuf)
				delete[] pRec->pHQRecBuf;

			delete pRec;
			pRec = NULL;
		}
	}

	m_SHMap.clear();
}

void CScanThread::FillValueToBuf(unsigned short wFieldType, const char* pFieldName,
								 unsigned int nDBFType, BOOL bIndex)
{
	tagFieldInfo   fieldInfo;
	char           szField[30];
	int            nFieldSize = 0;

	fieldInfo.wFieldType  = wFieldType;
	if (nDBFType == SZ_HQ_DBF)
		nFieldSize = m_dbfSZ.GetFieldValue(pFieldName, szField);
	else if (nDBFType == SZ_XX_DBF)
		nFieldSize = m_dbfSZXX.GetFieldValue(pFieldName, szField);
	else if (nDBFType == SH_HQ_DBF)
		nFieldSize = m_dbfSH.GetFieldValue(pFieldName, szField);

	switch(wFieldType)
	{
		//深圳行情
	case FD_ZRSP: case FD_JRKP: case FD_ZJCJ: case FD_ZGCJ: case FD_ZDCJ:
	case FD_SJW5: case FD_SJW4: case FD_SJW3:
	case FD_SJW2: case FD_SJW1: case FD_BJW1: case FD_BJW2: case FD_BJW3:
	case FD_BJW4: case FD_BJW5:
		//深圳信息
	case FD_MGMZ: case FD_JSFL: case FD_YHSL: case FD_GHFL:	case FD_JGDW:	
	case FD_JHCS: case FD_LXCS: case FD_ZTJG: case FD_DTJG:	case FD_ZHBL:
		{
			fieldInfo.nFieldValue = CCharTranslate::PriceToDigital(szField, nFieldSize);
			break;
		}
		//深圳
	case FD_CJSL: 
		{
			if (!bIndex)
				fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital100(szField, nFieldSize);
			else
			{
				if (nDBFType == SH_HQ_DBF)
					fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital(szField, nFieldSize);
				else
					fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital100(szField, nFieldSize);
			}
			break;
		}
		//深圳
	case FD_CJJE:
		{
			fieldInfo.nFieldValue = CCharTranslate::SumPriceToDigital1000(szField, nFieldSize);
			break;
		}
		//深圳
	case FD_CJBS: case FD_HYCC:	case FD_SSL5: case FD_SSL4: case FD_SSL3: case FD_SSL2:
	case FD_SSL1: case FD_BSL1: case FD_BSL2: case FD_BSL3: case FD_BSL4:
	case FD_BSL5:
		//深圳信息
	case FD_JYDW: case FD_MBXL: case FD_BLDW: case FD_SLDW: case FD_XJXZ:
		{
			fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital(szField, nFieldSize);
			break;
		}
		//深圳
	case FD_JSD1: case FD_JSD2: case FD_SYL1: case FD_SYL2:// 价格有正负之分，而发送的字段又是无符号整形
		//深圳信息
	case FD_SNLR: case FD_BNLR:
		{
			fieldInfo.nFieldValue = CCharTranslate::SignedPriceToDigital(szField, nFieldSize);
			break;
		}
		//深圳信息
	case FD_ZFXL:	case FD_LTGS:
		{
			fieldInfo.nFieldValue = CCharTranslate::VolumeToDigital10000(szField, nFieldSize);
			break;
		}
	}

	if (nDBFType == SZ_HQ_DBF)
	{
		memcpy(m_pSZCurSendBufPos, &fieldInfo, sizeof(tagFieldInfo));
		m_nSZPackSize          += sizeof(tagFieldInfo);
		m_pSZCurSendBufPos     += sizeof(tagFieldInfo);
	}
	else if (nDBFType == SZ_XX_DBF)
	{
		memcpy(m_pSZXXCurSendBufPos, &fieldInfo, sizeof(tagFieldInfo));
		m_nSZXXPackSize          += sizeof(tagFieldInfo);
		m_pSZXXCurSendBufPos     += sizeof(tagFieldInfo);
	}
	else if (nDBFType == SH_HQ_DBF)
	{
		memcpy(m_pSHCurSendBufPos, &fieldInfo, sizeof(tagFieldInfo));
		m_nSHPackSize          += sizeof(tagFieldInfo);
		m_pSHCurSendBufPos     += sizeof(tagFieldInfo);
	}
}

void CScanThread::SZStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist)
{
	if (bExist)  //股票代码存在，tagSZRecord指针存在，但行情数据不存在
	{
		tagSZRecord* pOldRec = m_SZiterator->second;
		int nRecordLen = m_dbfSZ.GetRecordSize();

		pOldRec->byScanTimes++;
		pOldRec->pHQRecBuf = new char[nRecordLen];
		if(pOldRec->pHQRecBuf == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec->pRecordBuf, function return\n", RPT_ERROR);
			return;
		}

		m_dbfSZ.GetRecordToBuffer(pOldRec->pHQRecBuf);
	}
	else
	{
		tagSZRecord* pRec = new tagSZRecord;
		if(!pRec)
		{
			REPORT_Ex("Allocate memory failed for pRec, function return\n", RPT_ERROR);
			return;
		}

		int nRecordLen = m_dbfSZ.GetRecordSize();
		pRec->byScanTimes++;

		pRec->pHQRecBuf = new char[nRecordLen];
		if(pRec->pHQRecBuf == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec->pRecordBuf, function return\n", RPT_ERROR);
			return;
		}

		m_dbfSZ.GetRecordToBuffer(pRec->pHQRecBuf);
		m_SZMap.insert(SZRecord_Pair(nCode, pRec));  //放入MAP中
	}

	//股票信息（信息头）
	tagNewPackInfo newPackInfo;

	if (bExist)
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NORMAL;
	else
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NEW;

	newPackInfo.packinfo.wTime       = wMinute;
	newPackInfo.packinfo.wFieldCount = SZ_MAX_FIELD;  //33个字段需要发送

	if(bIndex)
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "SZ");
	else
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "sz");

	m_dbfSZ.GetFieldValue("Hqzqdm", newPackInfo.packinfo.szStockCode);

	if (!bExist)
	{
		m_dbfSZ.GetFieldValue("Hqzqjc", newPackInfo.szStockName);
		Trim(newPackInfo.szStockName);
	}

	//内存大小检测*****************************************
	CheckSZSendBufForSend();
	//*****************************************************

	if (bExist)	//去掉股票名称
	{
		memcpy(m_pSZCurSendBufPos, &newPackInfo, sizeof(tagPackInfo));
		m_nSZPackSize += sizeof(tagPackInfo);
		m_pSZCurSendBufPos += sizeof(tagPackInfo);
	}
	else
	{
		memcpy(m_pSZCurSendBufPos, &newPackInfo, sizeof(tagNewPackInfo));
		m_nSZPackSize += sizeof(tagNewPackInfo);
		m_pSZCurSendBufPos += sizeof(tagNewPackInfo);
	}
	//字段值
	FillValueToBuf(FD_ZRSP, "Hqzrsp", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_JRKP, "Hqjrkp", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_ZJCJ, "Hqzjcj", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_CJSL, "Hqcjsl", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_CJJE, "Hqcjje", SZ_HQ_DBF, bIndex);

	FillValueToBuf(FD_CJBS, "Hqcjbs", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_ZGCJ, "Hqzgcj", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_ZDCJ, "Hqzdcj", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SYL1, "Hqsyl1", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SYL2, "Hqsyl2", SZ_HQ_DBF, bIndex);

	FillValueToBuf(FD_JSD1, "Hqjsd1", SZ_HQ_DBF, bIndex);//?有正负之分
	FillValueToBuf(FD_JSD2, "Hqjsd2", SZ_HQ_DBF, bIndex);//?
	FillValueToBuf(FD_HYCC, "Hqhycc", SZ_HQ_DBF, bIndex);

	FillValueToBuf(FD_SJW5, "Hqsjw5", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SSL5, "Hqssl5", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SJW4, "Hqsjw4", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SSL4, "Hqssl4", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SJW3, "Hqsjw3", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SSL3, "Hqssl3", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SJW2, "Hqsjw2", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SSL2, "Hqssl2", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SJW1, "Hqsjw1", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_SSL1, "Hqssl1", SZ_HQ_DBF, bIndex);

	FillValueToBuf(FD_BJW1, "Hqbjw1", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BSL1, "Hqbsl1", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BJW2, "Hqbjw2", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BSL2, "Hqbsl2", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BJW3, "Hqbjw3", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BSL3, "Hqbsl3", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BJW4, "Hqbjw4", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BSL4, "Hqbsl4", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BJW5, "Hqbjw5", SZ_HQ_DBF, bIndex);
	FillValueToBuf(FD_BSL5, "Hqbsl5", SZ_HQ_DBF, bIndex);
}

void CScanThread::SZXXStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist)
{
	if (bExist)  //股票代码存在，但行情数据不存在
	{
		tagSZRecord* pOldRec = m_SZiterator->second;
		int nRecordLen = m_dbfSZXX.GetRecordSize();

		pOldRec->byScanTimes++;
		pOldRec->pXXRecBuf = new char[nRecordLen];
		if(pOldRec->pXXRecBuf == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec->pRecordBuf, function return\n", RPT_ERROR);
			return;
		}

		m_dbfSZXX.GetRecordToBuffer(pOldRec->pXXRecBuf);
	}
	else
	{
		int nRecordLen = m_dbfSZXX.GetRecordSize();
		tagSZRecord* pRec = new tagSZRecord;
		if(pRec == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec, function return\n", RPT_ERROR);
			return;
		}

		pRec->byScanTimes++;
		pRec->pXXRecBuf = new char[nRecordLen];
		if(pRec->pXXRecBuf == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec->pRecordBuf, function return\n", RPT_ERROR);
			return;
		}

		m_dbfSZXX.GetRecordToBuffer(pRec->pXXRecBuf);
		m_SZMap.insert(SZRecord_Pair(nCode, pRec));  //放入MAP中
	}
	//股票信息（信息头）
	tagNewPackInfo newPackInfo;

	if (bExist)
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NORMAL;
	else
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NEW;

	newPackInfo.packinfo.wTime       = wMinute;
	newPackInfo.packinfo.wFieldCount = SZXX_MAX_FIELD;  //19个字段需要发送

	if(bIndex)
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "SZ");
	else
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "sz");

	m_dbfSZXX.GetFieldValue("Xxzqdm", newPackInfo.packinfo.szStockCode);

	if (!bExist) //股票代码不存在时，必须发送股票名称
	{
		m_dbfSZXX.GetFieldValue("Xxzqjc", newPackInfo.szStockName);
		Trim(newPackInfo.szStockName);
	}

	//内存大小检测*****************************************
	CheckSZXXSendBufForSend();
	//*****************************************************

	if (bExist) //去掉股票名称
	{
		memcpy(m_pSZXXCurSendBufPos, &newPackInfo, sizeof(tagPackInfo));
		m_nSZXXPackSize += sizeof(tagPackInfo);
		m_pSZXXCurSendBufPos += sizeof(tagPackInfo);
	}
	else
	{
		memcpy(m_pSZXXCurSendBufPos, &newPackInfo, sizeof(tagNewPackInfo));
		m_nSZXXPackSize += sizeof(tagNewPackInfo);
		m_pSZXXCurSendBufPos += sizeof(tagNewPackInfo);
	}
	//字段值
	FillValueToBuf(FD_JYDW, "Xxjydw", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_MGMZ, "Xxmgmz", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_ZFXL, "Xxzfxl", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_LTGS, "Xxltgs", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_SNLR, "Xxsnlr", SZ_XX_DBF, bIndex);

	FillValueToBuf(FD_BNLR, "Xxbnlr", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_JSFL, "Xxjsfl", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_YHSL, "Xxyhsl", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_GHFL, "Xxghfl", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_MBXL, "Xxmbxl", SZ_XX_DBF, bIndex);

	FillValueToBuf(FD_BLDW, "Xxbldw", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_SLDW, "Xxsldw", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_JGDW, "Xxjgdw", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_JHCS, "Xxjhcs", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_LXCS, "Xxlxcs", SZ_XX_DBF, bIndex);

	FillValueToBuf(FD_XJXZ, "Xxxjxz", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_ZTJG, "Xxztjg", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_DTJG, "Xxdtjg", SZ_XX_DBF, bIndex);
	FillValueToBuf(FD_ZHBL, "Xxzhbl", SZ_XX_DBF, bIndex);
}

void CScanThread::SZStockExist(unsigned short wMinute, BOOL bIndex)
{
	tagSZRecord* pRec = m_SZiterator->second;
	pRec->byScanTimes++;

	//股票信息（信息头）
	tagPackInfo packInfo;
	packInfo.wStockType  = STOCK_TYPE_NORMAL;
	packInfo.wTime       = wMinute;
	//packInfo.wFieldCount//现在不能确定

	if (bIndex)
		strcpy_s(packInfo.szMarketType, 6, "SZ");
	else
		strcpy_s(packInfo.szMarketType, 6, "sz");

	m_dbfSZ.GetFieldValue("Hqzqdm", packInfo.szStockCode);

	//内存大小检测*****************************************
	CheckSZSendBufForSend();
	//*****************************************************

	//留出信息头的空间，如果没有变化的字段，移动指针到原始处m_pCurSendBufPos=pSavePackInfoBuf
	char* pSavePackInfoBuf = m_pSZCurSendBufPos;
	m_nSZPackSize += sizeof(tagPackInfo);
	m_pSZCurSendBufPos += sizeof(tagPackInfo);	

	//比较字段
	BOOL  bFieldChanged = FALSE;
	int   nFieldCount   = 0;
	pRec->bNameChanged = IsStockNameChanged(packInfo.szStockCode, pRec->pHQRecBuf, SZ_HQ_DBF);
	//****************************************
	BOOL  bCjsl = FALSE;
	BOOL  bCjje = FALSE;

	bFieldChanged |= FillChangedValueToBuf(FD_ZRSP, "Hqzrsp", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_JRKP, "Hqjrkp", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_ZJCJ, "Hqzjcj", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);

	bCjsl = FillChangedValueToBuf(FD_CJSL, "Hqcjsl", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= bCjsl;

	bCjje = FillChangedValueToBuf(FD_CJJE, "Hqcjje", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= bCjje;

	bFieldChanged |= FillChangedValueToBuf(FD_CJBS, "Hqcjbs", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_ZGCJ, "Hqzgcj", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_ZDCJ, "Hqzdcj", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SYL1, "Hqsyl1", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SYL2, "Hqsyl2", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_JSD1, "Hqjsd1", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_JSD2, "Hqjsd2", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_HYCC, "Hqhycc", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_SJW5, "Hqsjw5", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SSL5, "Hqssl5", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SJW4, "Hqsjw4", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SSL4, "Hqssl4", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SJW3, "Hqsjw3", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SSL3, "Hqssl3", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SJW2, "Hqsjw2", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SSL2, "Hqssl2", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SJW1, "Hqsjw1", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SSL1, "Hqssl1", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);


	bFieldChanged |= FillChangedValueToBuf(FD_BJW1, "Hqbjw1", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BSL1, "Hqbsl1", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BJW2, "Hqbjw2", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BSL2, "Hqbsl2", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BJW3, "Hqbjw3", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BSL3, "Hqbsl3", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BJW4, "Hqbjw4", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BSL4, "Hqbsl4", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BJW5, "Hqbjw5", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_BSL5, "Hqbsl5", pRec->pHQRecBuf, nFieldCount, SZ_HQ_DBF, bIndex);

	if (bCjsl && bCjje && !bIndex && !IsGuoZaiHuiGou(SZ_HQ_DBF, packInfo.szStockCode)) //计算均价
	{
		// 由于精度问题，缩小后的成交金额会丢失数据，因此这里采用原始数据计算均价后再进行放大缩小

		char  szField[30] = {0};
		int   nFieldSize = m_dbfSZ.GetFieldValue("Hqcjsl", szField);
// 		unsigned int nCjsl = CCharTranslate::VolumeToDigital100(szField, nFieldSize);
		unsigned int nCjsl = atoi(szField);

		nFieldSize = m_dbfSZ.GetFieldValue("Hqcjje", szField);
// 		unsigned int nCjje = CCharTranslate::SumPriceToDigital1000(szField, nFieldSize);
		double fCjje = atof(szField);

		if (nCjsl != 0 && fCjje != 0)
		{
			nFieldCount++;
// 			unsigned int nAverage = (unsigned int)( ((double)nCjje / (double)nCjsl) * 10000 );  // 成交金额 / 成交数量 * 1000
			unsigned int nAverage = (unsigned int)(fCjje / nCjsl * 1000);

			tagFieldInfo* pFieldInfo = (tagFieldInfo*)m_pSZCurSendBufPos;
			pFieldInfo->wFieldType  = FD_AVERAGE;
			pFieldInfo->nFieldValue = nAverage;
			m_nSZPackSize          += sizeof(tagFieldInfo);
			m_pSZCurSendBufPos     += sizeof(tagFieldInfo);
		}
	}
	//****************************************
	if(bFieldChanged)
	{
		packInfo.wFieldCount = nFieldCount; //变化了的字段
		memcpy(pSavePackInfoBuf, &packInfo, sizeof(tagPackInfo));
		m_dbfSZ.GetRecordToBuffer(pRec->pHQRecBuf); //把最新的数据存入MAP
	}
	else  //移动指针到原始处m_pCurSendBufPos=pSavePackInfoBuf
	{
		m_nSZPackSize     -= sizeof(tagPackInfo);
		m_pSZCurSendBufPos = pSavePackInfoBuf;
	}
}

BOOL CScanThread::IsGuoZaiHuiGou(unsigned int nDBFType, char* pStockCode)
{
	char szSymbol[30] = {0};
	rpattern_c::backref_type br;

	if (nDBFType == SZ_HQ_DBF) //深圳
	{
		sprintf_s(szSymbol, 30, "%s.sz", pStockCode);
		
		rpattern_c pat(g_strSZRegex.c_str());
		match_results_c results;
		br = pat.match( szSymbol, results );
	}
	else					   //上海
	{
		sprintf_s(szSymbol, 30, "%s.sh", pStockCode);
		rpattern_c pat(g_strSHRegex.c_str());
		match_results_c results;
		br = pat.match( szSymbol, results );
	}

	if (br.matched)
		return TRUE;
	else
		return FALSE;
}

void CScanThread::SZXXStockExist(unsigned short wMinute, BOOL bIndex)
{
	tagSZRecord* pRec = m_SZiterator->second;
	pRec->byScanTimes++;

	//股票信息（信息头）
	tagPackInfo packInfo;
	packInfo.wStockType  = STOCK_TYPE_NORMAL;
	packInfo.wTime       = wMinute;
	//packInfo.wFieldCount//现在不能确定

	if (bIndex)
		strcpy_s(packInfo.szMarketType, 6, "SZ");
	else
		strcpy_s(packInfo.szMarketType, 6, "sz");

	m_dbfSZXX.GetFieldValue("Xxzqdm", packInfo.szStockCode);

	//内存大小检测*****************************************
	CheckSZXXSendBufForSend();
	//*****************************************************

	//留出信息头的空间，如果没有变化的字段，移动指针到原始处m_pCurSendBufPos=pSavePackInfoBuf
	char* pSavePackInfoBuf = m_pSZXXCurSendBufPos;
	m_nSZXXPackSize += sizeof(tagPackInfo);
	m_pSZXXCurSendBufPos += sizeof(tagPackInfo);	

	//比较字段
	BOOL  bFieldChanged = FALSE;
	int   nFieldCount   = 0;
	//****************************************
	bFieldChanged |= FillChangedValueToBuf(FD_JYDW, "Xxjydw", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_MGMZ, "Xxmgmz", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_ZFXL, "Xxzfxl", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_LTGS, "Xxltgs", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SNLR, "Xxsnlr", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_BNLR, "Xxbnlr", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_JSFL, "Xxjsfl", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_YHSL, "Xxyhsl", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_GHFL, "Xxghfl", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_MBXL, "Xxmbxl", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_BLDW, "Xxbldw", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_SLDW, "Xxsldw", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_JGDW, "Xxjgdw", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_JHCS, "Xxjhcs", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_LXCS, "Xxlxcs", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_XJXZ, "Xxxjxz", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_ZTJG, "Xxztjg", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_DTJG, "Xxdtjg", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_ZHBL, "Xxzhbl", pRec->pXXRecBuf, nFieldCount, SZ_XX_DBF, bIndex);

	//****************************************
	if(bFieldChanged)
	{
		packInfo.wFieldCount = nFieldCount; //变化了的字段
		memcpy(pSavePackInfoBuf, &packInfo, sizeof(tagPackInfo));
		m_dbfSZXX.GetRecordToBuffer(pRec->pXXRecBuf); //把最新的数据存入MAP
	}
	else  //移动指针到原始处m_pCurSendBufPos=pSavePackInfoBuf
	{
		m_nSZXXPackSize     -= sizeof(tagPackInfo);
		m_pSZXXCurSendBufPos = pSavePackInfoBuf;
	}
}

/**********************************************
如果字段值不同返回TRUE, nFieldCount加1，
**********************************************/
BOOL CScanThread::FillChangedValueToBuf(unsigned short wFieldType, const char* pFieldName,
										const char* pOldRecord,	int& nFieldCount, unsigned int nDBFType, BOOL bIndex)
{
	char* pNewField = NULL;

	if (nDBFType == SZ_HQ_DBF)
		pNewField = m_dbfSZ.CompFieldBuffer(pFieldName, pOldRecord);
	else if (nDBFType == SZ_XX_DBF)
		pNewField = m_dbfSZXX.CompFieldBuffer(pFieldName, pOldRecord);
	else if (nDBFType == SH_HQ_DBF)
		pNewField = m_dbfSH.CompFieldBuffer(pFieldName, pOldRecord);	

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
	//设置深圳MAP股票扫描的次数
	if(byMapType == MAP_TYPE_SZ)
	{
		map<int, tagSZRecord*>::iterator     SZiterator;

		for(SZiterator = m_SZMap.begin(); SZiterator != m_SZMap.end(); SZiterator++)
		{
			tagSZRecord* pRec = SZiterator->second;
			if(pRec)
			{
				pRec->byScanTimes = byScanTimes;
				pRec->bNameChanged = false;
			}
		}
	}//设置上海MAP股票扫描的次数
	else if(byMapType == MAP_TYPE_SH)
	{
		map<int, tagSHRecord*>::iterator     SHiterator;

		for(SHiterator = m_SHMap.begin(); SHiterator != m_SHMap.end(); SHiterator++)
		{
			tagSHRecord* pRec = SHiterator->second;
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
	int nSZSize = (m_dbfSZ.GetRecordCount() - 1) *  
		( sizeof(tagNewPackInfo) + SZ_MAX_FIELD * sizeof(tagFieldInfo) );  //最多发送33个字段

	int nSHSize = (m_dbfSH.GetRecordCount() - 1) *
		( sizeof(tagNewPackInfo) + SH_MAX_FIELD * sizeof(tagFieldInfo) );  //最多发送28个字段

	int nSZXXSize = (m_dbfSZXX.GetRecordCount() - 1) *  
		( sizeof(tagNewPackInfo) + SZXX_MAX_FIELD * sizeof(tagFieldInfo) );  //最多发送19个字段

	nSZSize   += PACK_HEADER_LEN; //最多需要的内存数  8个字节的报文头
	nSHSize   += PACK_HEADER_LEN; //最多需要的内存数  8个字节的报文头
	nSZXXSize += PACK_HEADER_LEN; //最多需要的内存数  8个字节的报文头

	m_pSZSendBuf      = new char[nSZSize];
	if(!m_pSZSendBuf)
	{
		REPORT_Ex("Allocate memory failed for SZ SendBuf\n", RPT_ERROR);
		return FALSE;
	}
	m_nSZSendBufSize  = nSZSize;

	m_pSHSendBuf      = new char[nSHSize];
	if(!m_pSHSendBuf)
	{
		REPORT_Ex("Allocate memory failed for SH SendBuf\n", RPT_ERROR);
		return FALSE;
	}
	m_nSHSendBufSize  = nSHSize;

	m_pSZXXSendBuf    = new char[nSZXXSize];
	if(!m_pSZXXSendBuf)
	{
		REPORT_Ex("Allocate memory failed for SZXX SendBuf\n", RPT_ERROR);
		return FALSE;
	}
	m_nSZXXSendBufSize  = nSZXXSize;

	//
	return TRUE;
}

void CScanThread::Run()
{
	Sleep(5000);  //等QuoteFarm连接后，再发送数据，避免数据丢失

	//读取数据
	if(!m_dbfSZ.Requery())
	{
		REPORT_Ex("Read SZ dbf record failed!\n", RPT_ERROR);
		return;
	}

	if(!m_dbfSH.Requery())
	{
		REPORT_Ex("Read SH dbf record failed!\n", RPT_ERROR);
		return;
	}

	if(!m_dbfSZXX.Requery())
	{
		REPORT_Ex("Read SZXX dbf record failed!\n", RPT_ERROR);
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
		//设置所有股票的扫描次数为0，查找时如果待查找的股票的扫描次数为1，则把股票改为普通状态，
		//扫描次数为0 则是已经删除的股票
		SetStockType(0, MAP_TYPE_SZ);  

		BOOL bRet = ScanSZDBF();  //扫描深圳行情库
		if(!m_bRun)
			break;

		bRet &= ScanSZXXDBF();   //扫描深圳信息库
		if(!m_bRun)
			break;

		if (bRet)
			ScanSZDeleteRecord(); //扫描已删除的记录
		if(!m_bRun)
			break;	
		//******************************************************************
		SetStockType(0, MAP_TYPE_SH);  

		bRet = ScanSHDBF();    //扫描上海行情库
		if(!m_bRun)
			break;

		if (bRet)
			ScanSHDeleteRecord(); //扫描已删除的记录
		if(!m_bRun)
			break;

		//判断是否超时
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

void CScanThread::ScanSZDeleteRecord()
{
	//重新开始扫描前清零
	memset(m_pSZSendBuf, 0, m_nSZSendBufSize);
	m_nSZPackSize      = 0;
	m_pSZCurSendBufPos = m_pSZSendBuf;

	//	m_packHdr.wPackNum = m_wPackNum;
	m_pSZCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	//****************************************************************
	map<int, tagSZRecord*>::iterator     SZiterator;
	for(SZiterator = m_SZMap.begin(); SZiterator != m_SZMap.end();)
	{
		tagSZRecord* pRec = SZiterator->second;
		if(pRec == NULL)
		{
			SZiterator++;
			continue;
		}
		if(pRec->byScanTimes != 0 && !pRec->bNameChanged) //扫描次数为0或股票名称变化标记为删除
		{
			SZiterator++;
			continue;
		}
		//STOCK_TYPE_DELETE 已删除的股票

		//股票信息（信息头）
		tagPackInfo packInfo;
		packInfo.wStockType  = STOCK_TYPE_DELETE;
		packInfo.wTime       = m_wSZMinute;
		packInfo.wFieldCount = 0;

		if(SZiterator->first >= SZ_INDEX_CODE_MIN)
			strcpy_s(packInfo.szMarketType, 6, "SZ");
		else
			strcpy_s(packInfo.szMarketType, 6, "sz");

		if (pRec->pHQRecBuf != NULL)												 //注意
			m_dbfSZ.GetFieldValue("Hqzqdm", pRec->pHQRecBuf, packInfo.szStockCode);
		else if (pRec->pXXRecBuf != NULL)
			m_dbfSZXX.GetFieldValue("Xxzqdm", pRec->pXXRecBuf, packInfo.szStockCode);

		//内存大小检测*****************************************
		CheckSZSendBufForSend();
		//*****************************************************

		memcpy(m_pSZCurSendBufPos, &packInfo, sizeof(tagPackInfo));
		m_nSZPackSize += sizeof(tagPackInfo);
		m_pSZCurSendBufPos += sizeof(tagPackInfo);	

		//从MAP中删除
		if (pRec->pHQRecBuf != NULL)
		{
			delete[] pRec->pHQRecBuf;
			pRec->pHQRecBuf = NULL;
		}

		if (pRec->pXXRecBuf != NULL)
		{
			delete[] pRec->pXXRecBuf;
			pRec->pXXRecBuf = NULL;
		}

		//所有的库中都没有这条记录时才删除
		if (pRec != NULL)
		{
			delete pRec;
			pRec = NULL;

			m_SZMap.erase(SZiterator++);
		}
	}
	//********************************************************
	m_packHdr.ulPackSize = m_nSZPackSize;
	memcpy(m_pSZSendBuf, &m_packHdr, sizeof(tagPackHeader));

	if(m_nSZPackSize > 0)
	{
		//Send   m_pSZSendBuf
		int nLen = m_nSZPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SZSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSZSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSZSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSZSendBuf+PACK_HEADER_LEN, m_nSZPackSize);

			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头
		}
	}
}


void CScanThread::ScanSHDeleteRecord()
{
	//重新开始扫描前清零
	memset(m_pSHSendBuf, 0, m_nSHSendBufSize);
	m_nSHPackSize      = 0;
	m_pSHCurSendBufPos = m_pSHSendBuf;

	tagPackHeader packHdr;
	//	packHdr.wPackNum = m_wPackNum;
	m_pSHCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	//****************************************************************
	map<int, tagSHRecord*>::iterator     SHiterator;

	for(SHiterator = m_SHMap.begin(); SHiterator != m_SHMap.end();)
	{
		tagSHRecord* pRec = SHiterator->second;
		if(!pRec)
		{
			SHiterator++;
			continue;
		}
		if(pRec->byScanTimes != 0 && !pRec->bNameChanged) //扫描次数为0或股票名称变化标记为删除
		{
			SHiterator++;
			continue;
		}
		//STOCK_TYPE_DELETE 已删除的股票

		//股票信息（信息头）
		tagPackInfo packInfo;
		packInfo.wStockType  = STOCK_TYPE_DELETE;
		packInfo.wTime       = m_wSHMinute;
		packInfo.wFieldCount = 0;

		if(SHiterator->first < SH_INDEX_CODE_MAX)
			strcpy_s(packInfo.szMarketType, 6, "SH");
		else
			strcpy_s(packInfo.szMarketType, 6, "sh");

		if (pRec->pHQRecBuf != NULL)
			m_dbfSH.GetFieldValue("S1", pRec->pHQRecBuf, packInfo.szStockCode);

		//内存大小检测*****************************************
		CheckSHSendBufForSend();
		//*****************************************************

		memcpy(m_pSHCurSendBufPos, &packInfo, sizeof(tagPackInfo));
		m_nSHPackSize += sizeof(tagPackInfo);
		m_pSHCurSendBufPos += sizeof(tagPackInfo);	

		//从MAP中删除
		if(pRec->pHQRecBuf != NULL)
		{
			delete[] pRec->pHQRecBuf;
			pRec->pHQRecBuf = NULL;
		}

		if (pRec != NULL)
		{
			delete pRec;
			pRec = NULL;
			m_SHMap.erase(SHiterator++);
		}
	}

	//********************************************************
	packHdr.ulPackSize = m_nSHPackSize;
	memcpy(m_pSHSendBuf, &packHdr, sizeof(tagPackHeader));

	if(m_nSHPackSize > 0)
	{
		//Send   m_pSHSendBuf
		int nLen = m_nSHPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SHSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSHSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSHSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSHSendBuf+PACK_HEADER_LEN, m_nSHPackSize);
			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头
		}
	}
}

void CScanThread::CheckSZSendBufForSend()
{
	int nNeedSize = PACK_HEADER_LEN + m_nSZPackSize + sizeof(tagNewPackInfo) + 
		sizeof(tagFieldInfo) * SZ_MAX_FIELD; //增加一条记录的缓冲区

	if(nNeedSize > m_nSZSendBufSize)
	{
		m_packHdr.ulPackSize = m_nSZPackSize;
		memcpy(m_pSZSendBuf, &m_packHdr, sizeof(tagPackHeader));

		//Send   m_pSZSendBuf
		int nLen = m_nSZPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SZSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSZSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSZSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSZSendBuf+PACK_HEADER_LEN, m_nSZPackSize);

			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头
		}
		memset(m_pSZSendBuf, 0, m_nSZSendBufSize);
		m_nSZPackSize      = 0;
		m_pSZCurSendBufPos = m_pSZSendBuf;

		m_pSZCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	}
}

void CScanThread::CheckSZXXSendBufForSend()
{
	int nNeedSize = PACK_HEADER_LEN + m_nSZXXPackSize + sizeof(tagNewPackInfo) + 
		sizeof(tagFieldInfo) * SZXX_MAX_FIELD; //增加一条记录的缓冲区

	if(nNeedSize > m_nSZXXSendBufSize)
	{
		m_packHdr.ulPackSize = m_nSZXXPackSize;
		memcpy(m_pSZXXSendBuf, &m_packHdr, sizeof(tagPackHeader));

		//Send   m_pSZXXSendBuf
		int nLen = m_nSZXXPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SZXXSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSZXXSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSZXXSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSZXXSendBuf+PACK_HEADER_LEN, m_nSZXXPackSize);

			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头
		}

		memset(m_pSZXXSendBuf, 0, m_nSZXXSendBufSize);
		m_nSZXXPackSize      = 0;
		m_pSZXXCurSendBufPos = m_pSZXXSendBuf;

		m_pSZXXCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	}
}

void CScanThread::CheckSHSendBufForSend()
{
	int nNeedSize = PACK_HEADER_LEN + m_nSHPackSize + sizeof(tagNewPackInfo) + 
		sizeof(tagFieldInfo) * SH_MAX_FIELD; //增加一条记录的缓冲区

	if(nNeedSize > m_nSHSendBufSize)
	{
		m_packHdr.ulPackSize = m_nSHPackSize;
		memcpy(m_pSHSendBuf, &m_packHdr, sizeof(tagPackHeader));

		//Send   m_pSHSendBuf
		int nLen = m_nSHPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SHSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSHSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSHSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSHSendBuf+PACK_HEADER_LEN, m_nSHPackSize);

			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头

		}

		memset(m_pSHSendBuf, 0, m_nSHSendBufSize);
		m_nSHPackSize      = 0;
		m_pSHCurSendBufPos = m_pSHSendBuf;

		m_pSHCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	}
}


BOOL CScanThread::ScanSZDBF()
{ 
	//重新开始扫描前清零
	memset(m_pSZSendBuf, 0, m_nSZSendBufSize);
	m_nSZPackSize      = 0;
	m_pSZCurSendBufPos = m_pSZSendBuf;

	m_pSZCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	//****************************************************************
	if (!m_dbfSZ.Requery())         //读取最新的数据到缓冲区
	{
		REPORT_Ex("Requery() failed\n", RPT_ERROR);
		return FALSE;
	}
	m_dbfSZ.MoveFirst();	   //移动到第一条记录

	//获取时间以秒为单位
	int nTime = 0;
	m_dbfSZ.GetFieldValue("Hqcjbs", nTime);
	unsigned short wMinute = TimeToMinute(nTime);
	if (wMinute > m_wSZMinute)   //获得最新的时间
		m_wSZMinute = wMinute;

	m_dbfSZ.MoveNext();       //移动指针到第二条数据
	int nCode = 0;
	char szCode[50];
	while(!m_dbfSZ.IsEOF())
	{
		//检测股票代码的有效性
		memset(szCode, 0, 50);
		m_dbfSZ.GetFieldValue("Hqzqdm", szCode);
		if (!IsValidStockCode(szCode))
		{
			REPORT_Ex((T("错误的股票代码:%s，请检测dbf\n", szCode)).c_str(), RPT_CRITICAL);
			return FALSE;
		}
		//end

		m_dbfSZ.GetFieldValue("Hqzqdm", nCode);
		m_SZiterator = m_SZMap.find(nCode);

		BOOL bIndex = FALSE;
		if(nCode >= SZ_INDEX_CODE_MIN)
			bIndex = TRUE;
		if( m_SZiterator == m_SZMap.end() )//不存在
		{
			SZStockNotExist(wMinute, nCode, bIndex);
		}
		else
		{
			tagSZRecord* pRec = m_SZiterator->second;
			if (pRec->pHQRecBuf == NULL)  //股票代码存在，但行情数据不存在
			{
				SZStockNotExist(wMinute, nCode, bIndex, TRUE);
			}
			else
			{
				SZStockExist(wMinute, bIndex);
			}
		}

		m_dbfSZ.MoveNext();
	}

	//查找被删除的股票
	//********************************************************
	m_packHdr.ulPackSize = m_nSZPackSize;
	memcpy(m_pSZSendBuf, &m_packHdr, sizeof(tagPackHeader));

	if(m_nSZPackSize > 0)
	{
		//Send   m_pSZSendBuf
		int nLen = m_nSZPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SZSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSZSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSZSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSZSendBuf+PACK_HEADER_LEN, m_nSZPackSize);
			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头
		}
	}

	return TRUE;
}


BOOL CScanThread::ScanSZXXDBF()
{
	//重新开始扫描前清零
	memset(m_pSZXXSendBuf, 0, m_nSZXXSendBufSize);
	m_nSZXXPackSize      = 0;
	m_pSZXXCurSendBufPos = m_pSZXXSendBuf;

	m_pSZXXCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	//****************************************************************
	if (!m_dbfSZXX.Requery())         //读取最新的数据到缓冲区
	{
		REPORT_Ex("Requery() failed\n", RPT_ERROR);
		return FALSE;
	}
	m_dbfSZXX.MoveFirst();	   //移动到第一条记录

	//获取时间以秒为单位
	int nTime = 0;
	m_dbfSZXX.GetFieldValue("Xxbldw", nTime);
	unsigned short wMinute = TimeToMinute(nTime);
	if (wMinute > m_wSZMinute)   //获得最新的时间
		m_wSZMinute = wMinute;

	m_dbfSZXX.MoveNext();       //移动指针到第二条数据
	int nCode = 0;
	char szCode[50];
	while(!m_dbfSZXX.IsEOF())
	{
		//检测股票代码的有效性
		memset(szCode, 0, 50);
		m_dbfSZXX.GetFieldValue("Xxzqdm", szCode);
		if (!IsValidStockCode(szCode))
		{
			REPORT_Ex((T("错误的股票代码:%s，请检测dbf\n", szCode)).c_str(), RPT_CRITICAL);
			return FALSE;
		}
		//end

		m_dbfSZXX.GetFieldValue("Xxzqdm", nCode);
		m_SZiterator = m_SZMap.find(nCode);

		BOOL bIndex = FALSE;
		if(nCode >= SZ_INDEX_CODE_MIN)
			bIndex = TRUE;

		if( m_SZiterator == m_SZMap.end() )//不存在
		{
			SZXXStockNotExist(wMinute, nCode, bIndex);
		}
		else
		{
			tagSZRecord* pRec = m_SZiterator->second;
			if (pRec->pXXRecBuf == NULL)  //股票代码存在，tagSZRecord指针存在,但行情数据不存在
			{
				SZXXStockNotExist(wMinute, nCode, bIndex, TRUE);
			}
			else
			{
				SZXXStockExist(wMinute, bIndex);
			}
		}

		m_dbfSZXX.MoveNext();
	}

	//查找被删除的股票
	//********************************************************
	m_packHdr.ulPackSize = m_nSZXXPackSize;
	memcpy(m_pSZXXSendBuf, &m_packHdr, sizeof(tagPackHeader));

	if(m_nSZXXPackSize > 0)
	{
		//Send   m_pSZXXSendBuf
		int nLen = m_nSZXXPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SZXXSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSZXXSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSZXXSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSZXXSendBuf+PACK_HEADER_LEN, m_nSZXXPackSize);

			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头
		}
	}

	return TRUE;
}

BOOL CScanThread::ScanSHDBF()
{
	//重新开始扫描前清零
	memset(m_pSHSendBuf, 0, m_nSHSendBufSize);
	m_nSHPackSize      = 0;
	m_pSHCurSendBufPos = m_pSHSendBuf;

	tagPackHeader packHdr;
	m_pSHCurSendBufPos += sizeof(tagPackHeader); //给包头留出位置
	//****************************************************************
	if (!m_dbfSH.Requery())         //读取最新的数据到缓冲区
	{
		REPORT_Ex("Requery() failed\n", RPT_ERROR);
		return FALSE;
	}
	m_dbfSH.MoveFirst();	   //移动到第一条记录

	//获取时间以秒为单位
	int nTime = 0;
	m_dbfSH.GetFieldValue("S2", nTime);
	unsigned short wMinute = TimeToMinute(nTime);
	if (wMinute > m_wSHMinute)
		m_wSHMinute = wMinute;

	m_dbfSH.MoveNext();       //移动指针到第二条数据
	int nCode = 0;
	char szCode[50];
	while(!m_dbfSH.IsEOF())
	{
		//检测股票代码的有效性
		memset(szCode, 0, 50);
		m_dbfSH.GetFieldValue("S1", szCode);
		if (!IsValidStockCode(szCode))
		{
			REPORT_Ex((T("错误的股票代码:%s，请检测dbf\n", szCode)).c_str(), RPT_CRITICAL);
			return FALSE;
		}
		//end

		m_dbfSH.GetFieldValue("S1", nCode);
		m_SHiterator = m_SHMap.find(nCode);

		BOOL bIndex = FALSE;
		if(nCode <= SH_INDEX_CODE_MAX)
			bIndex = TRUE;

		if( m_SHiterator == m_SHMap.end() )//不存在
		{
			SHStockNotExist(wMinute, nCode, bIndex);
		}
		else
		{
			tagSHRecord* pRec = m_SHiterator->second;
			if (pRec->pHQRecBuf == NULL)  //股票代码存在，tagSZRecord指针存在,但行情数据不存在
			{
				SHStockNotExist(wMinute, nCode, bIndex, TRUE);
			}
			else
			{
				SHStockExist(wMinute, bIndex);
			}
		}  

		m_dbfSH.MoveNext();
	}

	//********************************************************
	packHdr.ulPackSize = m_nSHPackSize;
	memcpy(m_pSHSendBuf, &packHdr, sizeof(tagPackHeader));

	if(m_nSHPackSize > 0)
	{
		//Send   m_pSHSendBuf
		int nLen = m_nSHPackSize + PACK_HEADER_LEN + RESERVE_LEN + CRC_FIELD_LEN; //保留4个字节给主程序使用
		char* pSendBuf = (char*)mpnew(nLen);
		if (pSendBuf == NULL)
		{
			REPORT_Ex("Allocate memory mpnew failed for SHSendBuf\n", RPT_ERROR);
		}
		else
		{
			tagPackHeader* pHeader = (tagPackHeader*)m_pSHSendBuf;
			pHeader->ulPackSize += CRC_FIELD_LEN;

			memset(pSendBuf, 0, RESERVE_LEN);
			memcpy(pSendBuf+RESERVE_LEN, m_pSHSendBuf, PACK_HEADER_LEN);

			memcpy(pSendBuf+RESERVE_LEN+PACK_HEADER_LEN+CRC_FIELD_LEN, m_pSHSendBuf+PACK_HEADER_LEN, m_nSHPackSize);
			g_pOwner->Quote(pSendBuf, nLen - RESERVE_LEN, NULL);  //8个字节的报文头
		}
	}

	return TRUE;
}

void CScanThread::SHStockNotExist(unsigned short wMinute, int nCode, BOOL bIndex, BOOL bExist)
{
	if (bExist)  //股票代码存在，tagSZRecord指针存在，但行情数据不存在
	{
		tagSHRecord* pOldRec = m_SHiterator->second;
		int nRecordLen = m_dbfSH.GetRecordSize();

		pOldRec->byScanTimes++;
		pOldRec->pHQRecBuf = new char[nRecordLen];
		if (pOldRec->pHQRecBuf == NULL)
		{
			REPORT_Ex("Allocate memory failed for pRec->pRecordBuf, function return\n", RPT_ERROR);
			return;
		}

		m_dbfSH.GetRecordToBuffer(pOldRec->pHQRecBuf);
	}
	else
	{
		int nRecordLen = m_dbfSH.GetRecordSize();
		tagSHRecord* pRec = new tagSHRecord;
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

		m_dbfSH.GetRecordToBuffer(pRec->pHQRecBuf);
		m_SHMap.insert(SHRecord_Pair(nCode, pRec));  //放入MAP中 
	}
	//股票信息（信息头）
	tagNewPackInfo newPackInfo;

	if (bExist)
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NORMAL;
	else
		newPackInfo.packinfo.wStockType  = STOCK_TYPE_NEW;

	newPackInfo.packinfo.wTime       = wMinute;
	newPackInfo.packinfo.wFieldCount = SH_MAX_FIELD;  //28个字段需要发送(上海共28个字段)

	if(bIndex)
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "SH");
	else
		strcpy_s(newPackInfo.packinfo.szMarketType, 6, "sh");

	m_dbfSH.GetFieldValue("S1", newPackInfo.packinfo.szStockCode);

	if (!bExist)
	{
		m_dbfSH.GetFieldValue("S2", newPackInfo.szStockName);
		Trim(newPackInfo.szStockName);
	}

	//内存大小检测*****************************************
	CheckSHSendBufForSend();
	//*****************************************************

	if (bExist)	//去掉股票名称
	{
		memcpy(m_pSHCurSendBufPos, &newPackInfo, sizeof(tagPackInfo));
		m_nSHPackSize += sizeof(tagPackInfo);
		m_pSHCurSendBufPos += sizeof(tagPackInfo);
	}
	else
	{
		memcpy(m_pSHCurSendBufPos, &newPackInfo, sizeof(tagNewPackInfo));
		m_nSHPackSize += sizeof(tagNewPackInfo);
		m_pSHCurSendBufPos += sizeof(tagNewPackInfo);
	}

	//字段值
	FillValueToBuf(FD_S3, "S3", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S4, "S4", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S5, "S5", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S6, "S6", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S7, "S7", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S8, "S8", SH_HQ_DBF, bIndex);
	//新增2008-10-17
	FillValueToBuf(FD_S9, "S9", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S10, "S10", SH_HQ_DBF, bIndex);
	//
	FillValueToBuf(FD_S11, "S11", SH_HQ_DBF, bIndex);

	FillValueToBuf(FD_S13, "S13", SH_HQ_DBF, bIndex);

	FillValueToBuf(FD_S15, "S15", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S16, "S16", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S17, "S17", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S18, "S18", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S19, "S19", SH_HQ_DBF, bIndex);

	FillValueToBuf(FD_S21, "S21", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S22, "S22", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S23, "S23", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S24, "S24", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S25, "S25", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S26, "S26", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S27, "S27", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S28, "S28", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S29, "S29", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S30, "S30", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S31, "S31", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S32, "S32", SH_HQ_DBF, bIndex);
	FillValueToBuf(FD_S33, "S33", SH_HQ_DBF, bIndex); 
}

void CScanThread::SHStockExist(unsigned short wMinute, BOOL bIndex)
{
	tagSHRecord* pRec = m_SHiterator->second;
	pRec->byScanTimes++;

	//股票信息（信息头）
	tagPackInfo packInfo;
	packInfo.wStockType  = STOCK_TYPE_NORMAL;
	packInfo.wTime       = wMinute;
	//packInfo.wFieldCount//现在不能确定

	if(bIndex)
		strcpy_s(packInfo.szMarketType, 6, "SH");
	else
		strcpy_s(packInfo.szMarketType, 6, "sh");	

	m_dbfSH.GetFieldValue("S1", packInfo.szStockCode);

	//内存大小检测*****************************************
	CheckSHSendBufForSend();
	//*****************************************************

	//留出信息头的空间，如果没有变化的字段，移动指针到原始处m_pCurSendBufPos=pSavePackInfoBuf
	char* pSavePackInfoBuf = m_pSHCurSendBufPos;
	m_nSHPackSize += sizeof(tagPackInfo);
	m_pSHCurSendBufPos += sizeof(tagPackInfo);	

	//比较字段
	BOOL  bFieldChanged = FALSE;
	int   nFieldCount   = 0;

	pRec->bNameChanged = IsStockNameChanged(packInfo.szStockCode, pRec->pHQRecBuf, SH_HQ_DBF);
	//****************************************
	BOOL  bCjsl = FALSE;
	BOOL  bCjje = FALSE;

	bFieldChanged |= FillChangedValueToBuf(FD_S3, "S3", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S4, "S4", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);

	bCjje = FillChangedValueToBuf(FD_S5, "S5", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= bCjje;
	bFieldChanged |= FillChangedValueToBuf(FD_S6, "S6", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S7, "S7", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S8,  "S8", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	//新增2008-10-27
	bFieldChanged |= FillChangedValueToBuf(FD_S9,  "S9", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S10,  "S10", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	//
	bCjsl = FillChangedValueToBuf(FD_S11, "S11", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= bCjsl;
	bFieldChanged |= FillChangedValueToBuf(FD_S13, "S13", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S15, "S15", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S16, "S16", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S17, "S17", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S18, "S18", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S19, "S19", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S21, "S21", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S22, "S22", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S23, "S23", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S24, "S24", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S25, "S25", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S26, "S26", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S27, "S27", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S28, "S28", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S29, "S29", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);

	bFieldChanged |= FillChangedValueToBuf(FD_S30, "S30", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S31, "S31", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S32, "S32", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);
	bFieldChanged |= FillChangedValueToBuf(FD_S33, "S33", pRec->pHQRecBuf, nFieldCount, SH_HQ_DBF, bIndex);

	if (bCjsl && bCjje && !bIndex && !IsGuoZaiHuiGou(SH_HQ_DBF, packInfo.szStockCode)) //计算均价
	{					  		
		char  szField[30] = {0};
		int   nFieldSize = m_dbfSH.GetFieldValue("S11", szField);
// 		unsigned int nCjsl = CCharTranslate::VolumeToDigital100(szField, nFieldSize);
		unsigned int nCjsl = atoi(szField);

		nFieldSize = m_dbfSH.GetFieldValue("S5", szField);
// 		unsigned int nCjje = CCharTranslate::SumPriceToDigital1000(szField, nFieldSize);
		double fCjje = atof(szField);

		if (nCjsl != 0 && fCjje != 0)
		{
			nFieldCount++;
// 			unsigned int nAverage = (unsigned int)( ((double)nCjje / (double)nCjsl) * 10000 );  // 成交金额 / 成交数量 * 1000
			unsigned int nAverage = (unsigned int)(fCjje / nCjsl * 1000);

			tagFieldInfo* pFieldInfo = (tagFieldInfo*)m_pSHCurSendBufPos;
			pFieldInfo->wFieldType  = FD_AVERAGE;
			pFieldInfo->nFieldValue = nAverage;
			m_nSHPackSize          += sizeof(tagFieldInfo);
			m_pSHCurSendBufPos     += sizeof(tagFieldInfo);
		}
	}
	//****************************************

	if(bFieldChanged)
	{
		packInfo.wFieldCount = nFieldCount; //变化了的字段
		memcpy(pSavePackInfoBuf, &packInfo, sizeof(tagPackInfo));
		m_dbfSH.GetRecordToBuffer(pRec->pHQRecBuf); //把最新的数据存入MAP
	}
	else  //移动指针到原始处m_pCurSendBufPos=pSavePackInfoBuf
	{
		m_nSHPackSize     -= sizeof(tagPackInfo);
		m_pSHCurSendBufPos = pSavePackInfoBuf;
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

BOOL CScanThread::FindSHRecord(int nStockCode)
{
	return FALSE;
}

BOOL CScanThread::IsValidStockCode(char* pCode)
{
	for (int i=0; i<6; i++)  //上海和深圳的股票代码都是6位
	{
		if(pCode[i] < '0' || pCode[i] > '9')
			return FALSE;
	}

	return TRUE;
}

bool CScanThread::IsStockNameChanged(const char* pStockCode, const char* pOldRecord, unsigned int nDBFType)
{
	char* pNewField = NULL;

	if (nDBFType == SZ_HQ_DBF)
		pNewField = m_dbfSZ.CompFieldBuffer("HQZQJC", pOldRecord);
	else if (nDBFType == SZ_XX_DBF)
		pNewField = m_dbfSZXX.CompFieldBuffer("XXZQJC", pOldRecord);
	else if (nDBFType == SH_HQ_DBF)
		pNewField = m_dbfSH.CompFieldBuffer("S2", pOldRecord);

	if(pNewField != NULL)
	{
		char szName[9] = {0};
		memcpy(szName, pNewField, 8);
		REPORT_Ex((T("股票代码[%s]的名称变化为[%s]\n", pStockCode, szName)).c_str(), RPT_INFO);
		return true;
	}

	return false;
}