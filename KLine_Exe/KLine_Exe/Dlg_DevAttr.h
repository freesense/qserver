#pragma once


// CDlg_DevAttr �Ի���

class CDlg_DevAttr : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevAttr)

public:
	CDlg_DevAttr(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_DevAttr();

// �Ի�������
	enum { IDD = IDD_DLG_DEVATTRIBUTE };

	BOOL      m_bAddDev;      //����豸�����޸��豸
	CString   m_strDevName;
	CString   m_strPath;
	CString   m_strRemark;    //��ע
	CString   m_strParam;     //����
	UINT      m_nParentID;    //���豸��������ID
	UINT      m_nDevID;       

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
