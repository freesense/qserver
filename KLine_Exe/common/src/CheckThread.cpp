//CheckThread.cpp
//
#include "stdafx.h"
#include "CheckThread.h"
#include "MulFuncList.h"
#include "../../public/CharTranslate.h"
#include "KLine_Exe.h"
#include <atlbase.h>
#include "FilePath.h"
#include <tlhelp32.h>


#define TEXT_COLOR	RGB(255,0,0)
#define BK_COLOR	RGB(255,240,240)
#define IMG_NORMAL  0
#define IMG_ERROR   1

#define FT_DAY      1
#define FT_WEK      2
#define FT_MON      3
#pragma warning(disable:4819)
//////////////////////////////////////////////////////////////////////////

CCheckThread::CCheckThread()
{
	m_hListCtrlWnd = NULL;
	m_hListBoxWnd  = NULL;
	m_nErrCount    = 0;
	m_nCheckedCount= 0;
	m_nCheckedFile = 0;
	m_nErrFile     = 0;
	m_bCheckLastOne = FALSE;
	m_bCheckFileSize = FALSE;
	m_pFtp = NULL;
	m_bRealHQ = FALSE;
}

CCheckThread::~CCheckThread()
{
}

void CCheckThread::SetCtrlWnd(HWND hListCtrlWnd, HWND hListBoxWnd, 
							  HWND hDialogWnd, HWND hListFile)
{
	m_hListCtrlWnd = hListCtrlWnd;
	m_hListBoxWnd  = hListBoxWnd;
	m_hDialogWnd   = hDialogWnd;
	m_hListFileWnd = hListFile;
}

void CCheckThread::Run()
{
	switch(m_nCmd)
	{
	case CMD_CHECK:
		{
			CheckSingleFile();
			break;
		}
	case CMD_RE_CHECK:
		{
			RecheckListCtrlFile();
			break;
		}
	case CMD_SAVE:
		{
			SaveFile();
			break;
		}
	case CMD_CHECK_ALL_FILE:
		{
			m_nCheckedFile = 0;
			m_nErrFile     = 0;
			CheckAllFile(m_strDirectory);
			if (!m_bRun)
				AddLog(-1, FALSE, "��������ļ�����ֹ!");
			AddLog(-1, FALSE, "\n========== �Ѽ���ļ�: %d �����ļ�: %d ==========",
				m_nCheckedFile, m_nErrFile);
			break;
		}
	case CMD_CHECK_ALL_USE_HTTP_DATA:
		{
			AddLog(-1, FALSE, "����ʹ����������У��...");
			CheckAllFileUseHttpData(m_strDirectory);
			if (m_bRun)
				AddLog(-1, FALSE, "ʹ����������У�����");
			else
				AddLog(-1, FALSE, "ʹ����������У����ֹ");
			break;
		}
	case CMD_CHECK_SINGLE_USE_HTTP_DATA:
		{
			CString strFolderName = CValidFunction::GetFileNamePreFolder(m_strFilePath);
			if (strFolderName.CompareNoCase("day") == 0) //ֻ�������ļ�����ֱ�ӵ�����У��
			{
				AddLog(-1, FALSE, "����ʹ����������У���ļ�[%s]...", m_strFilePath);
				CheckSingleFileUseHttpData();
				if (m_bRun)
					AddLog(-1, FALSE, "ʹ����������У�����");
				else
					AddLog(-1, FALSE, "ʹ����������У����ֹ");
			}
			else if (strFolderName.CompareNoCase("wek") == 0)
			{
				AddLog(-1, FALSE, "��������У�Ե������ļ�:[%s]", m_strFilePath);
			}
			else if (strFolderName.CompareNoCase("mon") == 0)
			{
				AddLog(-1, FALSE, "��������У�Ե������ļ�:[%s]", m_strFilePath);
			}
			else
			{
				AddLog(-1, FALSE, "���ɼ����ļ�:[%s]", m_strFilePath);
			}
			
			break;
		}
	case CMD_UNRAR_KFILE:
		{
// 			UnRarCurKLineFile();
			DownLoadHQKLine();
			break;
		}
	case CMD_UPLOAD_HQKFILE:
		{
			UploadHQKLine();
			break;
		}
	case CMD_DOWNLOAD_HQKFILE:
		{
			DownLoadHQKLine();
			break;
		}
	case CMD_CHECK_FILE_SIZE:
		{
			if (m_bRun)
			{
				AddLog(-1, FALSE, "���ڼ���ļ���С...");
				m_nCheckedFile = 0;

				CheckFileSize(m_strCheckSizeFilePath);
				if (m_bRun)
					AddLog(-1, FALSE, "����ļ���С���");
				else
					AddLog(-1, FALSE, "����ļ���С����ֹ");
				AddLog(-1, FALSE, "\n========== �Ѽ���ļ�: %d ==========", m_nCheckedFile);
			}
			m_bCheckFileSize = FALSE;
			break;
		}
	case CMD_RAR_CURKFILE:
		{
// 			RarCurKLineFile();
			break;
		}
	case CMD_WEIGHT:
		{
			::CoInitialize(NULL);
			StartWeightThread();
			::CoUninitialize();
			break;
		}
	case CMD_COMBINATION:
		{
			CombinationDatFile();
			break;
		}
	default:
		{
		}
	}

	m_pFtp = NULL;
	if (m_bRun)
	{
		Sleep(100);
		::SendMessage(m_hDialogWnd, WM_THREADEND, m_nCmd, 0);
	}
}

void CCheckThread::StartUnRarCurKLineFile()
{
	WaitThread();
	Stop();
	m_nCmd = CMD_UNRAR_KFILE;
	Start();
}

void CCheckThread::StartUploadHQKLineFile()
{
	WaitThread();
	Stop();
	m_nCmd = CMD_UPLOAD_HQKFILE;
	Start();
}

void CCheckThread::StartDownloadHQKLineFile()
{
	WaitThread();
	Stop();
	m_nCmd = CMD_DOWNLOAD_HQKFILE;
	Start();
}

void CCheckThread::StopUploadandDownload()
{
	if (m_pFtp != NULL)
		m_pFtp->Close();

	m_pFtp = NULL;
}

// BOOL CCheckThread::UnRarCurKLineFile()
// {
// 	CString strLocalFile = theApp.m_strKFilePath;
// 	if (!CValidFunction::IsPathExist(strLocalFile))
// 	{
// 		AddLog(-1, FALSE, "·��[%s]������", strLocalFile);
// 		if (!CValidFunction::CreatePath(strLocalFile))
// 		{
// 			AddLog(-1, FALSE, "����·��[%s]ʧ��", strLocalFile);
// 			return FALSE;
// 		}
// 		else
// 		{
// 			AddLog(-1, FALSE, "����·��[%s]�ɹ�", strLocalFile);
// 		}
// 	}
// 	strLocalFile += "\\KLine.rar";
// 
// 	AddLog(-1, FALSE, "��������FTPServer[%s]...", theApp.m_strServerAddr);
// 	CFTPTranData ftp;
// 	m_pFtp = &ftp;
// 	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
// 	{
// 		AddLog(-1, FALSE, "����FTPServer[%s]ʧ��", theApp.m_strServerAddr);
// 		return FALSE;
// 	}
// 	AddLog(-1, FALSE, "���ӳɹ������������ļ�...");
// 
// 	CString strFTPPath = theApp.m_strFTPPath;
// 	if (!strFTPPath.IsEmpty())
// 		strFTPPath += '/';
// 
// 	strFTPPath += "KLine.rar";
// 	if (!ftp.GetFile(strFTPPath, strLocalFile))
// 	{
// 		AddLog(-1, FALSE, "�ӷ�����[%s]�����ļ���[%s]ʧ��", strFTPPath, strLocalFile);
// 		ftp.Close();
// 		return FALSE;
// 	}
// 	
// 	ftp.Close();
// 	AddLog(-1, FALSE, "�ӷ�����[%s]�����ļ���[%s]�ɹ�", strFTPPath, strLocalFile);
// 	
// 	Sleep(1000);
// 	///���غ��ѹ�ļ�
// 	CString strRarFile = theApp.m_strKFilePath;
// 	strRarFile += "\\KLine.rar";
// 
// 	if (!CValidFunction::IsFileExist(strRarFile))
// 	{
// 		AddLog(-1, FALSE, "ѹ���ļ�[%s]������", strRarFile);
// 		return FALSE;
// 	}
// 
// 	CString strRarPath;
// 	if (!GetRarSoftWarePath(strRarPath))
// 		return FALSE;
// 
// 	strRarPath += "\\Rar.exe";
// 	CString strPreFolderPath = CValidFunction::GetPathPreFolderPath(theApp.m_strKFilePath);
// 	CString strFormat;
// 	strFormat.Format("x -o+ %s\\KLine.rar %s\\", theApp.m_strKFilePath, strPreFolderPath);
// 
// 	AddLog(-1, FALSE, "���ڽ�ѹk���ļ�[%s\\KLine.rar]...", theApp.m_strKFilePath);
// 	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
// 		AddLog(-1, FALSE, "��ѹʧ��");
// 
// 	KillRarProcess();
// 	AddLog(-1, FALSE, "��ѹ�ɹ�");
// 	//���ؽ�ѹ�ɹ���ɾ���ļ�
// 	if (::DeleteFile(strLocalFile))
// 		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]�ɹ�", strLocalFile);
// 	else
// 		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]ʧ�� code:%d", strLocalFile, GetLastError());
// 	return TRUE;
// }

BOOL CCheckThread::DownLoadHQKLine()
{
// 	string strFolder = CFilePath::GetPathLastFolderName(theApp.m_strKFilePath);
	CString strHQCur = theApp.m_strKFilePath/* + "\\" + strFolder.c_str()*/;

	CString strLocalFile = strHQCur;
	if (!CValidFunction::IsPathExist(strLocalFile))
	{
		AddLog(-1, FALSE, "·��[%s]������", strLocalFile);
		if (!CValidFunction::CreatePath(strLocalFile))
		{
			AddLog(-1, FALSE, "����·��[%s]ʧ��", strLocalFile);
			return FALSE;
		}
		else
		{
			AddLog(-1, FALSE, "����·��[%s]�ɹ�", strLocalFile);
		}
	}
	strLocalFile += "\\HQKLine.rar";

	AddLog(-1, FALSE, "��������FTPServer[%s]...", theApp.m_strServerAddr);
	CFTPTranData ftp;
	m_pFtp = &ftp;
	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
	{
		AddLog(-1, FALSE, "����FTPServer[%s]ʧ��", theApp.m_strServerAddr);
		return FALSE;
	}
	AddLog(-1, FALSE, "���ӳɹ������������ļ�...");

	CString strFTPPath = theApp.m_strHQFTPPath;
	if (!strFTPPath.IsEmpty())
		strFTPPath += '/';

	strFTPPath += "HQKLine.rar";
	if (!ftp.GetFile(strFTPPath, strLocalFile))
	{
		AddLog(-1, FALSE, "�ӷ�����[%s]�����ļ���[%s]ʧ��", strFTPPath, strLocalFile);
		ftp.Close();
		return FALSE;
	}
	
	ftp.Close();
	AddLog(-1, FALSE, "�ӷ�����[%s]�����ļ���[%s]�ɹ�", strFTPPath, strLocalFile);
	
	Sleep(1000);
	///���غ��ѹ�ļ�
	CString strRarFile = strHQCur;
	strRarFile += "\\HQKLine.rar";

	if (!CValidFunction::IsFileExist(strRarFile))
	{
		AddLog(-1, FALSE, "ѹ���ļ�[%s]������", strRarFile);
		return FALSE;
	}

	CString strRarPath;
	if (!GetRarSoftWarePath(strRarPath))
		return FALSE;

	strRarPath += "\\Rar.exe";
	CString strPreFolderPath = strHQCur;
	CString strFormat;
	strFormat.Format("x -o+ %s\\HQKLine.rar %s\\", strHQCur, strPreFolderPath);

	AddLog(-1, FALSE, "���ڽ�ѹk���ļ�[%s\\HQKLine.rar]...", strHQCur);
	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
		AddLog(-1, FALSE, "��ѹʧ��");

	KillRarProcess();
	AddLog(-1, FALSE, "��ѹ�ɹ�");
	//���ؽ�ѹ�ɹ���ɾ���ļ�
	if (::DeleteFile(strLocalFile))
		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]�ɹ�", strLocalFile);
	else
		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]ʧ��", strLocalFile);
	return TRUE;
}

BOOL CCheckThread::GetRarSoftWarePath(CString& strPath)
{
	//rar.exe����һ����
	::CValidFunction::GetWorkPath(strPath);
	/*char chPath[MAX_PATH];
	try
	{
		CRegKey regkey;
		if (regkey.Open(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\winrar.EXE",
			KEY_READ) != ERROR_SUCCESS)
		{
			AddLog(-1, FALSE, "����: ��ȡrar.exe·��ʧ��");
			return FALSE;
		}
		ULONG nSize = MAX_PATH;
		if (regkey.QueryStringValue("Path", chPath, &nSize) != ERROR_SUCCESS)
		{
			AddLog(-1, FALSE, "����: ��ȡrar.exe·��ʧ��");
			return FALSE;
		}
		regkey.Close();
	}
	catch(...)
	{
		AddLog(-1, FALSE, "����: ��ȡrar.exe·��ʧ��");
		return FALSE;
	}

	strPath = chPath;*/
	return TRUE;
}

