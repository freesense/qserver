//FTPTranData.cpp
//
#include "stdafx.h"
#include "FTPTranData.h"

///////////////////////////////////////////////////////////////////
CFTPTranData::CFTPTranData()
{
	m_pFtpCon    = NULL;
}

CFTPTranData::~CFTPTranData()
{
	Close();
}

BOOL CFTPTranData::Open( LPCTSTR pstrServer, LPCTSTR pstrUserName, LPCTSTR pstrPassword)
{
	try
	{
		m_pFtpCon = m_interSession.GetFtpConnection(pstrServer, pstrUserName, pstrPassword,
			INTERNET_INVALID_PORT_NUMBER, TRUE);
		if (m_pFtpCon == NULL)
			return FALSE;
	}
	catch(CInternetException* pInterException)
	{
		char ErrMsg[100];
		pInterException->GetErrorMessage(ErrMsg, 100);
		REPORT(MN, T("Òì³££º%s\n", ErrMsg), RPT_ERROR);
		pInterException->Delete();
		return FALSE;
	}

	return TRUE;
}

void CFTPTranData::Close()
{
	try
	{
		if (m_pFtpCon != NULL)
		{
			m_pFtpCon->Close();
			delete m_pFtpCon;
			m_pFtpCon = NULL;
		}
		m_interSession.Close();
	}
	catch(...)
	{
	}
}

BOOL CFTPTranData::GetFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile)
{
	if (m_pFtpCon == NULL)
		return FALSE;

	BOOL bRet = TRUE;
	try
	{
		bRet = m_pFtpCon->GetFile(pstrRemoteFile, pstrLocalFile, FALSE);
	}
	catch(CInternetException* pInterException)
	{
		char ErrMsg[100];
		pInterException->GetErrorMessage(ErrMsg, 100);
		REPORT(MN, T("Òì³££º%s\n", ErrMsg), RPT_ERROR);
		pInterException->Delete();
		return FALSE;
	}
	if (!bRet)
		REPORT(MN, T("GetFile() failed with code:%d\n", GetLastError()), RPT_ERROR);
	return bRet;
}

BOOL CFTPTranData::PutFile(LPCTSTR pstrLocalFile, LPCTSTR pstrRemoteFile) 
{
	if (m_pFtpCon == NULL)
		return FALSE;

	BOOL bRet = TRUE;
	try
	{
		bRet = m_pFtpCon->PutFile(pstrLocalFile, pstrRemoteFile);
	}
	catch(CInternetException* pInterException)
	{
		char ErrMsg[100];
		pInterException->GetErrorMessage(ErrMsg, 100);
		REPORT(MN, T("Òì³££º%s\n", ErrMsg), RPT_ERROR);
		pInterException->Delete();
		return FALSE;
	}

	if (!bRet)
		REPORT(MN, T("PutFile() failed with code:%d\n", GetLastError()), RPT_ERROR);
	return bRet;
}
