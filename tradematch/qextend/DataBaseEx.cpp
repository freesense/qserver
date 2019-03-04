//DataBaseEx.cpp
#include "stdafx.h"
#include "DataBaseEx.h"

extern void AddLog(const char *pData, int nLen);

////////////////////////////////////////////////////////////////////////////////
CDataBaseEx::CDataBaseEx()
{
	//::CoInitialize(NULL);
	m_pCon  = NULL;
	m_pRset = NULL;
	m_bIsConnected = false;
}

CDataBaseEx::~CDataBaseEx()
{
	//::CoUninitialize();
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
			REPORT(MN,T("m_pCon.CreateInstance failed!\n"),RPT_ERROR);
			return FALSE;
		}
		
		char szCon[200];
		sprintf_s(szCon, 200, "driver={SQL Server};Server=%s;DATABASE=%s;UID=%s;PWD=%s",
			m_strSrvIP.c_str(), m_strDBName.c_str(), m_strUserName.c_str(), m_strPassword.c_str());

		m_pCon->CursorLocation=adUseClient;   //游标类型  
		HRESULT hr = m_pCon->Open((_bstr_t)szCon, "", "", adConnectUnspecified);
		if(FAILED(hr))
		{
			REPORT(MN,T("Open database[%s:%s] failed!\n",m_strSrvIP.c_str(), m_strDBName.c_str()),RPT_ERROR);
			return FALSE;
		}
	}
	catch(_com_error e)
	{
		REPORT(MN,T("SQLError:%s\n",(char*)e.Description()),RPT_ERROR);
		return FALSE;
	}

	REPORT(MN,T("Open database[%s:%s] successed\n", m_strSrvIP.c_str(), m_strDBName.c_str()),RPT_INFO);
	m_bIsConnected = true;
	return TRUE;
}

void CDataBaseEx::Close()
{
	m_bIsConnected = false;
	try
	{
		CloseTable();
		if (m_pCon != NULL)
		{
			if(m_pCon->State)
			{
				m_pCon->Close();
				//m_pCon->Release();
				m_pCon = NULL;
			}
		}
	}
	catch(_com_error e)
	{
		REPORT(MN,T("SQLError:%s\n", (char*)e.Description()),RPT_ERROR);
	}
}

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
		return TRUE;
	}
	else
	{
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
			REPORT(MN,T("m_pRset.CreateInstance() failed\n"),RPT_ERROR);
			return FALSE;
		}

		char szCon[200];
		sprintf_s(szCon, 200, "SELECT * FROM %s WHERE IS_VLD=1", lpTableName);
		
		HRESULT hr = m_pRset->Open((_bstr_t)szCon, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
		{
			REPORT(MN,T("pRset->Open() failed\n"),RPT_ERROR);
			return FALSE;
		}
	}
	catch (_com_error e) 
	{
		REPORT(MN,T("SQLError:%s\n",(char*)e.Description()),RPT_ERROR);
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
			REPORT(MN,T("m_pRset.CreateInstance() failed\n"),RPT_ERROR);
			return FALSE;
		}

		HRESULT hr = m_pRset->Open((_bstr_t)lpSql, m_pCon.GetInterfacePtr(), 
			adOpenDynamic, adLockReadOnly, adCmdText);
		
		if(FAILED(hr))
		{
			REPORT(MN,T("pRset->Open() failed\n"),RPT_ERROR);
			return FALSE;
		}
	}
	catch (_com_error e) 
	{
		REPORT(MN,T("SQLError:%s\n",(char*)e.Description()),RPT_ERROR);		
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
		REPORT(MN,T("SQLError:%s\n", (char*)e.Description()),RPT_ERROR);
	}
}

void CDataBaseEx::MoveNext()
{
	m_pRset->MoveNext();
}

void CDataBaseEx::MoveFirst()
{
	m_pRset->MoveFirst(); 
}

void CDataBaseEx::MoveLast()
{
	m_pRset->MoveLast();
}

BOOL CDataBaseEx::IsEOF()
{
	return m_pRset->adoEOF;
}

BOOL CDataBaseEx::IsBOF()
{
	return m_pRset->BOF;
}

void CDataBaseEx::GetCollect(string strFieldName, unsigned int& Value)
{
	_variant_t var;
	var = m_pRset->GetCollect(strFieldName.c_str());
	if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		Value = (unsigned int)var;
	else
		Value = 0;
}
void CDataBaseEx::GetCollect(string strFieldName, int& Value)
{
	_variant_t var;
	var = m_pRset->GetCollect(strFieldName.c_str());
	if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		Value = (unsigned int)var;
	else
		Value = 0;
}

void CDataBaseEx::GetCollect(string strFieldName, unsigned short& Value)
{
	_variant_t var;
	var = m_pRset->GetCollect(strFieldName.c_str());
	if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		Value = (unsigned short)var;
	else
		Value = 0;
}

void CDataBaseEx::GetCollect(string strFieldName, double& Value)
{
	_variant_t var;
	var = m_pRset->GetCollect(strFieldName.c_str());
	if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		Value = (double)var;
	else
		Value = 0;
}

void CDataBaseEx::GetCollect(string strFieldName, string& strValue)
{
	_variant_t var;
	var = m_pRset->GetCollect(strFieldName.c_str());
	if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		strValue = (LPCSTR)_bstr_t(var);
	else
		strValue = "";
}

void CDataBaseEx::GetCollect(string strFieldName, LPCTSTR lpValue, int nMaxLen)
{
	_variant_t var;
	var = m_pRset->GetCollect(strFieldName.c_str());
	memset((char*)lpValue, 0, nMaxLen);

	if (var.vt != VT_NULL && var.vt != VT_EMPTY)
	{
		memcpy((char*)lpValue, (LPCSTR)_bstr_t(var), 
		nMaxLen > (int)strlen((LPCSTR)_bstr_t(var)) ? strlen((LPCSTR)_bstr_t(var)) : nMaxLen);
	}

}

BOOL CDataBaseEx::GetCollect(string strFieldName, SYSTEMTIME* pValue)
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

BOOL CDataBaseEx::Requery(LPCTSTR lpCondition)
{
	m_pRset->Filter = lpCondition;	  
	HRESULT hr = m_pRset->Requery(adCmdUnknown);
	if(FAILED(hr))
		return FALSE;
	else
		return TRUE;
}

DWORD CDataBaseEx::GetRecordCount() 
{
	return m_pRset->GetRecordCount();
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
		REPORT(MN,T("SQLError:%s",(char*)e.Description()),RPT_ERROR);
		return -1;
	}
	
	return nRecordCount;
}

bool CDataBaseEx::RunSql(char * c_pSql)
{
	CString strSQL;
	strSQL = c_pSql;
	_variant_t varSQL(strSQL); 

	COleVariant vtOptional((long)DISP_E_PARAMNOTFOUND,VT_ERROR); 
	
	try
	{
	m_pCon->Execute(_bstr_t(strSQL),&vtOptional,-1); 
	}
	catch(_com_error e)///捕捉异常
	{
		//CString temp; 
		//temp.Format(_T("ExecSql错误信息:%s"),e.ErrorMessage()); 
		//AfxMessageBox(e.Source()); 
		//AfxMessageBox(e.Description()); 
		return false;
	}
	return true;
}