// BOOL CCheckThread::RarCurKLineFile()
// {
// 	CString strRarPath;
// 	if (!GetRarSoftWarePath(strRarPath))
// 		return FALSE;
// 	m_file.Close();
// 	strRarPath += "\\Rar.exe";
// 	CString strFormat;
// 	if (!CValidFunction::IsPathExist(theApp.m_strKFilePath))
// 	{
// 		AddLog(-1, FALSE, "·��[%s]������", theApp.m_strKFilePath);
// 		return FALSE;
// 	}
// 	strFormat.Format("a -ep1 %s\\KLine.rar %s", theApp.m_strKFilePath, theApp.m_strKFilePath);
// 
// 	AddLog(-1, FALSE, "����ѹ��k���ļ�[%s\\KLine.rar]...", theApp.m_strKFilePath);
// 	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
// 		AddLog(-1, FALSE, "ѹ��ʧ��");
// 
// 	Sleep(1000); //�ȴ��ļ�����
// 	KillRarProcess();
// 
// 	CString strLocalFile = theApp.m_strKFilePath;
// 	strLocalFile += "\\KLine.rar";
// 	if (!CValidFunction::IsFileExist(strLocalFile))
// 	{
// 		AddLog(-1, FALSE, "�ļ�[%s]������, �ϴ�ʧ��", strLocalFile);
// 		return FALSE;
// 	}
// 	else
// 	{
// 		AddLog(-1, FALSE, "ѹ���ļ��ɹ�");
// 	}
// 
// 	AddLog(-1, FALSE, "��������FTPServer[%s]...", theApp.m_strServerAddr);
// 	CFTPTranData ftp;
// 	m_pFtp = &ftp;
// 	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
// 	{
// 		AddLog(-1, FALSE, "����FTPServer[%s]ʧ��", theApp.m_strServerAddr);
// 		return FALSE;
// 	}
// 	AddLog(-1, FALSE, "���ӳɹ��������ϴ��ļ�...");
// 
// 	CString strFTPPath = theApp.m_strFTPPath;
// 	if (!strFTPPath.IsEmpty())
// 		strFTPPath += '/';
// 
// 	strFTPPath += "KLine.rar";
// 	if (!ftp.PutFile(strLocalFile, strFTPPath))
// 	{
// 		AddLog(-1, FALSE, "�ϴ��ļ�[%s]��������[%s]ʧ��", strLocalFile, strFTPPath);
// 		ftp.Close();
// 		return FALSE;
// 	}
// 	
// 	ftp.Close();
// 	AddLog(-1, FALSE, "�ϴ��ļ�[%s]��������[%s]�ɹ�", strLocalFile, strFTPPath);
// 	//�ϴ��ɹ���ɾ���ļ�
// 	Sleep(1000);
// 
// 	if (::DeleteFile(strLocalFile))
// 		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]�ɹ�", strLocalFile);
// 	else
// 		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]ʧ��", strLocalFile);
// 	   
// 	return TRUE;
// }

BOOL CCheckThread::UploadHQKLine()
{
	CString strRarPath;
	if (!GetRarSoftWarePath(strRarPath))
		return FALSE;

	strRarPath += "\\Rar.exe";
	CString strFormat;
// 	string strFolder = CFilePath::GetPathLastFolderName(theApp.m_strKFilePath);

	CString strHQCur = theApp.m_strKFilePath;
	CString strHQRar = theApp.m_strKFilePath + "\\*.k"/*strFolder.c_str()*/;
// 	if (!CValidFunction::IsPathExist(strHQCur))
// 	{
// 		AddLog(-1, FALSE, "·��[%s]������", strHQCur);
// 		return FALSE;
// 	}
	strFormat.Format("a -ep1 %s\\HQKLine.rar \"%s\"", strHQCur, strHQRar);

	AddLog(-1, FALSE, "����ѹ��k���ļ�[%s\\HQKLine.rar]...", strHQCur);
	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
		AddLog(-1, FALSE, "ѹ��ʧ��");

	Sleep(1000); //�ȴ��ļ�����
	KillRarProcess();

	CString strLocalFile = strHQCur;
	strLocalFile += "\\HQKLine.rar";
	if (!CValidFunction::IsFileExist(strLocalFile))
	{
		AddLog(-1, FALSE, "�ļ�[%s]������, �ϴ�ʧ��", strLocalFile);
		return FALSE;
	}
	else
	{
		AddLog(-1, FALSE, "ѹ���ļ��ɹ�");
	}

	AddLog(-1, FALSE, "��������FTPServer[%s]...", theApp.m_strServerAddr);
	CFTPTranData ftp;
	m_pFtp = &ftp;
	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
	{
		AddLog(-1, FALSE, "����FTPServer[%s]ʧ��", theApp.m_strServerAddr);
		return FALSE;
	}
	AddLog(-1, FALSE, "���ӳɹ��������ϴ��ļ�...");

	CString strFTPPath = theApp.m_strHQFTPPath;
	if (!strFTPPath.IsEmpty())
		strFTPPath += '/';

	strFTPPath += "HQKLine.rar";
	if (!ftp.PutFile(strLocalFile, strFTPPath))
	{
		AddLog(-1, FALSE, "�ϴ��ļ�[%s]��������[%s]ʧ��", strLocalFile, strFTPPath);
		ftp.Close();
		return FALSE;
	}
	
	ftp.Close();
	AddLog(-1, FALSE, "�ϴ��ļ�[%s]��������[%s]�ɹ�", strLocalFile, strFTPPath);
	//�ϴ��ɹ���ɾ���ļ�
	Sleep(1000);
	if (::DeleteFile(strLocalFile))
		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]�ɹ�", strLocalFile);
	else
		AddLog(-1, FALSE, "ɾ��ѹ���ļ�[%s]ʧ��", strLocalFile);
	   
	return TRUE;
}

void CCheckThread::CheckWeekFile(CString strFile, BOOL bCheckLastOne)
{
	CString strExe = CValidFunction::GetExeName(strFile);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]!", m_strFilePath);
		return;
	}

	if (!m_file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", strFile);
		AddLog(-1, FALSE, "��ⱻ��ֹ!");
		return;
	}

	tagKLineInfo klInfo = {0};
	tagKLineInfo newInfo = {0};

	CString strDayFile = theApp.m_strKFilePath;
	strDayFile += "\\day\\";
	strDayFile += m_strFileName;
	BOOL bFileChanged = FALSE;
						  
	if (bCheckLastOne)
		m_file.MoveLast();

	while (!m_file.IsEOF())
	{
		if (!m_bRun)
		{
			break;
		}
		m_file.GetRecordToBuffer((char*)&klInfo);
		newInfo.nDate = klInfo.nDate;
		if(GetWeekData(&newInfo, strDayFile))
		{
			//DW("[%d] ����:%d ���:%d ���:%d ����:%d ����:%d ���:%d", klInfo.nDate, newInfo.nOpenPrice, newInfo.nMaxPrice, newInfo.nMinPrice,
			//	newInfo.nClosePrice, newInfo.nVolume, newInfo.nSum);
			BOOL bRet = TRUE;
			if (klInfo.nOpenPrice != newInfo.nOpenPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "����[%d]���̼۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nDate, klInfo.nOpenPrice, newInfo.nOpenPrice);
				AddLog(-1, FALSE, "[%d]���̼۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nOpenPrice);
			}

			if (klInfo.nMaxPrice != newInfo.nMaxPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "����[%d]��߼۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nDate, klInfo.nMaxPrice, newInfo.nMaxPrice);
				AddLog(-1, FALSE, "[%d]��߼۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nMaxPrice);
			}

			if (klInfo.nMinPrice != newInfo.nMinPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "����[%d]��ͼ۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nDate, klInfo.nMinPrice, newInfo.nMinPrice);
				AddLog(-1, FALSE, "[%d]��ͼ۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nMinPrice);
			}

			if (klInfo.nClosePrice != newInfo.nClosePrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "����[%d]���̼۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nDate, klInfo.nClosePrice, newInfo.nClosePrice);
				AddLog(-1, FALSE, "[%d]���̼۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nClosePrice);
			}

			if (klInfo.nVolume != newInfo.nVolume)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "����[%d]�ɽ�����һ��, ԭʼ����:%d �������:%d",
					klInfo.nDate, klInfo.nVolume, newInfo.nVolume);
				AddLog(-1, FALSE, "[%d]�ɽ����޸�Ϊ:%d", klInfo.nDate, newInfo.nVolume);
			}

			if (klInfo.nSum != newInfo.nSum)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "����[%d]�ɽ���һ��, ԭʼ����:%d �������:%d",
					klInfo.nDate, klInfo.nSum, newInfo.nSum);
				AddLog(-1, FALSE, "[%d]�ɽ�����޸�Ϊ:%d", klInfo.nDate, newInfo.nSum);
			}

			if (!bRet) //���ݲ���ȷ
			{
				bFileChanged = TRUE;;
				if (m_file.SetRecordBuffer((const char*)&newInfo))
					AddLog(-1, FALSE, "[%d]�����޸����ݳɹ�", klInfo.nDate);
				else
					AddLog(-1, FALSE, "[%d]�����޸�����ʧ��", klInfo.nDate);
			}
		}
		m_file.MoveNext();
	}

	if (bFileChanged)
	{
		if (!m_file.WriteData())
			AddLog(-1, FALSE, "���󣺱����ļ�[%s]ʧ��", strFile);
		else
			AddLog(-1, FALSE, "�����ļ�[%s]�ɹ�", strFile);
	}
}

void CCheckThread::CheckAllMonFile(CString strPath)
{
	if (strPath.IsEmpty())
		return;

	CString strFolder = strPath;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		if (!m_bRun)
		{
			return;
		}
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			CString strFolderName = ffind.GetFileName();

			if (strFolderName.CompareNoCase("mon") == 0) //ֻ���mon�ļ�
				CheckAllMonFile(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFileName();
			CString strFolder = ffind.GetFilePath();

			m_strFilePath = strFolder;
			m_strFileName = strFileName;
			if (m_hListCtrlWnd != NULL)
			{
				CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListCtrlWnd);
				pCtrl->DeleteAllItems();
			}

			CheckMonFile(strFolder, TRUE);
		}
	}
}

void CCheckThread::CheckMonFile(CString strFile, BOOL bCheckLastOne)
{
	CString strExe = CValidFunction::GetExeName(strFile);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", strFile);
		AddLog(-1, FALSE, "��ⱻ��ֹ!");
		return;
	}

	tagKLineInfo klInfo = {0};
	tagKLineInfo newInfo = {0};

	CString strDayFile = theApp.m_strKFilePath;
	strDayFile += "\\day\\";
	strDayFile += m_strFileName;
	BOOL bFileChanged = FALSE;

	if (bCheckLastOne)
		m_file.MoveLast();
	while (!m_file.IsEOF())
	{
		if (!m_bRun)
		{
			break;
		}
		m_file.GetRecordToBuffer((char*)&klInfo);
		newInfo.nDate = klInfo.nDate;
		if(GetMonData(&newInfo, strDayFile))
		{
			//DW("[%d] ����:%d ���:%d ���:%d ����:%d ����:%d ���:%d", klInfo.nDate, newInfo.nOpenPrice, newInfo.nMaxPrice, newInfo.nMinPrice,
			//	newInfo.nClosePrice, newInfo.nVolume, newInfo.nSum);
			BOOL bRet = TRUE;
			if (klInfo.nOpenPrice != newInfo.nOpenPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "���󣺿��̼۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nOpenPrice, newInfo.nOpenPrice);
				AddLog(-1, FALSE, "[%d]���̼۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nOpenPrice);
			}

			if (klInfo.nMaxPrice != newInfo.nMaxPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "������߼۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nMaxPrice, newInfo.nMaxPrice);
				AddLog(-1, FALSE, "[%d]��߼۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nMaxPrice);
			}

			if (klInfo.nMinPrice != newInfo.nMinPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "������ͼ۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nMinPrice, newInfo.nMinPrice);
				AddLog(-1, FALSE, "[%d]��ͼ۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nMinPrice);
			}

			if (klInfo.nClosePrice != newInfo.nClosePrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "�������̼۸�һ��, ԭʼ����:%d �������:%d",
					klInfo.nClosePrice, newInfo.nClosePrice);
				AddLog(-1, FALSE, "[%d]���̼۸��޸�Ϊ:%d", klInfo.nDate, newInfo.nClosePrice);
			}

			if (klInfo.nVolume != newInfo.nVolume)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "���󣺳ɽ�����һ��, ԭʼ����:%d �������:%d",
					klInfo.nVolume, newInfo.nVolume);
				AddLog(-1, FALSE, "[%d]�ɽ����޸�Ϊ:%d", klInfo.nDate, newInfo.nVolume);
			}

			if (klInfo.nSum != newInfo.nSum)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "���󣺳ɽ���һ��, ԭʼ����:%d �������:%d",
					klInfo.nSum, newInfo.nSum);
				AddLog(-1, FALSE, "[%d]�ɽ�����޸�Ϊ:%d", klInfo.nDate, newInfo.nSum);
			}

			if (!bRet) //���ݲ���ȷ
			{
				bFileChanged = TRUE;;
				m_file.SetRecordBuffer((const char*)&newInfo);
			}
		}
		m_file.MoveNext();
	}

	if (bFileChanged)
	{
		if (!m_file.WriteData())
			AddLog(-1, FALSE, "���󣺱����ļ�[%s]ʧ��", strFile);
		else
			AddLog(-1, FALSE, "�����ļ�[%s]�ɹ�", strFile);
	}
}

