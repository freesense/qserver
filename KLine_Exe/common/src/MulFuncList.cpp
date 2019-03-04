//MulFuncList.cpp
//
#include "stdafx.h"
#include "MulFuncList.h"

#define IDC_EDIT  0x01
//////////////////////////////////////////////////////////////////////////////////////

CMulFuncList::CMulFuncList()
{
	m_bEnableEdit = FALSE;
}

CMulFuncList::~CMulFuncList()
{
}

BEGIN_MESSAGE_MAP(CMulFuncList, CListCtrl)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PARENTNOTIFY()
	ON_MESSAGE(WM_USER_EDIT_END,OnEditEnd)
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &CMulFuncList::OnLvnDeleteitem)
	ON_NOTIFY_REFLECT(LVN_INSERTITEM, &CMulFuncList::OnLvnInsertitem)
END_MESSAGE_MAP()

void CMulFuncList::PreSubclassWindow()
{
	SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	CListCtrl::PreSubclassWindow();
}

void CMulFuncList::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_bEnableEdit)
	{
		CRect rcCtrl;
		LVHITTESTINFO lvhti;
		lvhti.pt = point;
		int nItem = SubItemHitTest(&lvhti);
		if(nItem == -1)
			return;

		int nSubItem = lvhti.iSubItem;
		GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rcCtrl); 
		ShowEdit(nItem, nSubItem, rcCtrl);
	}

	CListCtrl::OnLButtonDblClk(nFlags, point);
}

void CMulFuncList::ShowEdit(int nItem, int nSubItem, CRect rcCtrl, BOOL bShow)
{
	rcCtrl.InflateRect(0, 0, 0, 0);
	if(m_listEdit.m_hWnd == NULL)
	{
		m_listEdit.Create(ES_AUTOHSCROLL|WS_CHILD|ES_LEFT|ES_WANTRETURN|WS_BORDER,
			CRect(0,0,0,0),this,IDC_EDIT);
		m_listEdit.ShowWindow(SW_HIDE);

		CFont tpFont;
		tpFont.CreateStockObject(DEFAULT_GUI_FONT);
		m_listEdit.SetFont(&tpFont);
		tpFont.DeleteObject();
	}
	if(bShow == TRUE)
	{
		CString strItem = CListCtrl::GetItemText(nItem, nSubItem);
		m_listEdit.MoveWindow(rcCtrl);
		m_listEdit.ShowWindow(SW_SHOW);
		m_listEdit.SetWindowText(strItem);
		::SetFocus(m_listEdit.GetSafeHwnd());
		m_listEdit.SetSel(-1);  
		m_listEdit.SetCtrlData((DWORD)MAKEWPARAM(nSubItem, nItem));	
	}
	else
		m_listEdit.ShowWindow(SW_HIDE);
}

void CMulFuncList::OnParentNotify(UINT message, LPARAM lParam) 
{
	CListCtrl::OnParentNotify(message, lParam);
	//////////////////////////////////////////////////////////////////////////
	CHeaderCtrl* pHeaderCtrl = CListCtrl::GetHeaderCtrl();
	if(pHeaderCtrl == NULL)
		return;
	CRect rcHeader;
	pHeaderCtrl->GetWindowRect(rcHeader);
	ScreenToClient(rcHeader);
	//The x coordinate is in the low-order word and the y coordinate is in the high-order word.
	CPoint pt;
	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);
	if(rcHeader.PtInRect(pt) && message == WM_LBUTTONDOWN)
	{
		if(m_listEdit.m_hWnd != NULL)
		{
			DWORD dwStyle = m_listEdit.GetStyle();
			if((dwStyle&WS_VISIBLE) == WS_VISIBLE)
			{
				m_listEdit.ShowWindow(SW_HIDE);
			}
		}	
	}	
}

