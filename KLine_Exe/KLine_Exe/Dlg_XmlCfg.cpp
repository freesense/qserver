// Dlg_XmlCfg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "KLine_Exe.h"
#include "Dlg_XmlCfg.h"
#include "Dlg_Edit.h"
#include "MainDlg.h"
#include "FilePath.h"

// CDlg_XmlCfg �Ի���

IMPLEMENT_DYNAMIC(CDlg_XmlCfg, CDialog)

CDlg_XmlCfg::CDlg_XmlCfg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlg_XmlCfg::IDD, pParent)
{
   m_hMainWnd = NULL;
   m_hSock = INVALID_SOCKET;
}

CDlg_XmlCfg::~CDlg_XmlCfg()
{
}

void CDlg_XmlCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lstXml);
}


BEGIN_MESSAGE_MAP(CDlg_XmlCfg, CDialog)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD, &CDlg_XmlCfg::OnBnClickedBtnDownload)
	ON_BN_CLICKED(IDC_BTN_UPLOAD, &CDlg_XmlCfg::OnBnClickedBtnUpload)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CDlg_XmlCfg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDlg_XmlCfg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CDlg_XmlCfg::OnBnClickedBtnDel)
	ON_BN_CLICKED(IDC_BTN_UP, &CDlg_XmlCfg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_DOWN, &CDlg_XmlCfg::OnBnClickedBtnDown)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_MODIFY, &CDlg_XmlCfg::OnBnClickedBtnModify)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDlg_XmlCfg::OnNMDblclkList1)
END_MESSAGE_MAP()


// CDlg_XmlCfg ��Ϣ�������

void CDlg_XmlCfg::OnBnClickedBtnDownload()
{
	int nIndex = m_lstXml.GetSelectionMark();
	if (nIndex < 0)
		return;
	CString strPath = m_lstXml.GetItemText(nIndex, 0);

	ASSERT(m_hMainWnd != NULL);
	CMainDlg* pMainDlg = (CMainDlg*)CWnd::FromHandle(m_hMainWnd);
	std::string strDownload;
	strDownload = pMainDlg->m_xmlProtocol.DownloadXml(m_strDevName, strPath);

	int nSendSize = (int)strDownload.length() + sizeof(CommxHead);
	char* pSendBuf = new char[nSendSize];
	CommxHead *pHead = (CommxHead*)pSendBuf;
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = (int)strDownload.length();

	memcpy(pSendBuf + sizeof(CommxHead), strDownload.c_str(), strDownload.length());
	if (!pMainDlg->m_iOcpServer.SendPack(m_hSock, pSendBuf, nSendSize))
		MessageBox("����xml�����ļ�ʧ�ܣ����豸���������ӶϿ���������Ҫ���´򿪡�xml�����ļ����Ի�����³�ʼ����Ϣ", "����");
	delete []pSendBuf;
}

void CDlg_XmlCfg::OnBnClickedBtnUpload()
{
	int nIndex = m_lstXml.GetSelectionMark();
	if (nIndex < 0)
		return;
	CString strPath = m_lstXml.GetItemText(nIndex, 0);
	CString strXmlName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

	std::string strExePath = CFilePath::GetExeFilePath();
	CString strXmlPath = strExePath.c_str();
	strXmlPath += "\\Xml\\" + m_strDevName + m_strIP;
	strXmlPath += "\\" + strXmlName;

	if (!CFilePath::IsFileExist(strXmlPath))
	{
		MessageBox("�ļ�"+strXmlPath+"������", "����");
		return;
	}

	char* pData = NULL;
	DWORD dwLowSize = 0;
	HANDLE hFile = CreateFile(strXmlPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		REPORT(MN, T("Open file[%s]failed, ErrCode:%\n", strXmlPath, GetLastError()), RPT_ERROR);
		MessageBox("�ϴ�xml�����ļ�ʧ��", "����");
		return;
	}
	else
	{
		dwLowSize = GetFileSize(hFile, NULL);
		if (dwLowSize == INVALID_FILE_SIZE)
		{
			CloseHandle(hFile);
			MessageBox("�ϴ�xml�����ļ�ʧ��", "����");
			return;
		}

		pData = new char[dwLowSize];
		memset(pData, 0, dwLowSize);
		DWORD dwReaded = 0;
		ReadFile(hFile, pData, dwLowSize, &dwReaded, NULL);
		if(dwReaded != dwLowSize)
		{
			REPORT(MN, T("ReadFile failed, ErrCode:%\n", GetLastError()), RPT_ERROR);
			CloseHandle(hFile);
			MessageBox("�ϴ�xml�����ļ�ʧ��", "����");
			return;
		}

		CloseHandle(hFile);
	}

	int nBufSize = dwLowSize*2+1;
	char* pDesBuf = new char[nBufSize];
	memset(pDesBuf, 0, nBufSize);
	ChangeStringToBinString(pDesBuf, pData, dwLowSize);
	delete[]pData;

	///////////////
	ASSERT(m_hMainWnd != NULL);
	CMainDlg* pMainDlg = (CMainDlg*)CWnd::FromHandle(m_hMainWnd);
	std::string strUpload = pMainDlg->m_xmlProtocol.UploadXml(strPath, pDesBuf, nBufSize);

	int nSendSize = (int)strUpload.length() + sizeof(CommxHead);
	char* pSendBuf = new char[nSendSize];
	CommxHead *pHead = (CommxHead*)pSendBuf;
	pHead->Prop = CommxHead::MakeProp(0x03, CommxHead::ANSI, CommxHead::HOSTORDER);
	pHead->Length = (int)strUpload.length();

	memcpy(pSendBuf + sizeof(CommxHead), strUpload.c_str(), strUpload.length());
	if (!pMainDlg->m_iOcpServer.SendPack(m_hSock, pSendBuf, nSendSize))
		MessageBox("�ϴ�xml�����ļ�ʧ�ܣ����豸���������ӶϿ���������Ҫ���´򿪡�xml�����ļ����Ի�����³�ʼ����Ϣ", "����");
	else
		MessageBox("�ϴ�xml�����ļ��ɹ�", "��ʾ");
	delete []pSendBuf;
	delete[]pDesBuf;
}

