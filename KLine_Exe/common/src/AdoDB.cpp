//AdoDB.cpp
#include "stdafx.h"
#include "AdoDB.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////////
CDataBase::CDataBase()
{
	::CoInitialize(NULL);
	m_pCon = NULL;
	m_hWnd = NULL;
}

#ifdef _MFC_VER
CDataBase::CDataBase(HWND hWnd)
{
	::CoInitialize(NULL);
	m_pCon = NULL;
	m_hWnd = hWnd;
}
#endif

CDataBase::~CDataBase()
{
	Close();
	::CoUninitialize();
}

BOOL CDataBase::Open(LPCTSTR lpConStr, long nOptions)
{
	assert(lpConStr != NULL);
	try
	{
		m_pCon.CreateInstance(_uuidof(Connection));
		if(m_pCon == NULL)
		{	
			REPORT(MN, T("m_pCon.CreateInstance failed!\n"), RPT_ERROR);
			return FALSE;
		}

		m_pCon->Open((_bstr_t)lpConStr, "", "", nOptions);
	}
	catch(_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
	return TRUE;
}

void CDataBase::Close()
{
	try
	{
		if (m_pCon != NULL && IsOpen()) 
		{
			m_pCon->Close();
		}
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
	} 
}

BOOL CDataBase::IsOpen()
{
	try
	{
		return (m_pCon != NULL && (m_pCon->State & adStateOpen));
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
}

_RecordsetPtr CDataBase::Execute(LPCTSTR lpSQL, long nOptions)
{
	assert(m_pCon != NULL);

	try
	{
		return m_pCon->Execute(_bstr_t(lpSQL), NULL, nOptions);
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return NULL;
	} 
}

long CDataBase::BeginTrans()
{
	assert(m_pCon != NULL);
	try
	{
		return m_pCon->BeginTrans();
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return -1;
	} 
	return -1;
}

BOOL CDataBase::CommitTrans()
{
	assert(m_pCon != NULL);
	try
	{
		return SUCCEEDED(m_pCon->CommitTrans());
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
	return FALSE;
}

BOOL CDataBase::RollbackTrans()
{
	assert(m_pCon != NULL);
	try
	{
		return SUCCEEDED(m_pCon->RollbackTrans());
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
	return FALSE;
}

void CDataBase::SetCursorLocation(CursorLocationEnum CursorLocation)
{
	m_pCon->CursorLocation = CursorLocation;   //游标类型 
}

void CDataBase::LogEvent(unsigned short nLogType, const char* pFormat, ...)
{
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

CRecordSet::CRecordSet()
{
	m_pCon  = NULL;
	m_pRset	= NULL;
	m_pDB   = NULL;
}

CRecordSet::CRecordSet(_ConnectionPtr pCon)
{
	m_pCon  = pCon;
	m_pRset	= NULL;
	m_pDB   = NULL;
}

CRecordSet::CRecordSet(CDataBase* pDB)
{
	m_pCon  = pDB->GetConnection();
	m_pRset	= NULL;
	m_pDB = pDB;
}

CRecordSet::~CRecordSet()
{
}

BOOL CRecordSet::Open(LPCTSTR lpSql, CursorTypeEnum CursorType, LockTypeEnum LockType,
					  long lOption)
{
	assert(m_pCon != NULL);
	try
	{
		m_pRset.CreateInstance(_uuidof(Recordset));
		if (m_pRset == NULL)
		{
			REPORT(MN, T("m_pRset.CreateInstance() failed\n"), RPT_ERROR);
			return FALSE;
		}

		m_pRset->Open((_bstr_t)lpSql, m_pCon.GetInterfacePtr(), 
			CursorType, LockType, lOption);
	}
	catch (_com_error e) 
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}	

	return TRUE;
}

void CRecordSet::Close()
{
	try
	{
		if (m_pRset != NULL && m_pRset->State != adStateClosed)
		{
			m_pRset->Close();
			m_pRset = NULL;
		}
	}
	catch (const _com_error& e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
	}
}

void CRecordSet::LogEvent(unsigned short nLogType, const char* pFormat, ...)
{
	char szMsg[DB_MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(szMsg, DB_MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);
	////日志

#ifdef _MFC_VER
	SendMessage(m_pDB->GetMsgWnd(), WM_LOGEVENT, nLogType, (LPARAM)szMsg);
#endif
}

long CRecordSet::GetAbsolutePosition()
{
	assert(m_pRset != NULL);
	try
	{
		return m_pRset->GetAbsolutePosition();
	}
	catch(_com_error &e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return -1;
	}
}

BOOL CRecordSet::SetAbsolutePosition(int nPosition)
{
	assert(m_pRset != NULL);
	try
	{
		m_pRset->PutAbsolutePosition((enum PositionEnum)nPosition);		
		return TRUE;
	}
	catch(_com_error &e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
}

long CRecordSet::GetRecordCount()
{
	assert(m_pRset != NULL);
	try
	{
		long nCount = m_pRset->GetRecordCount();
		// 如果ado不支持此属性，则手工计算记录数目
		if (nCount < 0)
		{
			long nPos = GetAbsolutePosition();
			MoveFirst();
			nCount = 0;
			while (!IsEOF()) 
			{
				nCount++;
				MoveNext();
			}
			SetAbsolutePosition(nPos);
		}
		return nCount;
	}
	catch(const _com_error& e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return -1;
	}
}

BOOL CRecordSet::IsBOF()
{
	assert(m_pRset != NULL);
	return m_pRset->BOF;
}

BOOL CRecordSet::IsEOF()
{
	assert(m_pRset != NULL);
	return m_pRset->adoEOF;
}

void CRecordSet::MoveFirst()
{
	assert(m_pRset != NULL);
	m_pRset->MoveFirst();
}

void CRecordSet::MoveLast()
{
	assert(m_pRset != NULL);
	m_pRset->MoveLast();
}

void CRecordSet::MoveNext()
{
	assert(m_pRset);
	m_pRset->MoveNext();
}

void CRecordSet::MovePrevious()
{
	assert(m_pRset != NULL);
	m_pRset->MovePrevious();
}

BOOL CRecordSet::AddNew()
{
	assert(m_pRset != NULL);
	try
	{
		if (m_pRset->AddNew() == S_OK)
			return TRUE;
		else
			return FALSE;

	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
}

BOOL CRecordSet::Update()
{
	assert(m_pRset != NULL);
	try
	{
		if (m_pRset->Update() == S_OK)
			return TRUE;
		else
			return FALSE;

	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
}

BOOL CRecordSet::Delete(AffectEnum AffectRecords)
{
	assert(m_pRset != NULL);
	try
	{
		return (m_pRset->Delete(AffectRecords) == S_OK);
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
}

BOOL CRecordSet::Requery(long Options)
{
	assert(m_pRset != NULL);
	try
	{
		return (m_pRset->Requery(Options) == S_OK);
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
}

BOOL CRecordSet::SetFilter(LPCTSTR lpszFilter)
{
	assert(m_pRset != NULL);
	try
	{
		m_pRset->PutFilter(lpszFilter);
		return TRUE;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	} 
}

BOOL CRecordSet::SetSort(LPCTSTR lpszCriteria)
{
	assert(m_pRset != NULL);
	try
	{
		m_pRset->PutSort(lpszCriteria);
		return TRUE;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s\n", (char*)e.Description()), RPT_ERROR);
		return FALSE;
	}
}

void CRecordSet::PutCollect(LPCSTR lpFieldName, const _variant_t &value)
{
	try
	{
		assert(m_pRset != NULL);
		m_pRset->PutCollect(lpFieldName, value);
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), lpFieldName), RPT_ERROR);
	}
}

void CRecordSet::GetCollect(LPCSTR lpFieldName, unsigned int& nValue)
{
	try
	{
		assert(m_pRset != NULL);
		_variant_t var;
		var = m_pRset->GetCollect(lpFieldName);
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			nValue = (unsigned int)var;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), lpFieldName), RPT_ERROR);
	}
}

void CRecordSet::GetCollect(LPCSTR lpFieldName, int& nValue)
{
	try
	{
		assert(m_pRset != NULL);
		_variant_t var;
		var = m_pRset->GetCollect(lpFieldName);
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			nValue = (int)var;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), lpFieldName), RPT_ERROR);
	}
}

BOOL CRecordSet::GetCollect(LPCSTR lpFieldName, SYSTEMTIME* pValue)
{
	try
	{
		assert(m_pRset != NULL);
		_variant_t var;
		var = m_pRset->GetCollect(lpFieldName);
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
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), lpFieldName), RPT_ERROR);
		return FALSE;
	}
}