BOOL CCheckThread::GetMonData(tagKLineInfo *pklInfo, CString strFile)
{
	CString strExe = CValidFunction::GetExeName(strFile);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]!", m_strFilePath);
		return FALSE;
	}

	CKLineFile  file;
	if (!file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", strFile);
		return FALSE;
	}

	tagKLineInfo curInfo;
	BOOL bFirstData = FALSE;
	int nOldMon = 0;
	while(!file.IsEOF())
	{
		if (!m_bRun)
		{
			file.Close();
			return FALSE;
		}
		file.GetRecordToBuffer((char*)&curInfo);
		if (bFirstData)
		{
			int nNewMon = (curInfo.nDate % 10000) / 100;
			
			if (nOldMon != nNewMon)
				break;

			//DT("curInfo.nClosePrice = %d", curInfo.nClosePrice);
			pklInfo->nClosePrice = curInfo.nClosePrice;
			if (curInfo.nMaxPrice > pklInfo->nMaxPrice)
				pklInfo->nMaxPrice  = curInfo.nMaxPrice;
			if (curInfo.nMinPrice < pklInfo->nMinPrice)
				pklInfo->nMinPrice  = curInfo.nMinPrice;
			pklInfo->nSum += curInfo.nSum;
			pklInfo->nVolume += curInfo.nVolume;
		}

		if (curInfo.nDate == pklInfo->nDate)
		{
			pklInfo->nClosePrice = curInfo.nClosePrice;
			pklInfo->nOpenPrice = curInfo.nOpenPrice;
			pklInfo->nMaxPrice  = curInfo.nMaxPrice;
			pklInfo->nMinPrice  = curInfo.nMinPrice;
			pklInfo->nSum = curInfo.nSum;
			pklInfo->nVolume = curInfo.nVolume;
			nOldMon = (pklInfo->nDate % 10000) / 100;
			bFirstData = TRUE;
		}
		file.MoveNext();
	}

	file.Close();
	if (!bFirstData)
		return FALSE;
	else
		return TRUE;
}

BOOL CCheckThread::GetWeekData(tagKLineInfo *pklInfo, CString strFile)
{
	CString strExe = CValidFunction::GetExeName(strFile);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]!", m_strFilePath);
		return FALSE;
	}
	CKLineFile  file;
	if (!file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", strFile);
		return FALSE;
	}

	tagKLineInfo curInfo;
	BOOL bFirstData = FALSE;
	UINT nPreWek = 0;

	while(!file.IsEOF())
	{
		if (!m_bRun)
		{
			file.Close();
			return FALSE;
		}
		BOOL bRet = file.GetRecordToBuffer((char*)&curInfo);
		if (!bRet)
		{
			AddLog(-1, FALSE, "����: �ļ�[%s]GetRecordToBuffer()ʧ��!", strFile);
			continue;
		}
		
		if (!IsValidDate(curInfo.nDate))
		{
			file.MoveNext();
			continue;
		}

		if (bFirstData)
		{
			UINT nWek = GetDayofWeek(curInfo.nDate);
			if (nWek < nPreWek)
				break;
			nPreWek = nWek;

			pklInfo->nClosePrice = curInfo.nClosePrice;
			if (curInfo.nMaxPrice > pklInfo->nMaxPrice)
				pklInfo->nMaxPrice  = curInfo.nMaxPrice;
			if (curInfo.nMinPrice < pklInfo->nMinPrice)
				pklInfo->nMinPrice  = curInfo.nMinPrice;
			pklInfo->nSum += curInfo.nSum;
			pklInfo->nVolume += curInfo.nVolume;
		}

		if (curInfo.nDate == pklInfo->nDate)
		{
			pklInfo->nClosePrice = curInfo.nClosePrice;
			pklInfo->nOpenPrice = curInfo.nOpenPrice;
			pklInfo->nMaxPrice  = curInfo.nMaxPrice;
			pklInfo->nMinPrice  = curInfo.nMinPrice;
			pklInfo->nSum = curInfo.nSum;
			pklInfo->nVolume = curInfo.nVolume;

			bFirstData = TRUE;
			nPreWek = GetDayofWeek(curInfo.nDate);
		}
		file.MoveNext();
	}

	file.Close();
	if (!bFirstData)
		return FALSE;
	else
		return TRUE;
}

UINT CCheckThread::GetDayofWeek(UINT nDate)
{
	int nYear = nDate / 10000;
	int nMon  = (nDate % 10000) / 100;
	int nDay  = nDate % 100;

	CTime tmTime(nYear, nMon, nDay, 0, 0, 0);
	return tmTime.GetDayOfWeek()-1;
}

void CCheckThread::CheckAllWeekFile(CString strPath)
{
	if (strPath.IsEmpty())
		return;

	CString strFolder = strPath;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		if (!m_bRun)
		{
			return;
		}
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			CString strFolderName = ffind.GetFileName();

			if (strFolderName.CompareNoCase("wek") == 0) //ֻ���wek�ļ�
				CheckAllWeekFile(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFileName();
			CString strFolder = ffind.GetFilePath();

			m_strFilePath = strFolder;
			m_strFileName = strFileName;
			if (m_hListCtrlWnd != NULL)
			{
				CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListCtrlWnd);
				pCtrl->DeleteAllItems();
			}

			CheckWeekFile(strFolder, TRUE);
		}
	}
}

void CCheckThread::CheckFileSize(CString strFile)
{
	if (strFile.IsEmpty())
		return;

	CString strFolder = strFile;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		if (!m_bRun)
		{
			return;
		}
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			CheckFileSize(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFileName();
			CString strFolder = ffind.GetFilePath();
			if (strFolder.Left(strFolder.ReverseFind('\\')) == m_strCheckSizeFilePath)
				continue;
			m_strFilePath = strFolder;
			CutFile(strFolder);
		}
	}
}

void CCheckThread::CutFile(CString strFile)
{
	CString strExe = CValidFunction::GetExeName(strFile);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", m_strFilePath);
		return;
	}
	
	m_nCheckedFile++;  //�Ѽ���ļ�����
	int nRecCount  = m_file.GetRecordCount();
	int nReserveCount = theApp.m_nReserveCount < (UINT)nRecCount ? theApp.m_nReserveCount:nRecCount;
	int nDelCount  = nRecCount - nReserveCount;
	int nRecSize   = m_file.GetRecordSize();
	if (nDelCount == 0)
	{
		m_file.Close();
		return;
	}
	
	if (theApp.m_nDeleteOtiose == 0)   //����Ҫɾ���ļ�¼
	{
		CString strName = CValidFunction::GetFileNamePrePath(strFile);
		CString strHisPath = strFile.Left(strFile.Find(strName) - 1);
		strHisPath = strHisPath.Left(strHisPath.ReverseFind('\\') + 1);
		strHisPath += "history\\";
		CString strHisName = strHisPath + strName;
		strHisPath += CValidFunction::GetFileNamePreFolder(strFile);	
		if (!CValidFunction::IsPathExist(strHisPath))
		{
			CValidFunction::CreatePath(strHisPath);
		}
		
		//AddLog(-1, FALSE, "�������ļ�¼����ʷ�ļ�[%s]!", strHisName);
		CKLineFile kfileHis;
		if (!kfileHis.Open(strHisName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS))
		{
			AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", strHisName);
			m_file.Close();
			return;
		}

		if (!kfileHis.AllocateMemoryForSave(nDelCount))
		{
			m_file.Close();
			kfileHis.Close();
			AddLog(-1, FALSE, "����: ���ļ�[%s]�����ڴ�ʧ��!", strHisName);
			return;
		}

		kfileHis.MoveFirst();
		m_file.MoveFirst();
		char *pRecBuf = new char[nRecSize];
		while(!kfileHis.IsEOF())
		{
			m_file.GetRecordToBuffer(pRecBuf);
			kfileHis.SetRecordBuffer(pRecBuf);
			kfileHis.MoveNext();
			m_file.MoveNext();
		}
		delete []pRecBuf;
		pRecBuf = NULL;

		if (!kfileHis.WriteDataToEnd())
		{
			AddLog(-1, FALSE, "����: �����ļ�[%s]ʧ��!", strHisName);
		}

		kfileHis.Close();
	}
	
	{//ɾ������ļ�¼
		
		//AddLog(-1, FALSE, "[%s]ɾ������ļ�¼!", strFile);
		char* pRecBuf = new char[nRecSize*nReserveCount];
		if (pRecBuf == NULL)
		{
			AddLog(-1, FALSE, "����: �����ڴ�ʧ��!");
			m_file.Close();
			return;
		}
		
		char* pBufPos = pRecBuf + nRecSize*(nReserveCount-1);
		m_file.MoveLast();
		for(int i=0; i<nReserveCount; i++)
		{
			m_file.GetRecordToBuffer(pBufPos);
			m_file.MovePrev();
			pBufPos -= nRecSize;
		}
		
		if (!m_file.AllocateMemoryForSave(nReserveCount))
		{
			AddLog(-1, FALSE, "����: ���ļ�[%s]�����ڴ�ʧ��!", strFile);
			m_file.Close();
			return;
		}

		if (!m_file.SetAllRecordToBuffer(pRecBuf))
		{
			AddLog(-1, FALSE, "����: �����ļ�[%s]ʧ��!", strFile);
			m_file.Close();
			return;
		}
		if (!m_file.WriteData())
		{
			AddLog(-1, FALSE, "����: �����ļ�[%s]ʧ��!", strFile);
		}

		delete []pRecBuf;
		pRecBuf = NULL;
	}
	m_file.Close();
	//////////////////////////////////////
}

void CCheckThread::StartRarUpLoadKFile(BOOL bAuto)
{
	/*if (bAuto)   //�Ƿ��Զ�������ʱ����
	{
		m_bRarUpLoad = TRUE;
	}
	else*/
	{
		WaitThread();
		Stop();
		m_nCmd = CMD_RAR_CURKFILE;
		Start();
	}
}

void CCheckThread::StartCheckFileSize(CString strFilePath)
{
	m_strCheckSizeFilePath = strFilePath;
	WaitThread();
	Stop();
	m_nCmd = CMD_CHECK_FILE_SIZE;
	Start();
}

void CCheckThread::CheckSingleFile(CString strFilePath)
{
	WaitThread();
	Stop();
	m_nCmd = CMD_CHECK;
	m_strFilePath = strFilePath;
	m_bCheckLastOne = FALSE;
	m_strFileName = CValidFunction::GetFileNameFromFilePath(strFilePath);
	Start();
}

void CCheckThread::StartCheckAllFile(CString strDirectory, BOOL bCheckLastOne)
{
	WaitThread();
	Stop();
	m_bCheckLastOne = bCheckLastOne;
	m_nCmd = CMD_CHECK_ALL_FILE;
	m_strDirectory = strDirectory;
	Start();
}

void CCheckThread::StartCheckAllFileUseHttpData(CString strFileDirectory)
{
	WaitThread();
	Stop();
	m_nCmd = CMD_CHECK_ALL_USE_HTTP_DATA;
	m_strDirectory = strFileDirectory;
	m_bCheckLastOne = FALSE;
	Start();
}

void CCheckThread::CheckAllFileUseHttpData(CString strFile)
{
	if (strFile.IsEmpty())
		return;

	CString strFolder = strFile;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		if (!m_bRun)
		{
			return;
		}
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			CString strFolderName = ffind.GetFileName();

			if (strFolderName.CompareNoCase("day") == 0) //ֻ�������ļ�����ֱ�ӵ�����У��
				CheckAllFileUseHttpData(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFileName();
			CString strFolder = ffind.GetFilePath();

			m_strFilePath = strFolder;
			m_strFileName = strFileName;
			if (m_hListCtrlWnd != NULL)
			{
				CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListCtrlWnd);
				pCtrl->DeleteAllItems();
			}
			AddLog(-1, FALSE, "���ڼ���ļ�[%s]", m_strFilePath);
			CheckSingleFileUseHttpData();
		}
	}
}

void CCheckThread::StartCheckSingeleFileUseHttpData()
{
	WaitThread();
	Stop();
	m_nCmd = CMD_CHECK_SINGLE_USE_HTTP_DATA;
	m_bCheckLastOne = FALSE;
	Start();
}

