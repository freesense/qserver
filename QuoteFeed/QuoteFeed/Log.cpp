//Log.cpp
//
#include "stdafx.h"
#include "Log.h"
#include "config.h"
#include "../../public/commx/commxapi.h"
#include "../../public/CharTranslate.h"

///////////////////////////////////////
#define LOG_BUF_LEN      1000
#define FIELD_BUF_LEN    50
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

#define MT_SZ  0x01
#define MT_SH  0x02
#define MT_HK  0x03

char* pSHField[] =
{
	"     ",        //0
	"SH_S3_ZRSP",   //1
	"SH_S4_JRKP",   //2
	"SH_S8_ZJCJ",   //3
	"SH_S11_CJSL",  //4
	"SH_S5_CJJE",   //5
	"     ",   //6
	"SH_S6_ZGCJ",   //7
	"SH_S7_ZDCJ",   //8
	"SH_S13_SYL1",  //9
	"      ",  //10
	"      ",  //11
	"      ",  //12
	"      ",  //13
	"SH_S32_SJW5",  //14
	"SH_S33_SSL5",  //15
	"SH_S30_SJW4",  //16
	"SH_S31_SSL4",  //17
	"SH_S24_SJW3",  //18
	"SH_S25_SSL3",  //19
	"SH_S22_SJW2",  //20
	"SH_S23_SSL2",  //21
	"SH_S10_SJW1",  //22
	"SH_S21_SSL1",  //23
	"SH_S9_BJW1",  //24
	"SH_S15_BSL1",  //25
	"SH_S16_BJW2",  //26
	"SH_S17_BSL2",  //27
	"SH_S18_BJW3",  //28
	"SH_S19_BSL3",  //29
	"SH_S26_BJW4",  //30
	"SH_S27_BSL4",  //31
	"SH_S28_BJW5",  //32
	"SH_S29_BSL5",  //33
	
	
};
char* pSZField[] =
{
	"       ",	 //0
	"SZ_ZRSP",	 //1
	"SZ_JRKP",	 //2
	"SZ_ZJCJ",	 //3
	"SZ_CJSL",	 //4
	"SZ_CJJE",	 //5

	"SZ_CJBS",	 //6
	"SZ_ZGCJ",	 //7
	"SZ_ZDCJ",	 //8
	"SZ_SYL1",	 //9
	"SZ_SYL2",	 //10

	"SZ_JSD1",	 //11
	"SZ_JSD2",	 //12
	"SZ_HYCC",	 //13
	"SZ_SJW5",	 //14
	"SZ_SSL5",	 //15

	"SZ_SJW4",	 //16
	"SZ_SSL4",	 //17
	"SZ_SJW3",	 //18
	"SZ_SSL3",	 //19
	"SZ_SJW2",	 //20

	"SZ_SSL2",	 //21
	"SZ_SJW1",	 //22
	"SZ_SSL1",	 //23
	"SZ_BJW1",	 //24
	"SZ_BSL1",	 //25

	"SZ_BJW2",	 //26
	"SZ_BSL2",	 //27
	"SZ_BJW3",	 //28
	"SZ_BSL3",	 //29
	"SZ_BJW4",	 //30

	"SZ_BSL4",	 //31
	"SZ_BJW5",	 //32
	"SZ_BSL5",	 //33

	"XX_JYDW",	 //34
	"XX_MGMZ",	 //35
	"XX_ZFXL",	 //36
	"XX_LTGS",	 //37
	"XX_SNLR",	 //38

	"XX_BNLR",	 //39
	"XX_JSFL",	 //40
	"XX_YHSL",	 //41
	"XX_GHFL",	 //42
	"XX_MBXL",	 //43

	"XX_BLDW",	 //44
	"XX_SLDW",	 //45
	"XX_JGDW",	 //46
	"XX_JHCS",	 //47
	"XX_LXCS",	 //48

	"XX_XJXZ",	 //49
	"XX_ZTJG",	 //50
	"XX_DTJG",	 //51
	"XX_ZHBL",	 //52
	"SZ_AVERAGE",	 //53
};

