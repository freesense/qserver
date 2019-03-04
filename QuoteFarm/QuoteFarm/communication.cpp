
#include "stdafx.h"
#include "task.h"
#include "config.h"
#include "communication.h"
#include "../../public/commx/mery.h"

CFeedSocket::CFeedSocket(const char *lpFeedName)
{
	memset(&m_head, 0x00, sizeof(m_head));
	m_lpBuffer = NULL;
	m_nLength = 0;
	m_nSerialNo = 0;
	m_strFeedName = lpFeedName;
}

char* CFeedSocket::GetRecvBuffer(int readID)
{
	if (m_nLength == 0)
		return (char*)&m_head;
	else
		return m_lpBuffer;
}

unsigned int CFeedSocket::GetRecvBufSize(int readID)
{
	if (m_nLength == 0)
		return sizeof(m_head);
	else
		return m_nLength;
}

int CFeedSocket::Connect(const char *lpszAddress, const char *lpSendBuffer, unsigned int nSendLen)
{
	if (lpszAddress)
		m_strPeerAddress = lpszAddress;
	if (lpSendBuffer)
		m_strInitSend.assign(lpSendBuffer, nSendLen);
	return ICommX::Connect(m_strPeerAddress.c_str(), m_strInitSend.c_str(), (unsigned int)m_strInitSend.length());
}

int CFeedSocket::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	REPORT(MN, T("已连接行情数据服务器%s\n", GetPeerAddress()), RPT_INFO);
	g_cfg.ActivateFeed(m_strFeedName.c_str(), this);
	return Read(0, sizeof(m_head));
}

int CFeedSocket::handle_close(int iError)
{
	REPORT(MN, T("通讯错误[%d]，与行情数据服务器%s的连接断开\n", iError, GetPeerAddress()), RPT_WARNING);
	return ICommX::handle_close(iError);
}

void CFeedSocket::handle_release()
{
	g_cfg.DisableFeed(this);
	if (::WaitForSingleObject(gQuit, 0) == WAIT_OBJECT_0)
		delete this;
	else
		ConnectFeed(m_strFeedName.c_str(), m_strPeerAddress.c_str(), this);
}

int CFeedSocket::handle_read(char *lpBuffer, unsigned int nLength)
{
	if (m_nLength == 0)//包头
	{
		m_nLength = m_head.Length;
		m_lpBuffer = (char*)mpnew(m_nLength);
		CHECK(m_nSerialNo != 0 && (m_head.SerialNo != (m_nSerialNo+1)), MN,
			T("行情源[%s]数据序列不连续[%d-%d]\n", GetPeerAddress(), m_head.SerialNo, m_nSerialNo),
			RPT_WARNING);
		m_nSerialNo = m_head.SerialNo;
		CHECK_RUN(-1 == Read(0, m_nLength), MN, T("读%d数据失败\n", m_nLength), RPT_ERROR, return -1);
	}
	else//行情包体
	{
		unsigned long *pCrc = (unsigned long*)m_lpBuffer;
		unsigned long crc = CRC32::getCrc32Int(m_lpBuffer + sizeof(unsigned long), m_nLength - sizeof(unsigned long));

		if (*pCrc != crc)
		{
			HEXDUMP(MN, m_lpBuffer, m_nLength, T("CRCheck Error: %d\n", m_head.SerialNo), RPT_ERROR);
			mpdel(m_lpBuffer);
		}
		else
		{
			FeedHead *pHead = (FeedHead*)(m_lpBuffer + sizeof(unsigned long));
			gtask.AddFeed(pHead, m_nLength - sizeof(unsigned long));
			g_nFeedRecvLength += (m_nLength + sizeof(m_head));
			g_nFeedPacketCount += 1;
		}

		m_nLength = 0;
		CHECK_RUN(-1 == Read(0, sizeof(m_head)), MN, T("读%d数据失败\n", sizeof(m_head)), RPT_ERROR, return -1);
	}
	return 0;
}

