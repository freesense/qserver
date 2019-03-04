//IOCPServer.cpp
#include "stdafx.h"
#include "IOCPServer.h"
#include "KLine_Exe.h"
#include "MainDlg.h"
#include "FilePath.h"
//////////////////////////////////////////////////////////////////////////////////
CIOCPServer::CIOCPServer()
{
	m_hMsgWnd = NULL;
}

CIOCPServer::~CIOCPServer()
{
	map<SOCKET, tagRecvData*>::iterator pos = m_mapSocket.begin();
	while (pos != m_mapSocket.end())
	{
		delete pos->second;
		pos++;
	}
	m_mapSocket.clear();
}

/*void CIOCPServer::LogEvent(WORD nLogType, LPCSTR pFormat, ...)
{
	char szMsg[DB_MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(szMsg, DB_MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);
	////日志
	theApp.LogEvent(RPT_DEBUG, szMsg);
}*/

void CIOCPServer::OnDisconnect(LPHANDLE_CONTEXT pHandleContext)
{
	if (pHandleContext == NULL)
		return;

	m_csMap.Lock();
	map<SOCKET, tagRecvData*>::iterator find = m_mapSocket.find(pHandleContext->hSocket);
	if (find != m_mapSocket.end())
	{
		delete find->second;
		m_mapSocket.erase(find);
	}
	m_csMap.Unlock();

	ASSERT(m_hMsgWnd != NULL);
	UINT nPort = ntohs(pHandleContext->addr.sin_port);
	std::string strIP = inet_ntoa(pHandleContext->addr.sin_addr);
	if (!strIP.empty())
	{
		tagConnect con = {0};
		con.hSock = pHandleContext->hSocket;
		con.nPort = nPort;
		strcpy_s(con.szIP, 20, strIP.c_str());
		::SendMessage(m_hMsgWnd, WM_DEVSERVLOGOUT, (WPARAM)&con, 0);
	}
}

BOOL CIOCPServer::OnRecv(LPHANDLE_CONTEXT pHandleContext, LPIO_CONTEXT pIoContext,
						 DWORD dwDataSize)
{
	m_csMap.Lock();
	map<SOCKET, tagRecvData*>::iterator find = m_mapSocket.find(pIoContext->hCltSock);
	if (find == m_mapSocket.end())  
	{
		if (dwDataSize != PACKET_HEAD_LEN)
		{
			m_csMap.Unlock();
			return FALSE;
		}

		tagRecvData* pData = new tagRecvData;
		memset(pData, 0, sizeof(tagRecvData));
		memcpy(&pData->head, pIoContext->Buffer, sizeof(CommxHead));
		m_mapSocket.insert(std::make_pair(pIoContext->hCltSock, pData));
		RecvPack(pHandleContext, pIoContext, pData->head.Length);
	}
	else
	{
		find->second->nReaded += dwDataSize;
		if (find->second->nReaded != find->second->head.Length)
		{
			RecvPack(pHandleContext, pIoContext, find->second->head.Length - find->second->nReaded, find->second->nReaded);
			m_csMap.Unlock();
			return TRUE;
		}

		/*if (dwDataSize != find->second->Length)
		{
			delete find->second;
			m_mapSocket.erase(find);
			return FALSE;
		}*/

		if (!ProcessRequest(pHandleContext, pIoContext->Buffer, &find->second->head))
		{
			delete find->second;
			m_mapSocket.erase(find);
			m_csMap.Unlock();
			return FALSE;
		}

		delete find->second;
		m_mapSocket.erase(find);
		RecvPack(pHandleContext, pIoContext, PACKET_HEAD_LEN);
	}

	m_csMap.Unlock();
	return TRUE;
}

void CIOCPServer::OnSend(LPIO_CONTEXT pIoContext)
{
}

void CIOCPServer::OnConnect(SOCKET hSock)
{
	int i=0;
}

void CIOCPServer::OnTimeOut(LPIO_CONTEXT pIoContext)
{
	::SendMessage(m_hMsgWnd, WM_DEVSERV_TIMEOUT, (WPARAM)pIoContext->hCltSock, 0);
}

