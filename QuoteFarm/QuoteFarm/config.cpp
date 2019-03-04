
#include "stdafx.h"
#include "config.h"
#include "regexpr2.h"
#include <algorithm>

using namespace regex;

CConfig g_cfg;

CConfig::CConfig()
{
	m_mode = 0;
	m_nSymbolCount = 5000;
	m_nMinCount = m_nSymbolCount * 241;
	m_nTickCount = m_nMinCount * 12;
	m_iDetailDays = 0;
	m_nDetailDate = 0;
	m_nHolidayDay = 0;
}

CConfig::~CConfig()
{}

void CConfig::ActivateFeed(const char *lpName, void *pFeedHandle)
{
	WGUARD(LockSingle, m_feedLock, grd);
	for (unsigned int i = 0; i < m_vFeed.size(); i++)
	{
		if (m_vFeed[i].strName == lpName)
		{
			m_vFeed[i].bConnected = true;
			break;
		}
	}
}

void CConfig::DisableFeed(void *pFeedHandle)
{
	WGUARD(LockSingle, m_feedLock, grd);
	for (unsigned int i = 0; i < m_vFeed.size(); i++)
	{
		if (m_vFeed[i].pHandler == pFeedHandle)
		{
			m_vFeed[i].bConnected = false;
			break;
		}
	}
}

void CConfig::OnText(std::string NodeName, std::string NodeText)
{
	if (NodeName == "Ʒ��")
	{
		m_tmpPattern.RegexPattern = NodeText;
		m_vPattern.push_back(m_tmpPattern);
	}
	else if (NodeName == "Symbol")
		m_nSymbolCount = atoi(NodeText.c_str());
	else if (NodeName == "Min")
		m_nMinCount = atoi(NodeText.c_str());
	else if (NodeName == "Tick")
		m_nTickCount = atoi(NodeText.c_str());
	else if (NodeName == "·��")
		m_strHisPath = NodeText;
	else if (NodeName == "�����߳�")
		m_nTaskCount = atoi(NodeText.c_str());
	else if (NodeName == "ά����ַ")
		m_strCheckAddress = NodeText;
	else if (NodeName == "��ϸ")
		m_iDetailDays = atoi(NodeText.c_str());
	else if (NodeName == "Block")
		m_mpMemPool[m_tmp] = atoi(NodeText.c_str());
	else if (NodeName == "����")
	{
		m_tmpMarket.strMarketCode = NodeText;
		std::transform(m_tmpMarket.strMarketCode.begin(), m_tmpMarket.strMarketCode.end(), m_tmpMarket.strMarketCode.begin(), tolower);
		m_tmpMarket.nSumLastPrice = m_tmpMarket.nSumPrevClose = 0;		/** ��ʼ��Ϊ0 [7/31/2009 xinl] */
	}
	else if (NodeName == "ʱ��")
		m_tmpMarket.nTz = atoi(NodeText.c_str());
	else if (NodeName == "����")
		m_tmpMarket.nOpenTime = atoi(NodeText.c_str());
	else if (NodeName == "����")
		m_tmpMarket.nCloseTime = atoi(NodeText.c_str());
	else if (NodeName == "�����ļ�")
		m_strHolidayPath = NodeText;
	else if (NodeName == "��������")
		strcpy(m_tmpMarket.szWeekRest, NodeText.c_str());
	else if (NodeName == "����ʱ��")
	{
		m_tmpMarket.dqTime.clear();
		m_tmpMarket.nMinPerDay = 0;

		//���㽻��ʱ��
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

			/// ת���ɡ��ӵ�����㿪ʼ����ģ�����ǰʱ��Ϊֹ�ķ�������
			qts.hhmm_b = qts.hhmm_b/100*60+qts.hhmm_b%100;
			qts.hhmm_e = qts.hhmm_e/100*60+qts.hhmm_e%100;
			m_tmpMarket.dqTime.push_front(qts);
		}
		m_tmpMarket.nMinPerDay++;

		if (m_tmpFeed.bStatus)
			m_vMarket.push_back(m_tmpMarket);
	}
	else if (NodeName == "����仯֪ͨ")
		m_tmpAddin.bQuoteNotify = (NodeText == "YES" ? true : false);
	else if (NodeName == "��ʱ��")
	{
		char *lpToken = strtok((char*)NodeText.c_str(), ", ");
		while (lpToken)
		{
			m_tmpAddin.mpTimer[atoi(lpToken)] = _addin::WAIT;
			lpToken = strtok(NULL, ", ");
		}
		m_vAddin.push_back(m_tmpAddin);
	}
	else if (NodeName == "�豸������IP")
		m_strDevServIP = NodeText;
	else if (NodeName == "�豸�������˿�")
		m_nDevServPort = atoi(NodeText.c_str());
	else if (NodeName == "AppName")
		m_strAppName = NodeText;

	else if (m_mode == 1 && NodeName == "script")
		m_tmpWeight.script = NodeText;
	else if (m_mode == 1 && NodeName == "param")
		m_tmpWeight.params += NodeText + ",";
}