int CFeedSocket::handle_write(char *lpData, unsigned int nLength, bool bSendOK)
{
	mpdel(lpData);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
ICommX* CSharedServer::handle_accept(const char *lpLocalAddr, const char *lpPeerAddr)
{
	CSharedService *pHandler = new CSharedService;
	if (!pHandler)
		REPORT_RUN(MN, T("分配内存[%d]失败\n", sizeof(CSharedService)), RPT_CRITICAL, return NULL);
	pHandler->m_pServer = this;
	WGUARD(LockSingle, m_lock, grd);
	m_Services.push_back(pHandler);
	return pHandler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CSharedService::_check_query::cast(std::string *lpSymbol, RINDEX *pri, bool bAdd)
{
	switch (business)
	{
	case 65000+0x01://quote
		nAnswerLength = sizeof(CommxHead) + sizeof(Quote);
		lpAnswerBuffer = (char*)mpnew(nAnswerLength);
		CHECK_RUN(!lpAnswerBuffer, MN, T("分配内存[%d]失败\n", nAnswerLength), RPT_CRITICAL, return 0);
		memcpy(lpAnswerBuffer + sizeof(CommxHead), m_pFarm->GetQuote(pri->idxQuote), sizeof(Quote) - sizeof(unsigned int) * 4);
		break;
	case 65000+0x02://mink
		{
			nAnswerLength = sizeof(CommxHead) + pri->cntMinK * sizeof(MinUnit);
			lpAnswerBuffer = (char*)mpnew(nAnswerLength);
			CHECK_RUN(!lpAnswerBuffer, MN, T("分配内存[%d]失败\n", nAnswerLength), RPT_CRITICAL, return 0);

			unsigned int d = pri->cntMinK / MINK_PERBLOCK;
			unsigned int v = pri->cntMinK % MINK_PERBLOCK;
			unsigned int idx = pri->idxMinK;
			unsigned int cnt = 0;

			for (unsigned int i = 0; i < d; i++)
			{
				MinBlock *pmb = m_pFarm->GetMinK(idx);
				memcpy(lpAnswerBuffer + sizeof(CommxHead) + cnt * sizeof(MinUnit), pmb->unit, sizeof(MinUnit) * MINK_PERBLOCK);
				cnt += MINK_PERBLOCK;
				idx = pmb->next;
			}

			if (v)
			{
				MinBlock *pmb = m_pFarm->GetMinK(idx);
				memcpy(lpAnswerBuffer + sizeof(CommxHead) + cnt * sizeof(MinUnit), pmb->unit, sizeof(MinUnit) * v);
			}
		}
		break;
	case 65000+0x03://tick
		{
			nAnswerLength = sizeof(CommxHead) + pri->cntTick * sizeof(TickUnit);
			lpAnswerBuffer = (char*)mpnew(nAnswerLength);
			CHECK_RUN(!lpAnswerBuffer, MN, T("分配内存[%d]失败\n", nAnswerLength), RPT_CRITICAL, return 0);

			unsigned int d = pri->cntTick / TICK_PERBLOCK;
			unsigned int v = pri->cntTick % TICK_PERBLOCK;
			unsigned int idx = pri->idxTick;
			unsigned int cnt = 0;

			for (unsigned int i = 0; i < d; i++)
			{
				TickBlock *ptb = m_pFarm->GetTick(idx);
				memcpy(lpAnswerBuffer + sizeof(CommxHead) + cnt * sizeof(TickUnit), ptb->unit, sizeof(TickUnit) * TICK_PERBLOCK);
				cnt += TICK_PERBLOCK;
				idx = ptb->next;
			}

			if (v)
			{
				TickBlock *ptb = m_pFarm->GetTick(idx);
				memcpy(lpAnswerBuffer + sizeof(CommxHead) + cnt * sizeof(TickUnit), ptb->unit, sizeof(TickUnit) * v);
			}
		}
		break;
	default:
		REPORT_RUN(MN, T("非法请求类型:0x%02d\n", business), RPT_WARNING, return 0);
	}
	return 0;
}

char* CSharedService::GetRecvBuffer(int readID)
{
	return (char*)&m_protocol;
}

unsigned int CSharedService::GetRecvBufSize(int readID)
{
	return sizeof(_check_protocol);
}

int CSharedService::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	CHECK_RUN(-1 == Read(0, sizeof(_check_protocol)), MN, T("调度读操作失败\n"), RPT_ERROR, return -1);
	REPORT(MN, T("[%s]已连接到本地的维护端口\n", GetPeerAddress()), RPT_INFO);
	return 0;
}

void CSharedService::handle_release()
{
	REPORT(MN, T("[%s]维护结束\n", GetPeerAddress()), RPT_INFO);
	delete this;
}

int CSharedService::handle_read(char *lpBuffer, unsigned int nLength)
{
	if (m_protocol.head.GetVersion() != 0x03 || m_protocol.head.Length != sizeof(m_protocol) - sizeof(m_protocol.head))
		HEXDUMP_RUN(MN, &m_protocol, sizeof(m_protocol), T("收到来自于[%s]的非法请求\n", GetPeerAddress()), RPT_WARNING, return -1);

	switch (m_protocol.business)
	{
	case 65535:/** 系统退出 [6/11/2009 xinl] */
		::SetEvent(gQuit);
		return -1;
	case 65534:/** 系统自检 [6/11/2009 xinl] */
		m_pServer->m_pFarm->SelfCheck("sz|sh|hk");
		break;
	default:
		break;
	}

	/** 数据检查，三种数据：Quote,Mink,Tick [6/11/2009 xinl] */
	_check_query cq;
	cq.lpAnswerBuffer = NULL;
	cq.nAnswerLength = 0;
	cq.business = m_protocol.business;
	cq.m_pFarm = m_pServer->m_pFarm;
	m_pServer->m_pFarm->m_hashIdx.cast(m_protocol.symbol, cq);

	if (cq.lpAnswerBuffer == NULL)
	{
		REPORT(MN, T("Request from %s: [%s][%d] failed.\n", GetPeerAddress(), m_protocol.symbol, cq.business), RPT_INFO);
		cq.nAnswerLength = sizeof(CommxHead);
		cq.lpAnswerBuffer = (char*)mpnew(cq.nAnswerLength);
		CHECK_RUN(!cq.lpAnswerBuffer, MN, T("分配内存失败\n"), RPT_CRITICAL, return -1);
	}

	CommxHead *pHead = (CommxHead*)cq.lpAnswerBuffer;
	memcpy(pHead, &m_protocol.head, sizeof(CommxHead));
	pHead->Length = cq.nAnswerLength - sizeof(CommxHead);
	return Write(pHead, cq.nAnswerLength);
}

int CSharedService::handle_write(char *lpData, unsigned int nLength, bool bSendOK)
{
	mpdel(lpData);
	return -1;		/** 主动关闭维护连接 [7/6/2009 xinl] */
}
