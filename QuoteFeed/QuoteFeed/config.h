
#ifndef __QUOTEFEED_CONFIG_H__
#define __QUOTEFEED_CONFIG_H__

#include "stdafx.h"
#include "../../public/xml/xmlparser.h"
#include <vector>
#include <string>
#include "../../public/commx/xdll.h"

class FeedDll : public XDLL
{
public:
	std::vector<std::string> m_strParams;
	std::string m_strName;
	bool m_bLoad;
};

class CConfig : public TiXmlParser
{
public:
	CConfig();
	~CConfig();

	virtual void OnText(std::string NodeName, std::string NodeText);
	virtual void OnAttr(std::string NodeName, std::string AttrName, std::string AttrText);

	std::vector<FeedDll*> m_vDll;
	unsigned short m_nServiceCount;
	std::string m_strServiceAddress;
	std::string m_strRecordPath;
	char m_cRecordMode;

	BOOL               m_bUserDBF;     //����������ʹ��DBF����ʹ����־�е�ԭʼ����
	BOOL               m_bSingleFile;  //�����ļ�ѭ������������Ŀ¼�µ��ļ�ѭ��
	std::string        m_strFileName;  //�����ļ�����
	unsigned int       m_nInterval;    //ʱ����
	unsigned int       m_nStartDate;   //��ʼ�ļ�����
	unsigned int       m_nEndDate;     //�����ļ�����

	std::string        m_strDevServIP; //�豸������IP
	unsigned int       m_nDevServPort; //�豸�������˿�
	std::string        m_strAppName;   //AppName

private:
	FeedDll *m_pTmp;
	std::string m_strParam;
};

extern CConfig g_cfg;

#endif