BOOL CMulFuncList::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_TAB && m_listEdit.m_hWnd!= NULL)
		{
			DWORD dwStyle = m_listEdit.GetStyle();
			if((dwStyle&WS_VISIBLE) == WS_VISIBLE)
			{
				OnEditEnd(TRUE,TRUE);
				CRect rcCtrl;		 
				int nItem;
				int nSub;
				if(FALSE == Key_Ctrl(nItem,nSub))
					Key_Shift(nItem,nSub);
				CListCtrl::GetSubItemRect(nItem,nSub,LVIR_LABEL,rcCtrl); 
				CPoint pt(rcCtrl.left+1,rcCtrl.top+1);
				OnLButtonDblClk(0,pt);
				POSITION pos = CListCtrl::GetFirstSelectedItemPosition();
				if (pos == NULL)
				{
				}
				else
				{
					while (pos)
					{
						int ntpItem = CListCtrl::GetNextSelectedItem(pos);
						CListCtrl::SetItemState(ntpItem,0,LVIS_SELECTED);
					}
				}
				CListCtrl::SetItemState(nItem,  LVIS_SELECTED,  LVIS_SELECTED);
				return TRUE;
			}
		}
	}

	return CListCtrl::PreTranslateMessage(pMsg);
}

BOOL CMulFuncList::Key_Ctrl(int& nItem,int &nSub)
{
    short sRet = GetKeyState(VK_CONTROL);
	DWORD dwData = m_listEdit.GetCtrlData();
	nItem= dwData>>16;
	nSub = dwData&0x0000ffff;
	
	sRet = sRet >>15;
	int nItemCount = CListCtrl::GetItemCount();
	if(sRet == 0)
	{
		
	}
	else
	{
		nItem = nItem >=nItemCount-1? 0:nItem+=1;
		return TRUE;
	}
	
	return FALSE;
}

BOOL CMulFuncList::Key_Shift(int& nItem,int& nSub)
{
	int nItemCount = CListCtrl::GetItemCount();
	DWORD dwData = m_listEdit.GetCtrlData();
	nItem= dwData>>16;
	nSub = dwData&0x0000ffff;
	
	CHeaderCtrl* pHeader = CListCtrl::GetHeaderCtrl();
	if(pHeader == NULL)
		return FALSE;
	
	short sRet = GetKeyState(VK_SHIFT);
	int nSubcCount = pHeader->GetItemCount();
	sRet = sRet >>15;
	if(sRet == 0)
	{
		nSub += 1;
		if(nSub >= nSubcCount)
		{
			if(nItem == nItemCount-1)
			{
				nItem = 0;
				nSub  = 0;
			}
			else
			{
				nSub = 0;
				nItem += 1;
			}
		}
		if(nItem >= nItemCount)
			nItem = nItemCount-1;
		return FALSE;
	}
	else
	{
		nSub -= 1;
		if(nSub < 0)
		{
			
			nSub = nSubcCount -1;
			nItem --;
		}
		
		if(nItem < 0)
			nItem = nItemCount-1;
		return TRUE;
		
	}
	return FALSE;
}

void CMulFuncList::SendMessageToParent(UINT message)  
{
	CWnd* pParent = GetParent();
	if (pParent != NULL)
		pParent->SendMessage(message);
}

LRESULT CMulFuncList::OnEditEnd(WPARAM wParam,LPARAM lParam)
{
	if(wParam == TRUE)
	{
		CString strText(_T(""));
		m_listEdit.GetWindowText(strText);
		DWORD dwData = m_listEdit.GetCtrlData();
		int nItem= dwData>>16;
		int nIndex = dwData&0x0000ffff;
		CString strOldText = GetItemText(nItem,nIndex);
		if (strOldText.CompareNoCase(strText) != 0) //字段值发生变化
		{
			SendMessageToParent(WM_VALUE_CHANGED);
		}
		CListCtrl::SetItemText(nItem,nIndex,strText);
	}
	else
	{
	   
	}

    if(lParam == FALSE)
	    m_listEdit.ShowWindow(SW_HIDE);

	return 0;
}

void CMulFuncList::DeleteSelectItem()
{
	SendMessageToParent(WM_VALUE_CHANGED);
	POSITION pt = GetFirstSelectedItemPosition();
	while(pt != NULL)
	{
		int nCurSel = GetNextSelectedItem(pt);
		if (nCurSel >= 0 && nCurSel <GetItemCount())
			DeleteItem(nCurSel);
		pt = GetFirstSelectedItemPosition();
	}
}

