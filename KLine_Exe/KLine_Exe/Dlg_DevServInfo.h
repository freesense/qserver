#pragma once


// CDlg_DevServInfo �Ի���

class CDlg_DevServInfo : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevServInfo)

public:
	CDlg_DevServInfo(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_DevServInfo();

// �Ի�������
	enum { IDD = IDD_DLG_DEVSERVINFO };

	BOOL      m_bAddDevServ;    //����豸�����������޸���Ϣ
	CString   m_strDevServName;
	CString   m_strDevServIP;
	CString   m_strRemark;      //��ע
	UINT      m_nDDevServID;    //�豸������ID

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOk();

	DECLARE_MESSAGE_MAP()
};
