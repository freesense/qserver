//DataBaseEx.cpp
#include "stdafx.h"
#include "DataBaseEx.h"

extern void AddLog(const char *pData, int nLen);

////////////////////////////////////////////////////////////////////////////////
CDataBaseEx::CDataBaseEx()
{
	::CoInitialize(NULL);
	m_pCon  = NULL;
	m_pRset = NULL;
}

CDataBaseEx::~CDataBaseEx()
{
	::CoUninitialize();
}

BOOL CDataBaseEx::Open(string strSerIP, string strDBName, string strUserName, 
					   string strPassword)
{
	m_strSrvIP    = strSerIP;
	m_strDBName   = strDBName;
	m_strUserName = strUserName;
	m_strPassword = strPassword;
	try
	{
		m_pCon.CreateInstance(_uuidof(Connection));
		if(m_pCon == NULL)
		{
			REPORT(MN, T("m_pCon.CreateInstance failed code:%d\n", GetLastError()), RPT_ERROR);
			return FALSE;
		}
		
		char szCon[200];
		sprintf_s(szCon, 200, "driver={SQL Server};Server=%s;DATABASE=%s;UID=%s;PWD=%s",
			m_strSrvIP.c_str(), m_strDBName.c_str(), m_strUserName.c_str(), m_strPassword.c_str());

		m_pCon->CursorLocation=adUseClient;   //游标类型  
		HRESULT hr = m_pCon->Open((_bstr_t)szCon, "", "", adConnectUnspecified);
		if(FAILED(hr))
		{
			REPORT(MN, T("Open database[%s:%s] failed\n", m_strSrvIP.c_str(), m_strDBName.c_str()), RPT_ERROR);
			return FALSE;
		}
	}
	catch(_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}

	REPORT(MN, T("Open database[%s:%s] successed\n", m_strSrvIP.c_str(), m_strDBName.c_str()), RPT_INFO);
	return TRUE;
}

void CDataBaseEx::Close()
{
	try
	{
		CloseTable();
		if (m_pCon != NULL)
		{
			if(m_pCon->State)
			{
				m_pCon->Close();
				m_pCon = NULL;
			}
		}
	}
	catch(_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
	}
}
/*
void CDataBaseEx::LogEvent(LPCTSTR lpFormat, ...)
{
	char chMsg[200];

	va_list pArg;
	va_start(pArg, lpFormat);
	vsprintf_s(chMsg, 200, lpFormat, pArg);
	va_end(pArg);
	////日志
	AddLog(chMsg, (int)strlen(chMsg));
}  */