void CCheckThread::CheckSingleFileUseHttpData()
{
	CString strExe = CValidFunction::GetExeName(m_strFilePath);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(m_strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", m_strFilePath);
		AddLog(-1, FALSE, "У�Ա���ֹ!");
		return;
	}
	
	CString strCode   = m_strFileName.Left(m_strFileName.Find('.'));
	CString strMarket = m_strFileName.Right(m_strFileName.GetLength() - 
		m_strFileName.Find('.') - 1);
	CString strURL;
	if (strMarket.CompareNoCase("sz") == 0 
		|| strMarket.CompareNoCase("szidx") == 0 )
	{
		strMarket = "sz";
	}
	else if(strMarket.CompareNoCase("sh") == 0 
		|| strMarket.CompareNoCase("shidx") == 0 )
	{
		strMarket = "sh";
	}
	else
	{
		AddLog(-1, FALSE, "����: ����У�Ե��ļ�[%s]", m_strFileName);
		AddLog(-1, FALSE, "У�Ա���ֹ!");
		m_file.Close();
		return;
	}
	strURL.Format(HTTPFORMAT, strMarket, strCode);
	if(!m_DownHttpData.OpenURL(strURL, strMarket))
	{
		AddLog(-1, FALSE, "����: ����ҳ[%s]ʧ��", strURL);
		AddLog(-1, FALSE, "У�Ա���ֹ!");
		m_file.Close();
		return;
	}

	tagKLineInfo klInfoHttp = {0};
	if (!m_DownHttpData.GetKLineData(&klInfoHttp))
	{
		AddLog(-1, FALSE, "����: ����ҳ��ȡ��Ʊ[%s.%s]��Ϣʧ��", strCode, strMarket);
		AddLog(-1, FALSE, "У�Ա���ֹ!");
		m_file.Close();
		return;
	}

	//��ʼ�������
	tagKLineInfo klInfo;
	m_file.MoveFirst();
	while(!m_file.IsEOF())
	{
		m_file.GetRecordToBuffer((char*)&klInfo);
		if (klInfoHttp.nDate == klInfo.nDate)
			break;
		m_file.MoveNext();
	}

	if (m_file.IsEOF())
	{
		AddLog(-1, FALSE, "û������������ƥ��ļ�¼");
		m_file.Close();
		return;
	}

	BOOL bRet = TRUE;

	if (klInfoHttp.nClosePrice != klInfo.nClosePrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "����[%d]���̼۸�һ�������ϣ�%d ���أ�%d", 
			klInfoHttp.nDate, klInfoHttp.nClosePrice, klInfo.nClosePrice);
		klInfo.nClosePrice = klInfoHttp.nClosePrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]���̼۸��޸�Ϊ%d", klInfoHttp.nDate, klInfoHttp.nClosePrice);
	}
	if (klInfoHttp.nMaxPrice != klInfo.nMaxPrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "����[%d]��߼۲�һ�������ϣ�%d ���أ�%d", 
			klInfoHttp.nDate, klInfoHttp.nMaxPrice, klInfo.nMaxPrice);
		klInfo.nMaxPrice = klInfoHttp.nMaxPrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]��߼��޸�Ϊ%d", klInfoHttp.nDate, klInfoHttp.nMaxPrice);
	}
	if (klInfoHttp.nMinPrice != klInfo.nMinPrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "����[%d]��ͼ۲�һ�������ϣ�%d ���أ�%d", 
			klInfoHttp.nDate, klInfoHttp.nMinPrice, klInfo.nMinPrice);
		klInfo.nMinPrice = klInfoHttp.nMinPrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]��ͼ��޸�Ϊ%d", klInfoHttp.nDate, klInfoHttp.nMinPrice);
	}
	if (klInfoHttp.nOpenPrice != klInfo.nOpenPrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "����[%d]���̼۲�һ�������ϣ�%d ���أ�%d", 
			klInfoHttp.nDate, klInfoHttp.nOpenPrice, klInfo.nOpenPrice);
		klInfo.nOpenPrice = klInfoHttp.nOpenPrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]���̼��޸�Ϊ%d", klInfoHttp.nDate, klInfoHttp.nOpenPrice);
	}
	if (klInfoHttp.nVolume != klInfo.nVolume)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "����[%d]�ɽ�����һ�������ϣ�%d ���أ�%d", 
			klInfoHttp.nDate, klInfoHttp.nVolume, klInfo.nVolume);
		klInfo.nVolume = klInfoHttp.nVolume;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]�ɽ����޸�Ϊ%d", klInfoHttp.nDate, klInfoHttp.nVolume);
	}
	if (klInfoHttp.nSum != klInfo.nSum)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "����[%d]�ɽ���һ�������ϣ�%d ���أ�%d", 
			klInfoHttp.nDate, klInfoHttp.nSum, klInfo.nSum);
		klInfo.nSum = klInfoHttp.nSum;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]�ɽ�����޸�Ϊ%d", klInfoHttp.nDate, klInfoHttp.nSum);
	}
	if (!bRet)
	{
		if (theApp.m_bUpdateData)
		{
			if(m_file.SetRecordBuffer((char*)&klInfo))
			{
				if(!m_file.WriteData())
					AddLog(-1, FALSE, "���󣺱���[%d]��������ʧ��", klInfo.nDate);
				else
					AddLog(-1, FALSE, "����[%d]�������ݳɹ�", klInfo.nDate);
			}
			else
			{
				AddLog(-1, FALSE, "���󣺱���[%d]��������ʧ��", klInfo.nDate);
			}
		}
	}

	AddLog(-1, FALSE, "У�����");
	m_file.Close();
}

void CCheckThread::RecheckFile()
{
	WaitThread();
	Stop();
	m_nCmd = CMD_RE_CHECK;
	m_bCheckLastOne = FALSE;
	Start();
}

void CCheckThread::CheckAllFile(CString strFile)
{
	if (strFile.IsEmpty())
		return;

	CString strFolder = strFile;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		if (!m_bRun)
		{
			return;
		}
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			CString strFolderName = ffind.GetFileName();

			CheckAllFile(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFileName();
			CString strFolder = ffind.GetFilePath();

			m_strFileName = strFileName;
			m_strFilePath = strFolder;

			if (m_hListCtrlWnd != NULL)
			{
				CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListCtrlWnd);
				pCtrl->DeleteAllItems();
			}
			CheckSingleFile(FALSE);
		}
	}
}

void CCheckThread::SaveDataToFile()
{
	WaitThread();
	Stop();
	m_nCmd = CMD_SAVE;
	Start();
}

void CCheckThread::CheckSingleFile(BOOL bShowRecord)
{
	CString strExe = CValidFunction::GetExeName(m_strFilePath);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(m_strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", m_strFilePath);
		AddLog(-1, FALSE, "��ⱻ��ֹ!");
		return;
	}

	UINT nFileType = 0;
	{//�ж��ļ����� ���ߡ����ߡ�����
		CString strFolder = CValidFunction::GetFileNamePreFolder(m_strFilePath);
		if (strFolder.CompareNoCase("day") == 0)
			nFileType = FT_DAY;
		else if(strFolder.CompareNoCase("wek") == 0)
			nFileType = FT_WEK;
		else if(strFolder.CompareNoCase("mon") == 0)
			nFileType = FT_MON;
	}
	tagKLineInfo klInfo;

	AddLog(-1, FALSE, "���ڼ��[%s]...", m_strFilePath);
	m_nErrCount    = 0;
	m_nCheckedCount= 0;
	unsigned int nRecordCount = m_file.GetRecordCount();

	BOOL bRet = TRUE;
	if (m_bCheckLastOne)
		m_file.MoveLast();
	while(!m_file.IsEOF())
	{
		if(!m_bRun)
		{
			AddLog(-1, FALSE, "��ⱻȡ��...");
			break;
		}
		m_file.GetRecordToBuffer((char*)&klInfo);
		bRet &= ProcessRecord(&klInfo, bShowRecord, nFileType);

		m_file.MoveNext();
	}

	if (m_bRun)
	{
		AddLog(-1, FALSE, "������"); 
	}

	if (bShowRecord)
	{
		if (nRecordCount == 0)
			AddLog(-1, FALSE, "�ļ���û�м�¼"); 
		AddLog(-1, FALSE, "========== ����: %d �Ѽ��: %d ����: %d ==========", 
			nRecordCount, m_nCheckedCount, m_nErrCount);
	}
	m_nCheckedFile++;
	
	if (!bRet)  //�д����¼���ļ�
	{
		m_nErrFile++;
		if (m_hListFileWnd != NULL && !bShowRecord)
		{
			CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListFileWnd);
			int nIndex = pCtrl->GetItemCount();
			AddLog(nIndex, FALSE, "�ļ��к��д����¼");
			CString strShort = CValidFunction::GetFileNamePrePath(m_strFilePath);
			pCtrl->InsertItem(nIndex, strShort, 0);
			pCtrl->SetItemTextColor(TEXT_COLOR, nIndex);
			pCtrl->SetItemText(nIndex, 0, strShort);
			pCtrl->SetItemText(nIndex, 1, m_strFilePath);
		}
	}

	m_file.Close();
}

void CCheckThread::RecheckListCtrlFile()
{
	m_bCheckLastOne = FALSE;
	CheckSingleFile();
}

BOOL CCheckThread::ProcessRecord(tagKLineInfo *pKLInfo, BOOL bShowRecord, UINT nFileType)
{
	if(m_hListCtrlWnd == NULL)
		return FALSE;

	CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListCtrlWnd);
	int nIndex = pCtrl->GetItemCount();

	CString strTemp;
	if (bShowRecord)
	{
		strTemp.Format("%d", pKLInfo->nDate);
		pCtrl->InsertItem(nIndex, strTemp, 0);
	}
	//*********************************************************
	//������ݵ���ȷ��

	BOOL bRet = TRUE;
	if (pKLInfo->nDate > 20300000 || !IsValidDate(pKLInfo->nDate))
	{
		bRet = FALSE;
		if (bShowRecord)
		{
			pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 0);
			AddLog(nIndex, TRUE, "����: %d�� %d��  ����[%d]����ȷ ",
				nIndex+1, 1, pKLInfo->nDate);
		}
	}
	if (pKLInfo->nMaxPrice < pKLInfo->nOpenPrice
		|| pKLInfo->nMaxPrice < pKLInfo->nMinPrice
		|| pKLInfo->nMaxPrice < pKLInfo->nClosePrice)
	{
		bRet = FALSE;
		if (bShowRecord)
		{
			pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 2);

			CString strErr;
			CString strErrTemp;
			if (pKLInfo->nMaxPrice < pKLInfo->nOpenPrice)
			{
				strErrTemp.Format("��߼�[%d]���ڿ��̼�[%d]; ", pKLInfo->nMaxPrice,
					pKLInfo->nOpenPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMaxPrice < pKLInfo->nMinPrice)
			{
				strErrTemp.Format("��߼�[%d]������ͼ�[%d]; ", pKLInfo->nMaxPrice,
					pKLInfo->nMinPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMaxPrice < pKLInfo->nClosePrice)
			{
				strErrTemp.Format("��߼�[%d]�������̼�[%d]; ", pKLInfo->nMaxPrice,
					pKLInfo->nClosePrice);
				strErr += strErrTemp;
			}
			AddLog(nIndex, TRUE, "����: %d�� %d��  %s", nIndex+1, 3, strErr);
		}
	}

	if (pKLInfo->nMinPrice > pKLInfo->nOpenPrice
		|| pKLInfo->nMinPrice > pKLInfo->nMaxPrice
		|| pKLInfo->nMinPrice > pKLInfo->nClosePrice)
	{
		bRet = FALSE;
		if (bShowRecord)
		{
			pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 3);

			CString strErr;
			CString strErrTemp;
			if (pKLInfo->nMinPrice > pKLInfo->nOpenPrice)
			{
				strErrTemp.Format("��ͼ�[%d]���ڿ��̼�[%d]; ", pKLInfo->nMinPrice,
					pKLInfo->nOpenPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMinPrice > pKLInfo->nMaxPrice)
			{
				strErrTemp.Format("��ͼ�[%d]������߼�[%d]; ", pKLInfo->nMinPrice,
					pKLInfo->nMaxPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMinPrice > pKLInfo->nClosePrice)
			{
				strErrTemp.Format("��ͼ�[%d]�������̼�[%d]; ", pKLInfo->nMinPrice,
					pKLInfo->nClosePrice);
				strErr += strErrTemp;
			}

			AddLog(nIndex, TRUE, "����: %d�� %d��  %s", nIndex+1, 4, strErr);
		}
	}

	/*CString strDayFile = theApp.m_strKFilePath;
	strDayFile += "\\day\\";
	strDayFile += m_strFileName;
	tagKLineInfo newInfo = {0};
	newInfo.nDate = pKLInfo->nDate;

	BOOL bGetData = FALSE;
	if (nFileType == FT_WEK && IsValidDate(pKLInfo->nDate)) //��������ߣ��������ļ������������ݱȽ�
	{
		if(GetWeekData(&newInfo, strDayFile))
			bGetData = TRUE;
	}
	else if (nFileType == FT_MON && IsValidDate(pKLInfo->nDate))
	{
		if(GetMonData(&newInfo, strDayFile))
			bGetData = TRUE;
	}

	if ((nFileType == FT_WEK || nFileType == FT_MON) && bGetData)
	{
		if (pKLInfo->nOpenPrice != newInfo.nOpenPrice)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "����[%d]���̼۸�һ��, ԭʼ����:%d �������:%d",	pKLInfo->nDate, pKLInfo->nOpenPrice, newInfo.nOpenPrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 1);
			}
		}
		if (pKLInfo->nMaxPrice != newInfo.nMaxPrice)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "����[%d]��߼۸�һ��, ԭʼ����:%d �������:%d",pKLInfo->nDate, pKLInfo->nMaxPrice, newInfo.nMaxPrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 2);
			}
		}
		if (pKLInfo->nMinPrice != newInfo.nMinPrice)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "����[%d]��ͼ۸�һ��, ԭʼ����:%d �������:%d",pKLInfo->nDate, pKLInfo->nMinPrice, newInfo.nMinPrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 3);
			}
		}
		if (pKLInfo->nClosePrice != newInfo.nClosePrice)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "����[%d]���̼۸�һ��, ԭʼ����:%d �������:%d",pKLInfo->nDate, pKLInfo->nClosePrice, newInfo.nClosePrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 4);
			}
		}
		if (pKLInfo->nVolume != newInfo.nVolume)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "����[%d]�ɽ�����һ��, ԭʼ����:%d �������:%d",pKLInfo->nDate, pKLInfo->nVolume, newInfo.nVolume);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 5);
			}
		}
		if (pKLInfo->nSum != newInfo.nSum)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "����[%d]�ɽ���һ��, ԭʼ����:%d �������:%d",pKLInfo->nDate, pKLInfo->nSum, newInfo.nSum);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 6);
			}
		}
	}  */
	//��ǰ��¼������һ������
	if(!bRet)
	{
		m_nErrCount++;

		if (bShowRecord)
		{
			pCtrl->SetItemBackColor(BK_COLOR, nIndex);
			pCtrl->SetItemImage(nIndex, IMG_ERROR);
		}
	}

	//*********************************************************
	int nSubItem = 0;
	if (bShowRecord)
	{
		pCtrl->SetItemText(nIndex, nSubItem++, strTemp);
		strTemp.Format("%d", pKLInfo->nOpenPrice);
		pCtrl->SetItemText(nIndex, nSubItem++, strTemp);
		strTemp.Format("%d", pKLInfo->nMaxPrice);
		pCtrl->SetItemText(nIndex, nSubItem++, strTemp);
		strTemp.Format("%d", pKLInfo->nMinPrice);
		pCtrl->SetItemText(nIndex, nSubItem++, strTemp);
		strTemp.Format("%d", pKLInfo->nClosePrice);
		pCtrl->SetItemText(nIndex, nSubItem++, strTemp);
		strTemp.Format("%u", pKLInfo->nVolume);
		pCtrl->SetItemText(nIndex, nSubItem++, strTemp);
		strTemp.Format("%u", pKLInfo->nSum);
		pCtrl->SetItemText(nIndex, nSubItem++, strTemp);

		if(pCtrl->GetNextItem(-1, LVNI_SELECTED) == nIndex -1)
		{
			pCtrl->SetItemState(nIndex, LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
			pCtrl->EnsureVisible(nIndex, FALSE);
		}
	}
	m_nCheckedCount++;

	return bRet;
}

void CCheckThread::SaveFile()
{
	CString strExe = CValidFunction::GetExeName(m_strFilePath);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "����: �޷��������ļ�[%s]�� ����ʧ��!!", m_strFilePath);
		return;
	}
	if (!m_file.Open(m_strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "����: ���ļ�[%s]ʧ��!", m_strFilePath);
		return;
	}

	if(m_hListCtrlWnd == NULL)
	{
		AddLog(-1, FALSE, "����ʧ��!");
		::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
		return;
	}

	CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListCtrlWnd);
	AddLog(-1, FALSE, "���ڱ�������[%s]...", m_strFilePath);

	int nItemCount = pCtrl->GetItemCount();
	int nOldRecCount = m_file.GetRecordCount();
	if (nItemCount != nOldRecCount) //Ŀǰ��¼��������ǰ��¼��ʱ���������·����ڴ�
	{
		if (!m_file.AllocateMemoryForSave(nItemCount))
		{
			AddLog(-1, FALSE, "�����ݷ����ڴ�ʧ��, �������ݱ�����ֹ");
			::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
			m_file.Close();
			return;
		}
	}

	m_file.MoveFirst();
	int nSaved = 0;
	for (int i=0; i<nItemCount; i++)
	{
		char chTemp[50];
		tagKLineInfo klInfo;
		pCtrl->GetItemText(i, 0, chTemp, 50);
		klInfo.nDate = CCharTranslate::CharToDigital(chTemp, (int)strlen(chTemp));
		pCtrl->GetItemText(i, 1, chTemp, 50);
		klInfo.nOpenPrice = CCharTranslate::CharToDigital(chTemp, (int)strlen(chTemp));
		pCtrl->GetItemText(i, 2, chTemp, 50);
		klInfo.nMaxPrice = CCharTranslate::CharToDigital(chTemp, (int)strlen(chTemp));
		pCtrl->GetItemText(i, 3, chTemp, 50);
		klInfo.nMinPrice = CCharTranslate::CharToDigital(chTemp, (int)strlen(chTemp));
		pCtrl->GetItemText(i, 4, chTemp, 50);
		klInfo.nClosePrice = CCharTranslate::CharToDigital(chTemp, (int)strlen(chTemp));
		pCtrl->GetItemText(i, 5, chTemp, 50);
		klInfo.nVolume = CCharTranslate::CharToDigital(chTemp, (int)strlen(chTemp));
		pCtrl->GetItemText(i, 6, chTemp, 50);
		klInfo.nSum = CCharTranslate::CharToDigital(chTemp, (int)strlen(chTemp));

		if (m_file.IsEOF())
		{
			AddLog(-1, FALSE, "�����ݷ����ڴ治��, �������ݶ�ʧ");
			::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
			break;
		}

		m_file.SetRecordBuffer((char*)&klInfo);
		m_file.MoveNext();
		nSaved++;
	}

	if (!m_file.WriteData())
	{
		AddLog(-1, FALSE, "����ʧ��");
		::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
	}
	else
	{
		if (nSaved < nItemCount)
		{
			AddLog(-1, FALSE, "����: �������ݱ���ʧ��");
			::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
		}
		else
		{
			AddLog(-1, FALSE, "����ɹ�"); 
		}
	}

	m_file.Close();
	AddLog(-1, FALSE, "========== ����: %d �ѱ���: %d ==========", 
		nItemCount, nSaved);
}

