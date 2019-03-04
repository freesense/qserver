//FTPTranData.h
//

#ifndef _FTPTRANDATA_H_
#define _FTPTRANDATA_H_
#include <afxinet.h>
//////////////////////////////////////////////////////////////////////////
class CFTPTranData
{
public:
	CFTPTranData();
	virtual ~CFTPTranData();

	BOOL Open( LPCTSTR pstrServer, LPCTSTR pstrUserName, LPCTSTR pstrPassword);
	void Close();

	BOOL PutFile(LPCTSTR pstrLocalFile, LPCTSTR pstrRemoteFile);
	BOOL GetFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile);
protected:
	CInternetSession        m_interSession;
	CFtpConnection*         m_pFtpCon;

};






//////////////////////////////////////////////////////////////////////////
#endif