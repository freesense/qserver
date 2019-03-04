// Dlg_NetHistory.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_NetHistory.h"


// CDlg_NetHistory 对话框

IMPLEMENT_DYNAMIC(CDlg_NetHistory, CDialog)

CDlg_NetHistory::CDlg_NetHistory(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_NetHistory::IDD, pParent)
{

}

CDlg_NetHistory::~CDlg_NetHistory()
{
}

void CDlg_NetHistory::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_INFO, m_listInfo);
}


BEGIN_MESSAGE_MAP(CDlg_NetHistory, CDialog)
	ON_BN_CLICKED(IDC_BTN_BROWSEBTN, &CDlg_NetHistory::OnBnClickedBtnBrowsebtn)
	ON_BN_CLICKED(IDC_BTN_OPENFOLDER, &CDlg_NetHistory::OnBnClickedBtnOpenfolder)
END_MESSAGE_MAP()


// CDlg_NetHistory 消息处理程序

void CDlg_NetHistory::OnBnClickedBtnBrowsebtn()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST);

	int StructSize = 0;
	DWORD dwVersion = GetVersion();
	if(dwVersion < 0x80000000)			//winnt/xp
		StructSize = 88;				//show new dialog
	else								//win9x
		StructSize = 76;				//show old dialog
	
	CString strDir;
	CValidFunction::GetWorkPath(strDir);
	strDir += "\\HttpData";

	dlg.m_ofn.lStructSize		= StructSize;
	dlg.m_ofn.lpstrInitialDir	= strDir;
	
	if(dlg.DoModal() == IDOK)
	{
		m_listInfo.DeleteAllItems();
		CString strPathName = dlg.GetPathName();
		GetDlgItem(IDC_EDIT_FILEPATH)->SetWindowText(strPathName);
		if (!m_file.Open(strPathName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
		{
			MessageBox("错误: 打开文件失败!");
			return;
		}
		
		int nIndex = 0;
		tagHttpKFileInfo info;
		while(!m_file.IsEOF())
		{
			m_file.GetRecordToBuffer((char*)&info);
			CString strTemp;
			strTemp.Format("%d", info.nDate);
			m_listInfo.InsertItem(nIndex, strTemp);
			strTemp.Format("%d", info.nNewPrice);
			m_listInfo.SetItemText(nIndex, 1, strTemp);

			strTemp.Format("%d", info.nOpenPrice);
			m_listInfo.SetItemText(nIndex, 2, strTemp);

			strTemp.Format("%d", info.nPreClosePrice);
			m_listInfo.SetItemText(nIndex, 3, strTemp);

			strTemp.Format("%d", info.nMaxPrice);
			m_listInfo.SetItemText(nIndex, 4, strTemp);

			strTemp.Format("%d", info.nMinPrice);
			m_listInfo.SetItemText(nIndex, 5, strTemp);

			strTemp.Format("%d", info.nVolume);
			m_listInfo.SetItemText(nIndex, 6, strTemp);

			strTemp.Format("%d", info.nSum);
			m_listInfo.SetItemText(nIndex, 7, strTemp);

			m_file.MoveNext();
			nIndex++;
		}

		m_file.Close();
	}
}

void CDlg_NetHistory::OnBnClickedBtnOpenfolder()
{
}

BOOL CDlg_NetHistory::OnInitDialog()
{
	CDialog::OnInitDialog();

	int nColumnIndex = 0;
	m_listInfo.InsertColumn(nColumnIndex++, "日期", LVCFMT_RIGHT, 80);
	m_listInfo.InsertColumn(nColumnIndex++, "最新价", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "开盘价格", LVCFMT_RIGHT, 60);
	m_listInfo.InsertColumn(nColumnIndex++, "前收盘价格", LVCFMT_RIGHT, 80);
	m_listInfo.InsertColumn(nColumnIndex++, "最高价格", LVCFMT_RIGHT, 80);
	m_listInfo.InsertColumn(nColumnIndex++, "最低价格", LVCFMT_RIGHT, 80);
	m_listInfo.InsertColumn(nColumnIndex++, "成交数量", LVCFMT_RIGHT, 80);
	m_listInfo.InsertColumn(nColumnIndex++, "成交金额", LVCFMT_RIGHT, 80);
	m_listInfo.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
