//ThreadEx.h ͷ�ļ�
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
	HANDLE        m_hThread;   //�߳̾��
	BOOL          m_bRun;

	static UINT WINAPI _ThreadProc(LPVOID pParam);
};