BOOL CDataBaseEx::IsConnectNormal(LPCTSTR lpTableName)
{
	if(m_pCon == NULL)
		return FALSE;
	
	_RecordsetPtr pRset;
	
	try
	{
		pRset.CreateInstance(_uuidof(Recordset));
		if(pRset == NULL)
			return FALSE;
		
		char szCon[200];
		sprintf_s(szCon, 200, "SELECT * FROM %s", lpTableName);
		
		HRESULT hr = pRset->Open((_bstr_t)szCon, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
			return FALSE;
		
		pRset->Close();
	}
	catch (_com_error e) 
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL CDataBaseEx::Reconnect()
{
	Close();
	if (Open(m_strSrvIP.c_str(), m_strDBName.c_str(), m_strUserName.c_str(), 
		m_strPassword.c_str()))
	{
		REPORT(MN, T("Reconnect() successed\n"), RPT_INFO);
		return TRUE;
	}
	else
	{
		REPORT(MN, T("Reconnect() failed\n"), RPT_ERROR);
		return FALSE; 
	}
}

BOOL CDataBaseEx::OpenTable(LPCTSTR lpTableName)
{
	if(m_pCon == NULL)
		return FALSE;

	try
	{
		m_pRset.CreateInstance(_uuidof(Recordset));
		if (m_pRset == NULL)
		{
			REPORT(MN, T("m_pRset.CreateInstance failed code:%d\n", GetLastError()), RPT_ERROR);
			return FALSE;
		}

		char szCon[200];
		sprintf_s(szCon, 200, "SELECT * FROM %s WHERE IS_VLD=1", lpTableName);
		
		HRESULT hr = m_pRset->Open((_bstr_t)szCon, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
		{
			REPORT(MN, T("pRset->Open() failed\n"), RPT_ERROR);
			return FALSE;
		}
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}	

	return TRUE;
}


BOOL CDataBaseEx::OpenTableUseSql(LPCTSTR lpSql)
{
	if(m_pCon == NULL)
		return FALSE;

	try
	{
		m_pRset.CreateInstance(_uuidof(Recordset));
		if (m_pRset == NULL)
		{
			REPORT(MN, T("m_pRset.CreateInstance() failed\n"), RPT_ERROR);
			return FALSE;
		}

		HRESULT hr = m_pRset->Open((_bstr_t)lpSql, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
		{
			REPORT(MN, T("pRset->Open() failed\n"), RPT_ERROR);
			return FALSE;
		}
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}		
	return TRUE;
}

void CDataBaseEx::CloseTable()
{
	try
	{
		if (m_pRset != NULL)
		{
			m_pRset->Close();
			m_pRset = NULL;
		}
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
	}
}

void CDataBaseEx::MoveNext()
{
	try
	{
		m_pRset->MoveNext();
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
	}
}

void CDataBaseEx::MoveFirst()
{
	try
	{
		m_pRset->MoveFirst(); 
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
	}
}

void CDataBaseEx::MoveLast()
{
	try
	{
		m_pRset->MoveLast();
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
	}
}

BOOL CDataBaseEx::IsEOF()
{
	try
	{
		return m_pRset->adoEOF;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
}

BOOL CDataBaseEx::IsBOF()
{
	try
	{
		return m_pRset->BOF;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
}

void CDataBaseEx::GetCollect(string strFieldName, unsigned int& Value)
{
	try
	{
		_variant_t var;
		var = m_pRset->GetCollect(strFieldName.c_str());
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			Value = (unsigned int)var;
		else
			Value = 0;
	}
	catch (_com_error e)
	{
		Value = 0;
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), strFieldName.c_str()), RPT_ERROR);
	} 
}

void CDataBaseEx::GetCollect(string strFieldName, unsigned short& Value)
{
	try
	{
		_variant_t var;
		var = m_pRset->GetCollect(strFieldName.c_str());
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			Value = (unsigned short)var;
		else
			Value = 0;
	}
	catch (_com_error e)
	{
		Value = 0;
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), strFieldName.c_str()), RPT_ERROR);
	}
}

void CDataBaseEx::GetCollect(string strFieldName, double& Value)
{
	try
	{
		_variant_t var;
		var = m_pRset->GetCollect(strFieldName.c_str());
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			Value = (double)var;
		else
			Value = 0;
	}
	catch (_com_error e)
	{
		Value = 0;
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), strFieldName.c_str()), RPT_ERROR);
	}
}

void CDataBaseEx::GetCollect(string strFieldName, string& strValue)
{
	try
	{
		_variant_t var;
		var = m_pRset->GetCollect(strFieldName.c_str());
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			strValue = (LPCSTR)_bstr_t(var);
		else
			strValue = "";
	}
	catch (_com_error e)
	{
		strValue = "";
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), strFieldName.c_str()), RPT_ERROR);
	}
}

void CDataBaseEx::GetCollect(string strFieldName, LPCTSTR lpValue, int nMaxLen)
{
	try
	{
		_variant_t var;
		var = m_pRset->GetCollect(strFieldName.c_str());
		memset((char*)lpValue, 0, nMaxLen);

		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		{
			memcpy((char*)lpValue, (LPCSTR)_bstr_t(var), 
				nMaxLen > (int)strlen((LPCSTR)_bstr_t(var)) ? strlen((LPCSTR)_bstr_t(var)) : nMaxLen);
		}

		TrimRight((char*)lpValue);
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), strFieldName.c_str()), RPT_ERROR);
	}								  	
}

