
#include "stdafx.h"
#include "server_auth.h"
#include <string>
#include <functional>
#include "../public/protocol.h"
#include "../public/commx/mery.h"
#include "../public/auth/auth_struct.h"
#include "../public/devmonitor/auth_xml_protocol.h"
#include "WSAEventSelect.h"

CServerAuth *g_pAuth = NULL;

CServerAuth::CServerAuth(IServerAuthBase *pSvrAuthBase)
: m_pCallback(pSvrAuthBase)
{}

CServerAuth::~CServerAuth()
{}

int CServerAuth::OnTransmit(char *lpData, unsigned int nLength)
{
	if (!g_pProxySocket || !g_pProxySocket->m_bValid)
		return -1;

	char *lpTmp = (char*)mpnew(nLength);
	CHECK_RUN(!lpTmp, MODULE_NAME, T("ÄÚ´æ·ÖÅä[%d]Ê§°Ü\n", nLength), RPT_CRITICAL, return -1);
	memcpy(lpTmp, lpData, nLength);
	return g_pProxySocket->Write(lpTmp, nLength);
}

int CServerAuth::OnServerLogin(const char *lpFingerPrinter, const char *lpDevList)
{
	if (lpFingerPrinter)
	{
		//save fingerprinter and dev table for reconnect
		m_strFingerPrinter = lpFingerPrinter;
		if (lpDevList)
			m_strDevTable = lpDevList;
		else
			m_strDevTable = "";
	}
	else if (m_strFingerPrinter.length() == 0)
		return -1;

	//make xml string
	std::string s = g_pProxySocket->m_ap.ServerLogin(m_strFingerPrinter.c_str(), m_strDevTable.c_str());

	char *lpTmp = (char*)mpnew(s.length() + sizeof(CommxHead));
	CHECK_RUN(!lpTmp, MODULE_NAME, T("ÄÚ´æ·ÖÅä[%d]Ê§°Ü\n", s.length() + sizeof(CommxHead)), RPT_CRITICAL, return -1);

	CommxHead *pHead = (CommxHead*)lpTmp;
	memset(pHead, 0x00, sizeof(CommxHead));
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = s.length();

	memcpy(lpTmp + sizeof(CommxHead), s.c_str(), pHead->Length);
 	g_pProxySocket->Write(lpTmp, sizeof(CommxHead) + pHead->Length);
	return 0;
}

int CServerAuth::OnUserLogin(const char *lpUser, const char *lpPwd, const char *lpServerFingerPrinter)
{
	return 0;
}

int CServerAuth::OnUserLogout(const char *lpUser, const char *lpServerFingerPrinter)
{
	return 0;
}
