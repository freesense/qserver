//ThreadEx.h 头文件
//

#pragma once

////////////////////////////////////////////////////////////////////////

class CThreadEx
{
public:
	CThreadEx();
	virtual ~CThreadEx();

	BOOL Start();
	virtual void Run();
	virtual void Stop();

protected:
	HANDLE        m_hThread;   //线程句柄
	BOOL          m_bRun;

	static UINT WINAPI _ThreadProc(LPVOID pParam);
};