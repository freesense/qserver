// Dlg_DevVer.cpp : 实现文件
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_DevVer.h"


// CDlg_DevVer 对话框

IMPLEMENT_DYNAMIC(CDlg_DevVer, CDialog)

CDlg_DevVer::CDlg_DevVer(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_DevVer::IDD, pParent)
{

}

CDlg_DevVer::~CDlg_DevVer()
{
}

void CDlg_DevVer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VER, m_lstVer);
}


BEGIN_MESSAGE_MAP(CDlg_DevVer, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CDlg_DevVer 消息处理程序

BOOL CDlg_DevVer::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_lstVer.InsertColumn(0, "模块名称", 0, 200);
	m_lstVer.InsertColumn(1, "版本信息", 0, 100);
	m_lstVer.SetExtendedStyle(LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	CString strText;
	strText.Format("设备[%s]版本信息", m_strDevName);
	SetWindowText(strText);
	
	CString strAllVer;

	int nPos = m_strVer.Find('$');
	CString strMN, strVer;

	int nBeginPos  = 0;
	int nMidPos = 0;
	int nEndPos	= 0;

	while(nPos != -1)
	{
		nBeginPos = nPos;
		nMidPos = m_strVer.Find('#', nBeginPos);
		strMN = m_strVer.Mid(nBeginPos+1, nMidPos - nBeginPos - 1);
		nEndPos = m_strVer.Find('$', nMidPos);
		strVer= m_strVer.Mid(nMidPos+1, nEndPos - nMidPos - 1);

		nPos = m_strVer.Find('$', nEndPos+1);
		if (strAllVer.Find(strMN) < 0)
		{
			strAllVer += strMN;
			int nIndex = m_lstVer.GetItemCount();
			m_lstVer.InsertItem(nIndex, strMN);
			m_lstVer.SetItemText(nIndex, 1, strVer);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlg_DevVer::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}
