//FilePath.cpp
#include "stdafx.h"
#include "FilePath.h"
#include <assert.h>

/////////////////////////////////////////////////////////////////////////////////
CFilePath::CFilePath()
{
}

CFilePath::~CFilePath()
{
}

string CFilePath::GetExeFilePath()
{
	char szFilePath[_MAX_PATH];
	DWORD dwSize = GetModuleFileName(NULL, szFilePath, _MAX_PATH);
	string strPath = szFilePath;
	return strPath.substr(0, strPath.rfind("\\")); //去掉字符串后的"\\"
}

BOOL CFilePath::IsPathExist(LPCTSTR lpPath)
{
	assert(lpPath != NULL);
	DWORD dwAttr = ::GetFileAttributes(lpPath);

	return (dwAttr!=-1 && (dwAttr&FILE_ATTRIBUTE_DIRECTORY) );
}

BOOL CFilePath::IsFileExist(LPCTSTR lpFileName)
{
	assert(lpFileName != NULL);
	DWORD dwAttr = ::GetFileAttributes(lpFileName);
	return (dwAttr!=-1 && !(dwAttr&FILE_ATTRIBUTE_DIRECTORY) );
}

string CFilePath::GetPathLastFolderName(LPCTSTR lpPath)
{
	string strPath = lpPath;
	if (strPath.substr(strPath.size()-1, 1) == "\\")
		strPath = strPath.substr(0, strPath.size()-2);
	return strPath.substr(strPath.rfind("\\")+1, strPath.size() - strPath.rfind("\\") - 1);
}

BOOL CFilePath::IsFolderHasFile(LPCTSTR lpPath)
{
	if (lpPath == NULL)
		return FALSE;

	string strFolder = lpPath;
	if (strFolder.substr(strFolder.size()-1, 1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder.c_str());
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			string strPath = ffind.GetFilePath(); //得到路径
			if (IsFolderHasFile(strPath.c_str()))
				return TRUE;
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CFilePath::DeleteFolderAllFile(LPCTSTR lpPath)
{
	if (lpPath == NULL)
		return;

	string strFolder = lpPath;
	if (strFolder.substr(strFolder.size()-1, 1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder.c_str());
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			string strPath = ffind.GetFilePath(); //得到路径
			DeleteFolderAllFile(strPath.c_str());
			RemoveDirectory(strPath.c_str());
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			string strFile = ffind.GetFilePath();
			::DeleteFile(strFile.c_str());
		}
	}
}

BOOL CFilePath::CopyFolderAllFile(LPCTSTR strDes, LPCTSTR strSrc)
{
	string tmpFrom = strSrc;
	string tmpTo   = strDes;   
	if (!CFilePath::IsPathExist(tmpTo.c_str()))
	{
		CreateDirectory(tmpTo.c_str(), NULL);
	}
	if (CFilePath::IsPathExist(tmpFrom.c_str()))
	{           
		WIN32_FIND_DATA FindFileData;
		string tmpFind = tmpFrom;
		tmpFind += _T("\\*.*");

		HANDLE hFind=::FindFirstFile(tmpFind.c_str(), &FindFileData);
		if(INVALID_HANDLE_VALUE == hFind)
			return FALSE;

		while(TRUE)
		{
			string tmpFile = tmpTo;
			tmpFile += _T("\\");
			tmpFile += FindFileData.cFileName;

			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //目录
			{
				if(FindFileData.cFileName[0]!='.')
				{
					/*CString strFoldle =	tmpFile.c_str();
					strFoldle = strFoldle.Right(strFoldle.GetLength() - strFoldle.ReverseFind('\\') - 1);
					if (strFoldle.CompareNoCase("day") == 0 ||
						strFoldle.CompareNoCase("mon") == 0 ||
						strFoldle.CompareNoCase("wek") == 0)*/
					{
						if (!CFilePath::IsPathExist(tmpFile.c_str()))        //目录不存在
						{
							CreateDirectory(tmpFile.c_str(), NULL);   
						}

						CopyFolderAllFile(tmpFile.c_str(), (tmpFrom + _T("\\") + FindFileData.cFileName).c_str());               
					}
				}               
			}
			else                    //文件
			{               
				//if (!CFilePath::IsFileExist(tmpFile.c_str()))    //文件不存在就复制
				{
					CString strExe = CValidFunction::GetExeName(tmpFile.c_str());
					if (strExe.CompareNoCase(".sz") == 0    ||
						strExe.CompareNoCase(".szidx") == 0	||
						strExe.CompareNoCase(".sh") == 0    ||
						strExe.CompareNoCase(".shidx") == 0)
					{
						CopyFile( (tmpFrom + "\\" + FindFileData.cFileName).c_str(), tmpFile.c_str(), TRUE);    //拷贝文件
					}
				}
			}

			if(!FindNextFile(hFind, &FindFileData))   
				break;
		}
		FindClose(hFind);
	}   

	return TRUE;
}

string CFilePath::GetExeName(LPCTSTR lpPath)
{
	string strPath = lpPath;
	return strPath.substr(strPath.rfind(".")+1, strPath.size() - strPath.rfind(".") - 1);
}