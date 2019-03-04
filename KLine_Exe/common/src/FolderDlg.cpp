//CFolderDlg.cpp
//

#include "stdafx.h"
#include "FolderDlg.h"
////////////////////////////////////////////////////////

CFolderDlg::CFolderDlg()
{
	memset(&m_bi, 0, sizeof(BROWSEINFO));
	m_pidl = NULL;
}

CFolderDlg::~CFolderDlg()
{
}

UINT CFolderDlg::DoModal()
{
	m_bi.hwndOwner = NULL; 
	m_bi.pidlRoot = NULL; 
	m_bi.pszDisplayName = m_chPath; 
	m_bi.lpszTitle = "选择一个目录"; 
	m_bi.ulFlags = BIF_RETURNONLYFSDIRS; 
	m_bi.lpfn = NULL; 
	m_bi.lParam = 0; 
	m_bi.iImage = 0; 

	m_pidl = SHBrowseForFolder( &m_bi );          

	if ( m_pidl == NULL )
	{
		m_chPath[0] = 0;
		return IDCANCEL;
	}

	if (!SHGetPathFromIDList( m_pidl, m_chPath ))       
	{
		m_chPath[0] = 0;
	}

	return IDOK;
}

CString CFolderDlg::GetPathName()
{
	return m_chPath;
}