void CRecordSet::GetCollect(LPCSTR lpFieldName, LPCSTR lpValue, int nMaxLen)
{
	try
	{
		assert(m_pRset != NULL);
		memset((void*)lpValue, 0, nMaxLen);
		_variant_t var;
		var = m_pRset->GetCollect(lpFieldName);

		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
		{
			int nLen = (int)strlen((LPCSTR)_bstr_t(var));
			memcpy((char*)lpValue, (LPCSTR)_bstr_t(var), nLen<nMaxLen ? nLen:nMaxLen);
		}
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), lpFieldName), RPT_ERROR);
	}
}

void CRecordSet::GetCollect(LPCSTR lpFieldName, std::string &strValue)
{
	try
	{
		assert(m_pRset != NULL);
		_variant_t var;
		var = m_pRset->GetCollect(lpFieldName);
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			strValue = (LPCSTR)_bstr_t(var);
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), lpFieldName), RPT_ERROR);
	}
}

void CRecordSet::GetCollect(LPCSTR lpFieldName, double &Value)
{
	try
	{
		assert(m_pRset != NULL);
		_variant_t var;
		var = m_pRset->GetCollect(lpFieldName);
		if (var.vt != VT_NULL && var.vt != VT_EMPTY)
			Value = (double)var;
	}
	catch (_com_error e)
	{
		REPORT(MN, T("SQLError:%s field:%s\n", (char*)e.Description(), lpFieldName), RPT_ERROR);
	}
}


