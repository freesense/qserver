
#include "stdafx.h"
#include "feedservice.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////////////////////
void CFeedServer::Close()
{
	WGUARD(LockRW, m_lockFeedService, grd);
	for (std::set<CFeedService*>::iterator iter = m_setFeedService.begin(); iter != m_setFeedService.end(); ++iter)
		(*iter)->Close();
	ICommX::Close();
}

ICommX* CFeedServer::handle_accept(const char *lpLocalAddr, const char *lpPeerAddr)
{
	WGUARD(LockRW, m_lockFeedService, grd);
	if (m_setFeedService.size() >= g_cfg.m_nServiceCount)
		REPORT_RUN(MN, T("达到服务上限，不能再提供额外的行情数据服务\n"), RPT_INFO, return NULL);

	CFeedService *pService = new CFeedService(this);
	if (!pService)
		REPORT_RUN(MN, T("分配内存失败\n"), RPT_CRITICAL, return NULL);

	m_setFeedService.insert(pService);
	return pService;
}

void CFeedServer::SendQuote(CFeedService *pService, void *lpData, unsigned int nLen)
{
	RGUARD(LockRW, m_lockFeedService, grd);

	unsigned long nRefCount = 0;  //add by dj 如果没有一个连接可以发送，则释放内存
	if (!pService)
	{//广播数据
		nRefCount = (unsigned long)m_setFeedService.size();
		memcpy(lpData, &nRefCount, sizeof(nRefCount));
		for (std::set<CFeedService*>::iterator iter = m_setFeedService.begin(); iter != m_setFeedService.end(); ++iter)
			(*iter)->SendQuote((char*)lpData + sizeof(nRefCount), nLen);
	}
	else if (m_setFeedService.find(pService) != m_setFeedService.end())
	{//定点数据
		nRefCount = 1;
		memcpy(lpData, &nRefCount, sizeof(nRefCount));
		pService->SendQuote((char*)lpData + sizeof(nRefCount), nLen);
	}

	if (nRefCount == 0)	//add by dj
		mpdel(lpData);
}

void CFeedServer::OnDataWrited(char *lpData, unsigned int nLength)
{
	if (0 == ::InterlockedDecrement((long*)(lpData - sizeof(unsigned long))))
		mpdel(lpData - sizeof(unsigned long));
}

void CFeedServer::OnServiceClosed(CFeedService *pService)
{
	WGUARD(LockRW, m_lockFeedService, grd);
	m_setFeedService.erase(pService);
}

//////////////////////////////////////////////////////////////////////////////////////////
CFeedService::CFeedService(CFeedServer *pServer)
	: m_pServer(pServer), m_nSendBytes(0), m_nSerialNo(0), m_bRecvStep(false)
{
	memset(&m_recvHead, 0x00, sizeof(m_recvHead));
	memset(m_szRecvData, 0x00, sizeof(m_szRecvData));
}

char* CFeedService::GetRecvBuffer(int readID)
{
	if (m_recvHead.GetVersion() == 0x00)
		return (char*)&m_recvHead;
	else
		return m_szRecvData;
}

unsigned int CFeedService::GetRecvBufSize(int readID)
{
	if (m_recvHead.GetVersion() == 0x00)
		return sizeof(m_recvHead);
	else
		return MAX_SERVICE_LENGTH;
}

int CFeedService::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	int iRecv = Read(0, sizeof(CommxHead));
	REPORT_RUN(MN, T("%s已经连接\n", GetPeerAddress()), RPT_INFO, return iRecv == 0 ? MAX_SERVICE_LENGTH + sizeof(CommxHead) : -1);
}

int CFeedService::handle_read(char *lpBuffer, unsigned int nLength)
{
	m_bRecvStep = !m_bRecvStep;
	if (m_bRecvStep)
		return Read(0, m_recvHead.Length);
	else										
	{	/*
		unsigned short *pFuncNo = (unsigned short*)lpBuffer;
		if (*pFuncNo == 0)
		{
			tm tmCur;
			localtime_s(&tmCur, (time_t*)(&m_szRecvData[sizeof(unsigned short)]));
			REPORT(MN, T("心跳包 时间-%d:%d:%d\n", tmCur.tm_hour, tmCur.tm_min, tmCur.tm_sec), RPT_INFO);
		}
		//*/
		//处理收到的行情服务器业务
		return Read(0, sizeof(CommxHead));
	}
}

int CFeedService::handle_write(char *lpData, unsigned int nLength, bool bSendOK)
{
	::EnterCriticalSection(&g_csCount);
	g_nSendSuccess += (double)(nLength) / 1024 / 1024;
	::LeaveCriticalSection(&g_csCount);
	//REPORT(MN, T("handle_write: %d\n", nLength), RPT_INFO);
	m_nSendBytes += nLength;
	if (lpData)
		m_pServer->OnDataWrited(lpData, nLength);
	return 0;
}

void CFeedService::SendQuote(void *lpSendData, unsigned int nLen)
{
	//REPORT(MN, T("SendQuote: %d\n", nLen), RPT_INFO);
	assert(lpSendData && nLen);
	CommxHead *pHead = (CommxHead*)lpSendData;
	pHead->SerialNo = m_nSerialNo++;

	unsigned long nValue = CRC32::getCrc32Int((const char*)((char*)lpSendData + sizeof(CommxHead) + sizeof(unsigned long)), 
		pHead->Length - sizeof(unsigned long));
	memcpy((char*)lpSendData + sizeof(CommxHead), &nValue, sizeof(unsigned long));
	int nRet = Write(lpSendData, nLen);
	//REPORT(MN, T("SendQuote Write return: %d\n", nRet), RPT_INFO);
}

int CFeedService::handle_close(int iError)
{
	m_pServer->OnServiceClosed(this);
	REPORT(MN, T("%s已经断开, iError=%d\n", GetPeerAddress(), iError), RPT_INFO);
	return ICommX::handle_close(iError);
}

void CFeedService::handle_release()
{
	delete this;
}
