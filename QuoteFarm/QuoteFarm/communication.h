
#ifndef __QUOTE_FARM_COMMUNICATION_H__
#define __QUOTE_FARM_COMMUNICATION_H__

#include <list>
#include <string>
#include "farm.h"
#include "../../public/data_struct.h"
#include "../../public/commx/commxbase.h"
#include "../../public/protocol.h"

class CSharedServer;

template <class T> class write_failed : public std::unary_function<T, bool> 
{
public:
	write_failed(const char *pBuf, unsigned int nLen) : m_pBuf(pBuf), m_nLen(nLen) {}
	bool operator() (T& val)
	{
		CSharedService *pService = (CSharedService*)val;
		if (-1 != pService->Write((char*)(m_pBuf + sizeof(long)), m_nLen - sizeof(long)))
			return false;

		pService->Close();
		return true;
	}

private:
	const char *m_pBuf;
	unsigned int m_nLen;
};

class CFeedSocket : public ICommX
{
public:
	CFeedSocket(const char *lpFeedName);

	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);

	virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);
	virtual int handle_read(char *lpBuffer, unsigned int nLength);
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);
	virtual int handle_close(int iError);
	virtual void handle_release();

	int Connect(const char *lpszAddress = 0, const char *lpSendBuffer = 0, unsigned int nSendLen = 0);

private:
	CommxHead m_head;
	char *m_lpBuffer;
	unsigned int m_nLength;
	unsigned short m_nSerialNo;			/** 接收的Feed包的序列号 [7/10/2009 xinl] */

	std::string m_strFeedName;
	std::string m_strPeerAddress;
	std::string m_strInitSend;
};

class CSharedService : public ICommX
{
	struct _check_protocol
	{
		CommxHead head;
		unsigned short business;
		char symbol[15];
	};

	struct _check_query
	{
		int cast(std::string *lpSymbol, RINDEX *pri, bool bAdd);

		CDataFarm *m_pFarm;
		unsigned short business;
		char *lpAnswerBuffer;
		unsigned int nAnswerLength;
	};

public:
	virtual char* GetRecvBuffer(int readID);
	virtual unsigned int GetRecvBufSize(int readID);
	virtual int handle_open(char *lpInitialData, unsigned int nInitialSize);
	virtual void handle_release();
	virtual int handle_read(char *lpBuffer, unsigned int nLength);
	virtual int handle_write(char *lpData, unsigned int nLength, bool bSendOK);

	CSharedServer *m_pServer;

private:
	_check_protocol m_protocol;
};

class CSharedServer : public ICommX
{
public:
	virtual ICommX* handle_accept(const char *lpLocalAddr, const char *lpPeerAddr);

	CDataFarm *m_pFarm;

private:
	LockSingle m_lock;
	std::list<ICommX*> m_Services;
};

#endif