void CCheckThread::AddLog(int nIndex, BOOL bListInfo, LPCTSTR pFormat, ...) 
{
	char chMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(chMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);

	//strcat_s(chMsg, MSG_BUF_LEN, "\n");

	REPORT(MN, T("%s\n", chMsg), RPT_INFO);
	 	
	if (m_hListBoxWnd == NULL)
		return;

	CListBox *pListBox = (CListBox *)CListBox::FromHandle(m_hListBoxWnd);
	ASSERT(pListBox);
	int nRet = pListBox->AddString(chMsg);

	if (nRet != LB_ERR)
	{
		if (bListInfo) // ��λ������ΪlistInfo  1 �� listfile 0�ı�־λ 
		{
			nIndex *= 10;
			nIndex++;
		}
		else
		{
			nIndex *= 10;
		}
		pListBox->SetItemData(nRet, nIndex);
		pListBox->SetCurSel(nRet);
	}
}

BOOL CCheckThread::IsValidDate(unsigned int nDate)
{
	int nYear = nDate / 10000;
	int nMon  = (nDate % 10000) / 100;
	int nDay  = nDate % 100;

	if( nYear < 1900 ||
		nMon < 1     ||
		nMon > 12    ||
		nDay < 1     ||
		nDay > 31 )
		return FALSE;
	else
		return TRUE;
}

void CCheckThread::KillRarProcess()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32 = {0};

	hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hProcessSnap == INVALID_HANDLE_VALUE )
		return;

	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle( hProcessSnap ); 
		return;
	}

	do
	{
		if (_stricmp("Rar.exe", pe32.szExeFile) != 0)
			continue;

		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if(hProcess == NULL)
			return;


		WaitForSingleObject(hProcess, INFINITE);

		/*if (!TerminateProcess(hProcess, 0))
		{
			//REPORT(MN, T("TerminateProcess() failed with error:%d\n", GetLastError()), RPT_ERROR);
			CloseHandle(hProcess);
			CloseHandle(hProcessSnap);
			return;
		}
		else */
		{
			CloseHandle(hProcess);
			CloseHandle(hProcessSnap);
			return;
		}
	}
	while(Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
}

void CCheckThread::StartWeightThread(BOOL bRealHQ)
{
	m_bRealHQ = bRealHQ; 
	WaitThread();
	Stop();
	m_nCmd = CMD_WEIGHT;
	Start();
}

void CCheckThread::AddMsg(UINT nPackType, LPCSTR pFormat, ...)
{
	char szMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(szMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);

	REPORT(MN, T("%s\n", szMsg), nPackType);
	::SendMessage(m_hDialogWnd, WM_LIST_MSG, (WPARAM)szMsg, 0);
}

void CCheckThread::StopWeightThread()
{
	m_bStartWeight = FALSE;
}

void CCheckThread::StartWeightThread()
{
	m_bStartWeight = TRUE;
// 	if (m_bRealHQ) //ʵʱ��Ȩ
// 		AddMsg(RPT_INFO, "���ڿ�ʼK��ʵʱ��Ȩ...");
// 	else
// 		AddMsg(RPT_INFO, "���ڿ�ʼK����ʷ��Ȩ...");
// 
// 	if (!BackupKLine())
// 	{
// 		AddMsg(RPT_ERROR, "����K���ļ�����ʱ�ļ���ʧ��");
// 		return;
// 	}
// 	AddMsg(RPT_INFO, "����K���ļ�����ʱ�ļ������");
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return;
// 	}
// 
// 	if (m_bRealHQ)
// 	{
// 		AddMsg(RPT_INFO, "������������ɵ������ļ�����ʱ�ļ����еĻ�Ȩ�����ļ���...");
// 		UpdateDayLineToWeight();
// 	}
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return;
// 	}
// 
// 	if (!QueryWeight())  //��û�Ȩ����
// 		return;
// 
// 	//��ʼ��k�߻�Ȩ
// 	AddMsg(RPT_INFO, "�������߻�Ȩ...");
// 	CTime tmCur = CTime::GetCurrentTime();
// 	UINT ntmCur = tmCur.GetYear()*10000 + tmCur.GetMonth()*100 + tmCur.GetDay();
// 
// 	CYinZiList::iterator pos = m_lstYinZi.begin();
// 	CString strSymbol;
// 	UINT    nDate = 0;
// 	while(pos != m_lstYinZi.end())
// 	{
// 		if ( (ntmCur < (*pos)->nDate) || (strSymbol.Compare((*pos)->szStockCode) == 0 && nDate == (*pos)->nDate) )
// 		{
// 			delete (*pos);
// 			pos++;
// 			continue;
// 		}
// 
// 		strSymbol = (*pos)->szStockCode;
// 		nDate     = (*pos)->nDate;
// 		if (m_bStartWeight)
// 		{
// 			StartWeight(*pos);
// 			StartWeight(*pos, FALSE);
// 		}
// 		delete (*pos);
// 		pos++;
// 	}
// 	m_lstYinZi.clear();
// 	m_KFileCur.Close();
// 	m_KFileHis.Close();
// 	AddMsg(RPT_INFO, "���߻�Ȩ���");
	//�������ļ����������ļ���������
// 	if (!m_bStartWeight)
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return;
// 	}
// 
// 	CString strDay = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day";
// 
// 	AddMsg(RPT_INFO, "���ڲ������ߺ������ļ�...");
// 	if (!GetWekMonLine(strDay))
// 	{
// 		AddMsg(RPT_ERROR, "�������ļ��������ߺ������ļ�ʧ��");
// 		return;
// 	}
// 	AddMsg(RPT_INFO, "�������ߺ��������");
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return;
// 	}
// 
// 	if (m_bRealHQ) //ʵʱ��Ȩ�����ļ���С�������100��
// 	{
// 		AddMsg(RPT_INFO, "���ڼ���ļ���С...");
// 		CString strFilePath = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str();
// 		CheckFileSize(strFilePath);
// 		AddMsg(RPT_INFO, "����ļ���С���");
// 	}
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return;
// 	}
// 	//��Ȩ��Ϻ󣬴���ʱ�ļ��и��ƻ�Ȩ��K���ļ�����Ȩk���ļ���
// 	if (m_bStartWeight)
// 	{
// 		//����Ѿ����ڻ�Ȩk���ļ�����Ȩǰѯ���û��Ƿ�ɾ�����ļ�
// 		if (m_bStartWeight && CFilePath::IsFolderHasFile(theApp.m_strKFilePath))
// 		{
// 			UINT nRetID = IDYES;
// 			if (!m_bRealHQ)
// 				nRetID = ::MessageBox(m_hDialogWnd,"��Ȩk���ļ�Ŀ¼["+theApp.m_strKFilePath +"]���Ѵ����ļ�, �ڻ�Ȩǰ�Ƿ���ɾ�����ļ�?", "����", MB_YESNO);
// 
// 			if (IDYES == nRetID)
// 			{//ɾ���ļ�
// 				AddMsg(RPT_INFO, "����ɾ����Ȩk���ļ�Ŀ¼[%s]����ļ�...", theApp.m_strKFilePath);
// 				CFilePath::DeleteFolderAllFile(theApp.m_strKFilePath);
// 				AddMsg(RPT_INFO, "ɾ���ļ����");
// 			}
// 		}
// 
// 		if (!m_bStartWeight)  //��ֹ��Ȩ
// 		{
// 			AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 			return ;
// 		}
// 		AddMsg(RPT_INFO, "���ڴ���ʱ�ļ����и��ƻ�Ȩ���k���ļ���[%s]...", theApp.m_strKFilePath);
// 		if (!CFilePath::CopyFolderAllFile(theApp.m_strKFilePath, theApp.m_strKLBkPath))
// 			AddMsg(RPT_ERROR, "����ʱ�ļ����и��ƻ�Ȩ���k���ļ���[%s]ʧ��", theApp.m_strKFilePath);
// 
// 		AddMsg(RPT_INFO, "���ƻ�Ȩ���k���ļ����");
// 
// 		if (!m_bStartWeight)  //��ֹ��Ȩ
// 		{
// 			AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 			return;
// 		}
// 		AddMsg(RPT_INFO, "����ɾ����ʱ�ļ�Ŀ¼[%s]����ļ�...", theApp.m_strKLBkPath);
// 		CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
// 		AddMsg(RPT_INFO, "ɾ���ļ����");
// 
// 		if (m_bRealHQ) //ʵʱ��Ȩ
// 			AddMsg(RPT_INFO, "K��ʵʱ��Ȩ���");
// 		else
// 			AddMsg(RPT_INFO, "K����ʷ��Ȩ���");
// 	}
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return;
// 	}

	//ʵʱ��Ȩ���ϴ�
	if (/*m_bRealHQ &&*/ theApp.m_bRWUpload) //ʵʱ��Ȩ
	{
		AddMsg(RPT_INFO, "�����ϴ���Ȩk��...");
		//m_checkThread.StartUploadHQKLineFile();

		HWND hDlgInfoWnd = theApp.m_pDlgInfoWnd->GetSafeHwnd();
		if (theApp.m_pDlgInfoWnd != NULL && hDlgInfoWnd != NULL)
		{
			::SendMessage(hDlgInfoWnd, WM_CHANGEINFO, 0, 0);
			((CDlg_Info*)theApp.m_pDlgInfoWnd)->m_strInfo = "�����ϴ���ǰ��Ļ�ȨK��...";
		}
		UploadHQKLine();
	}
}