void CDlg_XmlCfg::ChangeStringToBinString(const char* lpDesBuf, const char* lpSrcBuf, int nSrcBufSize)
{
	char* pDesBufPos = (char*)lpDesBuf;
	char* pSrcBufPos = (char*)lpSrcBuf;

	char szTemp[10];
	for(int i=0; i<nSrcBufSize; i++)
	{
		sprintf_s(szTemp, 10, "%02x", (unsigned char)pSrcBufPos[i]);
		memcpy(pDesBufPos, szTemp, 2);
		pDesBufPos += 2;
	}
}

void CDlg_XmlCfg::OnBnClickedBtnOpen()
{
	int nIndex = m_lstXml.GetSelectionMark();
	if (nIndex < 0)
		return;

	CString strPath = m_lstXml.GetItemText(nIndex, 0);
	CString strXmlName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);

	std::string strExePath = CFilePath::GetExeFilePath();
	CString strXmlPath = strExePath.c_str();
	strXmlPath += "\\Xml\\" + m_strDevName + m_strIP;
	strXmlPath += "\\" + strXmlName;

	if (!CFilePath::IsFileExist(strXmlPath))
	{
		MessageBox("�ļ�"+strXmlPath+"������", "����");
		return;
	}
	ShellExecute(NULL, "open", "notepad.exe", strXmlPath, NULL, SW_SHOW);
}

void CDlg_XmlCfg::OnBnClickedBtnAdd()
{
	CDlg_Edit dlg;
	if (dlg.DoModal() == IDOK && !dlg.m_strPath.IsEmpty())
	{
		string strExe = CFilePath::GetExeName(dlg.m_strPath);
		if (strExe != "xml" && strExe != "XML")
		{
			MessageBox("������Ч��xml�����ļ�", "����");
			return;
		}
		int nIndex = m_lstXml.GetItemCount();
		m_lstXml.InsertItem(nIndex, dlg.m_strPath);
	}
}

void CDlg_XmlCfg::OnBnClickedBtnDel()
{
	POSITION pt = m_lstXml.GetFirstSelectedItemPosition();
	if (pt == NULL)
		return;

	if (MessageBox("ȷ��Ҫɾ��ѡ�еļ�¼��?", "����", MB_YESNO) == IDYES)
		m_lstXml.DeleteSelectItem();
}

void CDlg_XmlCfg::OnBnClickedBtnUp()
{
	m_lstXml.MoveSelItemToPrev();
}

void CDlg_XmlCfg::OnBnClickedBtnDown()
{
	m_lstXml.MoveSelItemToNext();
}

BOOL CDlg_XmlCfg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	m_lstXml.InsertColumn(0, "�����ļ�·��", 0, 270);

	int nPos = m_strXmlPath.Find('$');
	int nBeginPos  = 0;

	CString strPath;

	while(nPos != -1)
	{
		strPath = m_strXmlPath.Mid(nBeginPos, nPos - nBeginPos);
		nBeginPos = nPos+1;
		nPos = m_strXmlPath.Find('$', nBeginPos);

		int nIndex = m_lstXml.GetItemCount();
		m_lstXml.InsertItem(nIndex, strPath);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CDlg_XmlCfg::OnDestroy()
{
	int nItemCount = m_lstXml.GetItemCount();
	CString strXmlPath;
	CString strTemp;
	for(int i=0; i<nItemCount; i++)
	{
		strTemp = m_lstXml.GetItemText(i, 0);
		if (strTemp.IsEmpty())
			continue;
		
		strXmlPath += strTemp;
		strXmlPath += "$";
	}

	m_strXmlPath = strXmlPath;

	CDialog::OnDestroy();
}

void CDlg_XmlCfg::OnBnClickedBtnModify()
{
	int nIndex = m_lstXml.GetSelectionMark();
	if (nIndex < 0)
		return;
	CString strPath = m_lstXml.GetItemText(nIndex, 0);
	CDlg_Edit dlg;
	dlg.m_strPath = strPath;
	if (dlg.DoModal() == IDOK)
	{
		string strExe = CFilePath::GetExeName(dlg.m_strPath);
		if (strExe != "xml" && strExe != "XML")
		{
			MessageBox("������Ч��xml�����ļ�", "����");
			return;
		}
		m_lstXml.SetItemText(nIndex, 0, dlg.m_strPath);
	}
}

BOOL CDlg_XmlCfg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

void CDlg_XmlCfg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnBnClickedBtnOpen();
	*pResult = 0;
}
