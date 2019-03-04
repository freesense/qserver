//DownHttpData.h
//

#ifndef _DOWN_HTTP_DATA_H_
#define _DOWN_HTTP_DATA_H_
#include <afxinet.h>
#include "HttpDataFile.h"
////////////////////////////////////////////////////////////////////////////////////////
class CDownHttpData
{
public:
	CDownHttpData();
	virtual ~CDownHttpData();
	BOOL OpenURL(LPCTSTR pstrURL, LPCTSTR pMktType);
	void Close();
	BOOL IsOpen(){return m_bIsOpen;}
	BOOL GetKLineData(tagKLineInfo* pInfo);

protected:
	CInternetSession  m_interSession;
	CHttpFile*        m_pHttpFile;           
	char*             m_pBuf;
	UINT              m_nBufSize;
private:
	DWORD StrDateToDWORD(CString strDate);
	UINT PriceToUINT(CString strPrice);

	BOOL              m_bIsOpen;
	tagKLineInfo	  m_info;
	CString           m_strStockCode;
	CHttpDataFile     m_httpDataFile;    //保留网上数据对象

	CString           m_strMarket;
};

///////////////////////////////////////////////////////////////////////////////////////
#endif