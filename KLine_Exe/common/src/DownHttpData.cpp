//DownHttpData.cpp
//

#include "stdafx.h"
#include "DownHttpData.h"
#include "KLine_Exe.h"
////////////////////////////////////////////////////////////////////////////////////

CDownHttpData::CDownHttpData()
{
	m_pHttpFile = NULL;	
	m_bIsOpen   = FALSE;
	memset(&m_info, 0, sizeof(tagKLineInfo));
	m_pBuf     = NULL;
	m_nBufSize = 0;
}

CDownHttpData::~CDownHttpData()
{
	if (m_pBuf != NULL)
	{
		delete[] m_pBuf;
	}
	Close();
	m_httpDataFile.Close();
}

BOOL CDownHttpData::OpenURL(LPCTSTR pstrURL, LPCTSTR pMktType)
{
	m_strMarket = pMktType;
	m_strStockCode = pstrURL;
	m_strStockCode = m_strStockCode.Right(m_strStockCode.GetLength() - 
		m_strStockCode.ReverseFind('=') - 3);

	if (m_pHttpFile != NULL)
	{
		m_pHttpFile->Close();
		delete   m_pHttpFile;
		m_pHttpFile = NULL;
	}
	
	try
	{
		m_pHttpFile = (CHttpFile*)m_interSession.OpenURL(pstrURL);
	}
	catch(CInternetException* pInterException)
	{
		char ErrMsg[100];
		pInterException->GetErrorMessage(ErrMsg, 100);
		REPORT(MN, T("异常：%s\n", ErrMsg), RPT_ERROR);
		pInterException->Delete();
		return FALSE;
	}

	if (m_pHttpFile == NULL)
	{
		return FALSE;
	}
	else
	{
		m_bIsOpen = TRUE;
		if(m_pBuf == NULL)
		{
			m_nBufSize = 1024*50;
			m_pBuf = new char[m_nBufSize]; //50k
			if (m_pBuf == NULL)
				return FALSE;
		}
		return TRUE;
	}
}

void CDownHttpData::Close()
{
	if (m_bIsOpen)
	{
		if (m_pHttpFile != NULL)
		{
			m_pHttpFile->Close();
			delete   m_pHttpFile;
			m_pHttpFile = NULL;
		}
		m_interSession.Close();
		m_bIsOpen   = FALSE;
	}
}