int CMulFuncList::InsetItemInCurSel()
{
	SendMessageToParent(WM_VALUE_CHANGED);
	POSITION pt = GetFirstSelectedItemPosition();
	int nCurSel = GetNextSelectedItem(pt);
	int nItemCount = GetItemCount();

	if (nCurSel >= 0 && nCurSel < nItemCount)
	{
		InsertItem(nCurSel, "");
		return nCurSel;
	}
	else
	{
		InsertItem(nItemCount, "");
		return nItemCount;
	}
}

int CMulFuncList::AddItem()
{
	SendMessageToParent(WM_VALUE_CHANGED);
	POSITION pt = GetFirstSelectedItemPosition();
	int nCurSel = GetNextSelectedItem(pt);
	int nItemCount = GetItemCount();

	/*if (nCurSel >= 0 && nCurSel < nItemCount)
	{
		InsertItem(nCurSel, "");
		return nCurSel;
	}
	else */
	{
		InsertItem(nItemCount, "");
		return nItemCount;
	}
}

void CMulFuncList::MoveSelItemToPrev()
{
	SendMessageToParent(WM_VALUE_CHANGED);
	POSITION pt = GetFirstSelectedItemPosition();
	if (pt == NULL)
		return;

	int nColumCount = 0;
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	int nHdrCtrlHeight = 0;
	if (pHeaderCtrl)
	{
		CRect rcHdrCtrl;
		pHeaderCtrl->GetItemRect(0, rcHdrCtrl);
		nHdrCtrlHeight = rcHdrCtrl.Height();
		nColumCount = pHeaderCtrl->GetItemCount();
	}

	CRect rcItem;
	GetItemRect(0, rcItem, LVIR_BOUNDS);
	while (pt != NULL)
	{
		int nCurSel = GetNextSelectedItem(pt);
		if (nCurSel - 1 < 0)
			return;

		m_arrItemText.RemoveAll();
		for (int i=0; i<nColumCount; i++)
		{
			CString strText = GetItemText(nCurSel-1, i);
			m_arrItemText.Add(strText);
		}

		LVITEM item = {0};
		item.mask = LVIF_IMAGE| LVIF_PARAM;
		item.iItem = nCurSel-1;
		GetItem(&item);
		tagItemData* pItemData = NULL;
		
		pItemData = (tagItemData*)item.lParam;
		tagItemData* pNewItemData = NULL;
		if(pItemData != NULL)
		{
			pNewItemData = new tagItemData[nColumCount];
			memcpy(pNewItemData, pItemData, sizeof(tagItemData)*nColumCount);
		}

		if ((nCurSel-1) >= 0 && (nCurSel-1) <GetItemCount())
			DeleteItem(nCurSel-1);

		for (int i=0; i<nColumCount; i++)
		{
			if (i == 0)
				InsertItem(nCurSel, m_arrItemText[i], item.iImage);
			else
				SetItemText(nCurSel, i, m_arrItemText[i]);
		}

		tagItemData *pData = (tagItemData *)GetItemLParam(nCurSel);
		if (pData != NULL)
		{
			delete pData;
			pData = NULL;
		}

		if (pNewItemData != NULL)
		{
			memset(&item, 0, sizeof(LVITEM));
			item.mask = LVIF_PARAM;
			item.iItem = nCurSel;
			item.lParam = (LPARAM)pNewItemData;
			SetItem(&item);
		}

		/*POINT pt;
		GetItemPosition(nCurSel, &pt);
		POINT ptFirst;
		GetItemPosition(0, &ptFirst);

		CRect rcClient;
		GetClientRect(rcClient);
		if (pt.y - rcItem.Height() < nHdrCtrlHeight)
		{
			Scroll(CSize(0, -rcItem.Height()));
			}*/
		EnsureVisible(nCurSel-1,TRUE);
	}
}