BOOL CIOCPServer::ProcessRequest(LPHANDLE_CONTEXT pHandleContext, void* pData, CommxHead* pHead)
{
	/*
	{//Test
		int nSendSize = pHead->Length + sizeof(CommxHead);
		char szSendBuf[BUF_SIZE];
		memcpy(szSendBuf, pHead, sizeof(CommxHead));
		memcpy(szSendBuf+sizeof(CommxHead), pData, pHead->Length);
		if (!SendPack(pHandleContext->hSocket, szSendBuf, nSendSize))
		{
			REPORT(MN, T("SendPack() failed\n"), RPT_WARNING);
			return FALSE;
		}
		return TRUE;
	}
	//*/
	TiXmlDocument doc;
	doc.Parse((const char*)pData);

	TiXmlElement *pNode = doc.RootElement();
	if (NULL == pNode)
	{
		FILE* pfile = fopen("recv.dat", "a+");
		fwrite(pData, 1, pHead->Length, pfile);
		fclose(pfile);

		REPORT(MN, T("无效数据\n"), RPT_WARNING);
		return FALSE;
	}

	BOOL bRet = TRUE;
	if (!strcmp(pNode->Value(), "HeartBeat"))
	{
		//REPORT(MN, T("HeartBeat\n"), RPT_INFO);
		//心跳包原样奉还
		/*int nSendSize = pHead->Length + sizeof(CommxHead);
		char szSendBuf[BUF_SIZE];
		memcpy(szSendBuf, pHead, sizeof(CommxHead));
		memcpy(szSendBuf+sizeof(CommxHead), pData, pHead->Length);
		if (!SendPack(pHandleContext->hSocket, szSendBuf, nSendSize))
		{
			REPORT(MN, T("SendPack() failed\n"), RPT_WARNING);
			return FALSE;
		}*/
	}
	else if (!strcmp(pNode->Value(), "ServerLogin"))
	{
		bRet = ParseServerLogin(pNode, pHandleContext);
	}
    else if(!strcmp(pNode->Value(), "Log"))
	{
        bRet = ParseLog(pNode, pHandleContext);
	}
	else if(!strcmp(pNode->Value(), "XmlData"))
	{
		bRet = ParseXmlData(pNode, pHandleContext);
	}

	return bRet;
}

BOOL CIOCPServer::ParseLoginDevList(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext)
{
	ASSERT(pNode != NULL);
    TiXmlElement *pServer = pNode->FirstChildElement();
	std::string strIP = inet_ntoa(pHandleContext->addr.sin_addr);

	while (pServer)
	{
		TiXmlAttribute *pAttr = pServer->FirstAttribute();
		if (pAttr != NULL)
		{  
			CString strProID = pAttr->Value();

			TiXmlNode* node = pServer->FirstChild();
			CString strDevName = node->Value();	//FingerPrinter

			tagDevLogin devLogin = {0};
			strcpy_s(devLogin.szIP, 20, strIP.c_str());
			devLogin.dwProID = atoi(strProID);

			if (0 == ::SendMessage(m_hMsgWnd, WM_DEVLOGIN, (WPARAM)&devLogin,
				(LPARAM)&strDevName))
			{
				REPORT(MN, T("没有注册的设备[%s:%s]登录\n", strIP.c_str(), strDevName), RPT_WARNING);
			}  
		}

		//Next server
		pServer = pServer->NextSiblingElement();
	}

	return TRUE;
}

BOOL CIOCPServer::ParseServerLogin(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext)
{
	ASSERT(pNode != NULL);
    TiXmlElement *pServer = pNode->FirstChildElement();
	std::string strIP = inet_ntoa(pHandleContext->addr.sin_addr);
	
	ASSERT(m_hMsgWnd != NULL);
	UINT nPort = ntohs(pHandleContext->addr.sin_port);
	if (!strIP.empty())
	{
		tagConnect con = {0};
		con.hSock = pHandleContext->hSocket;
		con.nPort = nPort;
		strcpy_s(con.szIP, 20, strIP.c_str());

		::SendMessage(m_hMsgWnd, WM_DEVSERVLOGIN, (WPARAM)&con, 0);
	}
	
	if (pServer)
	{
		TiXmlAttribute *pAttr = pServer->FirstAttribute();
		if (pAttr == NULL)
			return TRUE;

		CString strDevName = pAttr->Value();	   //FingerPrinter
		TiXmlNode* node = pServer->FirstChild();
		CString strDevList;
		
		if (node != NULL)
			strDevList = node->Value();	       //设备列表

		::SendMessage(m_hMsgWnd, WM_UPDATE_DEV_STATUS, (WPARAM)&strDevList, (LPARAM)strIP.c_str());
	}

	return TRUE;
}

BOOL CIOCPServer::ParseServerLogout(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext)
{
	ASSERT(pNode != NULL);
    TiXmlElement *pServer = pNode->FirstChildElement();
	std::string strIP = inet_ntoa(pHandleContext->addr.sin_addr);

	while (pServer)
	{
		TiXmlAttribute *pAttr = pServer->FirstAttribute();
		if (pAttr != NULL)
		{  
			CString strDevName = pAttr->Value();	//FingerPrinter
			if (0 == ::SendMessage(m_hMsgWnd, WM_DEVLOGOUT, (WPARAM)strIP.c_str(),
				(LPARAM)&strDevName))
			{
				REPORT(MN, T("没有注册的设备[%s:%s]退出\n", strIP.c_str(), strDevName), RPT_WARNING);
			}  
		}
		//Next server
		pServer = pServer->NextSiblingElement();
	}

	return TRUE;
}

