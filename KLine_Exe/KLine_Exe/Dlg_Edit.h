#pragma once


// CDlg_Edit �Ի���

class CDlg_Edit : public CDialog
{
	DECLARE_DYNAMIC(CDlg_Edit)

public:
	CDlg_Edit(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_Edit();

	CString    m_strPath;
// �Ի�������
	enum { IDD = IDD_DLG_EDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnDestroy();
};
