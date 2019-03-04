//MulFuncList.h 
//

#ifndef _MULFUNCLIST_H_
#define _MULFUNCLIST_H_

#define WM_USER_EDIT_END           WM_USER+1001
#define WM_VALUE_CHANGED           WM_USER+1001
/////////////////////////////////////////////////////////////////////////////////////
class CListEdit : public CEdit
{
public:
	CListEdit();
	virtual ~CListEdit();

	void  SetCtrlData(DWORD dwData);
	DWORD GetCtrlData();

protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	DWORD     m_dwData;
	BOOL      m_bExchange;
};

//////////////////////////////////////////////////////////////////////////////////////
struct tagItemData
{
	tagItemData()
	{
		clrBk   = RGB(255,255,255);
		clrText = RGB(0,0,0);
	}

	COLORREF clrBk;
	COLORREF clrText;
};

class CMulFuncList : public CListCtrl
{
public:
	CMulFuncList();
	virtual ~CMulFuncList();

	void DeleteSelectItem();
	int  InsetItemInCurSel();
	void MoveSelItemToPrev();
	void MoveSelItemToNext();
	BOOL SetItemImage(int nIndex, int nImg);
	BOOL SetItemBackColor(COLORREF clrBk, int nItem, int nSubItem = -1);
	BOOL SetItemTextColor(COLORREF clrText, int nItem, int nSubItem = -1);
	void EnableEdit(BOOL bEnable = TRUE);
	int	 GetColumCount();
	int AddItem();
protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg LRESULT OnEditEnd(WPARAM ,LPARAM lParam = FALSE);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);	
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnInsertitem(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

private:
	void ShowEdit(int nItem, int nSubItem, CRect rcCtrl, BOOL bShow = TRUE);
	BOOL Key_Shift(int& nItem,int& nSub);
	BOOL Key_Ctrl(int& nItem,int &nSub);
	void* GetItemLParam(int nItem);
	void FreeItemData(int nItem);
	
	void SendMessageToParent(UINT message);

	CListEdit        m_listEdit;
	CStringArray	 m_arrItemText;
	CUIntArray       m_arrItemIndex;

	BOOL    m_bEnableEdit;
};

////////////////////////////////////////////////////////////////////////////////////////
#endif
