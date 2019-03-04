
#ifndef __SOCKETX_GPX_H__
#define __SOCKETX_GPX_H__

#include <set>
#include "../../public/commx/synch.h"
#include "../../public/commx/commxbase.h"
#include "../../public/protocol.h"

#define HTTP_MAX_LENGTH 1024
#define ANSWER_LENGTH 400 * 1024

//////////////////////////////////////////////////////////////////////////////////
class C2cs;

class C2cc : public ICommX
{
	struct _data_head
	{
		unsigned short ns[4];
		char buf32[32];
	};

public:
	C2cc(C2cs *pcs);
	virtual ~C2cc();

	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);

	virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);
	virtual int handle_read(char *lpBuffer, unsigned int nLength);
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);
	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);

	virtual int handle_close(int iError);
	virtual void handle_release();

	void Answer(char *lpData, unsigned int nLength, unsigned short serialno);

private:
	const char* getMarket(unsigned short mcode);
	unsigned short GetCodeMarketType(char* pSymbol, SRequestData* pstRequestData);
	void GetField(unsigned short m_usType, char* pRecvBuf);
	void GetSymbol(unsigned short m_usType, char* pRecvBuf);
	void GetSymbol(SCodeInfo* pstCodeInfo, char* pSymbol);

	bool OnRealMinsData(SMRequestData* pstMRequestData, unsigned char* pucData);
	char* AtRealMinsData(char *lpData, unsigned int &nLen, unsigned short serialno);
	
	bool OnReportData(SMRequestData* pstMRequestData, unsigned char* pucData);
	char* AtReportData(char *lpData, unsigned int &nLen, unsigned short serialno);
	char* AtReportStatData(char *lpData, unsigned int &nLen, unsigned short serialno);
	
	bool OnHisKData(SMRequestData* pstMRequestData, unsigned char* pucData);
	char* AtHisKData(char *lpData, unsigned int &nLen, unsigned short serialno);
	
	bool OnSortData(SMRequestData* pstMRequestData, unsigned char* pucData);
	char* AtSortData(char *lpData, unsigned int &nLen, unsigned short serialno);
	
	bool OnStatData(SMRequestData* pstMRequestData, unsigned char* pucData);
	char* AtStatData(char *lpData, unsigned int &nLen, unsigned short serialno);

	bool OnRealMinsOverlapData(SMRequestData* pstMRequestData, unsigned char* pucData);
	char* AtRealMinsOverlapData(char *lpData, unsigned int &nLen, unsigned short serialno);

	void MakeStatData(char* pBuf, unsigned int& nLen, SRequestData* pstRequestData, tagQuotedPrice* pQuotePrice);
	void MakeOverlapIndex(char* pBuf, unsigned int& nLen, unsigned int nRecords, char* pMinsBuf, 
		SRequestData* pstRequestData, tagQuotedPrice* pQuotePrice);
	
	bool GetInitInfo(SMRequestData* pstMRequestData);
	void SendErrorMsg(char* pMsg);

	bool IsSpell(char* pStock);
	bool SendCodeList(char* pCode, int nCount, unsigned char* pucData);
	void GetMktType(char* pSymbol, unsigned short& nMktType);

	char m_szHttpHead[HTTP_MAX_LENGTH];
	struct _old_head
	{
		unsigned int nSize;
		unsigned int reserved;
	} m_oldHead;
	char *m_lpBuffer;
	int m_nReadID;

	C2cs *m_pcs;
	unsigned short	  m_usType;        //请求类型
	unsigned char*	  m_pucMRequestData;//用来存放请求数据
	char              m_szHead[512];
	char              m_szCodeLstBuf[1024];
};

//////////////////////////////////////////////////////////////////////////////////
class C2cs : public ICommX
{
public:
	C2cs();
	~C2cs();

	virtual ICommX* handle_accept(const char *lpLocalAddr, const char *lpPeerAddr);
	virtual int handle_close(int iError);
	virtual void handle_release();

	void OnClientDisconnect(C2cc *pClient);

private:
	LockRW m_lock;
	std::set<C2cc*> m_clients;

	HANDLE m_hEvent;
	atomint m_nClients;
};

//////////////////////////////////////////////////////////////////////////////////
class C2q : public ICommX
{
public:
	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);

	virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);
	virtual int handle_read(char *lpBuffer, unsigned int nLength);
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);
	virtual int handle_timeout(ITimerCallbackBase *pTimerParam);

	virtual int handle_close(int iError);
	virtual void handle_release();

private:
	CommxHead m_head;
	char m_szBuffer[ANSWER_LENGTH];
	int m_nReadID;

	ITimerCallbackBase  m_TimerParam;
};

extern C2cs *g_pcs;
extern C2q *g_pcq;

#endif
