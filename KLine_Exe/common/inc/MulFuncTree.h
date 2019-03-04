//MulFuncTree.h
#ifndef _MULFUNCTREE_H_
#define _MULFUNCTREE_H_

#define WM_DELITEMDATA WM_USER + 132
#define WM_GETITEMDATA WM_USER + 133
///////////////////////////////////////////////////////////////////////////
class CMulFuncTree : public CTreeCtrl
{
public:
	CMulFuncTree();
	virtual ~CMulFuncTree();

	void DelAllItemData(HTREEITEM hRoot);
	void SetMsgWnd(HWND hWnd){m_MsgWnd = hWnd;}
	void GetAllItemData(HTREEITEM hRoot);
protected:
	virtual void OnDelItemData(DWORD dwData);
	virtual void OnGetItemData(DWORD dwData);

private:
	HWND     m_MsgWnd;
};
///////////////////////////////////////////////////////////////////////////
#endif