//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
CAccessDB::CAccessDB()
{
}

#ifdef _MFC_VER
CAccessDB::CAccessDB(HWND hWnd)
: CDataBase(hWnd)
{
}
#endif

CAccessDB::~CAccessDB()
{
}

void CAccessDB::LogEvent(unsigned short nLogType, const char* pFormat, ...)
{
	char szMsg[DB_MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(szMsg, DB_MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);
	////日志

#ifdef _MFC_VER
	SendMessage(m_hWnd, WM_LOGEVENT, nLogType, (LPARAM)szMsg);
#endif
}

BOOL CAccessDB::Open(LPCTSTR lpDBPath, LPCTSTR lpPassword, long lOptions)
{
	std::string strCon = "Provider=Microsoft.Jet.OLEDB.4.0; Data Source=";
	strCon += lpDBPath;

	if (lpPassword != NULL)
	{
		strCon += "Jet OLEDB:Database Password=";
		strCon += lpPassword;
		strCon += ";";
	}

	return CDataBase::Open(strCon.c_str(), lOptions);
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
CSQLServerDB::CSQLServerDB()
{
	m_lOptions = adConnectUnspecified;
}

CSQLServerDB::~CSQLServerDB()
{
}

BOOL CSQLServerDB::Open(string strSerIP, string strDBName, string strUserName, 
						string strPassword, long lOptions)
{
	std::string strCon = "driver={SQL Server};Server=";
	strCon += strSerIP;
	strCon += ";DATABASE=";
	strCon += strDBName;
	strCon += ";UID=";
	strCon += strUserName;
	strCon += ";PWD=";
	strCon += strPassword;

	m_strCon   = strCon;
	m_lOptions = lOptions;
	return CDataBase::Open(strCon.c_str(), lOptions);
}

BOOL CSQLServerDB::IsConnectNormal(string strTestTable)
{
	CRecordSet reSet(this);
	string strCon = "SELECT * FROM ";
	strCon += strTestTable;

	BOOL bRet = reSet.Open(strCon.c_str(), adOpenDynamic, adLockReadOnly);
	reSet.Close();

	return bRet;
}

BOOL CSQLServerDB::Reconnect()
{
	CDataBase::Close();

	return CDataBase::Open(m_strCon.c_str(), m_lOptions);
}