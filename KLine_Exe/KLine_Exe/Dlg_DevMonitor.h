#pragma once


// CDlg_DevMonitor �Ի���

class CDlg_DevMonitor : public CDialog
{
	DECLARE_DYNAMIC(CDlg_DevMonitor)

public:
	CDlg_DevMonitor(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlg_DevMonitor();

// �Ի�������
	enum { IDD = IDD_DLG_DEVMONITOR };

	BOOL SaveConfig();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