BOOL CIOCPServer::ParseLog(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext)
{
	std::string strIP = inet_ntoa(pHandleContext->addr.sin_addr);
	tagLog* pLog = new tagLog;
	memset(pLog, 0, sizeof(tagLog));

	int nLevel = 0;

	TiXmlAttribute *pAttr = pNode->FirstAttribute();
	while (pAttr)
	{
		std::string strName = pAttr->Name();
		if(strName == "Module")
			strcpy_s(pLog->szDevName, 50, pAttr->Value());
		else if(strName == "Level")
			nLevel = atoi(pAttr->Value());

		pAttr = pAttr->Next();
	}

	strcpy_s(pLog->szIP, 50, strIP.c_str());

	pLog->nLevel = nLevel;
	if ((nLevel & 0xFF) == RPT_DEBUG)
		strcpy_s(pLog->szLogType, 50, "RPT_DEBUG");
	else if ((nLevel & 0xFF) == RPT_INFO)
		strcpy_s(pLog->szLogType, 50, "RPT_INFO");
	else if ((nLevel & 0xFF) == RPT_WARNING)
		strcpy_s(pLog->szLogType, 50, "RPT_WARNING");
	else if ((nLevel & 0xFF) == RPT_ERROR)
		strcpy_s(pLog->szLogType, 50, "RPT_ERROR");
	else if ((nLevel & 0xFF) == RPT_CRITICAL)
		strcpy_s(pLog->szLogType, 50, "RPT_CRITICAL");
	else if ((nLevel & 0xFF) == RPT_HEARTBEAT)
		strcpy_s(pLog->szLogType, 50, "RPT_HEARTBEAT");
	else if ((nLevel & 0xFF) == RPT_ADDI_INFO)
		strcpy_s(pLog->szLogType, 50, "RPT_ADDI_INFO");
	else if ((nLevel & 0xFF) == RPT_VERSION)
		strcpy_s(pLog->szLogType, 50, "RPT_VERSION");
	else
		strcpy_s(pLog->szLogType, 50, "LT_UNKNOWN");

	TiXmlNode* node = pNode->FirstChild();
	ASSERT(node != NULL);
	
	CString strMsg = node->Value();
	CString strTime = strMsg.Left(strMsg.Find('['));
	strcpy_s(pLog->szTime, 50, strTime);

	CString strModule = strMsg.Mid(strMsg.Find('[')+1, strMsg.Find(']') - strMsg.Find('[') - 1);
	strcpy_s(pLog->szModule, 50, strModule);

	CString strThradID = strMsg.Mid(strMsg.Find(']')+1, strMsg.Find('@') - strMsg.Find(']') - 1); 
	pLog->dwThreadID = atoi(strThradID);

	strMsg = strMsg.Right(strMsg.GetLength() - strMsg.ReverseFind('@'));
	CString strPos = strMsg.Mid(strMsg.Find('@')+1, strMsg.Find(':') - strMsg.Find('@') - 1); 
	strcpy_s(pLog->szLogPos, 50, strPos);

	strMsg = strMsg.Right(strMsg.GetLength() 
		- strMsg.Find(':') - 1);
	strncpy_s(pLog->szMsg, 1024, strMsg, strMsg.GetLength() < 1023 ? strMsg.GetLength() : 1023); //截掉超过1024的字符串

	strcpy_s(pLog->szDevServIP, 50, inet_ntoa(pHandleContext->addr.sin_addr));

	theApp.AddLog(nLevel, pLog);
	return TRUE;
}

BOOL CIOCPServer::ParseXmlData(TiXmlElement *pNode, LPHANDLE_CONTEXT pHandleContext)
{
	std::string strIP = inet_ntoa(pHandleContext->addr.sin_addr);

	ASSERT(pNode != NULL);
    TiXmlAttribute *pAttr = pNode->FirstAttribute();
	std::string strDevName;
	if (pAttr != NULL)
		strDevName = pAttr->Value();

	pAttr = pAttr->Next();
	int nDataSize = 0;
	if (pAttr != NULL)
		nDataSize = atoi(pAttr->Value());

	if (nDataSize == 0)
	{
		REPORT(MN, T("下载xml配置文件失败\n"), RPT_WARNING);
		AfxMessageBox("下载xml配置文件失败", MB_OK);
		return TRUE;
	}

	pAttr = pAttr->Next();
	std::string strXmlName;
	if (pAttr != NULL)
		strXmlName = pAttr->Value();

	TiXmlNode* node = pNode->FirstChild();
	std::string strXmlData;
	if (node != NULL)
		strXmlData = node->Value();	

	char* pDataBuf = new char[(nDataSize-1)/2+30];
	memset(pDataBuf, 0, (nDataSize-1)/2+30);
	ChangeBinStringToString(pDataBuf, strXmlData.c_str(), nDataSize-1);
	
	std::string strXmlPath = CFilePath::GetExeFilePath();
	strXmlPath += "\\Xml\\" + strDevName + strIP;
	if (!CFilePath::IsPathExist(strXmlPath.c_str()))
	{
		CValidFunction::CreatePath(strXmlPath.c_str());
	}
	strXmlPath += "\\" + strXmlName;

	theApp.DownloadXml(strXmlPath.c_str(), pDataBuf, (nDataSize-1)/2);
	delete[]pDataBuf;	

	return TRUE;
}

void CIOCPServer::ChangeBinStringToString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize)
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


