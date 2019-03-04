//UseLogData.h
#ifndef _USELOGDATA_H_
#define _USELOGDATA_H_

#include "feedowner.h"
#include "WorkThread.h"
////////////////////////////////////////////////////////////////////////
class CUseLogData
{
public:
	CUseLogData(CFeedOwner *pServer);
	virtual ~CUseLogData();

	BOOL Init();
	void Release();

protected:
	static unsigned int _stdcall _WorkThread(void* pParam);
	void WorkThread();
	void ReadSingleFile(string strFilePath);
	void ListAllFile(string strPath);
private:
	CFeedOwner*   m_pServer;
	CWorkThread*  m_pWorkThread;
	BOOL          m_bRun;
	unsigned int  m_bPrevTime; 
	HANDLE        m_hEvent;
};
///////////////////////////////////////////////////////////////////////
#endif