BOOL CCheckThread::BackupKLine()
{
	return TRUE;
// 	AddMsg(RPT_INFO, "���ڱ���k�����ݵ���ʱĿ¼[%s]...", theApp.m_strKLBkPath);
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return FALSE;
// 	}
// 	if (CFilePath::IsFolderHasFile(theApp.m_strKLBkPath))
// 	{
// 		UINT nRetID = IDYES;
// 		if (!m_bRealHQ)
// 			nRetID = ::MessageBox(m_hDialogWnd, "��ʱĿ¼["+theApp.m_strKLBkPath +"]���Ѵ����ļ�, �ڸ����ļ�ǰ�Ƿ���ɾ�����ļ�?", "����", MB_YESNO);
// 
// 		if (nRetID == IDYES)
// 		{//ɾ���ļ�
// 			AddMsg(RPT_INFO, "����ɾ����ʱĿ¼[%s]����ļ�...", theApp.m_strKLBkPath);
// 			CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
// 			AddMsg(RPT_INFO, "ɾ���ļ����");
// 		}
// 	}
// 
// 	m_strCurKLine = CFilePath::GetPathLastFolderName(theApp.m_strKFilePath);
// 	m_strHisKLine = CFilePath::GetPathLastFolderName(theApp.m_strHisKFilePath);
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return FALSE;
// 	}
// 
// 	BOOL bRet = TRUE;
// 	if (m_bRealHQ) //ʵʱ��Ȩ
// 	{
// 		AddMsg(RPT_INFO, "���ڸ���[%s]����ļ�����ʱĿ¼[%s]...", theApp.m_strKFilePath, theApp.m_strKLBkPath);
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath, theApp.m_strKFilePath);
// 	}
// 	else		   //��ʷ��Ȩ
// 	{
// 		AddMsg(RPT_INFO, "���ڸ���[%s]����ļ�����ʱĿ¼[%s]...", theApp.m_strKFilePath, theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str(), theApp.m_strKFilePath);
// 		AddMsg(RPT_INFO, "���ڸ���[%s]����ļ�����ʱĿ¼[%s]...", theApp.m_strHisKFilePath, theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str(), theApp.m_strHisKFilePath);
// 	}
// 
// 	if (!m_bStartWeight)  //��ֹ��Ȩ
// 	{
// 		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
// 		return FALSE;
// 	}
// 	return bRet;
}

BOOL CCheckThread::QueryWeight()
{
	AddMsg(RPT_INFO, "���ڴ����ݿ�[%s:%s]��ȡ��Ȩ����...", theApp.m_strSqlIP, theApp.m_strSqlDBN);
	//�����ݿ��ȡ���ݵ��ڴ�
	std::string strIP, strDBN, strUID, strPw;
	strIP  = theApp.m_strSqlIP.GetBuffer();
	theApp.m_strSqlIP.ReleaseBuffer();
	strDBN = theApp.m_strSqlDBN.GetBuffer();
	theApp.m_strSqlDBN.ReleaseBuffer();
	strUID = theApp.m_strSqlUID.GetBuffer();
	theApp.m_strSqlUID.ReleaseBuffer();
	strPw  = theApp.m_strSqlPw.GetBuffer();
	theApp.m_strSqlPw.ReleaseBuffer();

	if (!m_bStartWeight)  //��ֹ��Ȩ
	{
		AddMsg(RPT_WARNING, "k�߻�Ȩ����ֹ");
		return FALSE;
	}

	if (!m_sqlDB.Open(strIP, strDBN, strUID, strPw))
	{
		AddMsg(RPT_ERROR, "�򿪻�Ȩ�������ݿ�ʧ��");
		return FALSE;
	}

	CRecordSet reSet(&m_sqlDB);
	string strCon = "SELECT * FROM IND_S_RIGHT";
	if (m_bRealHQ) //ʵʱ��Ȩ
	{
		strCon += " WHERE F0010='";
		CTime tmCur = CTime::GetCurrentTime();
		CString strTime;
		strTime.Format("%d-%d-%d", tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay());
		strCon += strTime;
		strCon += "'";
	}
	else
	{
		strCon += " WHERE F0010<'";
		CTime tmCur = CTime::GetCurrentTime();
		CString strTime;
		strTime.Format("%d-%d-%d", tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay());
		strCon += strTime;
		strCon += "'";
	}

	strCon += " ORDER BY SEC_CD, F0010 DESC";

	if (!reSet.Open(strCon.c_str(), adOpenDynamic, adLockReadOnly))
	{
		AddMsg(RPT_ERROR, "�򿪻�Ȩ�������ݿ��IND_S_RIGHTʧ��");

		reSet.Close();
		m_sqlDB.Close();
		return FALSE;
	}

	while(!reSet.IsEOF() && m_bStartWeight)
	{
		tagHQYinZi* pYinZi = new tagHQYinZi;
		memset(pYinZi, 0, sizeof(tagHQYinZi));

		reSet.GetCollect("SEC_CD", pYinZi->szStockCode, 8);
		SYSTEMTIME systm = {0};
		reSet.GetCollect("F0010", &systm);
		pYinZi->nDate = systm.wYear*10000 + systm.wMonth*100 + systm.wDay;

		if (!CCheckThread::IsValidDate(pYinZi->nDate))
		{
			delete pYinZi;
			reSet.MoveNext();
			continue;
		}
		reSet.GetCollect("F0020", pYinZi->f0020);
		if (pYinZi->f0020 == 0)
			pYinZi->f0020 = 1;

		m_lstYinZi.push_back(pYinZi);
		reSet.MoveNext();
	}

	reSet.Close();
	m_sqlDB.Close();

	if (m_bStartWeight)
		AddMsg(RPT_INFO, "��ȡ��Ȩ������ϣ���[%d]��¼", m_lstYinZi.size());
	return TRUE;
}

BOOL CCheckThread::GetWekMonLine(CString strDayLine)
{
	if (strDayLine.IsEmpty())
		return FALSE;

	CString strFolder = strDayLine;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			GetWekMonLine(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFolder = ffind.GetFilePath(); //ָ��ҲҪ�������ߺ�����
			GetSingleWekMonLine(strFolder);
		}
	}
	return TRUE;
}

