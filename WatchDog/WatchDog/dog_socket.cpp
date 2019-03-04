
#include "stdafx.h"
#include "dog_socket.h"
#include "DogService.h"
#include "../../public/auth/auth_struct.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////
ICommX* CDogListener::handle_accept(const char *lpLocalAddr, const char *lpPeerAddr)
{
	CDogSocket *pHandle = new CDogSocket(this);
	CHECK_RUN(!pHandle, MODULE_NAME, T("分配内存失败[%d]\n", sizeof(CDogSocket)), RPT_CRITICAL, return NULL);
	WGUARD(LockSingle, m_lock, grd);
	m_setHandle.insert(pHandle);
	return pHandle;
}

CDogListener::~CDogListener()
{
	WGUARD(LockSingle, m_lock, grd);
	for (std::set<CDogSocket*>::iterator iter = m_setHandle.begin(); iter != m_setHandle.end(); ++iter)
		(*iter)->Close();

	//todo: 在这里等候所有连接对象析构
	Sleep(500);
}

void CDogListener::OnSocketClosed(CDogSocket *pHandle)
{
	WGUARD(LockSingle, m_lock, grd);
	m_setHandle.erase(pHandle);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
CDogSocket::CDogSocket(CDogListener* pServer)
: m_ptrServer(pServer)
{
	memset(m_lpFingerPrinter, 0x00, sizeof(m_lpFingerPrinter));
}

char* CDogSocket::GetRecvBuffer(int readID)
{
	if (readID == PART_HEAD)
		return m_szBuffer;
	else
		return m_szBuffer+sizeof(CommxHead);
}

unsigned int CDogSocket::GetRecvBufSize(int readID)
{
	if (readID == PART_HEAD)
		return sizeof(CommxHead);
	else
		return sizeof(m_szBuffer)-sizeof(CommxHead);
}

int CDogSocket::handle_open(char *lpInitialData, unsigned int nInitialSize)
{
	m_part = PART_HEAD;
	m_nHeartbeatCount = 0;
	m_timer.m_pCallback = this;
	m_timer.m_nOldHeartbeat = -1;
	REPORT(MODULE_NAME, T("设备[%s]已连接\n", GetPeerAddress()), RPT_INFO);

	m_timer.m_nTimerID = RegisterTimer(&m_timer, TIME_OUT*2, TIME_OUT*2);
	m_timer.m_nOldHeartbeat = m_nHeartbeatCount;

	CHECK_RUN(-1 == Read(m_part, sizeof(CommxHead)), MODULE_NAME, T("设备[%s]读取操作失败\n", GetPeerAddress()), RPT_ERROR, return -1);
	return MAX_DOGBUF;
}

int CDogSocket::CheckPackageType(char *lpData, unsigned int nLength)
{
	TiXmlDocument doc;
	doc.Parse(lpData);
	TiXmlElement *pNode = doc.RootElement();
	if (NULL == pNode)
		return 0;

	if (!strcmp(pNode->Value(), "HeartBeat"))//心跳包直接返回
	{
		m_nHeartbeatCount++;
		return 0;
	}
	else if (!strcmp(pNode->Value(), "ServerLogin"))
	{
		TiXmlElement *pServer = pNode->FirstChildElement();
		if(!pServer)
			return 0;
		TiXmlAttribute *pAttr = pServer->FirstAttribute();	   //add OnServerLogin
		if (pAttr)
		{  
			memset(m_lpFingerPrinter, 0x00, sizeof(m_lpFingerPrinter));
			strcpy(m_lpFingerPrinter, pAttr->Value());

			TiXmlNode* node = pServer->FirstChild();

			std::string strProID;
			if (node != NULL)
				strProID = node->Value();
			m_nProID = atoi(strProID.c_str());

			AddDev(m_lpFingerPrinter, m_nProID);  //更新设备列表
			SendDevList();						  //发送设备列表
			return 0;
		}
	}
	return 1;
}

int CDogSocket::handle_read(char *lpBuffer, unsigned int nLength)
{
	CommxHead *pHead = (CommxHead*)m_szBuffer;
	if (m_part == PART_HEAD)
	{
		CHECK_RUN(-1 == Read((m_part = PART_BODY), pHead->Length), MODULE_NAME,
			T("设备[%s]读取操作失败\n", GetPeerAddress()), RPT_ERROR, return -1);
	}
	else
	{
		if (CheckPackageType(lpBuffer, pHead->Length))
			g_pAuthProxy->OnTransmit(m_szBuffer, pHead->Length+sizeof(CommxHead));

		CHECK_RUN(-1 == Read((m_part = PART_HEAD), sizeof(CommxHead)), MODULE_NAME,
			T("设备[%s]读取操作失败\n", GetPeerAddress()), RPT_ERROR, return -1);
	}
	return 0;
}

int CDogSocket::handle_timeout(ITimerCallbackBase *pTimerParam)
{
	if (m_timer.m_nOldHeartbeat == m_nHeartbeatCount)
		REPORT_RUN(MODULE_NAME, T("设备[%s]超时无心跳\n", GetPeerAddress()), RPT_WARNING, Close());
	m_timer.m_nOldHeartbeat = m_nHeartbeatCount;
	return 0;
}

int CDogSocket::handle_close(int iError)
{
	CancelTimer(m_timer.m_nTimerID);
	m_timer.m_nTimerID = 0;
	REPORT(MODULE_NAME, T("设备[%s]连接断开[%d]\n", GetPeerAddress(), iError), RPT_ERROR);
	return ICommX::handle_close(iError);
}

void CDogSocket::handle_release()   //add OnServerLogin()
{
	REPORT(MODULE_NAME, T("设备[%s]进程ID[%d]从设备列表移除\n", m_lpFingerPrinter, m_nProID), RPT_INFO);
	DelDev(m_nProID);  //更新设备列表
	SendDevList();			    //发送设备列表

	m_ptrServer->OnSocketClosed(this);
	delete this;
}
