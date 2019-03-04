
#include "stdafx.h"
#include "async_result.h"
#include "../public/report.h"
#include <MSWSock.h>

#pragma warning(disable:4018)

IAsyncOverlappedResult::IAsyncOverlappedResult()
{
	this->Internal = 0;
	this->InternalHigh = 0;
	this->Offset = 0;
	this->OffsetHigh = 0;
	this->hEvent = 0;
	this->m_pNext = NULL;
}

IAsyncOverlappedResult::~IAsyncOverlappedResult()
{
	if (m_pNext)
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncAcceptResult::CAsyncAcceptResult(unsigned int nInitialSize)
{
	/// todo: new异常
	m_nBufferSize = nInitialSize + 2 * (sizeof(sockaddr_in) + 16);
	m_lpBuffer = new char[m_nBufferSize];
	memset(m_lpBuffer, 0x00, m_nBufferSize);
}

CAsyncAcceptResult::~CAsyncAcceptResult()
{
	::closesocket(m_socket);
	if (m_nBufferSize)
	{
		delete []m_lpBuffer;
		m_lpBuffer = NULL;
	}
}

void CAsyncAcceptResult::complete(ICommX *pCommX, unsigned long nTransCount)
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(m_socket)) == SOCKET_ERROR)
	{
		::closesocket(m_socket);
		pCommX->accept(this);
		::InterlockedDecrement(&pCommX->m_nAsyncCount);
		return;
	}

	SOCKADDR addr_peer, addr_local;
	int sa_len = sizeof(addr_peer);
	getpeername(m_socket, &addr_peer, &sa_len);
	getsockname(m_socket, &addr_local, &sa_len);

	char szLocal[22], szPeer[22];
	ICommX::GetAddrFromSockaddr(szLocal, sizeof(szLocal), &addr_local);
	ICommX::GetAddrFromSockaddr(szPeer, sizeof(szPeer), &addr_peer);
	ICommX *pHandle = pCommX->handle_accept(szLocal, szPeer);

#ifdef _DEBUG
	REPORT(MN, T("远程连接[0x%08x]，本地[%s]-远程[%s]\n", pHandle, szLocal, szPeer), RPT_DEBUG);
#endif

	if (pHandle == NULL)
		::closesocket(m_socket);
	else
	{
		pHandle->m_socket = (UINT)m_socket;
		memcpy(pHandle->m_szLocalAddress, szLocal, sizeof(szLocal));
		memcpy(pHandle->m_szPeerAddress, szPeer, sizeof(szPeer));
		pHandle->m_nAsyncCount = 0;
		::CreateIoCompletionPort((HANDLE)(SOCKET)m_socket, g_hIocp, (ULONG_PTR)pHandle, 0);
		int iMaxRecvSize = pHandle->handle_open(m_lpBuffer, nTransCount);
		if (iMaxRecvSize < 0)
		{
			pHandle->handle_close(-1);
			::closesocket(m_socket);
		}
	}
	pCommX->accept(this);
	::InterlockedDecrement(&pCommX->m_nAsyncCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAsyncConnectResult::complete(ICommX *pCommX, unsigned long nTransCount)
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(m_socket)) == SOCKET_ERROR)
	{
		::closesocket(m_socket);
		::InterlockedDecrement(&pCommX->m_nAsyncCount);
		return;
	}

	SOCKADDR addr_peer, addr_local;
	int sa_len = sizeof(addr_peer);
	getpeername(m_socket, &addr_peer, &sa_len);
	getsockname(m_socket, &addr_local, &sa_len);

	ICommX::GetAddrFromSockaddr(pCommX->m_szLocalAddress, sizeof(pCommX->m_szLocalAddress), &addr_local);
	ICommX::GetAddrFromSockaddr(pCommX->m_szPeerAddress, sizeof(pCommX->m_szPeerAddress), &addr_peer);

#ifdef _DEBUG
	REPORT(MN, T("连接成功: 0x%08x\n", pCommX), RPT_DEBUG);