BOOL CCheckThread::GetSingleWekMonLine(CString strDayFile)
{
	//day
// 	CKLineList lstDayLine;
// 	CKLineList lstWekLine;
// 	CKLineList lstMonLine;
// 
// 	CString strFileName = strDayFile.Right(strDayFile.GetLength() - strDayFile.ReverseFind('\\') - 1);
// 	CString strHisWekFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\wek\\" + strFileName;
// 	CString strHisMonFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\mon\\" + strFileName;
// 
// 	CString strCurWekFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\wek\\" + strFileName;
// 	CString strCurMonFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\mon\\" + strFileName;
// 
// 	for (int i=0; i<2; i++)
// 	{
// 		if (i == 1) //��ǰ��
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 		else
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\day\\" + strFileName;
// 
// 		CKLineFile kfile;
// 		if (!kfile.Open(strDayFile,  GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 		{
// 			AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strDayFile);
// 			continue;
// 		}
// 		while(!kfile.IsEOF())
// 		{
// 			tagKLineInfo* pInfo = new tagKLineInfo;
// 			memset(pInfo, 0, sizeof(tagKLineInfo));
// 
// 			kfile.GetRecordToBuffer((char*)pInfo);
// 			lstDayLine.push_back(pInfo);
// 
// 			kfile.MoveNext();
// 		}
// 		kfile.Close();
// 	}
// 	//��������
// 	{
// 		tagKLineInfo*  pWekInfo = NULL;
// 		BOOL bWekFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("��Ч������:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				pos++;
// 				continue;
// 			}
// 			//UINT nWek = CCheckThread::GetDayofWeek((*pos)->nDate);
// 			UINT nPreDate =	(*pos)->nDate;
// 			if (bWekFirstDay)
// 			{
// 				bWekFirstDay = FALSE;
// 				pWekInfo = new tagKLineInfo;
// 				memcpy(pWekInfo, (*pos), sizeof(tagKLineInfo));
// 			}
// 			else
// 			{
// 				pWekInfo->nClosePrice = (*pos)->nClosePrice;
// 				if ((*pos)->nMaxPrice > pWekInfo->nMaxPrice)
// 					pWekInfo->nMaxPrice  = (*pos)->nMaxPrice;
// 				if ((*pos)->nMinPrice < pWekInfo->nMinPrice)
// 					pWekInfo->nMinPrice  = (*pos)->nMinPrice;
// 				pWekInfo->nSum += (*pos)->nSum;
// 				pWekInfo->nVolume += (*pos)->nVolume;
// 			}
// 			pos++;
// 			if (pos == lstDayLine.end())
// 				break;
// 
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("��Ч������:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				continue;
// 			}
// 			UINT nNextDate = (*pos)->nDate;
// 			if (!IsSameWeek(nPreDate, nNextDate)) //��һ��	  
// 			{
// 				bWekFirstDay = TRUE;
// 				lstWekLine.push_back(pWekInfo);
// 				pWekInfo = NULL;
// 			}
// 		}
// 		if (pWekInfo != NULL)
// 			lstWekLine.push_back(pWekInfo); //���һ��
// 	}
// 	//��������
// 	{
// 		tagKLineInfo*  pMonInfo = NULL;
// 		BOOL bMonFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("��Ч������:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				delete (*pos);
// 				pos++;
// 				continue;
// 			}
// 			if (bMonFirstDay)
// 			{
// 				bMonFirstDay = FALSE;
// 				pMonInfo = new tagKLineInfo;
// 				memcpy(pMonInfo, (*pos), sizeof(tagKLineInfo));
// 			}
// 			else
// 			{
// 				pMonInfo->nClosePrice = (*pos)->nClosePrice;
// 				if ((*pos)->nMaxPrice > pMonInfo->nMaxPrice)
// 					pMonInfo->nMaxPrice  = (*pos)->nMaxPrice;
// 				if ((*pos)->nMinPrice < pMonInfo->nMinPrice)
// 					pMonInfo->nMinPrice  = (*pos)->nMinPrice;
// 				pMonInfo->nSum += (*pos)->nSum;
// 				pMonInfo->nVolume += (*pos)->nVolume;
// 			}
// 			delete (*pos);
// 			pos++;
// 			if (pos == lstDayLine.end())
// 				break;
// 
// 			if ((*pos)->nDate % 10000 / 100 != pMonInfo->nDate % 10000 / 100) //��һ��
// 			{
// 				bMonFirstDay = TRUE;
// 				lstMonLine.push_back(pMonInfo);
// 				pMonInfo = NULL;
// 			}
// 		}
// 		if (pMonInfo != NULL)
// 			lstMonLine.push_back(pMonInfo); //���һ��
// 	}
// 
// 	{//��������
// 		CFile fHisWekL;
// 		CFile fCurWekL;
// 
// 		if (!fCurWekL.Open(strCurWekFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strCurWekFile);
// 
// 		CKLineList::iterator pos = lstWekLine.begin();
// 		int nHisCount = (int)lstWekLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0) //�ж���ļ�¼�Ŵ���ʷ�ļ�
// 		{
// 			if (!fHisWekL.Open(strHisWekFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strHisWekFile);
// 		}
// 
// 		while(pos != lstWekLine.end())
// 		{
// 			if (nHisCount <= 0) //�����ڵ�ǰ��
// 			{
// 				fCurWekL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//��������ʷ��
// 			{
// 				fHisWekL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			delete (*pos);
// 			pos++;
// 			nHisCount--;
// 		}
// 
// 		if (nHisCount > 0)
// 			fHisWekL.Close();
// 		fCurWekL.Close();
// 	}
// 
// 	{//��������
// 		CFile fHisMonL;
// 		CFile fCurMonL;
// 		CKLineList::iterator pos = lstMonLine.begin();
// 		int nHisCount = (int)lstMonLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0)
// 		{
// 			if (!fHisMonL.Open(strHisMonFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strHisMonFile);
// 		}
// 
// 		if (!fCurMonL.Open(strCurMonFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strCurMonFile);
// 
// 		while(pos != lstMonLine.end())
// 		{
// 			if (nHisCount <= 0) //�����ڵ�ǰ��
// 			{
// 				fCurMonL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//��������ʷ��
// 			{
// 				fHisMonL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			delete (*pos);
// 			pos++;
// 			nHisCount--;
// 		}
// 
// 		if (nHisCount > 0)
// 			fHisMonL.Close();
// 		fCurMonL.Close();
// 	}

	return TRUE;
}

void CCheckThread::UpdateDayLineToWeight()  //����������ɵ��������ݵ���Ȩ���������У�Ȼ����ʵʱ��Ȩ
{
	UpdateAllFile(theApp.m_strKFilePath);
}

void CCheckThread::UpdateAllFile(CString strFile)
{
	if (strFile.IsEmpty())
		return;

	CString strFolder = strFile;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //�ļ���
		{
			CString strPath = ffind.GetFilePath(); //�õ�·��
			CString strFolder = CValidFunction::GetFilePathPreFolder(strPath);
			if (strFolder.CompareNoCase("day") == 0)
				UpdateAllFile(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFolder = ffind.GetFilePath();
			CString strFileName = ffind.GetFileName();
			UpdateSingleFile(strFolder, strFileName);
		}
	}
}

void CCheckThread::UpdateSingleFile(CString strFile, CString strFileName)
{
// 	CString strExe = CValidFunction::GetExeName(strFile);
// 	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
// 		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
// 		&& strExe.CompareNoCase(".hk") != 0 && strExe.CompareNoCase(".hkidx") != 0)
// 	{
// 		AddMsg(RPT_ERROR, "����: �޷��������ļ�[%s]!", strFile);
// 		return;
// 	}
// 
// 	CString strHQFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 
// 	CKLineFile hqfile;     //��Ȩ����
// 	CKLineFile orifile;	   //ԭʼ����
// 	if (!hqfile.Open(strHQFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS))
// 	{
// 		AddMsg(RPT_ERROR, "����: ���ļ�[%s]ʧ��!", strHQFile);
// 		return;
// 	}
// 	hqfile.MoveLast();
// 	tagKLineInfo info = {0};
// 	hqfile.GetRecordToBuffer((char*)&info);
// 
// 	if (!orifile.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 	{
// 		AddMsg(RPT_ERROR, "����: ���ļ�[%s]ʧ��!", strHQFile);
// 		return;
// 	}
// 
// 	char* pNewKLineBuf = new char[orifile.GetAllRecordSize()];
// 	char* pBufPos = pNewKLineBuf;
// 	tagKLineInfo tempinfo = {0};
// 	int nNeedAddCount = 0;
// 	while(!orifile.IsEOF())
// 	{
// 		orifile.GetRecordToBuffer((char*)&tempinfo);
// 		if (tempinfo.nDate > info.nDate)
// 		{
// 			//AddMsg(RPT_INFO, "Date:[%d] Date:[%d] FilePath:[%s]", tempinfo.nDate, info.nDate, strHQFile);
// 			memcpy(pBufPos, &tempinfo, sizeof(tagKLineInfo));
// 			pBufPos += sizeof(tagKLineInfo);
// 			nNeedAddCount++;
// 		}
// 		orifile.MoveNext();
// 	}
// 	orifile.Close();
// 
// 	if (nNeedAddCount == 0)
// 	{
// 		delete []pNewKLineBuf;
// 		return;
// 	}
// 
// 	int nHqKLineCount = hqfile.GetRecordCount();
// 	nHqKLineCount += nNeedAddCount;
// 
// 	char* pOldKLineBuf = new char[nHqKLineCount*sizeof(tagKLineInfo)];
// 	hqfile.GetAllRecordToBuffer(pOldKLineBuf);
// 	memcpy(pOldKLineBuf+(nHqKLineCount-nNeedAddCount)*sizeof(tagKLineInfo), pNewKLineBuf, nNeedAddCount*sizeof(tagKLineInfo));
// 	delete []pNewKLineBuf;
// 
// 	if (!hqfile.AllocateMemoryForSave(nHqKLineCount))
// 	{
// 		AddMsg(RPT_ERROR, "����: AllocateMemoryForSave() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.SetAllRecordToBuffer(pOldKLineBuf))
// 	{
// 		AddMsg(RPT_ERROR, "����: SetAllRecordToBuffer() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.WriteData())
// 	{
// 		AddMsg(RPT_ERROR, "����: WriteData() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	hqfile.Close();
// 	delete []pOldKLineBuf;
}

BOOL CCheckThread::IsSameWeek(unsigned int uiNow, unsigned int uiPrev)
{//�ȽϽ��պ������������Ƿ���ͬһ����
	tm    stPrevTime,stNowTime;
	memset(&stPrevTime,0,sizeof(tm));
	stPrevTime.tm_year=uiPrev/10000-1900;
	stPrevTime.tm_mon=uiPrev%10000/100-1;
	stPrevTime.tm_mday=uiPrev%100;
	time_t stPrev=mktime(&stPrevTime);

	memset(&stNowTime,0,sizeof(tm));
	stNowTime.tm_year=uiNow/10000-1900;
	stNowTime.tm_mon=uiNow%10000/100-1;
	stNowTime.tm_mday=uiNow%100;
	time_t stNow=mktime(&stNowTime);

	if(stNow>stPrev && difftime(stNow,stPrev)>=7*24*3600)
		return FALSE;
	else if(stPrev>stNow && difftime(stPrev,stNow)>=7*24*3600)
		return FALSE;
	if(stNowTime.tm_wday<stPrevTime.tm_wday && stNow>stPrev)
		return FALSE;
	else if(stNowTime.tm_wday>stPrevTime.tm_wday && stNow<stPrev)
		return FALSE;

	return TRUE;
}

void CCheckThread::StartWeight(tagHQYinZi* pWeight, BOOL bCurKLine)
{
// 	CString strStockCode = pWeight->szStockCode;
// 	int nStockCode = atoi(pWeight->szStockCode);
// 	if (nStockCode >= 600000)
// 		strStockCode += ".sh";
// 	else
// 		strStockCode += ".sz";
// 
// 	CString strFilePath = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str();
// 	if (!bCurKLine)
// 		strFilePath = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str();
// 
// 	strFilePath += "\\day\\" + strStockCode;
// 
// 	CKLineFile* pKLFile = NULL;
// 	if (bCurKLine)
// 		pKLFile = &m_KFileCur;
// 	else
// 		pKLFile = &m_KFileHis;
// 
// 	if (!CFilePath::IsFileExist(strFilePath))
// 	{
// 		AddMsg(RPT_WARNING, "k���ļ�[%s]������", strFilePath);
// 	}
// 	else
// 	{
// 		if ((bCurKLine && m_strCurKFilePath.Compare(strFilePath) == 0 || 
// 			!bCurKLine && m_strHisKFilePath.Compare(strFilePath) == 0)
// 			&& pKLFile->IsOpen())
// 		{ //ͬһ���ļ�ֻ��һ��
// 			pKLFile->MoveFirst();
// 		}
// 		else
// 		{
// 			if (pKLFile->IsOpen()) 
// 				pKLFile->Close(); //�ر��ϴδ򿪵��ļ�
// 
// 			if (!pKLFile->Open(strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 			{
// 				AddMsg(RPT_ERROR, "��k���ļ�[%s]ʧ��", strFilePath);
// 				if (bCurKLine)
// 					m_strCurKFilePath.Empty();
// 				else
// 					m_strHisKFilePath.Empty();
// 			}
// 			else
// 			{
// 				if (bCurKLine)
// 					m_strCurKFilePath = strFilePath; //������һ���ļ�·���������һ������һ����ͬһ���ļ�����ֻ��һ��
// 				else
// 					m_strHisKFilePath = strFilePath;
// 			}
// 		}
// 
// 		while(!pKLFile->IsEOF() && m_bStartWeight)
// 		{
// 			tagKLineInfo klInfo = {0};
// 			pKLFile->GetRecordToBuffer((char*)&klInfo);
// 
// 			if (klInfo.nDate >= pWeight->nDate)  //С�ڻ�Ȩ���ڵ�����k�߻�Ȩ
// 				break;
// 
// 			klInfo.nOpenPrice  *= pWeight->f0020;
// 			klInfo.nMaxPrice   *= pWeight->f0020;
// 			klInfo.nMinPrice   *= pWeight->f0020;
// 			klInfo.nClosePrice *= pWeight->f0020;
// 			klInfo.nVolume     /= pWeight->f0020;
// 
// 			pKLFile->SetRecordBuffer((char*)&klInfo);
// 			pKLFile->MoveNext();
// 		}
// 		if (!pKLFile->WriteData())
// 			AddMsg(RPT_ERROR, "�����ļ�[]ʧ��", strFilePath);
// 	}
}

void CCheckThread::StartCombinationDatFile()
{
	WaitThread();
	Stop();
	m_nCmd = CMD_COMBINATION;
	m_bRunCombDatFile = TRUE;
	Start();
}

BOOL CCheckThread::ReadFile1()
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	if (!m_bRunCombDatFile)
		return FALSE;

	m_bIsDataFile1 = TRUE;
	m_pdataFarm = new CDataFarm();
	if (!m_pdataFarm->mapfile(m_strPath1 + "\\index.dat", m_nMarketCount1, m_indexHead1.nBucketCount,
		m_indexHead1.nSymbolCount, m_indexHead1.nTickCount, m_indexHead1.nMinkCount))
	{
		REPORT(MN, T("mapfile() failed with file: %s\n", m_strPath1 + "\\index.dat"), RPT_ERROR);
		return FALSE;
	}

	if (m_pdataFarm->m_hashIdx.for_each(*this) == 0)
	{
		REPORT(MN, T("Dat�ļ���û�й�Ʊ��Ϣ\n"), RPT_WARNING);
		ReleaseMemory();  //�ͷ��ڴ� 
		return FALSE;
	}  

	delete m_pdataFarm;
	if (!m_bRunCombDatFile)
	{
		ReleaseMemory();  //�ͷ��ڴ� 
		return FALSE;
	}

	return TRUE;
}

BOOL CCheckThread::ReadFile2()
{
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_bIsDataFile1 = FALSE;
	m_pdataFarm = new CDataFarm();
	if (!m_pdataFarm->mapfile(m_strPath2 + "\\index.dat", m_nMarketCount2, m_indexHead2.nBucketCount,
		m_indexHead2.nSymbolCount, m_indexHead2.nTickCount, m_indexHead2.nMinkCount))
	{
		REPORT(MN, T("mapfile() failed with file: %s\n", m_strPath2 + "\\index.dat"), RPT_ERROR);
		ReleaseMemory();  //�ͷ��ڴ� 
		return FALSE;
	}

	if (m_pdataFarm->m_hashIdx.for_each(*this) == 0)
	{
		REPORT(MN, T("Dat�ļ���û�й�Ʊ��Ϣ\n"), RPT_WARNING);
		ReleaseMemory();  //�ͷ��ڴ� 
		return FALSE;
	}

	delete m_pdataFarm;
	if (!m_bRunCombDatFile)
	{
		ReleaseMemory();  //�ͷ��ڴ� 
		return FALSE;
	}

	return TRUE;
}

