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
				AddLog(-1, FALSE, "检测所有文件被终止!");
			AddLog(-1, FALSE, "\n========== 已检测文件: %d 错误文件: %d ==========",
				m_nCheckedFile, m_nErrFile);
			break;
		}
	case CMD_CHECK_ALL_USE_HTTP_DATA:
		{
			AddLog(-1, FALSE, "正在使用网上数据校对...");
			CheckAllFileUseHttpData(m_strDirectory);
			if (m_bRun)
				AddLog(-1, FALSE, "使用网上数据校对完毕");
			else
				AddLog(-1, FALSE, "使用网上数据校被终止");
			break;
		}
	case CMD_CHECK_SINGLE_USE_HTTP_DATA:
		{
			CString strFolderName = CValidFunction::GetFileNamePreFolder(m_strFilePath);
			if (strFolderName.CompareNoCase("day") == 0) //只有日线文件可以直接到网上校对
			{
				AddLog(-1, FALSE, "正在使用网上数据校对文件[%s]...", m_strFilePath);
				CheckSingleFileUseHttpData();
				if (m_bRun)
					AddLog(-1, FALSE, "使用网上数据校对完毕");
				else
					AddLog(-1, FALSE, "使用网上数据校被终止");
			}
			else if (strFolderName.CompareNoCase("wek") == 0)
			{
				AddLog(-1, FALSE, "不可网上校对的周线文件:[%s]", m_strFilePath);
			}
			else if (strFolderName.CompareNoCase("mon") == 0)
			{
				AddLog(-1, FALSE, "不可网上校对的月线文件:[%s]", m_strFilePath);
			}
			else
			{
				AddLog(-1, FALSE, "不可检测的文件:[%s]", m_strFilePath);
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
				AddLog(-1, FALSE, "正在检测文件大小...");
				m_nCheckedFile = 0;

				CheckFileSize(m_strCheckSizeFilePath);
				if (m_bRun)
					AddLog(-1, FALSE, "检测文件大小完毕");
				else
					AddLog(-1, FALSE, "检测文件大小被终止");
				AddLog(-1, FALSE, "\n========== 已检测文件: %d ==========", m_nCheckedFile);
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
// 		AddLog(-1, FALSE, "路径[%s]不存在", strLocalFile);
// 		if (!CValidFunction::CreatePath(strLocalFile))
// 		{
// 			AddLog(-1, FALSE, "创建路径[%s]失败", strLocalFile);
// 			return FALSE;
// 		}
// 		else
// 		{
// 			AddLog(-1, FALSE, "创建路径[%s]成功", strLocalFile);
// 		}
// 	}
// 	strLocalFile += "\\KLine.rar";
// 
// 	AddLog(-1, FALSE, "正在连接FTPServer[%s]...", theApp.m_strServerAddr);
// 	CFTPTranData ftp;
// 	m_pFtp = &ftp;
// 	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
// 	{
// 		AddLog(-1, FALSE, "连接FTPServer[%s]失败", theApp.m_strServerAddr);
// 		return FALSE;
// 	}
// 	AddLog(-1, FALSE, "连接成功，正在下载文件...");
// 
// 	CString strFTPPath = theApp.m_strFTPPath;
// 	if (!strFTPPath.IsEmpty())
// 		strFTPPath += '/';
// 
// 	strFTPPath += "KLine.rar";
// 	if (!ftp.GetFile(strFTPPath, strLocalFile))
// 	{
// 		AddLog(-1, FALSE, "从服务器[%s]下载文件到[%s]失败", strFTPPath, strLocalFile);
// 		ftp.Close();
// 		return FALSE;
// 	}
// 	
// 	ftp.Close();
// 	AddLog(-1, FALSE, "从服务器[%s]下载文件到[%s]成功", strFTPPath, strLocalFile);
// 	
// 	Sleep(1000);
// 	///下载后解压文件
// 	CString strRarFile = theApp.m_strKFilePath;
// 	strRarFile += "\\KLine.rar";
// 
// 	if (!CValidFunction::IsFileExist(strRarFile))
// 	{
// 		AddLog(-1, FALSE, "压缩文件[%s]不存在", strRarFile);
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
// 	AddLog(-1, FALSE, "正在解压k线文件[%s\\KLine.rar]...", theApp.m_strKFilePath);
// 	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
// 		AddLog(-1, FALSE, "解压失败");
// 
// 	KillRarProcess();
// 	AddLog(-1, FALSE, "解压成功");
// 	//下载解压成功后删除文件
// 	if (::DeleteFile(strLocalFile))
// 		AddLog(-1, FALSE, "删除压缩文件[%s]成功", strLocalFile);
// 	else
// 		AddLog(-1, FALSE, "删除压缩文件[%s]失败 code:%d", strLocalFile, GetLastError());
// 	return TRUE;
// }

BOOL CCheckThread::DownLoadHQKLine()
{
// 	string strFolder = CFilePath::GetPathLastFolderName(theApp.m_strKFilePath);
	CString strHQCur = theApp.m_strKFilePath/* + "\\" + strFolder.c_str()*/;

	CString strLocalFile = strHQCur;
	if (!CValidFunction::IsPathExist(strLocalFile))
	{
		AddLog(-1, FALSE, "路径[%s]不存在", strLocalFile);
		if (!CValidFunction::CreatePath(strLocalFile))
		{
			AddLog(-1, FALSE, "创建路径[%s]失败", strLocalFile);
			return FALSE;
		}
		else
		{
			AddLog(-1, FALSE, "创建路径[%s]成功", strLocalFile);
		}
	}
	strLocalFile += "\\HQKLine.rar";

	AddLog(-1, FALSE, "正在连接FTPServer[%s]...", theApp.m_strServerAddr);
	CFTPTranData ftp;
	m_pFtp = &ftp;
	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
	{
		AddLog(-1, FALSE, "连接FTPServer[%s]失败", theApp.m_strServerAddr);
		return FALSE;
	}
	AddLog(-1, FALSE, "连接成功，正在下载文件...");

	CString strFTPPath = theApp.m_strHQFTPPath;
	if (!strFTPPath.IsEmpty())
		strFTPPath += '/';

	strFTPPath += "HQKLine.rar";
	if (!ftp.GetFile(strFTPPath, strLocalFile))
	{
		AddLog(-1, FALSE, "从服务器[%s]下载文件到[%s]失败", strFTPPath, strLocalFile);
		ftp.Close();
		return FALSE;
	}
	
	ftp.Close();
	AddLog(-1, FALSE, "从服务器[%s]下载文件到[%s]成功", strFTPPath, strLocalFile);
	
	Sleep(1000);
	///下载后解压文件
	CString strRarFile = strHQCur;
	strRarFile += "\\HQKLine.rar";

	if (!CValidFunction::IsFileExist(strRarFile))
	{
		AddLog(-1, FALSE, "压缩文件[%s]不存在", strRarFile);
		return FALSE;
	}

	CString strRarPath;
	if (!GetRarSoftWarePath(strRarPath))
		return FALSE;

	strRarPath += "\\Rar.exe";
	CString strPreFolderPath = strHQCur;
	CString strFormat;
	strFormat.Format("x -o+ %s\\HQKLine.rar %s\\", strHQCur, strPreFolderPath);

	AddLog(-1, FALSE, "正在解压k线文件[%s\\HQKLine.rar]...", strHQCur);
	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
		AddLog(-1, FALSE, "解压失败");

	KillRarProcess();
	AddLog(-1, FALSE, "解压成功");
	//下载解压成功后删除文件
	if (::DeleteFile(strLocalFile))
		AddLog(-1, FALSE, "删除压缩文件[%s]成功", strLocalFile);
	else
		AddLog(-1, FALSE, "删除压缩文件[%s]失败", strLocalFile);
	return TRUE;
}

BOOL CCheckThread::GetRarSoftWarePath(CString& strPath)
{
	//rar.exe程序一起打包
	::CValidFunction::GetWorkPath(strPath);
	/*char chPath[MAX_PATH];
	try
	{
		CRegKey regkey;
		if (regkey.Open(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\winrar.EXE",
			KEY_READ) != ERROR_SUCCESS)
		{
			AddLog(-1, FALSE, "错误: 获取rar.exe路径失败");
			return FALSE;
		}
		ULONG nSize = MAX_PATH;
		if (regkey.QueryStringValue("Path", chPath, &nSize) != ERROR_SUCCESS)
		{
			AddLog(-1, FALSE, "错误: 获取rar.exe路径失败");
			return FALSE;
		}
		regkey.Close();
	}
	catch(...)
	{
		AddLog(-1, FALSE, "错误: 获取rar.exe路径失败");
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
// 		AddLog(-1, FALSE, "路径[%s]不存在", theApp.m_strKFilePath);
// 		return FALSE;
// 	}
// 	strFormat.Format("a -ep1 %s\\KLine.rar %s", theApp.m_strKFilePath, theApp.m_strKFilePath);
// 
// 	AddLog(-1, FALSE, "正在压缩k线文件[%s\\KLine.rar]...", theApp.m_strKFilePath);
// 	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
// 		AddLog(-1, FALSE, "压缩失败");
// 
// 	Sleep(1000); //等待文件生成
// 	KillRarProcess();
// 
// 	CString strLocalFile = theApp.m_strKFilePath;
// 	strLocalFile += "\\KLine.rar";
// 	if (!CValidFunction::IsFileExist(strLocalFile))
// 	{
// 		AddLog(-1, FALSE, "文件[%s]不存在, 上传失败", strLocalFile);
// 		return FALSE;
// 	}
// 	else
// 	{
// 		AddLog(-1, FALSE, "压缩文件成功");
// 	}
// 
// 	AddLog(-1, FALSE, "正在连接FTPServer[%s]...", theApp.m_strServerAddr);
// 	CFTPTranData ftp;
// 	m_pFtp = &ftp;
// 	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
// 	{
// 		AddLog(-1, FALSE, "连接FTPServer[%s]失败", theApp.m_strServerAddr);
// 		return FALSE;
// 	}
// 	AddLog(-1, FALSE, "连接成功，正在上传文件...");
// 
// 	CString strFTPPath = theApp.m_strFTPPath;
// 	if (!strFTPPath.IsEmpty())
// 		strFTPPath += '/';
// 
// 	strFTPPath += "KLine.rar";
// 	if (!ftp.PutFile(strLocalFile, strFTPPath))
// 	{
// 		AddLog(-1, FALSE, "上传文件[%s]到服务器[%s]失败", strLocalFile, strFTPPath);
// 		ftp.Close();
// 		return FALSE;
// 	}
// 	
// 	ftp.Close();
// 	AddLog(-1, FALSE, "上传文件[%s]到服务器[%s]成功", strLocalFile, strFTPPath);
// 	//上传成功后删除文件
// 	Sleep(1000);
// 
// 	if (::DeleteFile(strLocalFile))
// 		AddLog(-1, FALSE, "删除压缩文件[%s]成功", strLocalFile);
// 	else
// 		AddLog(-1, FALSE, "删除压缩文件[%s]失败", strLocalFile);
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
// 		AddLog(-1, FALSE, "路径[%s]不存在", strHQCur);
// 		return FALSE;
// 	}
	strFormat.Format("a -ep1 %s\\HQKLine.rar \"%s\"", strHQCur, strHQRar);

	AddLog(-1, FALSE, "正在压缩k线文件[%s\\HQKLine.rar]...", strHQCur);
	if (ShellExecute(NULL, "open", strRarPath, strFormat, NULL, SW_HIDE) <= (HINSTANCE)32)
		AddLog(-1, FALSE, "压缩失败");

	Sleep(1000); //等待文件生成
	KillRarProcess();

	CString strLocalFile = strHQCur;
	strLocalFile += "\\HQKLine.rar";
	if (!CValidFunction::IsFileExist(strLocalFile))
	{
		AddLog(-1, FALSE, "文件[%s]不存在, 上传失败", strLocalFile);
		return FALSE;
	}
	else
	{
		AddLog(-1, FALSE, "压缩文件成功");
	}

	AddLog(-1, FALSE, "正在连接FTPServer[%s]...", theApp.m_strServerAddr);
	CFTPTranData ftp;
	m_pFtp = &ftp;
	if (!ftp.Open(theApp.m_strServerAddr, theApp.m_strUserName, theApp.m_strPassword))
	{
		AddLog(-1, FALSE, "连接FTPServer[%s]失败", theApp.m_strServerAddr);
		return FALSE;
	}
	AddLog(-1, FALSE, "连接成功，正在上传文件...");

	CString strFTPPath = theApp.m_strHQFTPPath;
	if (!strFTPPath.IsEmpty())
		strFTPPath += '/';

	strFTPPath += "HQKLine.rar";
	if (!ftp.PutFile(strLocalFile, strFTPPath))
	{
		AddLog(-1, FALSE, "上传文件[%s]到服务器[%s]失败", strLocalFile, strFTPPath);
		ftp.Close();
		return FALSE;
	}
	
	ftp.Close();
	AddLog(-1, FALSE, "上传文件[%s]到服务器[%s]成功", strLocalFile, strFTPPath);
	//上传成功后删除文件
	Sleep(1000);
	if (::DeleteFile(strLocalFile))
		AddLog(-1, FALSE, "删除压缩文件[%s]成功", strLocalFile);
	else
		AddLog(-1, FALSE, "删除压缩文件[%s]失败", strLocalFile);
	   
	return TRUE;
}

void CCheckThread::CheckWeekFile(CString strFile, BOOL bCheckLastOne)
{
	CString strExe = CValidFunction::GetExeName(strFile);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]!", m_strFilePath);
		return;
	}

	if (!m_file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", strFile);
		AddLog(-1, FALSE, "检测被终止!");
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
			//DW("[%d] 开盘:%d 最高:%d 最低:%d 收盘:%d 数量:%d 金额:%d", klInfo.nDate, newInfo.nOpenPrice, newInfo.nMaxPrice, newInfo.nMinPrice,
			//	newInfo.nClosePrice, newInfo.nVolume, newInfo.nSum);
			BOOL bRet = TRUE;
			if (klInfo.nOpenPrice != newInfo.nOpenPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：[%d]开盘价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nDate, klInfo.nOpenPrice, newInfo.nOpenPrice);
				AddLog(-1, FALSE, "[%d]开盘价格修改为:%d", klInfo.nDate, newInfo.nOpenPrice);
			}

			if (klInfo.nMaxPrice != newInfo.nMaxPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：[%d]最高价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nDate, klInfo.nMaxPrice, newInfo.nMaxPrice);
				AddLog(-1, FALSE, "[%d]最高价格修改为:%d", klInfo.nDate, newInfo.nMaxPrice);
			}

			if (klInfo.nMinPrice != newInfo.nMinPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：[%d]最低价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nDate, klInfo.nMinPrice, newInfo.nMinPrice);
				AddLog(-1, FALSE, "[%d]最低价格修改为:%d", klInfo.nDate, newInfo.nMinPrice);
			}

			if (klInfo.nClosePrice != newInfo.nClosePrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：[%d]收盘价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nDate, klInfo.nClosePrice, newInfo.nClosePrice);
				AddLog(-1, FALSE, "[%d]收盘价格修改为:%d", klInfo.nDate, newInfo.nClosePrice);
			}

			if (klInfo.nVolume != newInfo.nVolume)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：[%d]成交量不一样, 原始数据:%d 检测数据:%d",
					klInfo.nDate, klInfo.nVolume, newInfo.nVolume);
				AddLog(-1, FALSE, "[%d]成交量修改为:%d", klInfo.nDate, newInfo.nVolume);
			}

			if (klInfo.nSum != newInfo.nSum)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：[%d]成交金额不一样, 原始数据:%d 检测数据:%d",
					klInfo.nDate, klInfo.nSum, newInfo.nSum);
				AddLog(-1, FALSE, "[%d]成交金额修改为:%d", klInfo.nDate, newInfo.nSum);
			}

			if (!bRet) //数据不正确
			{
				bFileChanged = TRUE;;
				if (m_file.SetRecordBuffer((const char*)&newInfo))
					AddLog(-1, FALSE, "[%d]保存修改数据成功", klInfo.nDate);
				else
					AddLog(-1, FALSE, "[%d]保存修改数据失败", klInfo.nDate);
			}
		}
		m_file.MoveNext();
	}

	if (bFileChanged)
	{
		if (!m_file.WriteData())
			AddLog(-1, FALSE, "错误：保存文件[%s]失败", strFile);
		else
			AddLog(-1, FALSE, "保存文件[%s]成功", strFile);
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			CString strFolderName = ffind.GetFileName();

			if (strFolderName.CompareNoCase("mon") == 0) //只检测mon文件
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
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", strFile);
		AddLog(-1, FALSE, "检测被终止!");
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
			//DW("[%d] 开盘:%d 最高:%d 最低:%d 收盘:%d 数量:%d 金额:%d", klInfo.nDate, newInfo.nOpenPrice, newInfo.nMaxPrice, newInfo.nMinPrice,
			//	newInfo.nClosePrice, newInfo.nVolume, newInfo.nSum);
			BOOL bRet = TRUE;
			if (klInfo.nOpenPrice != newInfo.nOpenPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：开盘价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nOpenPrice, newInfo.nOpenPrice);
				AddLog(-1, FALSE, "[%d]开盘价格修改为:%d", klInfo.nDate, newInfo.nOpenPrice);
			}

			if (klInfo.nMaxPrice != newInfo.nMaxPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：最高价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nMaxPrice, newInfo.nMaxPrice);
				AddLog(-1, FALSE, "[%d]最高价格修改为:%d", klInfo.nDate, newInfo.nMaxPrice);
			}

			if (klInfo.nMinPrice != newInfo.nMinPrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：最低价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nMinPrice, newInfo.nMinPrice);
				AddLog(-1, FALSE, "[%d]最低价格修改为:%d", klInfo.nDate, newInfo.nMinPrice);
			}

			if (klInfo.nClosePrice != newInfo.nClosePrice)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：收盘价格不一样, 原始数据:%d 检测数据:%d",
					klInfo.nClosePrice, newInfo.nClosePrice);
				AddLog(-1, FALSE, "[%d]收盘价格修改为:%d", klInfo.nDate, newInfo.nClosePrice);
			}

			if (klInfo.nVolume != newInfo.nVolume)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：成交量不一样, 原始数据:%d 检测数据:%d",
					klInfo.nVolume, newInfo.nVolume);
				AddLog(-1, FALSE, "[%d]成交量修改为:%d", klInfo.nDate, newInfo.nVolume);
			}

			if (klInfo.nSum != newInfo.nSum)
			{
				bRet = FALSE;
				AddLog(-1, FALSE, "错误：成交金额不一样, 原始数据:%d 检测数据:%d",
					klInfo.nSum, newInfo.nSum);
				AddLog(-1, FALSE, "[%d]成交金额修改为:%d", klInfo.nDate, newInfo.nSum);
			}

			if (!bRet) //数据不正确
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
			AddLog(-1, FALSE, "错误：保存文件[%s]失败", strFile);
		else
			AddLog(-1, FALSE, "保存文件[%s]成功", strFile);
	}
}

BOOL CCheckThread::GetMonData(tagKLineInfo *pklInfo, CString strFile)
{
	CString strExe = CValidFunction::GetExeName(strFile);
	if (strExe.CompareNoCase(".sz") != 0 && strExe.CompareNoCase(".szidx") != 0
		&& strExe.CompareNoCase(".sh") != 0 && strExe.CompareNoCase(".shidx") != 0
		&& strExe.CompareNoCase(".hk") != 0)
	{
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]!", m_strFilePath);
		return FALSE;
	}

	CKLineFile  file;
	if (!file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", strFile);
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
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]!", m_strFilePath);
		return FALSE;
	}
	CKLineFile  file;
	if (!file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", strFile);
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
			AddLog(-1, FALSE, "错误: 文件[%s]GetRecordToBuffer()失败!", strFile);
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			CString strFolderName = ffind.GetFileName();

			if (strFolderName.CompareNoCase("wek") == 0) //只检测wek文件
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
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
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", m_strFilePath);
		return;
	}
	
	m_nCheckedFile++;  //已检测文件数量
	int nRecCount  = m_file.GetRecordCount();
	int nReserveCount = theApp.m_nReserveCount < (UINT)nRecCount ? theApp.m_nReserveCount:nRecCount;
	int nDelCount  = nRecCount - nReserveCount;
	int nRecSize   = m_file.GetRecordSize();
	if (nDelCount == 0)
	{
		m_file.Close();
		return;
	}
	
	if (theApp.m_nDeleteOtiose == 0)   //保存要删除的记录
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
		
		//AddLog(-1, FALSE, "保存多余的记录到历史文件[%s]!", strHisName);
		CKLineFile kfileHis;
		if (!kfileHis.Open(strHisName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS))
		{
			AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", strHisName);
			m_file.Close();
			return;
		}

		if (!kfileHis.AllocateMemoryForSave(nDelCount))
		{
			m_file.Close();
			kfileHis.Close();
			AddLog(-1, FALSE, "错误: 给文件[%s]分配内存失败!", strHisName);
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
			AddLog(-1, FALSE, "错误: 保存文件[%s]失败!", strHisName);
		}

		kfileHis.Close();
	}
	
	{//删除多余的记录
		
		//AddLog(-1, FALSE, "[%s]删除多余的记录!", strFile);
		char* pRecBuf = new char[nRecSize*nReserveCount];
		if (pRecBuf == NULL)
		{
			AddLog(-1, FALSE, "错误: 分配内存失败!");
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
			AddLog(-1, FALSE, "错误: 给文件[%s]分配内存失败!", strFile);
			m_file.Close();
			return;
		}

		if (!m_file.SetAllRecordToBuffer(pRecBuf))
		{
			AddLog(-1, FALSE, "错误: 保存文件[%s]失败!", strFile);
			m_file.Close();
			return;
		}
		if (!m_file.WriteData())
		{
			AddLog(-1, FALSE, "错误: 保存文件[%s]失败!", strFile);
		}

		delete []pRecBuf;
		pRecBuf = NULL;
	}
	m_file.Close();
	//////////////////////////////////////
}

void CCheckThread::StartRarUpLoadKFile(BOOL bAuto)
{
	/*if (bAuto)   //是否自动检测完毕时启用
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			CString strFolderName = ffind.GetFileName();

			if (strFolderName.CompareNoCase("day") == 0) //只有日线文件可以直接到网上校对
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
			AddLog(-1, FALSE, "正在检测文件[%s]", m_strFilePath);
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
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(m_strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", m_strFilePath);
		AddLog(-1, FALSE, "校对被终止!");
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
		AddLog(-1, FALSE, "错误: 不能校对的文件[%s]", m_strFileName);
		AddLog(-1, FALSE, "校对被终止!");
		m_file.Close();
		return;
	}
	strURL.Format(HTTPFORMAT, strMarket, strCode);
	if(!m_DownHttpData.OpenURL(strURL, strMarket))
	{
		AddLog(-1, FALSE, "错误: 打开网页[%s]失败", strURL);
		AddLog(-1, FALSE, "校对被终止!");
		m_file.Close();
		return;
	}

	tagKLineInfo klInfoHttp = {0};
	if (!m_DownHttpData.GetKLineData(&klInfoHttp))
	{
		AddLog(-1, FALSE, "错误: 从网页获取股票[%s.%s]信息失败", strCode, strMarket);
		AddLog(-1, FALSE, "校对被终止!");
		m_file.Close();
		return;
	}

	//开始检测数据
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
		AddLog(-1, FALSE, "没有与网上日期匹配的记录");
		m_file.Close();
		return;
	}

	BOOL bRet = TRUE;

	if (klInfoHttp.nClosePrice != klInfo.nClosePrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "错误：[%d]收盘价格不一样，网上：%d 本地：%d", 
			klInfoHttp.nDate, klInfoHttp.nClosePrice, klInfo.nClosePrice);
		klInfo.nClosePrice = klInfoHttp.nClosePrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]收盘价格修改为%d", klInfoHttp.nDate, klInfoHttp.nClosePrice);
	}
	if (klInfoHttp.nMaxPrice != klInfo.nMaxPrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "错误：[%d]最高价不一样，网上：%d 本地：%d", 
			klInfoHttp.nDate, klInfoHttp.nMaxPrice, klInfo.nMaxPrice);
		klInfo.nMaxPrice = klInfoHttp.nMaxPrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]最高价修改为%d", klInfoHttp.nDate, klInfoHttp.nMaxPrice);
	}
	if (klInfoHttp.nMinPrice != klInfo.nMinPrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "错误：[%d]最低价不一样，网上：%d 本地：%d", 
			klInfoHttp.nDate, klInfoHttp.nMinPrice, klInfo.nMinPrice);
		klInfo.nMinPrice = klInfoHttp.nMinPrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]最低价修改为%d", klInfoHttp.nDate, klInfoHttp.nMinPrice);
	}
	if (klInfoHttp.nOpenPrice != klInfo.nOpenPrice)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "错误：[%d]开盘价不一样，网上：%d 本地：%d", 
			klInfoHttp.nDate, klInfoHttp.nOpenPrice, klInfo.nOpenPrice);
		klInfo.nOpenPrice = klInfoHttp.nOpenPrice;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]开盘价修改为%d", klInfoHttp.nDate, klInfoHttp.nOpenPrice);
	}
	if (klInfoHttp.nVolume != klInfo.nVolume)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "错误：[%d]成交量不一样，网上：%d 本地：%d", 
			klInfoHttp.nDate, klInfoHttp.nVolume, klInfo.nVolume);
		klInfo.nVolume = klInfoHttp.nVolume;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]成交量修改为%d", klInfoHttp.nDate, klInfoHttp.nVolume);
	}
	if (klInfoHttp.nSum != klInfo.nSum)
	{
		bRet = FALSE;
		AddLog(-1, FALSE, "错误：[%d]成交金额不一样，网上：%d 本地：%d", 
			klInfoHttp.nDate, klInfoHttp.nSum, klInfo.nSum);
		klInfo.nSum = klInfoHttp.nSum;
		if (theApp.m_bUpdateData)
			AddLog(-1, FALSE, "[%d]成交金额修改为%d", klInfoHttp.nDate, klInfoHttp.nSum);
	}
	if (!bRet)
	{
		if (theApp.m_bUpdateData)
		{
			if(m_file.SetRecordBuffer((char*)&klInfo))
			{
				if(!m_file.WriteData())
					AddLog(-1, FALSE, "错误：保存[%d]更改数据失败", klInfo.nDate);
				else
					AddLog(-1, FALSE, "保存[%d]更改数据成功", klInfo.nDate);
			}
			else
			{
				AddLog(-1, FALSE, "错误：保存[%d]更改数据失败", klInfo.nDate);
			}
		}
	}

	AddLog(-1, FALSE, "校对完毕");
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
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
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]!", m_strFilePath);
		return;
	}
	if (!m_file.Open(m_strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", m_strFilePath);
		AddLog(-1, FALSE, "检测被终止!");
		return;
	}

	UINT nFileType = 0;
	{//判断文件类型 日线、周线、月线
		CString strFolder = CValidFunction::GetFileNamePreFolder(m_strFilePath);
		if (strFolder.CompareNoCase("day") == 0)
			nFileType = FT_DAY;
		else if(strFolder.CompareNoCase("wek") == 0)
			nFileType = FT_WEK;
		else if(strFolder.CompareNoCase("mon") == 0)
			nFileType = FT_MON;
	}
	tagKLineInfo klInfo;

	AddLog(-1, FALSE, "正在检测[%s]...", m_strFilePath);
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
			AddLog(-1, FALSE, "检测被取消...");
			break;
		}
		m_file.GetRecordToBuffer((char*)&klInfo);
		bRet &= ProcessRecord(&klInfo, bShowRecord, nFileType);

		m_file.MoveNext();
	}

	if (m_bRun)
	{
		AddLog(-1, FALSE, "检测完毕"); 
	}

	if (bShowRecord)
	{
		if (nRecordCount == 0)
			AddLog(-1, FALSE, "文件中没有记录"); 
		AddLog(-1, FALSE, "========== 总数: %d 已检测: %d 错误: %d ==========", 
			nRecordCount, m_nCheckedCount, m_nErrCount);
	}
	m_nCheckedFile++;
	
	if (!bRet)  //有错误记录的文件
	{
		m_nErrFile++;
		if (m_hListFileWnd != NULL && !bShowRecord)
		{
			CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListFileWnd);
			int nIndex = pCtrl->GetItemCount();
			AddLog(nIndex, FALSE, "文件中含有错误记录");
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
	//检测数据的正确性

	BOOL bRet = TRUE;
	if (pKLInfo->nDate > 20300000 || !IsValidDate(pKLInfo->nDate))
	{
		bRet = FALSE;
		if (bShowRecord)
		{
			pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 0);
			AddLog(nIndex, TRUE, "错误: %d行 %d列  日期[%d]不正确 ",
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
				strErrTemp.Format("最高价[%d]低于开盘价[%d]; ", pKLInfo->nMaxPrice,
					pKLInfo->nOpenPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMaxPrice < pKLInfo->nMinPrice)
			{
				strErrTemp.Format("最高价[%d]低于最低价[%d]; ", pKLInfo->nMaxPrice,
					pKLInfo->nMinPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMaxPrice < pKLInfo->nClosePrice)
			{
				strErrTemp.Format("最高价[%d]低于收盘价[%d]; ", pKLInfo->nMaxPrice,
					pKLInfo->nClosePrice);
				strErr += strErrTemp;
			}
			AddLog(nIndex, TRUE, "错误: %d行 %d列  %s", nIndex+1, 3, strErr);
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
				strErrTemp.Format("最低价[%d]高于开盘价[%d]; ", pKLInfo->nMinPrice,
					pKLInfo->nOpenPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMinPrice > pKLInfo->nMaxPrice)
			{
				strErrTemp.Format("最低价[%d]高于最高价[%d]; ", pKLInfo->nMinPrice,
					pKLInfo->nMaxPrice);
				strErr += strErrTemp;
			}

			if (pKLInfo->nMinPrice > pKLInfo->nClosePrice)
			{
				strErrTemp.Format("最低价[%d]高于收盘价[%d]; ", pKLInfo->nMinPrice,
					pKLInfo->nClosePrice);
				strErr += strErrTemp;
			}

			AddLog(nIndex, TRUE, "错误: %d行 %d列  %s", nIndex+1, 4, strErr);
		}
	}

	/*CString strDayFile = theApp.m_strKFilePath;
	strDayFile += "\\day\\";
	strDayFile += m_strFileName;
	tagKLineInfo newInfo = {0};
	newInfo.nDate = pKLInfo->nDate;

	BOOL bGetData = FALSE;
	if (nFileType == FT_WEK && IsValidDate(pKLInfo->nDate)) //如果是周线，从日线文件产生周线数据比较
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
				AddLog(nIndex, TRUE, "错误：[%d]开盘价格不一样, 原始数据:%d 检测数据:%d",	pKLInfo->nDate, pKLInfo->nOpenPrice, newInfo.nOpenPrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 1);
			}
		}
		if (pKLInfo->nMaxPrice != newInfo.nMaxPrice)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "错误：[%d]最高价格不一样, 原始数据:%d 检测数据:%d",pKLInfo->nDate, pKLInfo->nMaxPrice, newInfo.nMaxPrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 2);
			}
		}
		if (pKLInfo->nMinPrice != newInfo.nMinPrice)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "错误：[%d]最低价格不一样, 原始数据:%d 检测数据:%d",pKLInfo->nDate, pKLInfo->nMinPrice, newInfo.nMinPrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 3);
			}
		}
		if (pKLInfo->nClosePrice != newInfo.nClosePrice)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "错误：[%d]收盘价格不一样, 原始数据:%d 检测数据:%d",pKLInfo->nDate, pKLInfo->nClosePrice, newInfo.nClosePrice);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 4);
			}
		}
		if (pKLInfo->nVolume != newInfo.nVolume)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "错误：[%d]成交量不一样, 原始数据:%d 检测数据:%d",pKLInfo->nDate, pKLInfo->nVolume, newInfo.nVolume);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 5);
			}
		}
		if (pKLInfo->nSum != newInfo.nSum)
		{
			bRet = FALSE;
			if (bShowRecord)
			{
				AddLog(nIndex, TRUE, "错误：[%d]成交金额不一样, 原始数据:%d 检测数据:%d",pKLInfo->nDate, pKLInfo->nSum, newInfo.nSum);
				pCtrl->SetItemTextColor(TEXT_COLOR, nIndex, 6);
			}
		}
	}  */
	//当前记录至少有一个错误
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
		AddLog(-1, FALSE, "错误: 无法解析的文件[%s]， 保存失败!!", m_strFilePath);
		return;
	}
	if (!m_file.Open(m_strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		AddLog(-1, FALSE, "错误: 打开文件[%s]失败!", m_strFilePath);
		return;
	}

	if(m_hListCtrlWnd == NULL)
	{
		AddLog(-1, FALSE, "保存失败!");
		::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
		return;
	}

	CMulFuncList *pCtrl = (CMulFuncList *)CListCtrl::FromHandle(m_hListCtrlWnd);
	AddLog(-1, FALSE, "正在保存数据[%s]...", m_strFilePath);

	int nItemCount = pCtrl->GetItemCount();
	int nOldRecCount = m_file.GetRecordCount();
	if (nItemCount != nOldRecCount) //目前记录数等于以前记录数时，不用重新分配内存
	{
		if (!m_file.AllocateMemoryForSave(nItemCount))
		{
			AddLog(-1, FALSE, "给数据分配内存失败, 保存数据被迫终止");
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
			AddLog(-1, FALSE, "给数据分配内存不够, 部分数据丢失");
			::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
			break;
		}

		m_file.SetRecordBuffer((char*)&klInfo);
		m_file.MoveNext();
		nSaved++;
	}

	if (!m_file.WriteData())
	{
		AddLog(-1, FALSE, "保存失败");
		::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
	}
	else
	{
		if (nSaved < nItemCount)
		{
			AddLog(-1, FALSE, "错误: 部分数据保存失败");
			::SendMessage(m_hDialogWnd, WM_SAVE_FAILED, 0, 0);
		}
		else
		{
			AddLog(-1, FALSE, "保存成功"); 
		}
	}

	m_file.Close();
	AddLog(-1, FALSE, "========== 总数: %d 已保存: %d ==========", 
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
		if (bListInfo) // 个位留出做为listInfo  1 和 listfile 0的标志位 
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
// 	if (m_bRealHQ) //实时还权
// 		AddMsg(RPT_INFO, "正在开始K线实时还权...");
// 	else
// 		AddMsg(RPT_INFO, "正在开始K线历史还权...");
// 
// 	if (!BackupKLine())
// 	{
// 		AddMsg(RPT_ERROR, "复制K线文件到临时文件夹失败");
// 		return;
// 	}
// 	AddMsg(RPT_INFO, "复制K线文件到临时文件夹完毕");
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return;
// 	}
// 
// 	if (m_bRealHQ)
// 	{
// 		AddMsg(RPT_INFO, "正在添加新生成的日线文件到临时文件夹中的还权日线文件中...");
// 		UpdateDayLineToWeight();
// 	}
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return;
// 	}
// 
// 	if (!QueryWeight())  //获得还权因子
// 		return;
// 
// 	//开始日k线还权
// 	AddMsg(RPT_INFO, "正在日线还权...");
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
// 	AddMsg(RPT_INFO, "日线还权完毕");
	//用日线文件产生周线文件和月线文
// 	if (!m_bStartWeight)
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return;
// 	}
// 
// 	CString strDay = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day";
// 
// 	AddMsg(RPT_INFO, "正在产生周线和月线文件...");
// 	if (!GetWekMonLine(strDay))
// 	{
// 		AddMsg(RPT_ERROR, "用日线文件生产周线和月线文件失败");
// 		return;
// 	}
// 	AddMsg(RPT_INFO, "产生周线和月线完毕");
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return;
// 	}
// 
// 	if (m_bRealHQ) //实时还权后检测文件大小保留最近100条
// 	{
// 		AddMsg(RPT_INFO, "正在检测文件大小...");
// 		CString strFilePath = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str();
// 		CheckFileSize(strFilePath);
// 		AddMsg(RPT_INFO, "检测文件大小完毕");
// 	}
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return;
// 	}
// 	//还权完毕后，从临时文件夹复制还权后K线文件到还权k线文件夹
// 	if (m_bStartWeight)
// 	{
// 		//如果已经存在还权k线文件，还权前询问用户是否删除该文件
// 		if (m_bStartWeight && CFilePath::IsFolderHasFile(theApp.m_strKFilePath))
// 		{
// 			UINT nRetID = IDYES;
// 			if (!m_bRealHQ)
// 				nRetID = ::MessageBox(m_hDialogWnd,"还权k线文件目录["+theApp.m_strKFilePath +"]中已存在文件, 在还权前是否先删除该文件?", "警告", MB_YESNO);
// 
// 			if (IDYES == nRetID)
// 			{//删除文件
// 				AddMsg(RPT_INFO, "正在删除还权k线文件目录[%s]里的文件...", theApp.m_strKFilePath);
// 				CFilePath::DeleteFolderAllFile(theApp.m_strKFilePath);
// 				AddMsg(RPT_INFO, "删除文件完毕");
// 			}
// 		}
// 
// 		if (!m_bStartWeight)  //终止还权
// 		{
// 			AddMsg(RPT_WARNING, "k线还权已终止");
// 			return ;
// 		}
// 		AddMsg(RPT_INFO, "正在从临时文件夹中复制还权后的k线文件到[%s]...", theApp.m_strKFilePath);
// 		if (!CFilePath::CopyFolderAllFile(theApp.m_strKFilePath, theApp.m_strKLBkPath))
// 			AddMsg(RPT_ERROR, "从临时文件夹中复制还权后的k线文件到[%s]失败", theApp.m_strKFilePath);
// 
// 		AddMsg(RPT_INFO, "复制还权后的k线文件完毕");
// 
// 		if (!m_bStartWeight)  //终止还权
// 		{
// 			AddMsg(RPT_WARNING, "k线还权已终止");
// 			return;
// 		}
// 		AddMsg(RPT_INFO, "正在删除临时文件目录[%s]里的文件...", theApp.m_strKLBkPath);
// 		CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
// 		AddMsg(RPT_INFO, "删除文件完毕");
// 
// 		if (m_bRealHQ) //实时还权
// 			AddMsg(RPT_INFO, "K线实时还权完毕");
// 		else
// 			AddMsg(RPT_INFO, "K线历史还权完毕");
// 	}
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return;
// 	}

	//实时还权后上传
	if (/*m_bRealHQ &&*/ theApp.m_bRWUpload) //实时还权
	{
		AddMsg(RPT_INFO, "正在上传还权k线...");
		//m_checkThread.StartUploadHQKLineFile();

		HWND hDlgInfoWnd = theApp.m_pDlgInfoWnd->GetSafeHwnd();
		if (theApp.m_pDlgInfoWnd != NULL && hDlgInfoWnd != NULL)
		{
			::SendMessage(hDlgInfoWnd, WM_CHANGEINFO, 0, 0);
			((CDlg_Info*)theApp.m_pDlgInfoWnd)->m_strInfo = "正在上传当前库的还权K线...";
		}
		UploadHQKLine();
	}
}

BOOL CCheckThread::BackupKLine()
{
	return TRUE;
// 	AddMsg(RPT_INFO, "正在备份k线数据到临时目录[%s]...", theApp.m_strKLBkPath);
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return FALSE;
// 	}
// 	if (CFilePath::IsFolderHasFile(theApp.m_strKLBkPath))
// 	{
// 		UINT nRetID = IDYES;
// 		if (!m_bRealHQ)
// 			nRetID = ::MessageBox(m_hDialogWnd, "临时目录["+theApp.m_strKLBkPath +"]中已存在文件, 在复制文件前是否先删除该文件?", "警告", MB_YESNO);
// 
// 		if (nRetID == IDYES)
// 		{//删除文件
// 			AddMsg(RPT_INFO, "正在删除临时目录[%s]里的文件...", theApp.m_strKLBkPath);
// 			CFilePath::DeleteFolderAllFile(theApp.m_strKLBkPath);
// 			AddMsg(RPT_INFO, "删除文件完毕");
// 		}
// 	}
// 
// 	m_strCurKLine = CFilePath::GetPathLastFolderName(theApp.m_strKFilePath);
// 	m_strHisKLine = CFilePath::GetPathLastFolderName(theApp.m_strHisKFilePath);
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return FALSE;
// 	}
// 
// 	BOOL bRet = TRUE;
// 	if (m_bRealHQ) //实时还权
// 	{
// 		AddMsg(RPT_INFO, "正在复制[%s]里的文件到临时目录[%s]...", theApp.m_strKFilePath, theApp.m_strKLBkPath);
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath, theApp.m_strKFilePath);
// 	}
// 	else		   //历史还权
// 	{
// 		AddMsg(RPT_INFO, "正在复制[%s]里的文件到临时目录[%s]...", theApp.m_strKFilePath, theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str(), theApp.m_strKFilePath);
// 		AddMsg(RPT_INFO, "正在复制[%s]里的文件到临时目录[%s]...", theApp.m_strHisKFilePath, theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str());
// 		bRet &= CFilePath::CopyFolderAllFile(theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str(), theApp.m_strHisKFilePath);
// 	}
// 
// 	if (!m_bStartWeight)  //终止还权
// 	{
// 		AddMsg(RPT_WARNING, "k线还权已终止");
// 		return FALSE;
// 	}
// 	return bRet;
}

BOOL CCheckThread::QueryWeight()
{
	AddMsg(RPT_INFO, "正在从数据库[%s:%s]获取还权因子...", theApp.m_strSqlIP, theApp.m_strSqlDBN);
	//从数据库读取数据到内存
	std::string strIP, strDBN, strUID, strPw;
	strIP  = theApp.m_strSqlIP.GetBuffer();
	theApp.m_strSqlIP.ReleaseBuffer();
	strDBN = theApp.m_strSqlDBN.GetBuffer();
	theApp.m_strSqlDBN.ReleaseBuffer();
	strUID = theApp.m_strSqlUID.GetBuffer();
	theApp.m_strSqlUID.ReleaseBuffer();
	strPw  = theApp.m_strSqlPw.GetBuffer();
	theApp.m_strSqlPw.ReleaseBuffer();

	if (!m_bStartWeight)  //终止还权
	{
		AddMsg(RPT_WARNING, "k线还权已终止");
		return FALSE;
	}

	if (!m_sqlDB.Open(strIP, strDBN, strUID, strPw))
	{
		AddMsg(RPT_ERROR, "打开还权因子数据库失败");
		return FALSE;
	}

	CRecordSet reSet(&m_sqlDB);
	string strCon = "SELECT * FROM IND_S_RIGHT";
	if (m_bRealHQ) //实时还权
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
		AddMsg(RPT_ERROR, "打开还权因子数据库表IND_S_RIGHT失败");

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
		AddMsg(RPT_INFO, "获取还权因子完毕，共[%d]记录", m_lstYinZi.size());
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			GetWekMonLine(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFolder = ffind.GetFilePath(); //指数也要生成日线和月线
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
// 		if (i == 1) //当前库
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 		else
// 			strDayFile = theApp.m_strKLBkPath + "\\" + m_strHisKLine.c_str() + "\\day\\" + strFileName;
// 
// 		CKLineFile kfile;
// 		if (!kfile.Open(strDayFile,  GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 		{
// 			AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strDayFile);
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
// 	//计算周线
// 	{
// 		tagKLineInfo*  pWekInfo = NULL;
// 		BOOL bWekFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("无效的日期:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
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
// 				REPORT(MN, T("无效的日期:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
// 				continue;
// 			}
// 			UINT nNextDate = (*pos)->nDate;
// 			if (!IsSameWeek(nPreDate, nNextDate)) //另一周	  
// 			{
// 				bWekFirstDay = TRUE;
// 				lstWekLine.push_back(pWekInfo);
// 				pWekInfo = NULL;
// 			}
// 		}
// 		if (pWekInfo != NULL)
// 			lstWekLine.push_back(pWekInfo); //最后一个
// 	}
// 	//计算月线
// 	{
// 		tagKLineInfo*  pMonInfo = NULL;
// 		BOOL bMonFirstDay = TRUE;
// 
// 		CKLineList::iterator pos = lstDayLine.begin();
// 		while(pos != lstDayLine.end())
// 		{
// 			if (!CCheckThread::IsValidDate((*pos)->nDate))
// 			{
// 				REPORT(MN, T("无效的日期:%d FilePath:[%s]\n", (*pos)->nDate, strDayFile), RPT_ERROR);
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
// 			if ((*pos)->nDate % 10000 / 100 != pMonInfo->nDate % 10000 / 100) //另一月
// 			{
// 				bMonFirstDay = TRUE;
// 				lstMonLine.push_back(pMonInfo);
// 				pMonInfo = NULL;
// 			}
// 		}
// 		if (pMonInfo != NULL)
// 			lstMonLine.push_back(pMonInfo); //最后一个
// 	}
// 
// 	{//保存周线
// 		CFile fHisWekL;
// 		CFile fCurWekL;
// 
// 		if (!fCurWekL.Open(strCurWekFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strCurWekFile);
// 
// 		CKLineList::iterator pos = lstWekLine.begin();
// 		int nHisCount = (int)lstWekLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0) //有多余的记录才打开历史文件
// 		{
// 			if (!fHisWekL.Open(strHisWekFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strHisWekFile);
// 		}
// 
// 		while(pos != lstWekLine.end())
// 		{
// 			if (nHisCount <= 0) //保存在当前库
// 			{
// 				fCurWekL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//保存在历史库
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
// 	{//保存月线
// 		CFile fHisMonL;
// 		CFile fCurMonL;
// 		CKLineList::iterator pos = lstMonLine.begin();
// 		int nHisCount = (int)lstMonLine.size() - theApp.m_nReserveCount;
// 
// 		if (nHisCount > 0)
// 		{
// 			if (!fHisMonL.Open(strHisMonFile, CFile::modeWrite|CFile::modeCreate))
// 				AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strHisMonFile);
// 		}
// 
// 		if (!fCurMonL.Open(strCurMonFile, CFile::modeWrite|CFile::modeCreate))
// 			AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strCurMonFile);
// 
// 		while(pos != lstMonLine.end())
// 		{
// 			if (nHisCount <= 0) //保存在当前库
// 			{
// 				fCurMonL.Write((*pos), sizeof(tagKLineInfo));
// 			}
// 			else				//保存在历史库
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

void CCheckThread::UpdateDayLineToWeight()  //更新最近生成的日线数据到还权日线数据中，然后再实时还权
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
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
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
// 		AddMsg(RPT_ERROR, "错误: 无法解析的文件[%s]!", strFile);
// 		return;
// 	}
// 
// 	CString strHQFile = theApp.m_strKLBkPath + "\\" + m_strCurKLine.c_str() + "\\day\\" + strFileName;
// 
// 	CKLineFile hqfile;     //还权日线
// 	CKLineFile orifile;	   //原始日线
// 	if (!hqfile.Open(strHQFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_ALWAYS))
// 	{
// 		AddMsg(RPT_ERROR, "错误: 打开文件[%s]失败!", strHQFile);
// 		return;
// 	}
// 	hqfile.MoveLast();
// 	tagKLineInfo info = {0};
// 	hqfile.GetRecordToBuffer((char*)&info);
// 
// 	if (!orifile.Open(strFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 	{
// 		AddMsg(RPT_ERROR, "错误: 打开文件[%s]失败!", strHQFile);
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
// 		AddMsg(RPT_ERROR, "错误: AllocateMemoryForSave() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.SetAllRecordToBuffer(pOldKLineBuf))
// 	{
// 		AddMsg(RPT_ERROR, "错误: SetAllRecordToBuffer() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	if (!hqfile.WriteData())
// 	{
// 		AddMsg(RPT_ERROR, "错误: WriteData() failed!");
// 		delete []pOldKLineBuf;
// 		return;
// 	}
// 
// 	hqfile.Close();
// 	delete []pOldKLineBuf;
}

BOOL CCheckThread::IsSameWeek(unsigned int uiNow, unsigned int uiPrev)
{//比较今日和所给定日期是否在同一周内
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
// 		AddMsg(RPT_WARNING, "k线文件[%s]不存在", strFilePath);
// 	}
// 	else
// 	{
// 		if ((bCurKLine && m_strCurKFilePath.Compare(strFilePath) == 0 || 
// 			!bCurKLine && m_strHisKFilePath.Compare(strFilePath) == 0)
// 			&& pKLFile->IsOpen())
// 		{ //同一个文件只打开一次
// 			pKLFile->MoveFirst();
// 		}
// 		else
// 		{
// 			if (pKLFile->IsOpen()) 
// 				pKLFile->Close(); //关闭上次打开的文件
// 
// 			if (!pKLFile->Open(strFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
// 			{
// 				AddMsg(RPT_ERROR, "打开k线文件[%s]失败", strFilePath);
// 				if (bCurKLine)
// 					m_strCurKFilePath.Empty();
// 				else
// 					m_strHisKFilePath.Empty();
// 			}
// 			else
// 			{
// 				if (bCurKLine)
// 					m_strCurKFilePath = strFilePath; //保存上一个文件路径，如果下一个和上一个是同一个文件，则只打开一次
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
// 			if (klInfo.nDate >= pWeight->nDate)  //小于还权日期的所有k线还权
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
// 			AddMsg(RPT_ERROR, "保存文件[]失败", strFilePath);
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
		REPORT(MN, T("Dat文件中没有股票信息\n"), RPT_WARNING);
		ReleaseMemory();  //释放内存 
		return FALSE;
	}  

	delete m_pdataFarm;
	if (!m_bRunCombDatFile)
	{
		ReleaseMemory();  //释放内存 
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
		ReleaseMemory();  //释放内存 
		return FALSE;
	}

	if (m_pdataFarm->m_hashIdx.for_each(*this) == 0)
	{
		REPORT(MN, T("Dat文件中没有股票信息\n"), RPT_WARNING);
		ReleaseMemory();  //释放内存 
		return FALSE;
	}

	delete m_pdataFarm;
	if (!m_bRunCombDatFile)
	{
		ReleaseMemory();  //释放内存 
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
		ReleaseMemory();  //释放内存 
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
		ReleaseMemory();  //释放内存 
		return;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////
	ReleaseMemory();  //释放内存 
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
			if (i == (pRindex1->cntTick - 1) ) //最后一根
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
					if (m_nTickFile1 != 0) //使用第二份dat文件
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
			if (i == (pRindex2->cntTick - 1) ) //最后一根
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
			REPORT(MN, T("打开文件[%s]失败 错误代码:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath2 + "\\quote.dat";
		if (!m_QuoteFile2.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("打开文件[%s]失败 错误代码:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath1 + "\\mink.dat";
		if (!m_MinkFile1.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("打开文件[%s]失败 错误代码:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath2 + "\\mink.dat";
		if (!m_MinkFile2.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("打开文件[%s]失败 错误代码:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath1 + "\\tick.dat";
		if (!m_TickFile1.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("打开文件[%s]失败 错误代码:%d\n", strtemp, GetLastError()), RPT_ERROR);

		strtemp = m_strPath2 + "\\tick.dat";
		if (!m_TickFile2.Open(strtemp, CFile::modeRead|CFile::shareDenyNone))
			REPORT(MN, T("打开文件[%s]失败 错误代码:%d\n", strtemp, GetLastError()), RPT_ERROR);

	}
	catch(...)
	{
		REPORT(MN, T("打开文件发送异常\n"), RPT_ERROR);
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
		REPORT(MN, T("打开文件发送异常\n"), RPT_ERROR);
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
