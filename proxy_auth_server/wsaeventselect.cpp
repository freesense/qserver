//WSAEventSelect.cpp
#include "stdafx.h"
#include "WSAEventSelect.h"
#include "server_auth.h"
#include <process.h>
#include "../public/auth/FilePath.h"
#include "../public/commx/mery.h"

CProxySocket *g_pProxySocket = NULL;
extern unsigned int g_nTimeout;
/////////////////////////////////////////////////////////////////////////////
void CProxySocket::finalize()
{
	m_bQuit = true;
	Close();
}

int CProxySocket::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	m_bValid = true;
	REPORT(MODULE_NAME, T("代理已经连接到[%s]\n", GetPeerAddress()), RPT_INFO);

	//make connection heartbeat packet
	std::string s = m_ap.HeartBeat();
	CommxHead *phead = (CommxHead*)m_szHeartbeatBuffer;
	memset(phead, 0x00, sizeof(CommxHead));
	phead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	phead->Length = s.length();
	memcpy(m_szHeartbeatBuffer+sizeof(CommxHead), s.c_str(), s.length());

	//prebuild receive buffer
	m_pHead = (CommxHead*)m_szBuffer;
	memset(m_pHead, 0x00, sizeof(CommxHead));

	//initialize heartbeat timer
	m_timer.m_pCallback = this;
	m_timer.m_nTimerID = RegisterTimer(&m_timer, (float)g_nTimeout, (float)g_nTimeout);

	//read commxhead
	CHECK_RUN(-1 == Read((m_nReadID = 0), sizeof(CommxHead)), MODULE_NAME, T("读取数据[%d]失败\n", sizeof(CommxHead)), RPT_ERROR, return -1);
	g_pAuth->OnServerLogin(NULL, NULL);
	return ICommX::handle_open(lpInitialData, nInitialSize);
}

int CProxySocket::handle_close(int iError)
{
	m_bValid = false;
	CancelTimer(m_timer.m_nTimerID);
	m_timer.m_nTimerID = 0;
	REPORT(MODULE_NAME, T("代理通讯连接断开[%d]\n", iError), RPT_ERROR);
	ICommX::handle_close(iError);
	if (m_bQuit)
		return -1;
	return Connect(GetPeerAddress());
}

void CProxySocket::handle_release()
{
	delete this;
	g_pProxySocket = NULL;
}

char* CProxySocket::GetRecvBuffer(int readID)
{
	if (0 == m_nReadID)
		return (char*)m_pHead;
	else
		return m_szBuffer + sizeof(CommxHead);
}

unsigned int CProxySocket::GetRecvBufSize(int readID)
{
	if (0 == m_nReadID)
		return sizeof(CommxHead);
	else
		return sizeof(m_szBuffer) - sizeof(CommxHead);
}

int CProxySocket::handle_read(char *lpBuffer, unsigned int nLength)
{
	if (0 == m_nReadID)
	{
		CHECK_RUN(-1 == Read((m_nReadID = 1), m_pHead->Length), MODULE_NAME,
			T("读取数据[%d]失败\n", m_pHead->Length), RPT_ERROR, return -1);
	}
	else
	{
		TiXmlDocument doc;
		doc.Parse(lpBuffer);
		TiXmlElement *pNode = doc.RootElement();
		if (pNode)
		{
			if (!strcmp(pNode->Value(), "Start"))
				ParseStart(pNode);
			else if (!strcmp(pNode->Value(), "Stop"))
				ParseStop(pNode);
			else if (!strcmp(pNode->Value(), "DownloadXml"))
				ParseDownloadXml(pNode);
			else if (!strcmp(pNode->Value(), "UploadXml"))
				ParseUploadXml(pNode);
			else
				REPORT(MODULE_NAME, T("未知的监控命令: %s\n", pNode->Value()), RPT_WARNING);
		}

		CHECK_RUN(-1 == Read((m_nReadID = 0), sizeof(CommxHead)), MODULE_NAME,
			T("读取数据[%d]失败\n", sizeof(CommxHead)), RPT_ERROR, return -1);
	}
	return 0;
}

