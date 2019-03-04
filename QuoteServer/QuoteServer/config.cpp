
#include "stdafx.h"
#include "config.h"
#include "PlugManager.h"
#include "Realdata.h"

CConfig g_cfg;

CConfig::CConfig()
{
	/*
	m_nSymbolCount = 5000;
	m_nMinCount = m_nSymbolCount * 241;
	m_nTickCount = m_nMinCount * 12;

	m_nLineTest = 30;
	m_nAcceptOut = 3;
	m_nTimeOut = 120;

	m_strQuoteServerAddr = "192.168.48.10";
	m_unQuoteServerPort = 9999;

	m_strQuoteFarmAddr = "192.168.48.10";
	m_unQuoteFarmPort = 9001;*/
	m_nDevTimeout = 120;
	m_pPlugItem = NULL;
	m_nMarketCount = 0;
	m_bDebug = false;
	m_bDumpData = false;
}

CConfig::~CConfig()
{}

void CConfig::OnNode(TiXmlElement *pElm, NODEWAY way)
{
	std::string NodeName = pElm->Value();
	if (NodeName == "插件")
	{
		m_pPlugItem = new SPlugItem();
		memset(m_pPlugItem,0,sizeof(SPlugItem));
		g_pPlugs->AddItem(m_pPlugItem);
	}
	else if (NodeName == "市场")
		m_nMarketCount++;
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (NodeName == "DEBUG")
	{
		if (NodeText == "TRUE")
			m_bDebug = true;
	}
	else
	if (NodeName == "DUMPDATA")
	{
		if (NodeText == "TRUE")
			m_bDumpData = true;
	}
	else
	if (NodeName == "设备服务器IP")
	{
		m_strDevServIP = NodeText;
	}
	else
	if (NodeName == "设备服务器端口")
	{
		m_nDevServPort = atoi(NodeText.c_str());
	}
	else
	if (NodeName == "不检测超时")
	{
		g_strNoLineTest += "<" + NodeText + ">";
	}
	else
	if (NodeName == "超时时间")
	{
		m_nDevTimeout = atoi(NodeText.c_str());
	}
	else
	if (NodeName == "设备服务地址")
	{
		m_strSviceAddr = NodeText;
	}
	else
	if (NodeName == "AppName")
	{
		m_strAppName = NodeText;
	}
	else
	if (NodeName == "DLL文件")
	{
		//m_pPlugItem = g_pPlugs->AddItem(NodeText);
		//if (m_pPlugItem)
		//	memset(m_pPlugItem->m_acParam,0,sizeof(m_pPlugItem->m_acParam));
		if (m_pPlugItem)
		{
			strcpy(m_pPlugItem->m_acFile,NodeText.c_str());
		}
	}
	else
	if(NodeName == "起始功能号")
	{
		if (m_pPlugItem)
			m_pPlugItem->m_unBeginFuncNo = atoi(NodeText.c_str());
	}
	else
	if(NodeName == "终止功能号")
	{
		if (m_pPlugItem)
			m_pPlugItem->m_unEndFuncNo = atoi(NodeText.c_str());
	}
	else
	if(NodeName == "参数")
	{
		if (m_pPlugItem)
		{
			strncpy(m_pPlugItem->m_acParam,NodeText.c_str(),255);
		}
	}
	else
	if(NodeName == "启用")
	{
		if (m_pPlugItem)
			m_pPlugItem->m_nState = atoi(NodeText.c_str());
	}

	else
	if(NodeName == "linetest")
		m_nLineTest = atoi(NodeText.c_str());
	else if(NodeName == "acceptout")
		m_nAcceptOut = atoi(NodeText.c_str());
	else if(NodeName == "timeout")
		m_nTimeOut = atoi(NodeText.c_str());
	else if(NodeName == "quoteserveraddr")
		m_strQuoteServerAddr = NodeText;
	else if(NodeName == "quoteserverport")
		m_unQuoteServerPort = atoi(NodeText.c_str());
	else if(NodeName == "quotefarmaddr")
		m_strQuoteFarmAddr = NodeText;
	else if(NodeName == "quotefarmport")
		m_unQuoteFarmPort = atoi(NodeText.c_str());
	else if (NodeName == "Symbol")
		m_nSymbolCount = atoi(NodeText.c_str());
	else if (NodeName == "Min")
		m_nMinCount = atoi(NodeText.c_str());
	else if (NodeName == "Tick")
		m_nTickCount = atoi(NodeText.c_str());
	else if (NodeName == "工作线程")
		m_nTaskCount = atoi(NodeText.c_str());
	else if (NodeName == "Block")
		m_mpMemPool[m_tmp] = atoi(NodeText.c_str());
	else if (NodeName == "代码")
		m_tmpMarket.strMarketCode = NodeText;
	else if (NodeName == "时差")
		m_tmpMarket.nTz = atoi(NodeText.c_str());
	else if (NodeName == "初始化")
		m_tmpMarket.nOpenTime = atoi(NodeText.c_str());
	else if (NodeName == "收盘")
		m_tmpMarket.nCloseTime = atoi(NodeText.c_str());
	else if (NodeName == "历史K线路径")
		m_strHistoryPath = NodeText;
	else
		if (NodeName == "历史分时路径")
		m_strDetailPath = NodeText;
	else if (NodeName == "历史还权K线路径")
		m_strHistoryWeightPath = NodeText;
	else if (NodeName == "交易时间")
	{
		m_tmpMarket.nMinPerDay = 0;
		m_tmpMarket.status = _market::CLOSED;

		//计算交易时间
		unsigned int nBlankMin = 0, nt = 0;
		char *lpToken = strtok((char*)NodeText.c_str(), "-");
		while (lpToken)
		{
			_market::quote_time_slice qts;
			qts.hhmm_b = atoi(lpToken);
			lpToken = strtok(NULL, ",");
			qts.hhmm_e = atoi(lpToken);
			m_tmpMarket.nMinPerDay += GetMinCount(qts.hhmm_b, qts.hhmm_e);
			lpToken = strtok(NULL, "-");
			qts.blank_min = nBlankMin;
			if (lpToken)
				nBlankMin += GetMinCount(qts.hhmm_e, atoi(lpToken));
			m_tmpMarket.dqTime.push_front(qts);
		}
		m_tmpMarket.nMinPerDay++;

		if (m_tmpFeed.bStatus)
			m_vMarket.push_back(m_tmpMarket);
	}
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
	if (NodeName == "Block" && AttrName == "Size")
		m_tmp = atoi(AttrText.c_str());
	else if (NodeName == "大盘统计" && AttrName == "code")
	{
		SummaryInfo * p;
		p = new SummaryInfo();
		memset(p,0,sizeof(SummaryInfo));
		g_IndexSummary.AddIndex((char *)AttrText.c_str(),p);
	}
	else if (NodeName == "品种" && AttrName=="code")
	{
		if (g_StockKindManager.AddStockKind(AttrText))
			REPORT("QS",T("Succ[%s][%s][%s]\n",NodeName.c_str(),AttrName.c_str(),AttrText.c_str()),RPT_INFO);
		else
			REPORT("QS",T("Fail[%s][%s][%s]\n",NodeName.c_str(),AttrName.c_str(),AttrText.c_str()),RPT_INFO);
	}
	else if (NodeName == "行情源")
	{
		if (AttrName == "命名")
			m_tmpFeed.strName = AttrText;
		else if (AttrName == "状态")
			m_tmpFeed.bStatus = AttrText == "正常" ? true : false;
		else if (AttrName == "地址")
		{
			m_tmpFeed.strAddress = AttrText;
			m_vFeed.push_back(m_tmpFeed);
		}
	}
}