BOOL CDownHttpData::GetKLineData(tagKLineInfo* pInfo)
{
	if (!IsOpen())
		return FALSE;

	if (m_pHttpFile == NULL)
		return FALSE;

	int nReaded = 0;
	char* pBufPos = m_pBuf;
	BOOL bRead = TRUE;
	memset(pBufPos, 0, m_nBufSize);
	while(1)
	{
		int nRet = 0;
		if (nReaded+100 > (int)m_nBufSize)
		{
			char* pNewBuf = new char[m_nBufSize+1000];
			if (pNewBuf == NULL)
				return FALSE;
			memcpy(pNewBuf, m_pBuf, m_nBufSize);
			delete []m_pBuf;
			m_pBuf = pNewBuf;
			pBufPos = m_pBuf + nReaded;
			m_nBufSize += 1000; 
		}
		try
		{
			nRet = m_pHttpFile->Read(pBufPos, 100);
			if (nRet <= 0)
				break;
		}
		catch(CInternetException* pInterException)
		{
			char ErrMsg[200];
			pInterException->GetErrorMessage(ErrMsg, 200);
			REPORT(MN, T("异常：%s\n", ErrMsg), RPT_ERROR);
			pInterException->Delete();
			return FALSE;
		}
		
		pBufPos += nRet;
		nReaded += nRet;
		m_pBuf[nReaded] = 0;
	}
	
	//解析数据
	///////////////////////////////////////////////////
	tagHttpKFileInfo  httpKFInfo;
	BOOL bRet = TRUE;
	CString strStockCode;
	CString strBuf = m_pBuf;
	//日期
	strBuf = strBuf.Left(strBuf.ReverseFind(','));
	CString strDate = strBuf.Mid(strBuf.ReverseFind(',')+1, strBuf.GetLength() - strBuf.ReverseFind(',') - 1);
	httpKFInfo.nDate = StrDateToDWORD(strDate);
	if (httpKFInfo.nDate == -1)
		bRet = FALSE;
	else
		pInfo->nDate = httpKFInfo.nDate;

	//股票代码
	strBuf = strBuf.Right(strBuf.GetLength() - strBuf.Find("hq_str_") - (int)strlen("hq_str_") - 2);
	strStockCode = strBuf.Left(strBuf.Find("="));
	if (m_strStockCode.CompareNoCase(strStockCode) != 0)
		bRet = FALSE;

	strBuf = strBuf.Right(strBuf.GetLength() - strBuf.Find("\"") - 1);
	int nPos = strBuf.Find(",");
	int nBeginPos = 0;
	CString strTemp;
	int nIndex = 0;
	while(nPos != -1)
	{
		strTemp = strBuf.Mid(nBeginPos, nPos-nBeginPos);
		switch(nIndex)
		{
		case 1:	//开盘价
			{
				pInfo->nOpenPrice = PriceToUINT(strTemp);
				httpKFInfo.nOpenPrice = pInfo->nOpenPrice;
				break;
			}
		case 2:	//前收盘
			{
				httpKFInfo.nPreClosePrice = PriceToUINT(strTemp);
				break;
			}
		case 3:	//最新价
			{
				pInfo->nClosePrice = PriceToUINT(strTemp);
				httpKFInfo.nNewPrice = pInfo->nClosePrice;
				break;
			}
		case 4:	//最高价
			{
				pInfo->nMaxPrice = PriceToUINT(strTemp);
				httpKFInfo.nMaxPrice = pInfo->nMaxPrice;
				break;
			}
		case 5:	//最低价
			{
				pInfo->nMinPrice = PriceToUINT(strTemp);
				httpKFInfo.nMinPrice = pInfo->nMinPrice;
				break;
			}
		case 8:	//成交量
			{
				pInfo->nVolume = PriceToUINT(strTemp) / 100;
				httpKFInfo.nVolume = pInfo->nVolume;
				break;
			}
		case 9:	//成交金额
			{
				int nDotPos = strTemp.Find(".");
				if (nDotPos >= 0)
					strTemp = strTemp.Left(nDotPos);

				pInfo->nSum = PriceToUINT(strTemp) / 100;
				httpKFInfo.nSum = pInfo->nSum;
				break;
			}
		}

		nBeginPos = nPos+1;
		nPos = strBuf.Find(",", nBeginPos);
		nIndex++;
		if (nIndex > 9)
			break;
	}
	
	{//保存网页上的数据
		CString strHttpDataPath;
		CValidFunction::GetWorkPath(strHttpDataPath);
		strHttpDataPath += "\\HttpData";
		if (!CValidFunction::IsPathExist(strHttpDataPath))
			CValidFunction::CreatePath(strHttpDataPath);
		strHttpDataPath += "\\";
		strHttpDataPath += strStockCode;
		strHttpDataPath += "." + m_strMarket;
		m_httpDataFile.Open(strHttpDataPath, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS);

		REPORT(MN, T("保存网页上的数据到[%s]!\n", strHttpDataPath), RPT_INFO);
		m_httpDataFile.AddRecord(&httpKFInfo);
		m_httpDataFile.Close();
	}
	return bRet;
}

DWORD CDownHttpData::StrDateToDWORD(CString strDate)
{
	strDate.TrimLeft();
	int nPos = strDate.Find('-');
	if (nPos == -1)
		nPos = strDate.Find('/');

	int nRevPos = strDate.ReverseFind('-');
	if (nRevPos == -1)
		nRevPos = strDate.ReverseFind('/');

	if (nPos == -1 || nRevPos == -1)
		return -1;

	CString strYear = strDate.Left(4);
	CString strMon  = strDate.Mid(nPos+1, (nRevPos - nPos - 1));
	CString strDay  = strDate.Mid(nRevPos+1, 2);
	strDay.TrimRight();

	int nYear = atoi(strYear);
	int nMon  = atoi(strMon);
	int nDay  = atoi(strDay);
		
	return (nYear*10000 + nMon*100 + nDay);
}

UINT CDownHttpData::PriceToUINT(CString strPrice)
{
	char chPrice[50];
	strcpy_s(chPrice, 50, strPrice);
	int iLen = strPrice.GetLength();

	unsigned int	uiValue=0;
	int i=0;
	for(; i<iLen; i++)
		if(chPrice[i]!=' ')
			break;

	for(; i<iLen && chPrice[i]!=0 && chPrice[i]!=' '; i++)
	{
		if(chPrice[i]=='.')
		{
			int j=0;
			for(i++; i<iLen && chPrice[i]!=' ' && j<3; i++)
				if(chPrice[i] >= '0' && chPrice[i] <= '9')
				{
					uiValue = uiValue * 10 + chPrice[i] - '0';
					j++;
				}

				for(; j<3; j++)
					uiValue *= 10;
				break;
		}

		if(chPrice[i] >= '0' && chPrice[i] <= '9')
			uiValue = uiValue * 10 + chPrice[i] - '0';
	}

	return	uiValue;
}