void CConfig::OnAttr(std::string NodeName, std::string AttrName, std::string AttrText)
{
	if (NodeName == "Ʒ��" && AttrName == "code")
		m_tmpPattern.PatternCode = AttrText;
	else if (NodeName == "Block" && AttrName == "Size")
		m_tmp = atoi(AttrText.c_str());
	else if (NodeName == "��ϸ" && AttrName == "Time")
		m_nDetailTime = atoi(AttrText.c_str());
	else if (NodeName == "��ϸ" && AttrName == "Date")
		m_nDetailDate = atoi(AttrText.c_str());
	else if (NodeName == "���" && AttrName == "Package")
	{
		m_tmpAddin.mpTimer.clear();
		m_tmpAddin.strPath = AttrText;
	}
	else if (NodeName == "���" && AttrName == "״̬")
		m_tmpAddin.bStatus = (AttrText == "����" ? true : false);
	else if (NodeName == "ά����ַ" && AttrName == "Heartbeat")
		m_nHeartbeat = atoi(AttrText.c_str());
	else if (NodeName == "ά����ַ" && AttrName == "�Լ�")
		m_nSelfTestCount = atoi(AttrText.c_str());
	else if (NodeName == "����Դ")
	{
		if (AttrName == "����")
			m_tmpFeed.strName = AttrText;
		else if (AttrName == "״̬")
			m_tmpFeed.bStatus = AttrText == "����" ? true : false;
		else if (AttrName == "��ַ")
		{
			m_tmpFeed.pHandler = NULL;
			m_tmpFeed.strAddress = AttrText;
			m_vFeed.push_back(m_tmpFeed);
		}
	}
	else if (NodeName == "����" && AttrName == "Date")
	{
		time_t now;
		time(&now);
		struct tm *lt = localtime(&now);
		m_tmpMarket.nOpenDate = atoi(AttrText.c_str());
		if (m_tmpMarket.nOpenDate == (lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday)
			m_tmpMarket.status = OPENED;
		else
			m_tmpMarket.status = CLOSED;
	}
	else if (NodeName == "����" && AttrName == "Date")
	{
		time_t now;
		time(&now);
		struct tm *lt = localtime(&now);
		m_tmpMarket.nCloseDate = atoi(AttrText.c_str());
		if (m_tmpMarket.nCloseDate == (lt->tm_year+1900)*10000+(lt->tm_mon+1)*100+lt->tm_mday)
			m_tmpMarket.status = CLOSED;
	}

	else if (NodeName == "��Ȩ" && AttrName == "enable")
		m_tmpWeight.bEnable = atoi(AttrText.c_str()) ? true : false;
	else if (NodeName == "��Ȩ" && AttrName == "Time")
		m_tmpWeight.nWeightTime = atoi(AttrText.c_str());
	else if (NodeName == "��Ȩ" && AttrName == "Day")
		m_tmpWeight.day = atoi(AttrText.c_str());
}

void CConfig::OnNode(TiXmlElement *pElm, NODEWAY way)
{
	if (!strcmp(pElm->Value(), "��Ȩ") && way == TiXmlParser::ENTER)
		m_mode = 1;

	if (!strcmp(pElm->Value(), "��Ȩ") && way == TiXmlParser::LEAVE)
	{
		m_vWeight.push_back(m_tmpWeight);
		m_mode = 0;
	}
}

unsigned int CConfig::ConvertHHMM2Min(unsigned short nTime)
{
	return nTime/100*60+nTime%100;
}

unsigned int CConfig::GetMarketDay(time_t now, const char *lpMarket)
{
	int tz = GetTz(lpMarket);
	time_t tzNow = now + tz * 3600;
	struct tm *tztm = gmtime(&tzNow);
	return (tztm->tm_year + 1900) * 10000 + (tztm->tm_mon + 1) * 100 + tztm->tm_mday;
}

unsigned int CConfig::GetMarketTime(time_t now, const char *lpMarket)
{
	int tz = GetTz(lpMarket);
	time_t tzNow = now + tz * 3600;
	struct tm *tztm = gmtime(&tzNow);
	return tztm->tm_hour * 10000 + tztm->tm_min * 100 + tztm->tm_sec;
}

unsigned short CConfig::GetMarketWeek(time_t now, const char *lpMarket)
{
	int tz = GetTz(lpMarket);
	time_t tzNow = now + tz * 3600;
	struct tm *tztm = gmtime(&tzNow);
	return tztm->tm_wday;
}

void CConfig::CheckTaskOnTime(time_t nTime, CHKMKT &chkmkt)
{
	char szTimeZoneToday[21], szTmp[256];
	struct tm *lt = localtime(&nTime);
	unsigned int today = (lt->tm_year + 1900) * 10000 + (lt->tm_mon + 1) * 100 + lt->tm_mday;
	unsigned short now = lt->tm_hour * 100 + lt->tm_min;

	if (m_nHolidayDay != today)
	{
		//���������ļ�
		m_nHolidayDay = today;
		m_mapHoliday.clear();
		std::ifstream fin(g_cfg.m_strHolidayPath.c_str());
		while (fin.is_open())
		{
			std::string sline;
			std::getline(fin, sline);
			if (fin.fail())
				break;
			std::string::size_type pos = sline.find(':');
			m_mapHoliday.insert(std::make_pair(sline.substr(0, pos), sline.substr(pos+1)));
		}

		//���ò����ʱ��״̬
		for (unsigned int i = 0; i < m_vAddin.size(); i++)
		{
			std::map<short, _addin::TIMER_STATUS>::iterator iter = m_vAddin[i].mpTimer.begin();
			for (; iter != m_vAddin[i].mpTimer.end(); ++iter)
				iter->second = _addin::WAIT;
		}
	}

	for (unsigned int i = 0; i < m_vAddin.size(); i++)
	{//�������ʱ��
		std::map<short, _addin::TIMER_STATUS>::iterator iter = m_vAddin[i].mpTimer.begin();
		for (; iter != m_vAddin[i].mpTimer.end(); ++iter)
		{
			if (m_vAddin[i].bStatus && iter->second == _addin::WAIT && now >= iter->first)
			{
				chkmkt.insert(std::make_pair(m_vAddin[i].strPath, itoa(m_vAddin[i].pfnTimer, szTmp, 10)));
				iter->second = _addin::TRIGGERED;
			}
		}
	}

	if (m_iDetailDays != 0 && m_nDetailDate != today && now >= m_nDetailTime)
	{//���浱����ϸ
		m_nDetailDate = today;
		chkmkt.insert(std::make_pair("DETAIL", "nouse"));
		ChangeDetailDate(today);			// todo: �Ƿ�Ӧ�õȵ�����������޸ı�ǣ�
	}

	//��Ȩ
	for (unsigned int i = 0; i < m_vWeight.size(); i++)
	{
		if (m_vWeight[i].bEnable && now >= m_vWeight[i].nWeightTime && m_vWeight[i].day < today)
		{
			m_vWeight[i].day = today;
			chkmkt.insert(std::make_pair("WEIGHT", m_vWeight[i].script + ":" + m_vWeight[i].params));
			ChangeWeightDate(today);
		}
	}

	for (unsigned int i = 0; i < m_vMarket.size(); i++)
	{//������
		unsigned short wekn = GetMarketWeek(nTime, m_vMarket[i].strMarketCode.c_str());
		if (strchr(m_vMarket[i].szWeekRest, '0'+wekn))
			continue;	// ����Ϣ��
		unsigned int today = GetMarketDay(nTime, m_vMarket[i].strMarketCode.c_str());
		unsigned int now = GetMarketTime(nTime, m_vMarket[i].strMarketCode.c_str()) / 100;
		itoa(today, szTimeZoneToday, 10);
		std::map<std::string, std::string>::iterator iterHoliday = m_mapHoliday.find(m_vMarket[i].strMarketCode);
		if (iterHoliday != m_mapHoliday.end() && iterHoliday->second.find(szTimeZoneToday) != std::string::npos)
			continue;	// ������

		if (now >= m_vMarket[i].nCloseTime)
		{
			if (m_vMarket[i].nCloseDate != today)
			{
				m_vMarket[i].nCloseDate = today;
				m_vMarket[i].status = CLOSED;
				chkmkt.insert(std::make_pair(m_vMarket[i].strMarketCode, itoa(m_vMarket[i].status, szTmp, 10)));
				ChangeOpenCloseDate(m_vMarket[i].strMarketCode.c_str(), today, false);
			}
		}
		else if (now >= m_vMarket[i].nOpenTime)
		{
			if (m_vMarket[i].nOpenDate != today)
			{
				m_vMarket[i].nOpenDate = today;
				m_vMarket[i].status = OPENED;
				chkmkt.insert(std::make_pair(m_vMarket[i].strMarketCode, itoa(m_vMarket[i].status, szTmp, 10)));
				ChangeOpenCloseDate(m_vMarket[i].strMarketCode.c_str(), today, true);
			}
		}
	}
}

void CConfig::ChangeDetailDate(unsigned int date)
{
	TiXmlDocument tiDoc;
	tiDoc.LoadFile(m_strXmlFile.c_str());

	TiXmlNode *node = tiDoc.FirstChild("QuoteFarm")->FirstChild("����")->FirstChild("��ʷ")->FirstChild("��ϸ");
	TiXmlElement *elm = node->ToElement();

	char szDate[11];
	elm->SetAttribute("Date", itoa(date, szDate, 10));
	tiDoc.SaveFile();
}

void CConfig::ChangeWeightDate(unsigned int date)
{
	TiXmlDocument doc;
	doc.LoadFile(m_strXmlFile.c_str());

	TiXmlNode *node = doc.FirstChild("QuoteFarm")->FirstChild("��Ȩ");
	TiXmlElement *pElm = node->ToElement();

	char szTmp[256];
	pElm->SetAttribute("Day", itoa(date, szTmp, 10));
	doc.SaveFile();
}

void CConfig::ChangeOpenCloseDate(const char *mktcode, unsigned int date, bool isOpen)
{
	TiXmlDocument tiDoc;
	tiDoc.LoadFile(m_strXmlFile.c_str());

	TiXmlNode *nodeParent = tiDoc.FirstChild("QuoteFarm")->FirstChild("����Դ")->FirstChild("�г�");
	TiXmlNode *node = nodeParent->FirstChild("����")->FirstChild();

	while (1)
	{
		if (strcmp(node->Value(), mktcode))
		{
			nodeParent = nodeParent->NextSibling();
			node = nodeParent->FirstChild("����")->FirstChild();
		}
		else
		{
			char szDate[11];
			char *lpSection = isOpen ? "����" : "����";
			node = node->Parent()->Parent();
			node = node->FirstChild(lpSection);
			TiXmlElement *elm = node->ToElement();
			elm->SetAttribute("Date", itoa(date, szDate, 10));
			tiDoc.SaveFile();
			break;
		}
	}
}

int CConfig::GetTz(const char *lpMarket)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	__try
	{
		for (unsigned int i = 0; i < m_vMarket.size(); i++)
		{
			if (m_vMarket[i].strMarketCode == lpTmp)
				return m_vMarket[i].nTz;
		}
		return 0;
	}
	__finally
	{
		free(lpTmp);
	}
}

