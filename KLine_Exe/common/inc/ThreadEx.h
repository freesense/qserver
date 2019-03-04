//ThreadEx.h ͷ�ļ�
//
#ifndef _THREADEX_H_
#define _THREADEX_H_
////////////////////////////////////////////////////////////////////////

class CThreadEx
{
public:
	CThreadEx();
	virtual ~CThreadEx();

	BOOL Start();
	virtual void Stop();
	void WaitThread();
protected:
	HANDLE        m_hThread;   //�߳̾��
	BOOL          m_bRun;

	virtual void Run();
	static UINT WINAPI _ThreadProc(LPVOID pParam);
	BOOL WaitThreadExitEx(HANDLE handle);
};

////////////////////////////////////////////////////////////////////////
#endif