unsigned int CConfig::ConvertHHMM2Min(unsigned short nTime)
{
	return nTime/100*60+nTime%100;
}

void CConfig::CheckTaskOnTime(unsigned short nTime, CHKMKT &chkmkt)
{
	for (unsigned int i = 0; i < m_vMarket.size(); i++)
	{
		if (nTime >= m_vMarket[i].nCloseTime && m_vMarket[i].status != _market::CLOSED)
		{
			m_vMarket[i].status = _market::CLOSED;
			chkmkt[m_vMarket[i].strMarketCode] = m_vMarket[i].status;
		}
		else if (nTime >= m_vMarket[i].nOpenTime && m_vMarket[i].status != _market::OPENED)
		{
			m_vMarket[i].status = _market::OPENED;
			chkmkt[m_vMarket[i].strMarketCode] = m_vMarket[i].status;
		}
	}
}

int CConfig::GetTz(const char *lpMarket)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	for (unsigned int i = 0; i < m_vMarket.size(); i++)
	{
		if (m_vMarket[i].strMarketCode == lpTmp)
			return m_vMarket[i].nTz;
	}
	return 0;
}

int CConfig::GetMinPerDay(const char *lpMarket)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	for (unsigned int i = 0; i < m_vMarket.size(); i++)
	{
		if (m_vMarket[i].strMarketCode == lpTmp)
			return m_vMarket[i].nMinPerDay;
	}
	return -1;
}

unsigned int CConfig::GetMinCount(unsigned short nBegin, unsigned short nEnd)
{
	return ConvertHHMM2Min(nEnd) - ConvertHHMM2Min(nBegin);
}

int CConfig::GetQuoteMin(const char *lpMarket, unsigned short nTime)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	for (unsigned int i = 0; i < m_vMarket.size(); i++)
	{
		if (m_vMarket[i].strMarketCode == lpTmp)
		{
			if (nTime <= m_vMarket[i].dqTime.back().hhmm_b)
				return 1;
			if (nTime >= m_vMarket[i].dqTime.front().hhmm_e)
				return m_vMarket[i].nMinPerDay;
			for (unsigned int j = 0; j < m_vMarket[i].dqTime.size(); j++)
			{
				if (nTime >= m_vMarket[i].dqTime[j].hhmm_b)
					return GetMinCount(m_vMarket[i].dqTime.back().hhmm_b, nTime) - m_vMarket[i].dqTime[j].blank_min;
			}
		}
	}
	return -1;
}
