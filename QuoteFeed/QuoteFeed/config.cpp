
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
	if (NodeName == "服务容量")
		m_nServiceCount = atoi(NodeText.c_str());
	else if (NodeName == "服务地址")
		m_strServiceAddress = NodeText;

	else if (NodeName == "Params")
	{
		m_strParam += NodeText;
		if (!m_pTmp)
			return;
		m_pTmp->m_strParams.push_back(m_strParam);
	}
	else if (NodeName == "行情记录")	
		m_cRecordMode = NodeText[0];
	else if (NodeName == "数据源")
	{
		if (atoi(NodeText.c_str()) == 1)
			m_bUserDBF = FALSE;
		else
			m_bUserDBF = TRUE;
	}
	else if (NodeName == "循环类型")
	{
		if (atoi(NodeText.c_str()) == 1)
			m_bSingleFile = FALSE;
		else
			m_bSingleFile = TRUE;
	}
	else if (NodeName == "时间间隔")
		m_nInterval = atoi(NodeText.c_str());
	else if (NodeName == "单个文件名称")
		m_strFileName = NodeText;
	else if (NodeName == "起始文件日期")
		m_nStartDate = atoi(NodeText.c_str());
	else if (NodeName == "结束文件日期")
		m_nEndDate = atoi(NodeText.c_str());
	else if (NodeName == "设备服务器IP")
		m_strDevServIP = NodeText;
	else if (NodeName == "设备服务器端口")
		m_nDevServPort = atoi(NodeText.c_str());
	else if (NodeName == "AppName")
		m_strAppName = NodeText;
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
	if (NodeName == "行情源")
	{
		if (AttrName == "命名")
		{
			m_pTmp = new FeedDll;
			if (m_pTmp)
				m_vDll.push_back(m_pTmp);
			else
				REPORT_RUN(MN, T("内存分配失败\n"), RPT_CRITICAL, return);

			m_pTmp->m_strName = AttrText;
		}
		else if (AttrName == "状态")
		{
			if (m_pTmp)
				m_pTmp->m_bLoad = AttrText == "正常" ? true : false;
		}
		else if (AttrName == "文件" && m_pTmp->m_bLoad)
		{
			if (m_pTmp && !m_pTmp->Open(AttrText.c_str()))
				REPORT_RUN(MN, T("装载行情源%s失败\n", AttrText.c_str()), RPT_ERROR, return);
		}
	}

	if (AttrName == "Flag")
		m_strParam = AttrText;

	if (AttrName == "Path")
		m_strRecordPath = AttrText;
}