CLog::CLog()
{
	m_pFile           = NULL;
	m_nOldDate        = 0;
}

CLog::~CLog()
{
	Close();
}

void CLog::Close()
{
	if(m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

BOOL CLog::IsOpen()
{
	if(m_pFile != NULL)
		return TRUE;
	else
		return FALSE;
}

void CLog::GetCurTime(char* pCurTime, int nBufLen)
{
	time_t  timeCur;
	time(&timeCur);

	tm tmCur;
	localtime_s(&tmCur, &timeCur);

	if(pCurTime != NULL && nBufLen > 0)
	{
		sprintf_s(pCurTime, nBufLen, "%04d-%02d-%02d %02d:%02d:%02d ", tmCur.tm_year+1900, tmCur.tm_mon+1, 
			tmCur.tm_mday, tmCur.tm_hour, tmCur.tm_min, tmCur.tm_sec);
	}
}

unsigned int CLog::GetCurrentDate(BOOL& bNewDate)
{
	time_t  timeCur;
	time(&timeCur);

	tm tmCur;
	localtime_s(&tmCur, &timeCur);

	unsigned int nCurDate = (tmCur.tm_year+1900)*10000 + (tmCur.tm_mon+1)*100 + tmCur.tm_mday;
	if(nCurDate != m_nOldDate)
	{
		m_nOldDate = nCurDate;
		bNewDate = TRUE;
	}

	return nCurDate;
}

BOOL CLog::Reopen()
{
	if (g_cfg.m_cRecordMode == '1')
		strcpy(m_chOpenMode, "a+b");
	else
		strcpy(m_chOpenMode, "a+");
	m_nShareFlag      = _SH_DENYNO;

	Close();

	char chLogName[MAX_PATH];
	std::string strExecutePath = GetExecutePath();
	strExecutePath = strExecutePath.substr(0, strExecutePath.rfind("\\") + 1);
	if (g_cfg.m_strRecordPath.empty()) //配置文件中没有设置路径，就和exe文件放在相同文件夹
	{
		sprintf_s(chLogName, MAX_PATH, "%srec%d.log", strExecutePath.c_str(), m_nOldDate);
	}
	else
	{
		std::string strRecLog = strExecutePath;
		strRecLog += g_cfg.m_strRecordPath;
		if (!IsPathExist(strRecLog.c_str())) //目录不存在则创建
		{
			if (!CreatePath(strRecLog.c_str()))
				return FALSE;
		}  

		sprintf_s(chLogName, MAX_PATH, "%s%s\\rec%d.log", strExecutePath.c_str(), 
			g_cfg.m_strRecordPath.c_str(), m_nOldDate);
	}

	m_pFile = _fsopen(chLogName, m_chOpenMode, m_nShareFlag);
	if(m_pFile == NULL)
		return FALSE;
	else
		return TRUE;
}

BOOL CLog::AddLog(const char *pData, int nLen)
{
	m_cMode = g_cfg.m_cRecordMode;

	BOOL bNewDate = FALSE;
	unsigned int nCurDate = GetCurrentDate(bNewDate);

	if(bNewDate)
	{
		if(!Reopen())
			return FALSE;
	}

	if (m_cMode == '1')
	{
		time_t  timeCur;
		time(&timeCur);
		tm tmCur;
		localtime_s(&tmCur, &timeCur);
		unsigned int nTime = tmCur.tm_hour*60*60 + tmCur.tm_min*60 + tmCur.tm_sec;
		fwrite(&nTime, sizeof(unsigned int), 1, m_pFile);
		fflush(m_pFile);
	}

	fwrite(pData, sizeof(char), nLen, m_pFile);
	fflush(m_pFile);
	return TRUE;
}

void CLog::ParseDataToLog(const char *pData, int nLength)
{
	m_cMode = g_cfg.m_cRecordMode;

	if (m_cMode == '0')
		return;

	if (m_cMode == '1')
		AddLog(pData, nLength);
	else if (m_cMode == '2')
	{
		const char* pCurData = pData + sizeof(long);
		nLength -= sizeof(long);
		tagPackHeader packHdr;
		memcpy(&packHdr, pCurData, sizeof(tagPackHeader));

		char szTime[100];
		GetCurTime(szTime, 100);
		AddLog(szTime, (int)strlen(szTime));

		char szPackMark[100];
		sprintf_s(szPackMark, 100, " pack size:%d ######################################\n\n", packHdr.ulPackSize);
		AddLog(szPackMark, (int)strlen(szPackMark));

#ifdef VERIFY_DATA
		//REPORT(MN, T("PackSize:%d nLength:%d\n", packHdr.ulPackSize, nLength), RPT_INFO);
		if(nLength - PACK_HEADER_LEN != packHdr.ulPackSize)
		{
			REPORT(MN, T("pack size is invalid, correct size:%d  real size:%d\n",
				packHdr.ulPackSize, nLength), RPT_ERROR);
		}
#endif

		pCurData += sizeof(tagPackHeader);
		pCurData += sizeof(unsigned long);

		tagNewPackInfo newPackInfo;
		memcpy(&newPackInfo, pCurData, sizeof(newPackInfo));

		tagFieldInfo finfo;

		if(newPackInfo.packinfo.wStockType == STOCK_TYPE_NEW)
			memcpy(&finfo, pCurData+sizeof(tagNewPackInfo), sizeof(tagFieldInfo));
		else if(newPackInfo.packinfo.wStockType == STOCK_TYPE_NORMAL)
			memcpy(&finfo, pCurData+sizeof(tagPackInfo), sizeof(tagFieldInfo));
		else if(newPackInfo.packinfo.wStockType == STOCK_TYPE_DELETE)
			memcpy(&finfo, pCurData+sizeof(tagPackInfo), sizeof(tagFieldInfo));
#ifdef VERIFY_DATA
		else
			REPORT(MN, T("invalid stock type: %d\n", newPackInfo.packinfo.wStockType),
			RPT_ERROR);
#endif

		BOOL bSZXX = FALSE;
		if(finfo.wFieldType > SZ_MAX_FIELD)
			bSZXX = TRUE;

#ifdef VERIFY_DATA
		CheckPackHdr(&newPackInfo, bSZXX);
#endif

		char szLog[LOG_BUF_LEN];
		char szField[FIELD_BUF_LEN];

		while(pCurData < pData+nLength) 
		{
			memcpy(&newPackInfo, pCurData, sizeof(newPackInfo));
			UINT nMarketType = 0;
			if(_stricmp(newPackInfo.packinfo.szMarketType, "SZ") == 0)
				nMarketType = MT_SZ;
			else if (_stricmp(newPackInfo.packinfo.szMarketType, "SH") == 0)
				nMarketType = MT_SH;
			else if (_stricmp(newPackInfo.packinfo.szMarketType, "HK") == 0)
				nMarketType = MT_HK;

			switch(newPackInfo.packinfo.wStockType)
			{
			case STOCK_TYPE_NEW:
				{
					memset(szLog, 0, LOG_BUF_LEN);
					strcpy_s(szLog, LOG_BUF_LEN, "New: ");
					strcat_s(szLog, LOG_BUF_LEN, newPackInfo.packinfo.szMarketType);
					strcat_s(szLog, LOG_BUF_LEN, " ");

					strcat_s(szLog, LOG_BUF_LEN, newPackInfo.packinfo.szStockCode);
					strcat_s(szLog, LOG_BUF_LEN, " ");
					strcat_s(szLog, LOG_BUF_LEN, newPackInfo.szStockName);

					pCurData += sizeof(tagNewPackInfo);
					for(int i=0; i<newPackInfo.packinfo.wFieldCount; i++)
					{
						tagFieldInfo fieldInfo;
						memcpy(&fieldInfo, pCurData, sizeof(tagFieldInfo));


						strcat_s(szLog, LOG_BUF_LEN, " ");
						if(MT_SZ == nMarketType)
							strcat_s(szLog, LOG_BUF_LEN, pSZField[fieldInfo.wFieldType]);
						else
							strcat_s(szLog, LOG_BUF_LEN, pSHField[fieldInfo.wFieldType]);

						sprintf_s(szField, FIELD_BUF_LEN, ":%d", fieldInfo.nFieldValue);
						strcat_s(szLog, LOG_BUF_LEN, szField);

#ifdef VERIFY_DATA
						CheckFieldInfo(&fieldInfo, newPackInfo.packinfo.szStockCode, nMarketType);
#endif
						pCurData += sizeof(tagFieldInfo);
					}

					strcat_s(szLog, LOG_BUF_LEN, "\n\n");
					AddLog(szLog, (int)strlen(szLog));

					break;
				}
			case STOCK_TYPE_NORMAL:
				{

					memset(szLog, 0, LOG_BUF_LEN);
					strcpy_s(szLog, LOG_BUF_LEN, "Nor: ");

					strcat_s(szLog, LOG_BUF_LEN, newPackInfo.packinfo.szMarketType);
					strcat_s(szLog, LOG_BUF_LEN, " ");

					strcat_s(szLog, LOG_BUF_LEN, newPackInfo.packinfo.szStockCode);

					pCurData += sizeof(tagPackInfo);
					for(int i=0; i<newPackInfo.packinfo.wFieldCount; i++)
					{
						tagFieldInfo fieldInfo;
						memcpy(&fieldInfo, pCurData, sizeof(tagFieldInfo));


						strcat_s(szLog, LOG_BUF_LEN, " ");
						if(MT_SZ == nMarketType)
							strcat_s(szLog, LOG_BUF_LEN, pSZField[fieldInfo.wFieldType]);
						else
						{
							if (fieldInfo.wFieldType == 53)
								strcat_s(szLog, LOG_BUF_LEN, "SH_AVERAGE");
							else
								strcat_s(szLog, LOG_BUF_LEN, pSHField[fieldInfo.wFieldType]);
						}

						sprintf_s(szField, FIELD_BUF_LEN, ":%d", fieldInfo.nFieldValue);
						strcat_s(szLog, LOG_BUF_LEN, szField);

#ifdef VERIFY_DATA
						CheckFieldInfo(&fieldInfo, newPackInfo.packinfo.szStockCode, nMarketType);
#endif
						pCurData += sizeof(tagFieldInfo);
					}


					strcat_s(szLog, LOG_BUF_LEN, "\n\n");
					AddLog(szLog, (int)strlen(szLog));

					break;
				}
			case STOCK_TYPE_DELETE:
				{

					memset(szLog, 0, LOG_BUF_LEN);
					strcpy_s(szLog, LOG_BUF_LEN, "Del: ");
					strcat_s(szLog, LOG_BUF_LEN, newPackInfo.packinfo.szMarketType);
					strcat_s(szLog, LOG_BUF_LEN, " ");

					strcat_s(szLog, LOG_BUF_LEN, newPackInfo.packinfo.szStockCode);

					pCurData += sizeof(tagPackInfo);


					strcat_s(szLog, LOG_BUF_LEN, "\n\n");
					AddLog(szLog, (int)strlen(szLog));

					break;
				}
			}
		}
	}
}

BOOL CLog::IsPathExist(LPCTSTR lpPath)
{
	if(lpPath == NULL)
		return FALSE;

	char szPath[MAX_PATH];
	strcpy_s(szPath, MAX_PATH, lpPath);
	char* pPos = szPath;
	for(int i=0; i<(int)strlen(szPath); i++)
	{
		if (*pPos == '/')
			*pPos = '\\';
		pPos++;
	}
	DWORD dwAttr = ::GetFileAttributes(szPath);

	return (dwAttr!=-1 && (dwAttr&FILE_ATTRIBUTE_DIRECTORY) );
}

BOOL CLog::CreatePath(LPCTSTR lpPath)
{
	if(lpPath==NULL || strlen(lpPath)==0)
		return FALSE;
	
	// base case . . .if directory exists
	if(IsPathExist(lpPath))
		return TRUE;
	
	char szPath[MAX_PATH];
	strcpy_s(szPath, MAX_PATH, lpPath);

	int nLen = (int)strlen(szPath);
	if(szPath[nLen-1] == '\\')
		szPath[nLen-1] = 0; 

	 // recursive call, one less directory
	char szSubPath[MAX_PATH];
	char *pRet = strrchr(szPath, '\\');
	pRet[0] = 0;
	strcpy(szSubPath, szPath);
	pRet[0] = '\\';

	if(! CreatePath(szSubPath) )
		return FALSE;

	return ::CreateDirectory(szPath,NULL);
}

#ifdef VERIFY_DATA
void CLog::CheckPackHdr(tagNewPackInfo* pNewPackInfo, BOOL bSZXX)
{
	if(_stricmp(pNewPackInfo->packinfo.szMarketType, "SZ") != 0 && 
		_stricmp(pNewPackInfo->packinfo.szMarketType, "SH") != 0 && 
		_stricmp(pNewPackInfo->packinfo.szMarketType, "HK") != 0)
	{
		REPORT(MN, T("szMarketType:%d is invalid\n", pNewPackInfo->packinfo.szMarketType),
			RPT_ERROR);
	}

	switch(pNewPackInfo->packinfo.wStockType)
	{
	case STOCK_TYPE_NORMAL:
		{
			//wFieldCount
			if(_stricmp(pNewPackInfo->packinfo.szMarketType, "SZ") == 0)
			{
				if(pNewPackInfo->packinfo.wFieldCount > SZ_MAX_FIELD
					|| pNewPackInfo->packinfo.wFieldCount <= 0)
				{
					REPORT(MN, T("wFieldCount is invalid, wFieldCount:%d\n", 
						pNewPackInfo->packinfo.wFieldCount), RPT_ERROR);
				}
			}

			if(_stricmp(pNewPackInfo->packinfo.szMarketType, "SH") == 0 ||
			   _stricmp(pNewPackInfo->packinfo.szMarketType, "HK") == 0)
			{
				if(pNewPackInfo->packinfo.wFieldCount > SH_MAX_FIELD
					|| pNewPackInfo->packinfo.wFieldCount <= 0)
				{
					REPORT(MN, T("wFieldCount is invalid, wFieldCount:%d\n", 
						pNewPackInfo->packinfo.wFieldCount), RPT_ERROR);
				}
			}
			break;
		}
	case STOCK_TYPE_NEW:
		{
			//wFieldCount
			if(_stricmp(pNewPackInfo->packinfo.szMarketType, "SZ") == 0)
			{
				if(pNewPackInfo->packinfo.wFieldCount != SZ_MAX_FIELD
					&& pNewPackInfo->packinfo.wFieldCount != SZXX_MAX_FIELD)
				{
					REPORT(MN, T("wFieldCount is invalid, wFieldCount:%d\n", 
						pNewPackInfo->packinfo.wFieldCount), RPT_ERROR);
				}
			}

			if(_stricmp(pNewPackInfo->packinfo.szMarketType, "SH") == 0 ||
				_stricmp(pNewPackInfo->packinfo.szMarketType, "HK") == 0)
			{
				if(pNewPackInfo->packinfo.wFieldCount != SH_MAX_FIELD)
				{
					REPORT(MN, T("wFieldCount is invalid, wFieldCount:%d\n", 
						pNewPackInfo->packinfo.wFieldCount), RPT_ERROR);
				}
			}
			break;
		}
	case STOCK_TYPE_DELETE:
		{
			//wFieldCount
			if(pNewPackInfo->packinfo.wFieldCount != 0)
			{
				REPORT(MN, T("wFieldCount is invalid, wFieldCount:%d\n", 
						pNewPackInfo->packinfo.wFieldCount), RPT_ERROR);
			}
			break;
		}
	default:
		{
			REPORT(MN, T("unknown stock type, stock type:%d\n", 
						pNewPackInfo->packinfo.wStockType), RPT_ERROR);
		}
	}
}

void CLog::CheckFieldInfo(tagFieldInfo* pFieldInfo, char* pStockCode, UINT nMarketType)
{
	unsigned int nStockCode = CCharTranslate::VolumeToDigital(pStockCode, (int)strlen(pStockCode));
	BOOL bIndex = FALSE;
	if(nMarketType == MT_SZ)
	{
		char szPrefixCode[3];
		memcpy(szPrefixCode, pStockCode, 2);
		szPrefixCode[2] = 0;
		if(_stricmp(szPrefixCode, "16") == 0) //特殊情况，价格超过20万
			return;
		
		if(nStockCode >= 300000)
			bIndex = TRUE;

		switch(pFieldInfo->wFieldType)
		{
		case FD_ZRSP: case FD_JRKP: case FD_ZJCJ: case FD_ZGCJ: case FD_ZDCJ:
		case FD_SJW5:  case FD_SJW4: case FD_SJW3:   case FD_SJW2:   case FD_SJW1:
		case FD_BJW1:  case FD_BJW2:  case FD_BJW3:	 case FD_BJW4:	case FD_BJW5:
			{
				if(pFieldInfo->wFieldType == FD_ZDCJ && pFieldInfo->nFieldValue == 99999999 )
						break;
				if(!bIndex)
				{
					//非指数价格不大于20万，不小于100

					if( (pFieldInfo->nFieldValue > 200000	|| pFieldInfo->nFieldValue < 100) && 
						pFieldInfo->nFieldValue != 0 )
					{
						REPORT(MN, T("SZ price is invalid, StockCode:%s, wFieldType:%d FieldValue:%d\n", 
							pStockCode, pFieldInfo->wFieldType, pFieldInfo->nFieldValue),RPT_ERROR);
					}
				}
				else
				{
					//指数价格不大于1000万，不小于100
					if( (pFieldInfo->nFieldValue > 10000000	|| pFieldInfo->nFieldValue < 100) &&
						pFieldInfo->nFieldValue != 0 )
					{
						REPORT(MN, T("SZ price is invalid, StockCode:%s, wFieldType:%d FieldValue:%d\n", 
							pStockCode, pFieldInfo->wFieldType, pFieldInfo->nFieldValue),RPT_ERROR);
					}
				}
			}
		default:
			return;
		}
	}
	else if(nMarketType == MT_SH)
	{
		if(nStockCode < 2000)
			bIndex = TRUE;

		if(pFieldInfo->wFieldType == FD_S6 && pFieldInfo->nFieldValue == 999000)
			return;

		switch(pFieldInfo->wFieldType)
		{
		case FD_S3:  case FD_S4:  case FD_S6:  case FD_S7:  case FD_S8: 
	    case FD_S13: case FD_S16: case FD_S18:	case FD_S22: case FD_S24:
		case FD_S26: case FD_S28: case FD_S30: case FD_S32:
			{
				if(!bIndex)
				{
					//非指数价格不大于40万
				}
				else
				{
				}
			}
		default:
			return;
		}
	}
	else if(nMarketType == MT_HK)
	{
		if (_strcmpi(pStockCode, "HSI")    == 0 ||
			_strcmpi(pStockCode, "HSCI")   == 0 ||
			_strcmpi(pStockCode, "HSCCI")  == 0 ||
			_strcmpi(pStockCode, "HSCFI")  == 0 ||
			_strcmpi(pStockCode, "HSHKCI") == 0 ||
			_strcmpi(pStockCode, "HSIFIN") == 0 ||
			_strcmpi(pStockCode, "HSIUTL") == 0 ||
			_strcmpi(pStockCode, "HSIPRO") == 0 ||
			_strcmpi(pStockCode, "HSICI")  == 0 ||
			_strcmpi(pStockCode, "HSMLCI") == 0 ||
			_strcmpi(pStockCode, "GEM")    == 0 ||
			_strcmpi(pStockCode, "HKL")    == 0 )
		{
			bIndex = TRUE;
		}

		switch(pFieldInfo->wFieldType)
		{
		case FD_S3:  case FD_S4:  case FD_S6:  case FD_S7:  case FD_S8: 
	    case FD_S13: case FD_S16: case FD_S18:	case FD_S22: case FD_S24:
		case FD_S26: case FD_S28: case FD_S30: case FD_S32:
			{
				if(!bIndex)
				{
				}
				else
				{
				}
			}
		default:
			return;
		}
	}
}
#endif