void CMulFuncList::MoveSelItemToNext()
{
	SendMessageToParent(WM_VALUE_CHANGED);
	POSITION pt = GetFirstSelectedItemPosition();
	if (pt == NULL)
		return;

	int nItemCount = GetItemCount();
	int nSelCount  = GetSelectedCount();

	m_arrItemIndex.RemoveAll();
	for (int i=0; pt != NULL; i++)
	{
		int nItemIndex = GetNextSelectedItem(pt);
		m_arrItemIndex.Add(nItemIndex);
	}

	if ((int)m_arrItemIndex[nSelCount-1] >= nItemCount - 1)
		return;

	int nColumCount = 0;
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	if (pHeaderCtrl)
	{
		nColumCount = pHeaderCtrl->GetItemCount();
	}

	CRect rcItem;
	GetItemRect(0, rcItem, LVIR_BOUNDS);

	for (int i=nSelCount-1; i>=0; i--)
	{
		int nCurSel	= m_arrItemIndex[i];
		
		m_arrItemText.RemoveAll();
		for (int i=0; i<nColumCount; i++)
		{
			CString strText = GetItemText(nCurSel, i);
			m_arrItemText.Add(strText);
		}

		tagItemData* pItemData = NULL;
		LVITEM item = {0};
		item.mask = LVIF_IMAGE | LVIF_PARAM;
		item.iItem = nCurSel;
		item.iSubItem = 0;
		GetItem(&item);

		pItemData = (tagItemData*)item.lParam;
		tagItemData* pNewItemData = NULL;
		if(pItemData != NULL)
		{
			pNewItemData = new tagItemData[nColumCount];
			memcpy(pNewItemData, pItemData, sizeof(tagItemData)*nColumCount);
		}

		if (nCurSel >= 0 && nCurSel < nItemCount)
			DeleteItem(nCurSel);

		for (int i=0; i<nColumCount; i++)
		{
			if (i == 0)
				InsertItem(nCurSel+1, m_arrItemText[i], item.iImage);
			else
				SetItemText(nCurSel+1, i, m_arrItemText[i]);
		}

		tagItemData *pData = (tagItemData *)GetItemLParam(nCurSel+1);
		if (pData != NULL)
		{
			delete pData;
			pData = NULL;
		}

		if (pNewItemData != NULL)
		{
			memset(&item, 0, sizeof(LVITEM));
			item.mask = LVIF_PARAM;
			item.iItem = nCurSel+1;
			item.lParam = (LPARAM)pNewItemData;
			SetItem(&item);
		}

		UINT flag = LVIS_SELECTED|LVIS_FOCUSED;
		SetItemState(nCurSel+1, flag, flag);

		/*POINT pt;
		GetItemPosition(nCurSel+1, &pt);

		CRect rcClient;
		GetClientRect(rcClient);
		if (pt.y + rcItem.Height() > rcClient.Height())
		{
			Scroll(CSize(0, rcItem.Height()));
		}	 */
		//SetItemState(GetSelectionMark(), 0 ,LVIS_SELECTED); 
		//SetItemState(nCurSel+1, LVIS_SELECTED ,LVIS_SELECTED);
		//SetSelectionMark(nCurSel+1);
		EnsureVisible(nCurSel+1,FALSE);   
	}
}