#endif

	pCommX->m_bRelease = false;
	int iMaxRecvSize = pCommX->handle_open(0, nTransCount);
	if (iMaxRecvSize < 0)
	{
		pCommX->handle_close(-2);
		::closesocket(m_socket);
	}
	::InterlockedDecrement(&pCommX->m_nAsyncCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncReadResult::CAsyncReadResult(const char *lpFileName)
	: m_lpReceiveFlag(0), m_nReceiveSize(0), m_nFlagRepeatTimes(0), m_nReceivedSize(0)
{
	ZeroMemory(&m_wb, sizeof(m_wb));
	if (!lpFileName)
		m_fp = NULL;
	else
	{
		m_fp = fopen(lpFileName, "a+b");
		CHECK(!m_fp, MN, T("打开数据缓冲文件[%s]失败\n", lpFileName), RPT_ERROR);
	}
}

CAsyncReadResult::~CAsyncReadResult()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

char* CAsyncReadResult::memstr(void* buf, unsigned int buflen, void* lpfind, unsigned int findlen, unsigned int nCount)
{
	unsigned int nPos = 0, n = 0;
	while (nPos < buflen)
	{
		char* lpTmp = (char*)memchr(&((char*)buf)[nPos], ((char*)lpfind)[0], buflen - nPos);
		if (!lpTmp)
			return NULL;

		if (0 == memcmp(lpTmp, lpfind, findlen))
		{
			if (++n == nCount)
				return lpTmp;
		}

		nPos = (unsigned int)(lpTmp - (char*)buf + 1);
	}
	return NULL;
}

void CAsyncReadResult::complete(ICommX *pCommX, unsigned long nTransCount)
{
	int result = 0;
	char *lpTmp = NULL;
	if (nTransCount == 0)
	{
		if (0 == pCommX->handle_result(this))
			pCommX->Close();
		return;
	}

	if (m_fp)
	{
		fwrite(m_wb.buf, 1, nTransCount, m_fp);
		fflush(m_fp);
	}
	m_nReceivedSize += nTransCount;

	if (!m_lpReceiveFlag && m_nReceivedSize >= m_nReceiveSize)/// 接收到了足够长度的数据
	{
		m_nReceiveSize = m_nReceiveSize ? m_nReceiveSize : m_nReceivedSize;
		m_nReceivedSize -= m_nReceiveSize;
		result = pCommX->handle_read(m_wb.buf, m_nReceiveSize);
	}
	else if (m_lpReceiveFlag && m_nReceiveSize &&	/// 接收到了特定字符流
		(lpTmp = memstr(m_wb.buf, m_wb.len, m_lpReceiveFlag, m_nReceiveSize, m_nFlagRepeatTimes)) != NULL)
	{
		m_nReceiveSize = (unsigned int)(lpTmp - m_wb.buf) + strlen(m_lpReceiveFlag) * m_nFlagRepeatTimes;
		m_nReceivedSize -= m_nReceiveSize;
		result = pCommX->handle_read(m_wb.buf, m_nReceiveSize);
	}
	else	/// 不符合接收停止条件，继续往下接收，接收条件不变
		pCommX->Read(nTransCount, -1);
	::InterlockedDecrement(&pCommX->m_nAsyncCount);

	/// 删除连接处理器
	if (-1 == result)
		pCommX->Close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAsyncWriteResult::CAsyncWriteResult(const char *lpFileName)
{
	if (!lpFileName)
		m_fp = NULL;
	else
	{
		m_fp = fopen(lpFileName, "a+b");
		CHECK(!m_fp, MN, T("打开数据缓冲文件[%s]失败\n", lpFileName), RPT_ERROR);
	}
}

CAsyncWriteResult::~CAsyncWriteResult()
{
	if (m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

unsigned int CAsyncWriteResult::AddPreSend(void *lpData, unsigned int nLen)
{
	_pre_data pd;
	pd.m_pData = lpData;
	pd.m_nLen = nLen;

	m_lock.lock();
	m_lstData.push_back(pd);
	unsigned int nCount = (unsigned int)m_lstData.size();
	m_lock.unlock();
	return nCount;
}

unsigned int CAsyncWriteResult::RmvSendData(void **ppData)
{
	unsigned int nLen = 0;
	m_lock.lock();
	if (m_lstData.size())
		m_lstData.pop_front();
	if (m_lstData.size())
	{
		_pre_data pd = m_lstData.front();
		*ppData = pd.m_pData;
		nLen = pd.m_nLen;
	}
	m_lock.unlock();
	return nLen;
}

unsigned int CAsyncWriteResult::GetSendData(void **ppData)
{
	unsigned int nLen = 0;
	m_lock.lock();
	if (m_lstData.size())
	{
		_pre_data pd = m_lstData.front();
		*ppData = pd.m_pData;
		nLen = pd.m_nLen;
	}
	m_lock.unlock();
	return nLen;
}

void CAsyncWriteResult::OnClose(ICommX *pCommX)
{
	m_lock.lock();
	for (std::list<_pre_data>::iterator iter = m_lstData.begin(); iter != m_lstData.end(); ++iter)
	{
		_pre_data pd = *iter;
		pCommX->handle_write((char*)pd.m_pData, pd.m_nLen, false);
	}
	m_lstData.clear();
	m_lock.unlock();
}

void CAsyncWriteResult::complete(ICommX *pCommX, unsigned long nTransCount)
{
	char *next_data = NULL;
	unsigned int next_size = 0;

	/** 没有发送完待发数据，但是在完整地发完之后，完成端口会再次回调本接口一次 [7/6/2009 xinl] */
 	if (nTransCount < m_wb.len)
		return;

	if (m_fp)
	{
		fwrite(m_wb.buf, 1, m_wb.len, m_fp);
		fflush(m_fp);
	}

	/** 已经发送完一个完整包 [7/6/2009 xinl] */
	if (-1 == pCommX->handle_write(m_wb.buf, m_wb.len, true))
		pCommX->m_bRelease = true;

	next_size = RmvSendData((void**)&next_data);
	if (next_size > 0)
	{/// 发送下一个包
		m_wb.buf = next_data;
		m_wb.len = next_size;
	}
	else
	{/// 没有待发数据了
		m_wb.buf = NULL;
		m_wb.len = 0;
	}

	pCommX->m_lock_result->lock();
	bool bRelease = pCommX->m_bRelease;
	pCommX->m_lock_result->unlock();
	if (bRelease)
	{
		if (0 == pCommX->handle_result(this))
			pCommX->Close();
		return;
	}

	if (m_wb.buf && m_wb.len)
	{
		DWORD dwSendBytes = 0, dwFlags = 0;
		int iWrite = ::WSASend(pCommX->m_socket, &m_wb, 1, &dwSendBytes, dwFlags, this, NULL);

		if (iWrite == 0)
			::InterlockedIncrement(&pCommX->m_nAsyncCount);
		else
		{
			iWrite = ::WSAGetLastError();
			if (ERROR_IO_PENDING == iWrite)
				::InterlockedIncrement(&pCommX->m_nAsyncCount);
		}
	}

	::InterlockedDecrement(&pCommX->m_nAsyncCount);
}