BOOL CDataBaseEx::GetCollect(string strFieldName, SYSTEMTIME* pValue)
{
	try
	{
		_variant_t var;
		var = m_pRset->GetCollect(strFieldName.c_str());
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		{
			VariantTimeToSystemTime(var.date, pValue);	    
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), strFieldName.c_str()), RPT_ERROR);
		return FALSE;
	}		
}

BOOL CDataBaseEx::Requery(LPCTSTR lpCondition)
{
	try
	{
		m_pRset->Filter = lpCondition;	  
		HRESULT hr = m_pRset->Requery(adCmdUnknown);
		if(FAILED(hr))
			return FALSE;
		else
			return TRUE;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s \n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
}

DWORD CDataBaseEx::GetRecordCount() 
{
	try
	{
		return m_pRset->GetRecordCount();
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s \n", (char*)e.Description()), RPT_ERROR);
		return 0;
	}
}

char* CDataBaseEx::GetMkt_Idx(LPCTSTR lpTableName, LPCTSTR lpIdxCode, int& nRecordCount)
{
	if(m_pCon == NULL)
		return FALSE;
	
	_RecordsetPtr pRset;
	
	try
	{
		pRset.CreateInstance(_uuidof(Recordset));
		if(pRset == NULL)
			return FALSE;
		
		char szCon[200];
		sprintf_s(szCon, 200, "SELECT * FROM %s WHERE IDX_CD='%s'", lpTableName, lpIdxCode);
		
		HRESULT hr = pRset->Open((_bstr_t)szCon, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
			return NULL;
		
		nRecordCount = pRset->GetRecordCount();
		if(nRecordCount <= 0)
		{
			pRset->Close();
			return NULL;
		}

		const int nCodeLen = 8;
		char* pValue = new char[nCodeLen*nRecordCount];
		if (pValue == NULL)
		{
			REPORT(MN, T("分配内存[pValue]失败\n"), RPT_ERROR);
			pRset->Close();
			return NULL;
		}
	
		char* pValuePos = pValue;
		while(!pRset->adoEOF)
		{
			char szStockCode[20] = {0};
			_variant_t var;
			var = pRset->GetCollect("SEC_CD");
			if (var.vt != VT_NULL)
			{
				memcpy(szStockCode, (LPCSTR)_bstr_t(var), strlen((LPCSTR)_bstr_t(var)));
				TrimRight((char*)szStockCode);
			}

			memcpy(pValuePos, szStockCode, nCodeLen);
			pValuePos += nCodeLen;
			pRset->MoveNext();
		}
		pRset->Close();
		return pValue;
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return NULL;
	}
	
	return NULL;
}

int CDataBaseEx::GetTableRecordCount(LPCTSTR lpTableName)
{
	int nRecordCount = 0;
	if(m_pCon == NULL)
		return -1;
	
	_RecordsetPtr pRset;
	
	try
	{
		pRset.CreateInstance(_uuidof(Recordset));
		if(pRset == NULL)
			return -1;
		
		char szCon[200];
		sprintf_s(szCon, 200, "SELECT * FROM %s", lpTableName);
		
		HRESULT hr = pRset->Open((_bstr_t)szCon, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
			return -1;
		
		nRecordCount = pRset->GetRecordCount();
		pRset->Close();
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return -1;
	}
	
	return nRecordCount;
}

BOOL CDataBaseEx::VerifyUserInfo(const char* pUserID, const char* pPassword, 
								 const char* pUserName, const char* pRole)
{
	_RecordsetPtr pRset;
	BOOL bRet = FALSE;
	try
	{
		pRset.CreateInstance(_uuidof(Recordset));
		if(pRset == NULL)
			return FALSE;
		
		char szCon[200];
		sprintf_s(szCon, 200, "SELECT * FROM User_Info WHERE UserID='%s' AND Password='%s' AND State='1'",
			pUserID, pPassword);
		
		HRESULT hr = pRset->Open((_bstr_t)szCon, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
			return FALSE;
		
		if (!pRset->adoEOF)
		{
			_variant_t var;
			var = pRset->GetCollect("UserName");
			if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			{
				memcpy((char*)pUserName, (LPCSTR)_bstr_t(var), strlen((LPCSTR)_bstr_t(var)));
				TrimRight((char*)pUserName);
			}

			var = pRset->GetCollect("Role");
			if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			{
				memcpy((char*)pRole, (LPCSTR)_bstr_t(var), strlen((LPCSTR)_bstr_t(var)));
				TrimRight((char*)pRole);
			}

			bRet = TRUE;
		}

		pRset->Close();
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
	return bRet;
}

BOOL CDataBaseEx::ModifyPassword(const char* pUserID, const char* pOldPw, 
								 const char* pNewPw)
{
	_RecordsetPtr pRset;
	BOOL bRet = FALSE;
	try
	{
		pRset.CreateInstance(_uuidof(Recordset));
		if(pRset == NULL)
			return FALSE;
		
		char szCon[200];
		sprintf_s(szCon, 200, "SELECT * FROM User_Info WHERE UserID='%s' AND Password='%s'  AND State='1'",
			pUserID, pOldPw);
		
		HRESULT hr = pRset->Open((_bstr_t)szCon, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockOptimistic, adCmdText);
		
		if(FAILED(hr))
			return FALSE;
		
		if (!pRset->adoEOF)
		{
			_variant_t var;
			pRset->PutCollect("Password", pNewPw);
			pRset->Update();

			bRet = TRUE;
		}

		pRset->Close();
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
	return bRet;
}

BOOL CDataBaseEx::AddRemark(tagAddRemark* pInfo, const char* pComment)
{
	_RecordsetPtr pRset;
	try
	{
		pRset.CreateInstance(_uuidof(Recordset));
		if(pRset == NULL)
			return FALSE;
		
		HRESULT hr = pRset->Open("SELECT * FROM ANA02_COMT", m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockOptimistic, adCmdText);
		
		if(FAILED(hr))
			return -1;
		
		pRset->AddNew();
		pRset->PutCollect("ANA_Account", (char*)pInfo->ANA_Account);
		pRset->PutCollect("SEC_CD", (char*)pInfo->SEC_CD);
		pRset->PutCollect("IDU_CD", (char*)pInfo->IDU_CD);
		pRset->PutCollect("Comment", pComment);
		
		char szDate[15] = {0};
		memcpy(szDate, pInfo->UPDT_TM, 4);
		szDate[4] = '-';
		memcpy(szDate+5, pInfo->UPDT_TM+4, 2);
		szDate[7] = '-';
		memcpy(szDate+8, pInfo->UPDT_TM+6, 2);
		pRset->PutCollect("UPDT_TM", szDate);

		pRset->Update();

		pRset->Close();
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
	return TRUE;
}
BOOL CDataBaseEx::GetORG_ID(const char* pStockCode, const char* pOrg_ID, BOOL bCN)
{
	_RecordsetPtr pRset;
	BOOL bRet = FALSE;
	try
	{
		pRset.CreateInstance(_uuidof(Recordset));
		if(pRset == NULL)
			return FALSE;
		
		string strSql;
		if (bCN)
		{
			strSql = "SELECT * FROM CN_STK01_Info WHERE IS_VLD=1 AND SEC_CD='";
			strSql += pStockCode;
			strSql += "'";
		}
		else 
		{
			strSql = "SELECT * FROM HK_STK01_Info WHERE IS_VLD=1 AND SEC_CD='";
			strSql += pStockCode;
			strSql += "'";
		}

		HRESULT hr = pRset->Open((_bstr_t)strSql.c_str(), m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
			return FALSE;
		
		if (!pRset->adoEOF)
		{
			_variant_t var;
			var = pRset->GetCollect("ORG_ID");
			if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			{
				memcpy((char*)pOrg_ID, (LPCSTR)_bstr_t(var), strlen((LPCSTR)_bstr_t(var)));
				TrimRight((char*)pOrg_ID);
			}
			
			bRet = TRUE;
		}

		pRset->Close();
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
	return bRet;
}

void CDataBaseEx::TrimRight(char* pData)
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