void CProxySocket::ParseStart(TiXmlElement *pNode)
{
	if (!pNode)
		return;

	TiXmlElement *pElm = pNode->FirstChildElement();
	if (!pElm)
		return;

	const char *lpExecutor = pElm->GetText();	//程序名称
	pElm = pElm->NextSiblingElement();
	const char *lpParams = pElm->GetText();		//执行参数
	pElm = pElm->NextSiblingElement();
	const char *lpWorkDir = pElm->GetText();	//工作目录

	if (g_pAuth && g_pAuth->m_pCallback)
		g_pAuth->m_pCallback->OnExecute(lpExecutor, lpParams, lpWorkDir);
}

void CProxySocket::ParseStop(TiXmlElement *pNode)
{
	if (!pNode)
		return;

	TiXmlElement *pElm = pNode->FirstChildElement();
	if (!pElm)
		return;
	const char *lpExecutor = pElm->GetText();	//程序名称
	pElm = pElm->NextSiblingElement();
	const char *lpParams = pElm->GetText();		//执行参数
	pElm = pElm->NextSiblingElement();
	const char *lpWorkDir = pElm->GetText();	//工作目录

	//停止程序
	if (g_pAuth && g_pAuth->m_pCallback)
		g_pAuth->m_pCallback->OnStop(lpExecutor, lpParams, lpWorkDir);
}

void CProxySocket::ParseDownloadXml(TiXmlElement *pNode)
{
	if (!pNode)
		return;

	TiXmlAttribute *pAttr = pNode->FirstAttribute();
	std::string strDevName;
	if (pAttr != NULL)
		strDevName = pAttr->Value();

	TiXmlNode* node = pNode->FirstChild();
	std::string strXmlPath;
	if (node != NULL)
		strXmlPath = node->Value();	

	//下载配置文件
	if (g_pAuth && g_pAuth->m_pCallback)
		g_pAuth->m_pCallback->OnDownloadXml(strDevName.c_str(), strXmlPath.c_str());
}

void CProxySocket::ParseUploadXml(TiXmlElement *pNode)
{
	if (!pNode)
		return;

	TiXmlAttribute *pAttr = pNode->FirstAttribute();
	std::string strXmlPath;
	if (pAttr != NULL)
		strXmlPath = pAttr->Value();

	pAttr = pAttr->Next();
	int nDataSize = 0;
	if (pAttr != NULL)
		nDataSize = atoi(pAttr->Value());

	TiXmlNode* node = pNode->FirstChild();
	std::string strXmlData;
	if (node != NULL)
		strXmlData = node->Value();	

	char* pDataBuf = (char*)mpnew((nDataSize-1)/2+30);
	memset(pDataBuf, 0, (nDataSize-1)/2+30);
	ChangeBinStringToString(pDataBuf, strXmlData.c_str(), nDataSize-1);

	//下载配置文件
	if (g_pAuth && g_pAuth->m_pCallback)
		g_pAuth->m_pCallback->OnUploadXml(strXmlPath.c_str(), pDataBuf, (nDataSize-1)/2);

	mpdel(pDataBuf);
}

void CProxySocket::ChangeBinStringToString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize)
{
	char* pDesBufPos = (char*)lpDesBuf;
	char* pSrcBufPos = (char*)lpSrcBuf;

	char szTemp[10];
	int nIndex = 0;
	for (int i=0; i<nSrcBufSize; i+=2)
	{
		szTemp[0] = pSrcBufPos[i];
		szTemp[1] = pSrcBufPos[i+1];
		szTemp[2] = 0;

		sscanf(szTemp, "%x", &pDesBufPos[nIndex++]);
	}
}

int CProxySocket::handle_write(char *lpData, unsigned int nLength, bool bSendOK)
{
	if (lpData != m_szHeartbeatBuffer)
		mpdel(lpData);
	return 0;
}

int CProxySocket::handle_timeout(ITimerCallbackBase *pTimerParam)
{
	CommxHead *phead = (CommxHead*)m_szHeartbeatBuffer;
	CHECK(-1 == Write(phead, phead->Length + sizeof(CommxHead)), MODULE_NAME, T("发送心跳失败\n"), RPT_ERROR);
	return 0;
}
