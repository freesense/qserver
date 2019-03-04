//CFolderDlg.h Í·ÎÄ¼þ
//

#ifndef _FOLDERDLG_H_
#define _FOLDERDLG_H_
//////////////////////////////////////////////////////////////////////
class CFolderDlg
{
public:
	CFolderDlg();
	~CFolderDlg();

	UINT DoModal();
	CString GetPathName();
protected:
	char           m_chPath[MAX_PATH];
	BROWSEINFO     m_bi; 
	ITEMIDLIST    *m_pidl;

private:

};























///////////////////////////////////////////////////////////////////////
#endif