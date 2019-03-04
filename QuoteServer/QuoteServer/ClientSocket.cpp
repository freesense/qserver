#include "StdAfx.h"
#include "stdio.h"
#include "iostream"
#include "Iomodel.h"
#include "ClientSocket.h"
#include "..\\..\\public\\commx\\synch.h"
#include "..\\..\\public\\commx\\report.h"

using namespace std;
#define MODULE_NAME "QS"
DWORD __stdcall MonSocketRoutine(LPVOID Param)
{
	CClientSocket* pThis = (CClientSocket*)Param;
	while (true)
	{
		if (!pThis->IsValidSocket())
		{
			//pThis->CreateLink();
			Sleep(30 * 1000);
		}
		Sleep(1000);
	}
}

CClientSocket::CClientSocket(void)
{
	memset(m_pHost,0,sizeof(m_pHost));
	m_sToServer = INVALID_SOCKET;
	m_handleThread = INVALID_HANDLE_VALUE;
}

CClientSocket::~CClientSocket(void)
{
	if (m_handleThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handleThread);
	}
}

void CClientSocket::SetServer(char *c_pHost, unsigned int c_unPort)
{
	strcpy(m_pHost,c_pHost);
	m_unPort = c_unPort;
}

void CClientSocket::CreateLink()
{
	if (strlen(m_pHost) > 0)
	{
		m_sToServer = m_pIoModel->ConnectTo(m_pHost,m_unPort);
		if(m_sToServer != 0xFFFFFFFF)
		{
			REPORT(MODULE_NAME,T("Create Link Success,socket=%d\n",m_sToServer),RPT_DEBUG);
		}
	}
}

bool CClientSocket::IsValidSocket()
{
	if (m_sToServer == INVALID_SOCKET)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CClientSocket::CreateMonThread()
{
	CreateThread(NULL, 0, MonSocketRoutine, (LPVOID)this, 0, NULL);
}

void CClientSocket::OnSocketClose(SOCKET c_s)
{
	if (c_s == m_sToServer)
	{
		m_sToServer = INVALID_SOCKET;
	}
}

void CClientSocket::SetIoModel(CompletionPortModel *c_pIoModel)
{
	m_pIoModel = c_pIoModel;
}