int CConfig::GetMinPerDay(const char *lpMarket)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	__try
	{
		for (unsigned int i = 0; i < m_vMarket.size(); i++)
		{
			if (m_vMarket[i].strMarketCode == lpTmp)
				return m_vMarket[i].nMinPerDay;
		}
		return -1;
	}
	__finally
	{
		free(lpTmp);
	}
}

unsigned int CConfig::GetMinCount(unsigned short nBegin, unsigned short nEnd)
{
	return ConvertHHMM2Min(nEnd) - ConvertHHMM2Min(nBegin);
}

int CConfig::GetQuoteMin(const char *lpMarket, unsigned short nTime)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	__try
	{
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
						return nTime - m_vMarket[i].dqTime.back().hhmm_b - m_vMarket[i].dqTime[j].blank_min;
				}
			}
		}
		return -1;
	}
	__finally
	{
		free(lpTmp);
	}
}

int CConfig::ConvergenceTime(const char *lpMarket, unsigned short nTime)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	__try
	{
		for (unsigned int i = 0; i < m_vMarket.size(); i++)
		{
			if (m_vMarket[i].strMarketCode != lpTmp)
				continue;

			if (nTime <= m_vMarket[i].dqTime.back().hhmm_b)
				return m_vMarket[i].dqTime.back().hhmm_b;
			if (nTime >= m_vMarket[i].dqTime.front().hhmm_e)
				return m_vMarket[i].dqTime.front().hhmm_e;
			for (unsigned int j = 0; j < m_vMarket[i].dqTime.size() - 1; j++)
			{
				if (nTime < m_vMarket[i].dqTime[j].hhmm_b && nTime > m_vMarket[i].dqTime[j+1].hhmm_e)
					return m_vMarket[i].dqTime[j+1].hhmm_e;
			}
		}
		return nTime;
	}
	__finally
	{
		free(lpTmp);
	}
}

