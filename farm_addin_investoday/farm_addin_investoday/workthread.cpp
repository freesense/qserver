
#include "stdafx.h"
#include "workthread.h"
#include "dblib.h"
#include "config.h"

void CWorkThread::close()
{
	::SetEvent(m_hQuit);
}

unsigned int CWorkThread::svc(void* lpParam)
{
	char *lpSql = NULL;
	m_hQuit = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	CHECK_RUN(!m_dbsql.Open(cfg.m_strDbAddress.c_str(), cfg.m_strUser.c_str(), cfg.m_strPassword.c_str()),
		MN, T("Open SqlServer[%s] Failed!\n", cfg.m_strDbAddress.c_str()), RPT_ERROR, return -1);

	while (::WaitForSingleObject(m_hQuit, 0) == WAIT_TIMEOUT)
	{
		m_queue.Pop(lpSql, 200);
		if (!lpSql)
			continue;

		bool bRun = m_dbsql.Run(lpSql);
		CHECK_RUN(!bRun, MN, T("[%d]: %s\n%s\n", m_dbsql.GetErrorCode(), m_dbsql.GetErrorMsg(), lpSql), RPT_ERROR, continue);
		mpdel(lpSql);
		lpSql = NULL;
	}

	m_dbsql.Close();
	return 0;
}

void CWorkThread::AddSql(char *lpSql)
{
	m_queue.Push(lpSql);
}
