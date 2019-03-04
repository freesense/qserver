/************************************************************************************
*
* �ļ���  : KillProcess.h
*
* �ļ�����: ����ָ������
*
* ������  : dingjing, 2009-02-10
*
* �汾��  : 1.0
*
* �޸ļ�¼:
*
************************************************************************************/
#ifndef _KILLPROCESS_H_
#define _KILLPROCESS_H_
#include <windows.h>
#include <tlhelp32.h>


//������־�ļ���
#ifndef	LT_DEBUG
#define LT_DEBUG		0x10
#endif

#ifndef	LT_INFO
#define LT_INFO		    0x20
#endif

#ifndef	LT_WARNING
#define LT_WARNING		0x30
#endif

#ifndef	LT_ERROR
#define LT_ERROR		0x40
#endif
/////////////////////////////////////////////////////////////////////////////////////
class CKillProcess
{
public:
	CKillProcess();
	virtual ~CKillProcess();

	/********************************************************************************
	* ������: KillProc
	* ��  ��:
	*		  DWORD dwProcessID [IN] : ����ID
	*         LPCSTR lpExeFile  [IN] : ��������ƣ�����.exe�� ����test.exe
	* ��  ��: ����ָ������
	* ����ֵ: ���ӳɹ�����TRUE, ����FALSE
	********************************************************************************/
	BOOL KillProc(DWORD dwProcessID, LPCSTR lpExeFile = NULL);

protected:
	/********************************************************************************
	* ������: LogEvent
	* ��  ��:
	*		  WORD nLogType [IN] : ��־������
	*                              LT_DEBUG   : ��ʾ�ǵ�����Ϣ
	*						       LT_INFO    : ��ʾ����ʾ��Ϣ
	*							   LT_WARNING : ��ʾ�Ǿ�����Ϣ
	*                              LT_ERROR   : ��ʾ�Ǵ�����Ϣ
	* ��  ��: ������־
	********************************************************************************/
	virtual void LogEvent(WORD nLogType, LPCSTR pFormat, ...);

private:
};

/////////////////////////////////////////////////////////////////////////////////////
#endif