CConfig::MKT_SECOND CConfig::GetMarketStatus(const char *lpMarket)
{
	char *lpTmp = strlwr(strdup(lpMarket));
	__try
	{
		for (unsigned int i = 0; i < m_vMarket.size(); i++)
		{
			if (m_vMarket[i].strMarketCode == lpTmp)
				return m_vMarket[i].status;
		}
		return UNKNOWN;
	}
	__finally
	{
		free(lpTmp);
	}
}

const char* CConfig::GetPattern(const char *symbol)
{
	static const char *lpBlank = "";
	match_results results;
	for (unsigned int i = 0; i < m_vPattern.size(); i++)
	{
		rpattern pat(m_vPattern[i].RegexPattern.c_str());
		match_results::backref_type br = pat.match(std::string(symbol), results);
		if (br.matched)
			return m_vPattern[i].PatternCode.c_str();
	}
	return lpBlank;
}

void CConfig::UpdateMarketStatus(MarketStatus *pms)
{
	for (unsigned int i = 0; i < m_vMarket.size(); i++)
	{
		strcpy(pms[i].szMarket, m_vMarket[i].strMarketCode.c_str());
		pms[i].dateOpen = m_vMarket[i].nOpenDate;
		pms[i].dateClose = m_vMarket[i].nCloseDate;
	}
}
