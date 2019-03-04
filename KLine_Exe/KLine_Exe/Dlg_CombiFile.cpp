// Dlg_CombiFile.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_CombiFile.h"
#include "FolderDlg.h"
#include "FilePath.h"

// CDlg_CombiFile 对话框

IMPLEMENT_DYNAMIC(CDlg_CombiFile, CDialog)

CDlg_CombiFile::CDlg_CombiFile(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_CombiFile::IDD, pParent)
	, m_nQuoteFile1(0)
	, m_nMinFile1(0)
	, m_nTickFile1(0)
	, m_nMinK(150000)
	, m_nTick(14400000)
	, m_nSymbol(10000)
	, m_nMarketCount1(3)
	, m_nMarketCount2(3)
	, m_nMarketCount3(3)
{

}

CDlg_CombiFile::~CDlg_CombiFile()
{
}

void CDlg_CombiFile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_R_QUOTE_FILE1, m_nQuoteFile1);
	DDX_Radio(pDX, IDC_R_MIN_FILE1, m_nMinFile1);
	DDX_Radio(pDX, IDC_R_TICK_FILE1, m_nTickFile1);
	DDX_Text(pDX, IDC_EDIT_MINKCOUNT, m_nMinK);
	DDX_Text(pDX, IDC_EDIT_TICKCOUNT, m_nTick);
	DDX_Text(pDX, IDC_EDIT_SYMBOLCOUNT, m_nSymbol);
	DDX_Text(pDX, IDC_EDIT_MAKETCOUNT1, m_nMarketCount1);
	DDX_Text(pDX, IDC_EDIT_MAKETCOUNT2, m_nMarketCount2);
	DDX_Text(pDX, IDC_EDIT_MAKETCOUNT3, m_nMarketCount3);
}


BEGIN_MESSAGE_MAP(CDlg_CombiFile, CDialog)
	ON_BN_CLICKED(IDCOMBINATION, &CDlg_CombiFile::OnBnClickedCombination)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, &CDlg_CombiFile::OnBnClickedBtnBrowse1)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, &CDlg_CombiFile::OnBnClickedBtnBrowse2)
	ON_BN_CLICKED(IDC_BTN_BROWSE3, &CDlg_CombiFile::OnBnClickedBtnBrowse3)
	ON_BN_CLICKED(IDC_BTN_OPEN1, &CDlg_CombiFile::OnBnClickedBtnOpen1)
	ON_BN_CLICKED(IDC_BTN_OPEN2, &CDlg_CombiFile::OnBnClickedBtnOpen2)
	ON_BN_CLICKED(IDC_BTN_OPEN3, &CDlg_CombiFile::OnBnClickedBtnOpen3)
END_MESSAGE_MAP()


// CDlg_CombiFile 消息处理程序

BOOL CDlg_CombiFile::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CenterWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CDlg_CombiFile::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

void CDlg_CombiFile::OnBnClickedBtnBrowse1()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_EDIT_DATPATH1)->SetWindowText(strPathName);
}

void CDlg_CombiFile::OnBnClickedBtnBrowse2()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_EDIT_DATPATH2)->SetWindowText(strPathName);
}

void CDlg_CombiFile::OnBnClickedBtnBrowse3()
{
	CFolderDlg dlg;
	if (dlg.DoModal() == IDCANCEL)
		return;

	CString strPathName = dlg.GetPathName();
	GetDlgItem(IDC_EDIT_DATPATH3)->SetWindowText(strPathName);
}

