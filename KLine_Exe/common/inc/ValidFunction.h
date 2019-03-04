
#if !defined(AFX_VALIDFUNCTION_H__F23994E6_9038_47E3_8716_9E907ADEC71E__INCLUDED_)
#define AFX_VALIDFUNCTION_H__F23994E6_9038_47E3_8716_9E907ADEC71E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class  CValidFunction  
{
public:
	static CString ShortPathToLong(LPCTSTR strPath);
	static CString LongPathToShort(LPCTSTR strPath);
	static void GetProgramFileDir(CString& strDir);
	static void GetWindowDir(CString& strDir);
	static void GetWindowDisk(CString& strDisk);
	static int IsInStringArrayNoCase(const CStringArray &strArr, const CString &str);
	static int IsInStringArray(const CStringArray & strArr,const CString& str);
	static void SetStaticText(CDialog* pDlg,UINT nID,LPCTSTR strText);
	static BOOL CreatePath(LPCTSTR strPath);
	static void GetWorkPath(CString& strPath);
	static BOOL IsPathExist(LPCTSTR strPath);
	static BOOL IsFileExist(LPCTSTR strFileName);
	static CString GetLONGLONGString(LONGLONG llValue);
	static CString CombineFilePath(LPCTSTR lpszPathName, LPCTSTR lpszFileName);
	static CString GetExeFileName(CString strExeName);
	static CString GetFileNamePrePath(CString strName);
	static CString GetFileNamePreFolder(CString strFilePath);
	static CString GetFileNameFromFilePath(CString strFilePath);
	static CString GetPathPreFolderPath(CString strFilePath);
	static CString GetFilePathPreFolder(CString strFilePath);
public:
	static CString GetExeName(CString strFile);
	CValidFunction(){}
	virtual ~CValidFunction(){}
};

#endif // !defined(AFX_VALIDFUNCTION_H__F23994E6_9038_47E3_8716_9E907ADEC71E__INCLUDED_)
