//DataBaseEx.h
#ifndef _DATABASEEX_H_
#define _DATABASEEX_H_
#include "string"
#include "TMDdef.h"
using namespace std;

#import "C:\Program Files\common files\system\ado\msado15.dll" no_namespace rename("EOF", "adoEOF")
////////////////////////////////////////////////////////////////////////

class CDataBaseEx
{
public:
	CDataBaseEx();
	virtual ~CDataBaseEx();

	BOOL Open(string strSerIP, string strDBName, string strUserName, string strPassword);
	void Close();

	BOOL IsConnectNormal(LPCTSTR lpTableName);
	BOOL Reconnect();

	BOOL OpenTable(LPCTSTR lpTableName);
	BOOL OpenTableUseSql(LPCTSTR lpSql);
	void CloseTable();

	void MoveNext();
	void MoveFirst();
	void MoveLast();
	BOOL IsBOF();
	BOOL IsEOF();

	void GetCollect(string strFieldName, unsigned int& Value);
	void GetCollect(string strFieldName, unsigned short& Value);
	void GetCollect(string strFieldName, double& Value);
	void GetCollect(string strFieldName, string& strValue);
	void GetCollect(string strFieldName, LPCTSTR lpValue, int nMaxLen);
	BOOL GetCollect(string strFieldName, SYSTEMTIME* pValue);
	BOOL Requery(LPCTSTR lpCondition);
	DWORD GetRecordCount();
	int GetTableRecordCount(LPCTSTR lpTableName);
	
	char* GetMkt_Idx(LPCTSTR lpTableName, LPCTSTR lpIdxCode, int& nRecordCount);
	BOOL VerifyUserInfo(const char* pUserID, const char* pPassword, 
								 const char* pUserName, const char* pRole);
	BOOL ModifyPassword(const char* pUserID, const char* pOldPw, 
		const char* pNewPw);
	BOOL AddRemark(tagAddRemark* pInfo, const char* pComment);
	BOOL GetORG_ID(const char* pStockCode, const char* pOrg_ID, BOOL bCN = TRUE);
	void TrimRight(char* pData);
protected:
	//virtual void LogEvent(LPCTSTR lpFormat, ...);

	//Attributes
	_ConnectionPtr        m_pCon;
	_RecordsetPtr         m_pRset;

private:
	string                m_strSrvIP;
	string                m_strDBName;
	string                m_strUserName;
	string                m_strPassword;
};

////////////////////////////////////////////////////////////////////////
#endif