void CDlg_CombiFile::OnBnClickedBtnOpen1()
{
	CString strPathName;
	GetDlgItem(IDC_EDIT_DATPATH1)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

void CDlg_CombiFile::OnBnClickedBtnOpen2()
{
	CString strPathName;
	GetDlgItem(IDC_EDIT_DATPATH2)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

void CDlg_CombiFile::OnBnClickedBtnOpen3()
{
	CString strPathName;
	GetDlgItem(IDC_EDIT_DATPATH3)->GetWindowText(strPathName);
	ShellExecute(NULL, "open", "explorer.exe", strPathName, NULL, SW_SHOW);
}

BOOL CDlg_CombiFile::ValidCheck()
{
	UpdateData();
	/*
	GetDlgItem(IDC_EDIT_DATPATH1)->SetWindowText("D:\\Data1");
	GetDlgItem(IDC_EDIT_DATPATH2)->SetWindowText("D:\\Data2");
	GetDlgItem(IDC_EDIT_DATPATH3)->SetWindowText("D:\\");
   //*/
	m_strPath1.Empty();
    GetDlgItem(IDC_EDIT_DATPATH1)->GetWindowText(m_strPath1);
	if (m_strPath1.IsEmpty())
	{
		MessageBox("待合并文件一Dat文件目录不能为空!", "警告");
		return FALSE;
	}

	m_strPath2.Empty();
	GetDlgItem(IDC_EDIT_DATPATH2)->GetWindowText(m_strPath2);
	if (m_strPath2.IsEmpty())
	{
		MessageBox("待合并文件二Dat文件目录不能为空!", "警告");
		return FALSE;
	}

	m_strPath3.Empty();
	GetDlgItem(IDC_EDIT_DATPATH3)->GetWindowText(m_strPath3);
	if (m_strPath3.IsEmpty())
	{
		MessageBox("合并后文件Dat文件目录不能为空!", "警告");
		return FALSE;
	}

	if (m_nMinK <= 0)
	{
		MessageBox("分钟k线的数量必须大于0!", "警告");
		return FALSE;
	}

	if (m_nTick <= 0)
	{
		MessageBox("分笔成交的数量必须大于0!", "警告");
		return FALSE;
	}

	if (m_nSymbol <= 0)
	{
		MessageBox("Symbol的数量必须大于0!", "警告");
		return FALSE;
	}

	if (m_nMarketCount1 <= 0)
	{
		MessageBox("待合并文件一的市场个数必须大于0!", "警告");
		return FALSE;
	}

	if (m_nMarketCount2 <= 0)
	{
		MessageBox("待合并文件二的市场个数必须大于0!", "警告");
		return FALSE;
	}

	if (m_nMarketCount3 <= 0)
	{
		MessageBox("合并后文件的市场个数必须大于0!", "警告");
		return FALSE;
	}

	return TRUE;
}

BOOL CDlg_CombiFile::InitIndexHead(CString strIndexFile, IndexHead* pIndex)
{
	memset(pIndex, 0, sizeof(IndexHead));
	//读取index.dat文件头
	CFile file;
	if (!CValidFunction::IsFileExist(strIndexFile))
	{
		MessageBox("文件" + strIndexFile + "不存在，请检测dat文件路径");
		return FALSE;
	}
	if (!file.Open(strIndexFile, CFile::modeRead|CFile::shareDenyNone))
	{
		MessageBox("打开文件" + strIndexFile + "失败");
		return FALSE;
	}

	UINT nReaded = file.Read(pIndex, sizeof(IndexHead));
	if (nReaded != sizeof(IndexHead))
	{
		file.Close();
		MessageBox("读取文件" + strIndexFile + "失败");
		return FALSE;
	}
	file.Close();
	return TRUE;
}

void CDlg_CombiFile::OnBnClickedCombination()
{
	if (!ValidCheck())
		return;

	CString strTemp = m_strPath1 + "\\index.dat";
	if (!InitIndexHead(strTemp, &theApp.m_pcheckThread->m_indexHead1))
		return;

	strTemp = m_strPath2 + "\\index.dat";
	if (!InitIndexHead(strTemp, &theApp.m_pcheckThread->m_indexHead2))
		return;	

	//*
	{
		strTemp = m_strPath3 + "\\index.dat";
		if (CFilePath::IsFileExist(strTemp))
			::DeleteFile(strTemp);

		strTemp = m_strPath3 + "\\mink.dat";
		if (CFilePath::IsFileExist(strTemp))
			::DeleteFile(strTemp);

		strTemp = m_strPath3 + "\\quote.dat";
		if (CFilePath::IsFileExist(strTemp))
			::DeleteFile(strTemp);

		strTemp = m_strPath3 + "\\tick.dat";
		if (CFilePath::IsFileExist(strTemp))
			::DeleteFile(strTemp);
	}//*/

	theApp.m_pcheckThread->m_nQuoteFile1 = m_nQuoteFile1;
	theApp.m_pcheckThread->m_nMinFile1   = m_nMinFile1;
	theApp.m_pcheckThread->m_nTickFile1  = m_nTickFile1;

	theApp.m_pcheckThread->m_nMinK   = m_nMinK;
	theApp.m_pcheckThread->m_nTick   = m_nTick;
	theApp.m_pcheckThread->m_nSymbol = m_nSymbol;

	theApp.m_pcheckThread->m_strPath1 = m_strPath1;
	theApp.m_pcheckThread->m_strPath2 = m_strPath2;
	theApp.m_pcheckThread->m_strPath3 = m_strPath3;

	theApp.m_pcheckThread->m_nMarketCount1 = m_nMarketCount1;
	theApp.m_pcheckThread->m_nMarketCount2 = m_nMarketCount2;
	theApp.m_pcheckThread->m_nMarketCount3 = m_nMarketCount3;
	
	REPORT(MN, T("正在合并Dat文件...\n"), RPT_INFO);
	CDlg_Info dlgInfo;
	theApp.m_pDlgInfoWnd = &dlgInfo;
	theApp.m_pcheckThread->StartCombinationDatFile();
	dlgInfo.m_strInfo = "正在合并Dat文件...";
	if (dlgInfo.DoModal() == IDCANCEL)
	{
		REPORT(MN, T("用户取消合并Dat文件\n"), RPT_INFO);
		CDlg_Info dlgInfo;
		theApp.m_pDlgInfoWnd = &dlgInfo;
		theApp.m_pcheckThread->StopCombinationDatFile();

		dlgInfo.m_strInfo = "正在终止合并Dat文件...";
		dlgInfo.m_bEnableBtn = FALSE;
		dlgInfo.DoModal();
		dlgInfo.m_bEnableBtn = TRUE;
	}
	else
	{
		REPORT(MN, T("合并Dat文件完成\n"), RPT_INFO);
		MessageBox("合并Dat文件完成");
	}
}