void CMulFuncList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	ShowEdit(0, 0, NULL, 0);
	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CMulFuncList::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*)pNMHDR;

	*pResult = CDRF_DODEFAULT;

	if(pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if(pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if(pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
	{
		int nSubItem = pLVCD->iSubItem;
		tagItemData *pItemData = (tagItemData *)pLVCD->nmcd.lItemlParam;
		if(pItemData != NULL)
		{
			pLVCD->clrTextBk = pItemData[nSubItem].clrBk;
			pLVCD->clrText   = pItemData[nSubItem].clrText;
		}
	}
}

BOOL CMulFuncList::SetItemImage(int nIndex, int nImg)
{
	int nItemCount = GetItemCount();
	if (nIndex < 0 || nIndex >= nItemCount)
		return FALSE;

	LVITEM item = {0};
	item.mask   = LVIF_IMAGE;
	item.iItem  = nIndex;
	item.iImage = nImg;
	return SetItem(&item);
}

void* CMulFuncList::GetItemLParam(int nItem)
{
	LVITEM item = {0};
	item.mask   = LVIF_PARAM;
	item.iItem  = nItem;
	if (!GetItem(&item))
		return NULL;

	return (void*)item.lParam;
}

void CMulFuncList::FreeItemData(int nItem)
{
	LVITEM item = {0};
	item.mask   = LVIF_PARAM;
	item.iItem  = nItem;
	if (!GetItem(&item))
		return;

	tagItemData* pData = (tagItemData*)item.lParam;
	if (pData != NULL)
	{
		delete pData;
		pData = NULL;
	}
}

int	 CMulFuncList::GetColumCount()
{
	int nColumCount = 0;
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	if (pHeaderCtrl)
	{
		nColumCount = pHeaderCtrl->GetItemCount();
	}

	return nColumCount;
}

BOOL CMulFuncList::SetItemBackColor(COLORREF clrBk, int nItem, int nSubItem)
{
	int nItemCount = GetItemCount();
	if (nItem < 0 || nItem >= nItemCount)
		return FALSE;

	int nColumCount = GetColumCount();

	LVITEM item = {0};
	item.mask   = LVIF_PARAM;
	item.iItem  = nItem;

	tagItemData *pItemData = (tagItemData *)GetItemLParam(nItem);
	if (pItemData == NULL)
	{
		pItemData = new tagItemData[nColumCount];
	}

	for(int i=0; i<nColumCount; i++)
	{
		if(nSubItem == -1)
			pItemData[i].clrBk = clrBk;
		else if(nSubItem == i)
			pItemData[i].clrBk = clrBk;
	}
	
	item.lParam = (LPARAM)pItemData;
	return SetItem(&item);
}

BOOL CMulFuncList::SetItemTextColor(COLORREF clrText, int nItem, int nSubItem)
{
	int nItemCount = GetItemCount();
	if (nItem < 0 || nItem >= nItemCount)
		return FALSE;

	int nColumCount = GetColumCount();

	LVITEM item = {0};
	item.mask   = LVIF_PARAM;
	item.iItem  = nItem;

	tagItemData *pItemData = (tagItemData *)GetItemLParam(nItem);
	if (pItemData == NULL)
	{
		pItemData = new tagItemData[nColumCount];
	}

	for(int i=0; i<nColumCount; i++)
	{
		if(nSubItem == -1)
			pItemData[i].clrText = clrText;
		else if(nSubItem == i)
			pItemData[i].clrText = clrText;
	}
	
	item.lParam = (LPARAM)pItemData;
	return SetItem(&item);
}

void CMulFuncList::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	FreeItemData(pNMLV->iItem);

	*pResult = 0;
}

void CMulFuncList::OnLvnInsertitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;
}

void CMulFuncList::EnableEdit(BOOL bEnable)
{
	m_bEnableEdit = bEnable;
}

///////////////////////////////////////////////////////////////////////////////////
CListEdit::CListEdit()
{
}

CListEdit::~CListEdit()
{
}

BEGIN_MESSAGE_MAP(CListEdit, CEdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void CListEdit::SetCtrlData(DWORD dwData)
{
	m_dwData = dwData;
}

DWORD CListEdit::GetCtrlData()
{
	return m_dwData;
}

void CListEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	CWnd* pParent = this->GetParent();
	::PostMessage(pParent->GetSafeHwnd(),WM_USER_EDIT_END,m_bExchange,0);
}

BOOL CListEdit::PreTranslateMessage(MSG* pMsg) 
{

	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_RETURN)
		{
			CWnd* pParent = this->GetParent();
			m_bExchange = TRUE;
			::PostMessage(pParent->GetSafeHwnd(),WM_USER_EDIT_END,m_bExchange,0);
		}
		else if(pMsg->wParam == VK_ESCAPE)
		{
			CWnd* pParent = this->GetParent();
			m_bExchange = FALSE;
			::PostMessage(pParent->GetSafeHwnd(),WM_USER_EDIT_END,m_bExchange,0);
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void CListEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
	m_bExchange = TRUE;	
}






