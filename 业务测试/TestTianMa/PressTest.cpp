//PressTest.cpp
#include "stdafx.h"
#include "PressTest.h"
#include "TestTianMaDlg.h"
////////////////////////////////////////////////////////////////////////////////////////////////
CPressTest::CPressTest(CWnd* pMainDlg)
{
	m_pMainDlg = pMainDlg;
	m_nPackType = 0;
	m_time = 0;
	m_pRecvBuf = NULL;
	m_nBufLen = 0;
	m_pBufPos = NULL;
}

CPressTest::~CPressTest()
{
	if (m_pRecvBuf != NULL)
	{
		delete []m_pRecvBuf;
		m_pRecvBuf = NULL;
	}
}

void CPressTest::OnConnect()
{
	m_pBufPos = NULL;

	char szBuf[400] = {0};
	int nLen = 0;
	m_nPackType = ((CTestTianMaDlg*)m_pMainDlg)->GetSendBuf(szBuf, nLen);
	if (!Write(szBuf, nLen))
		OnWriteFailed(); 
}

void CPressTest::OnDisconnect()
{
}

void CPressTest::OnWrite()
{
}

void CPressTest::OnRead()
{
	int nRet = 0;
	if (m_pBufPos == NULL)   //包头
	{
		memset(&m_packHead, 0, sizeof(m_packHead));
		nRet = Read((const char*)&m_packHead, sizeof(m_packHead));
		if (nRet != sizeof(CommxHead))
		{
			REPORT(MN, T("Read()返回%d和包头不一致\n", nRet), RPT_ERROR);
			Reconnect();
			return;
		}

		int nDataSize = m_packHead.Length + sizeof(CommxHead);
		if (m_nBufLen < nDataSize)
		{
			if (m_pRecvBuf != NULL)
			{
				delete []m_pRecvBuf;
				m_nBufLen = 0;
			}
			m_pRecvBuf = new char[nDataSize];
			if (m_pRecvBuf == NULL)
			{
				REPORT(MN, T("new m_pRecvBuf failed\n"), RPT_ERROR);
				return;
			}
		}
		
		m_nBufLen = nDataSize;
		memcpy(m_pRecvBuf, &m_packHead, sizeof(CommxHead));
		m_pBufPos = m_pRecvBuf + sizeof(CommxHead);
		m_nLeaveSize = m_packHead.Length;
	}
	else  //包体
	{
		nRet = Read((const char*)m_pBufPos, m_nLeaveSize);
		if (nRet == -1)
		{
			REPORT(MN, T("Read()失败返回-1\n"), RPT_ERROR);
		
			Reconnect();
			return;
		}

		if (nRet != m_nLeaveSize)
		{
			m_nLeaveSize -= nRet;
			m_pBufPos += nRet;
		}
		else
		{
			m_pBufPos = NULL;
			m_nLeaveSize = 0;
	
			tagPackHead* ph = (tagPackHead*)m_pRecvBuf;
			if (ph->nFuncNo != 0)
			{
				((CTestTianMaDlg*)m_pMainDlg)->OnRead(m_pRecvBuf, m_packHead.Length + sizeof(CommxHead), m_nPackType, m_time);
			}
		}
	}  
}

void CPressTest::OnTimeOut()
{ 
}

void CPressTest::OnConnectFailed()
{
}

void CPressTest::OnReconnect()
{
}

void CPressTest::OnReadFailed()
{
}

void CPressTest::OnWriteFailed()
{
}