void CCheckThread::CombinationDatFile()
{
	if (!ReadFile1())
		return;
	if (!ReadFile2())
		return;
	////////////////////////////////////////////////////////////////////////////////////////////////
	m_pdataFarm = new CDataFarm();
	if (!m_pdataFarm->mapfile(m_strPath3 + "\\index.dat", m_nMarketCount3, m_nSymbol,
		m_nSymbol, m_nTick, m_nMinK))
	{
		REPORT(MN, T("mapfile() failed with file: %s\n", m_strPath3 + "\\index.dat"), RPT_ERROR);
		ReleaseMemory();  //�ͷ��ڴ� 
		return;
	}

	OpenDataFile();
	CDataInfoMap::iterator pos = m_mapDataInfo.begin();
	while(pos != m_mapDataInfo.end())
	{
		if (!m_bRunCombDatFile)
			break;
		m_pStockDataInfo = pos->second;
		m_pdataFarm->m_hashIdx.cast(pos->second->strSymbol, *this, false);
		pos++;
	}				

	CloseDataFile(); 
	delete m_pdataFarm;			 
	if (!m_bRunCombDatFile)
	{
		ReleaseMemory();  //�ͷ��ڴ� 
		return;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////
	ReleaseMemory();  //�ͷ��ڴ� 
}

void CCheckThread::GetQuote(RINDEX *pRindex1, RINDEX *pRindex2, Quote** pQuote)
{
	Quote quote1 = {0};
	Quote quote2 = {0};

	if (pRindex1 != NULL)
	{
		m_QuoteFile1.SeekToBegin();
		m_QuoteFile1.Seek(pRindex1->idxQuote * sizeof(Quote)+sizeof(int*), 0);
		m_QuoteFile1.Read(&quote1, sizeof(Quote));
	}

	if (pRindex2 != NULL)
	{
		m_QuoteFile2.SeekToBegin();
		m_QuoteFile2.Seek(pRindex2->idxQuote * sizeof(Quote)+sizeof(int*), 0);
		m_QuoteFile2.Read(&quote2, sizeof(Quote));
	}

	if ( (m_nQuoteFile1 == 0 && pRindex1 != NULL) || pRindex2 == NULL )
	{
		memcpy(*pQuote, &quote1, sizeof(Quote));
	}
	else
	{
		memcpy(*pQuote, &quote2, sizeof(Quote));
	}
}

void CCheckThread::GetMink(RINDEX *pRindex1, RINDEX *pRindex2, RINDEX *pRindex)
{
	CMinUnitMap mapMinUnit;
	if (pRindex1 != NULL)
	{
		m_MinkFile1.SeekToBegin();
		m_MinkFile1.Seek(pRindex1->idxMinK * sizeof(MinBlock) + sizeof(int*), 0);
		MinBlock block = {0};
		m_MinkFile1.Read(&block, sizeof(MinBlock));

		int nIndex = 0;
		for (int i=0; i<pRindex1->cntMinK; i++)
		{
			mapMinUnit.insert(std::make_pair(block.unit[nIndex].Time, block.unit[nIndex]));

			nIndex++;
			if (nIndex == MINK_PERBLOCK)
			{
				nIndex = 0;
				int nNextIndex = block.next;
				if (nNextIndex == 0)
					break;

				m_MinkFile1.SeekToBegin();
				m_MinkFile1.Seek(nNextIndex * sizeof(MinBlock) + sizeof(int*), 0);
				memset(&block, 0, sizeof(MinBlock));
				m_MinkFile1.Read(&block, sizeof(MinBlock));
			}
		}
	}
	
	if (pRindex2 != NULL)
	{
		m_MinkFile2.SeekToBegin();
		m_MinkFile2.Seek(pRindex2->idxMinK * sizeof(MinBlock) + sizeof(int*), 0);
		MinBlock block = {0};
		m_MinkFile2.Read(&block, sizeof(MinBlock));

		int nIndex = 0;
		for (int i=0; i<pRindex2->cntMinK; i++)
		{
			CMinUnitMap::iterator find = mapMinUnit.find(block.unit[nIndex].Time);
			if (find == mapMinUnit.end())
			{
				mapMinUnit.insert(find, std::make_pair(block.unit[nIndex].Time, block.unit[nIndex]));
			}
			else
			{
				if (m_nMinFile1 != 0)
				{
					find->second = block.unit[nIndex];
				}
			}

			nIndex++;
			if (nIndex == MINK_PERBLOCK)
			{
				nIndex = 0;
				int nNextIndex = block.next;
				if (nNextIndex == 0)
					break;

				m_MinkFile2.SeekToBegin();
				m_MinkFile2.Seek(nNextIndex * sizeof(MinBlock) + sizeof(int*), 0);
				memset(&block, 0, sizeof(MinBlock));
				m_MinkFile2.Read(&block, sizeof(MinBlock));
			}
		}
	}

	CMinUnitMap::iterator pos = mapMinUnit.begin();
	while(pos != mapMinUnit.end())
	{
		m_pdataFarm->AddMink(pRindex, &pos->second);
		pos++;
	}
	mapMinUnit.clear();
}

void CCheckThread::GetTick(RINDEX *pRindex1, RINDEX *pRindex2, RINDEX *pRindex)
{
	CTickUnitMap mapTickUnit;
	if (pRindex1 != NULL)
	{
		m_TickFile1.SeekToBegin();
		m_TickFile1.Seek(pRindex1->idxTick * sizeof(TickBlock) + sizeof(int*), 0);
		TickBlock block = {0};
		m_TickFile1.Read(&block, sizeof(TickBlock));

		int nIndex = 0;
		unsigned int nNowTime = block.unit[nIndex].Time;
		int nCurTimeCount = 0;

		int nMaxCount = 50;
		TickUnit* pTickUnit = new TickUnit[nMaxCount];

		for (int i=0; i<pRindex1->cntTick; i++)
		{
			if (nNowTime == block.unit[nIndex].Time)
			{
				memcpy(&pTickUnit[nCurTimeCount], &block.unit[nIndex], sizeof(TickUnit));

				nCurTimeCount++;
				if ( (nCurTimeCount+1) > nMaxCount)
				{
					TickUnit* pNewTickUnit = new TickUnit[nMaxCount + 50];
					memcpy(pNewTickUnit, pTickUnit, sizeof(TickUnit) * nMaxCount);
					nMaxCount += 50;
					delete []pTickUnit;
					pTickUnit = pNewTickUnit;
				}
			}
			else
			{
				tagTickInfo* pTickInfo = new tagTickInfo;
				pTickInfo->nTickCount = nCurTimeCount;
				pTickInfo->pTickUnit = new TickUnit[nCurTimeCount];
				memcpy(pTickInfo->pTickUnit, pTickUnit, sizeof(TickUnit) * nCurTimeCount);

				mapTickUnit.insert(std::make_pair(nNowTime, pTickInfo));

				memset(pTickUnit, 0, sizeof(TickUnit) * nMaxCount);

				nCurTimeCount = 0;
				memcpy(&pTickUnit[nCurTimeCount], &block.unit[nIndex], sizeof(TickUnit));
				nCurTimeCount++;
				nNowTime = block.unit[nIndex].Time;
			}

			nIndex++;
			if (i == (pRindex1->cntTick - 1) ) //���һ��
			{
				tagTickInfo* pTickInfo = new tagTickInfo;
				pTickInfo->nTickCount = nCurTimeCount;
				pTickInfo->pTickUnit = new TickUnit[nCurTimeCount];
				memcpy(pTickInfo->pTickUnit, pTickUnit, sizeof(TickUnit) * nCurTimeCount);

				mapTickUnit.insert(std::make_pair(nNowTime, pTickInfo));
			}

			if (nIndex == TICK_PERBLOCK)
			{
				nIndex = 0;
				int nNextIndex = block.next;
				if (nNextIndex == 0)
					break;

				m_TickFile1.SeekToBegin();
				m_TickFile1.Seek(nNextIndex * sizeof(TickBlock) + sizeof(int*), 0);
				memset(&block, 0, sizeof(TickBlock));
				m_TickFile1.Read(&block, sizeof(TickBlock));
			}
		}

		delete []pTickUnit;
	}

	if (pRindex2 != NULL)
	{
		m_TickFile2.SeekToBegin();
		m_TickFile2.Seek(pRindex2->idxTick * sizeof(TickBlock) + sizeof(int*), 0);
		TickBlock block = {0};
		m_TickFile2.Read(&block, sizeof(TickBlock));

		int nIndex = 0;
		unsigned int nNowTime = block.unit[nIndex].Time;
		int nCurTimeCount = 0;

		int nMaxCount = 50;
		TickUnit* pTickUnit = new TickUnit[nMaxCount];

		for (int i=0; i<pRindex2->cntTick; i++)
		{
			if (nNowTime == block.unit[nIndex].Time)
			{
				memcpy(&pTickUnit[nCurTimeCount], &block.unit[nIndex], sizeof(TickUnit));

				nCurTimeCount++;
				if ( (nCurTimeCount+1) > nMaxCount)
				{
					TickUnit* pNewTickUnit = new TickUnit[nMaxCount + 50];
					memcpy(pNewTickUnit, pTickUnit, sizeof(TickUnit) * nMaxCount);
					nMaxCount += 50;
					delete []pTickUnit;
					pTickUnit = pNewTickUnit;
				}
			}
			else
			{
				tagTickInfo* pTickInfo = new tagTickInfo;
				pTickInfo->nTickCount = nCurTimeCount;
				pTickInfo->pTickUnit = new TickUnit[nCurTimeCount];
				memcpy(pTickInfo->pTickUnit, pTickUnit, sizeof(TickUnit) * nCurTimeCount);

				CTickUnitMap::iterator find = mapTickUnit.find(nNowTime);
				if (find == mapTickUnit.end())
				{
					mapTickUnit.insert(std::make_pair(nNowTime, pTickInfo));
				}
				else
				{
					if (m_nTickFile1 != 0) //ʹ�õڶ���dat�ļ�
					{
						delete[]find->second->pTickUnit;
						delete find->second;
						find->second = pTickInfo;
					}
					else
					{
						delete[]pTickInfo->pTickUnit;
						delete pTickInfo;
					}
				}

				memset(pTickUnit, 0, sizeof(TickUnit) * nMaxCount);

				nCurTimeCount = 0;
				memcpy(&pTickUnit[nCurTimeCount], &block.unit[nIndex], sizeof(TickUnit));
				nCurTimeCount++;
				nNowTime = block.unit[nIndex].Time;
			}

			nIndex++;
			if (i == (pRindex2->cntTick - 1) ) //���һ��
			{
				tagTickInfo* pTickInfo = new tagTickInfo;
				pTickInfo->nTickCount = nCurTimeCount;
				pTickInfo->pTickUnit = new TickUnit[nCurTimeCount];
				memcpy(pTickInfo->pTickUnit, pTickUnit, sizeof(TickUnit) * nCurTimeCount);

				CTickUnitMap::iterator find = mapTickUnit.find(nNowTime);
				if (find == mapTickUnit.end())
				{
					mapTickUnit.insert(std::make_pair(nNowTime, pTickInfo));
				}
				else
				{
					if (m_nTickFile1 != 0)
					{
						delete[]find->second->pTickUnit;
						delete find->second;
						find->second = pTickInfo;
					}
					else
					{
						delete[]pTickInfo->pTickUnit;
						delete pTickInfo;
					}
				}
			}

			if (nIndex == TICK_PERBLOCK)
			{
				nIndex = 0;
				int nNextIndex = block.next;
				if (nNextIndex == 0)
					break;

				m_TickFile2.SeekToBegin();
				m_TickFile2.Seek(nNextIndex * sizeof(TickBlock) + sizeof(int*), 0);
				memset(&block, 0, sizeof(TickBlock));
				m_TickFile2.Read(&block, sizeof(TickBlock));
			}
		}

		delete []pTickUnit;
	}

	CTickUnitMap::iterator pos = mapTickUnit.begin();
	while(pos != mapTickUnit.end())
	{
		for (unsigned int i=0; i<pos->second->nTickCount; i++)
			m_pdataFarm->AddTick(pRindex, &pos->second->pTickUnit[i]);
		
		delete []pos->second->pTickUnit;
		delete pos->second;

		pos++;
	}
	mapTickUnit.clear();
}

void CCheckThread::OpenDataFile()
{
	try
	{
		CString strtemp = m_strPath1 + "\\quote.dat";
		if (!m_QuoteFile1.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("���ļ�[%s]ʧ�� �������:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath2 + "\\quote.dat";
		if (!m_QuoteFile2.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("���ļ�[%s]ʧ�� �������:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath1 + "\\mink.dat";
		if (!m_MinkFile1.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("���ļ�[%s]ʧ�� �������:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath2 + "\\mink.dat";
		if (!m_MinkFile2.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("���ļ�[%s]ʧ�� �������:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath1 + "\\tick.dat";
		if (!m_TickFile1.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("���ļ�[%s]ʧ�� �������:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath2 + "\\tick.dat";
		if (!m_TickFile2.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("���ļ�[%s]ʧ�� �������:%d\n", strtemp, GetLastError()), RPT_ERROR);

	}
	catch(...)
	{
		REPORT(MN, T("���ļ������쳣\n"), RPT_ERROR);
	}
}

void CCheckThread::CloseDataFile()
{
	try
	{
		m_QuoteFile1.Close();
		m_QuoteFile2.Close();

		m_MinkFile1.Close();
		m_MinkFile2.Close();

		m_TickFile1.Close();
		m_TickFile2.Close();
	}
	catch(...)
	{
		REPORT(MN, T("���ļ������쳣\n"), RPT_ERROR);
	}
}

void CCheckThread::ReleaseMemory()
{
	{
		CDataInfoMap::iterator pos = m_mapDataInfo.begin();
		while(pos != m_mapDataInfo.end())
		{
			if (pos->second->pidx1 != NULL)
				delete pos->second->pidx1;
			if (pos->second->pidx2 != NULL)
				delete pos->second->pidx2;
			delete pos->second;
			pos++;
		}
		m_mapDataInfo.clear();
	}
}

long CCheckThread::cast(std::string *pStr, RINDEX *pRindex, bool bQuitIfNoKey)
{
	if (bQuitIfNoKey)
	{
		//if ((*pStr) == "HSI.HK")
		//	int i=0;
		m_pdataFarm->AddSymbolIndex(m_pStockDataInfo->strSymbol, pRindex);
		m_pdataFarm->AddPydm(m_pStockDataInfo->strSymbol, m_pStockDataInfo->strName);

		Quote* pQuote = m_pdataFarm->GetQuote(pRindex->idxQuote);
		GetQuote(m_pStockDataInfo->pidx1, m_pStockDataInfo->pidx2, &pQuote);
		//
		GetMink(m_pStockDataInfo->pidx1, m_pStockDataInfo->pidx2, pRindex);
		//
		GetTick(m_pStockDataInfo->pidx1, m_pStockDataInfo->pidx2, pRindex);
	}
	return 0;
}

int CCheckThread::OnElement(std::string *pSymbol, RINDEX *pIdx)
{
	if ( (*pSymbol).empty() )
		return 1;

	//if ((*pSymbol) == "HSI.HK")
	//	int i=0;
	Quote *pQuote = m_pdataFarm->GetQuote(pIdx->idxQuote);

	CDataInfoMap::iterator find = m_mapDataInfo.find((*pSymbol));
	if (find == m_mapDataInfo.end())
	{
		tagStockDataInfo* pInfo = new tagStockDataInfo();
		memset(pInfo, 0, sizeof(tagStockDataInfo));
		strcpy_s(pInfo->strSymbol, 20, (*pSymbol).c_str());
		strcpy_s(pInfo->strName, 30, pQuote->szStockName);

		if (m_bIsDataFile1)
		{
			pInfo->pidx1 = new RINDEX;
			memcpy(pInfo->pidx1, pIdx, sizeof(RINDEX));
		}
		else
		{
			pInfo->pidx2 = new RINDEX;
			memcpy(pInfo->pidx2, pIdx, sizeof(RINDEX));
		}

		m_mapDataInfo.insert(std::make_pair((*pSymbol), pInfo));
	}
	else
	{
		if (m_bIsDataFile1)
		{
			find->second->pidx1 = new RINDEX;
			memcpy(find->second->pidx1, pIdx, sizeof(RINDEX));
		}
		else
		{
			find->second->pidx2 = new RINDEX;
			memcpy(find->second->pidx2, pIdx, sizeof(RINDEX));
		}
	}
	return 1;
}

void CCheckThread::StopCombinationDatFile()
{
	m_bRunCombDatFile = FALSE;
}
