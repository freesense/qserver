//MulFuncTree.cpp
#include "stdafx.h"
#include "MulFuncTree.h"

////////////////////////////////////////////////////////////////////////////////////
CMulFuncTree::CMulFuncTree()
{
	m_MsgWnd = NULL;
}

CMulFuncTree::~CMulFuncTree()
{
}

void CMulFuncTree::OnDelItemData(DWORD dwData)
{
	void* pData = (void*)dwData;
	if (pData != NULL)
	{
		ASSERT(m_MsgWnd != NULL);
		::SendMessage(m_MsgWnd, WM_DELITEMDATA, dwData, 0);
		delete pData;
		pData = NULL;
	}
}

void CMulFuncTree::DelAllItemData(HTREEITEM hRoot)
{
	if (hRoot == NULL)
		return;

	DWORD dwData = (DWORD)GetItemData(hRoot);
	if (dwData != 0)
		OnDelItemData(dwData);

	HTREEITEM hNextItem  = NULL;
	HTREEITEM hChildItem = GetChildItem(hRoot);

	while (hChildItem != NULL)
	{
		DelAllItemData(hChildItem);

		hNextItem = GetNextItem(hChildItem, TVGN_NEXT);
		hChildItem = hNextItem;
	}
}

void CMulFuncTree::GetAllItemData(HTREEITEM hRoot)
{
	if (hRoot == NULL)
		return;

	DWORD dwData = (DWORD)GetItemData(hRoot);
	if (dwData != 0)
		OnGetItemData(dwData);

	HTREEITEM hNextItem  = NULL;
	HTREEITEM hChildItem = GetChildItem(hRoot);

	while (hChildItem != NULL)
	{
		GetAllItemData(hChildItem);

		hNextItem = GetNextItem(hChildItem, TVGN_NEXT);
		hChildItem = hNextItem;
	}
}

void CMulFuncTree::OnGetItemData(DWORD dwData)
{
	void* pData = (void*)dwData;
	if (pData != NULL)
	{
		ASSERT(m_MsgWnd != NULL);
		::SendMessage(m_MsgWnd, WM_GETITEMDATA, dwData, 0);
	}
}



