//FilePath.cpp
//#include "stdafx.h"
#include<windows.h>
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
	unsigned long dwSize = GetModuleFileName(NULL, szFilePath, _MAX_PATH);
	string strPath = szFilePath;
	return strPath.substr(0, strPath.rfind("\\")); //È¥µô×Ö·û´®ºóµÄ"\\"
}

bool CFilePath::IsPathExist(const char* lpPath)
{
	assert(lpPath != NULL);
	unsigned long dwAttr = ::GetFileAttributes(lpPath);

	return (dwAttr!=-1 && (dwAttr&FILE_ATTRIBUTE_DIRECTORY) );
}

bool CFilePath::IsFileExist(const char* lpFileName)
{
	assert(lpFileName != NULL);
	unsigned long dwAttr = ::GetFileAttributes(lpFileName);
	return (dwAttr!=-1 && !(dwAttr&FILE_ATTRIBUTE_DIRECTORY) );
}

string CFilePath::GetPathLastFolderName(const char* lpPath)
{
	string strPath = lpPath;
	if (strPath.substr(strPath.size()-1, 1) == "\\")
		strPath = strPath.substr(0, strPath.size()-2);
	return strPath.substr(strPath.rfind("\\")+1, strPath.size() - strPath.rfind("\\") - 1);
}

string CFilePath::GetExeName(const char* lpPath)
{
	string strPath = lpPath;
	return strPath.substr(strPath.rfind(".")+1, strPath.size() - strPath.rfind(".") - 1);
}

string CFilePath::GetFileName(const char* lpPath)
{
	string strPath = lpPath;
	return strPath.substr(strPath.rfind("\\")+1, strPath.size() - strPath.rfind("\\") - 1);
}