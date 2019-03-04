#pragma once
class CompletionPortModel;

class CClientSocket
{
private:
	char   m_pHost[128];
	unsigned int m_unPort;
	SOCKET	m_sToServer;
	HANDLE m_handleThread;
	CompletionPortModel * m_pIoModel;
public:
	CClientSocket(void);
	~CClientSocket(void);

	void SetServer(char * c_pHost,unsigned int c_unPort);
	void SetIoModel(CompletionPortModel * c_pIoModel);
	void CreateLink();
	bool IsValidSocket();
	void CreateMonThread();
	void OnSocketClose(SOCKET c_s);
};
