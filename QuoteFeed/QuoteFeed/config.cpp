
#include "stdafx.h"
#include "config.h"

CConfig g_cfg;

CConfig::CConfig()
{
	m_nServiceCount = 10;
	m_pTmp = NULL;
	m_cRecordMode = '0';

	m_bUserDBF    = TRUE;
	m_bSingleFile = TRUE;
	m_nInterval   = 1;
	m_nStartDate  = 0;
	m_nEndDate    = 0;
}

CConfig::~CConfig()
{
	for (unsigned int i = 0; i < m_vDll.size(); i++)
		delete m_vDll[i];
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (NodeName == "��������")
		m_nServiceCount = atoi(NodeText.c_str());
	else if (NodeName == "�����ַ")
		m_strServiceAddress = NodeText;

	else if (NodeName == "Params")
	{
		m_strParam += NodeText;
		if (!m_pTmp)
			return;
		m_pTmp->m_strParams.push_back(m_strParam);
	}
	else if (NodeName == "�����¼")	
		m_cRecordMode = NodeText[0];
	else if (NodeName == "����Դ")
	{
		if (atoi(NodeText.c_str()) == 1)
			m_bUserDBF = FALSE;
		else
			m_bUserDBF = TRUE;
	}
	else if (NodeName == "ѭ������")
	{
		if (atoi(NodeText.c_str()) == 1)
			m_bSingleFile = FALSE;
		else
			m_bSingleFile = TRUE;
	}
	else if (NodeName == "ʱ����")
		m_nInterval = atoi(NodeText.c_str());
	else if (NodeName == "�����ļ�����")
		m_strFileName = NodeText;
	else if (NodeName == "��ʼ�ļ�����")
		m_nStartDate = atoi(NodeText.c_str());
	else if (NodeName == "�����ļ�����")
		m_nEndDate = atoi(NodeText.c_str());
	else if (NodeName == "�豸������IP")
		m_strDevServIP = NodeText;
	else if (NodeName == "�豸�������˿�")
		m_nDevServPort = atoi(NodeText.c_str());
	else if (NodeName == "AppName")
		m_strAppName = NodeText;
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
	if (NodeName == "����Դ")
	{
		if (AttrName == "����")
		{
			m_pTmp = new FeedDll;
			if (m_pTmp)
				m_vDll.push_back(m_pTmp);
			else
				REPORT_RUN(MN, T("�ڴ����ʧ��\n"), RPT_CRITICAL, return);

			m_pTmp->m_strName = AttrText;
		}
		else if (AttrName == "״̬")
		{
			if (m_pTmp)
				m_pTmp->m_bLoad = AttrText == "����" ? true : false;
		}
		else if (AttrName == "�ļ�" && m_pTmp->m_bLoad)
		{
			if (m_pTmp && !m_pTmp->Open(AttrText.c_str()))
				REPORT_RUN(MN, T("װ������Դ%sʧ��\n", AttrText.c_str()), RPT_ERROR, return);
		}
	}

	if (AttrName == "Flag")
		m_strParam = AttrText;

	if (AttrName == "Path")
		m_strRecordPath = AttrText;
}
