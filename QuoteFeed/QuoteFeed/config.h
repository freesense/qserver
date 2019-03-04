
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

	BOOL               m_bUserDBF;     //行情数据是使用DBF还是使用日志中的原始数据
	BOOL               m_bSingleFile;  //单个文件循环，还是整个目录下的文件循环
	std::string        m_strFileName;  //单个文件名称
	unsigned int       m_nInterval;    //时间间隔
	unsigned int       m_nStartDate;   //起始文件日期
	unsigned int       m_nEndDate;     //结束文件日期

	std::string        m_strDevServIP; //设备服务器IP
	unsigned int       m_nDevServPort; //设备服务器端口
	std::string        m_strAppName;   //AppName

private:
	FeedDll *m_pTmp;
	std::string m_strParam;
};

extern CConfig g_cfg;

#endif
