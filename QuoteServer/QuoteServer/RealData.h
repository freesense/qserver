#pragma once
#include <string>
#include "..\\..\\public\\data_struct.h"

class CRealData
{
public:
	CRealData(void);
	~CRealData(void);
	void Reset();
	// 检查接收到的数据，返回指针，REQ长度
	bool PharseReq(char * c_pReqBuf,unsigned short &c_usFuncNo,unsigned int & c_unReqLen);
	//char * CheckReq(char * c_pReqBuf,unsigned int c_iLen,unsigned int & c_unReqLen,unsigned int & c_unPassLen);
	//bool CheckReq(CompletionPortModel * c_pModel,PPER_IO_CONTEXT c_lpIO);
	// 处理请求，结果放在m_szBuffer中，长度放在m_unLen中
	bool GetData(char * c_pReq);
	unsigned int m_unRepLen;
	unsigned int m_unReqLen;
	char  * m_pReqBuf;
	char * m_pRepBuf;
	unsigned int m_unReqBufLen;
	unsigned int m_unRepBufLen;

	unsigned short m_usSeq;//请求包序号
	unsigned short m_usFuncNo;//请求包功能号
	bool	m_bRealPush;//是否是推送请求

	RINDEX m_Idx;
	int cast(std::string  * c_pKey, RINDEX * c_pValue,bool c_bFlag);

	char * GetRepBuf(void);
	unsigned int GetRepLen(void);
	unsigned int GetPreDate(void);
	// 根据请求生成数据
	int MakeData(char * c_pBuf, char * c_pReq);
	
	int MakeINITDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen);
	int MakeLINETESTREP(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen);
	int MakeREPORTDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen);
	int MakeTRACEDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen);
	int MakeREALMINSDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen);
	int MakeREALMINSDATA2(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen);
	int MakeHISKDATA(char * c_pDataBuf, char * c_pReqItem, int & c_iPassLen, unsigned short c_usType);

	int MakeLINETESTREQ(char * c_pDataBuf);

	int GetRealMinData(char * c_pBuf,char *c_pSymbol,unsigned int c_unDate);
	int GetMinKData(char * c_pSymbol,char * c_pRetBuf,int c_nMaxLen);
	int GetTraceData(char * c_pSymbol,char * c_pRetBuf,int c_nMaxLen);
	int GetHisKData(char * c_pSymbol,unsigned int c_unBeginDate,unsigned int c_unEndData, unsigned short c_usType, unsigned short c_usRight,char * c_pRetBuf,unsigned int c_unMaxLen);

	int MakeWrongReturn(char * c_pReq,char * c_pErrMsg,int c_nLen);
	unsigned int GetKDate(unsigned int c_unKDate,unsigned int c_unDate,unsigned short c_usType);
	bool CRealData::IsSameWeek(unsigned int uiNow, unsigned int uiPrev);
	bool CRealData::IsSameMonth(unsigned int uiNow, unsigned int uiPrev);
};

class CQuoteData
{
public:	
	CQuoteData(CRealData * c_pRealData,int c_nType=0)
	{
		m_pRealData = c_pRealData;
		m_Idx.idxQuote = -1;
		m_nType = c_nType;
	};
	CQuoteData()
	{
		m_pRealData = NULL;
		m_Idx.idxQuote = -1;
		m_nType = 0;
	};

	~CQuoteData(void)
	{
	};

public:
	int m_nType;
	CRealData * m_pRealData;
	int OnElement(std::string  * c_pKey,RINDEX * c_pValue);
	RINDEX m_Idx;
	int cast(std::string  * c_pKey,RINDEX * c_pValue,bool c_bFlag);

};

struct SummaryInfo{
	unsigned short  m_usCode;
	unsigned short	m_usUp;
	unsigned short  m_usNoChange;
	unsigned short	m_usDown;
	char m_acReq[128];
};


class CIndexSummary
{
public:
	CIndexSummary()
	{
	};
	~CIndexSummary()
	{
		std::map<std::string,SummaryInfo *>::iterator iter;
		for(iter=m_mapSum.begin(); iter!=m_mapSum.end(); iter++)
		{
			if (iter->second)
				delete iter->second;
		}
		m_mapSum.clear();
	};
public:
	int	AddIndex(char * c_pIndex,SummaryInfo * c_pSum)
	{
		std::string str;
		str = c_pIndex;
		std::map<std::string,SummaryInfo *>::iterator iter;
		iter =m_mapSum.find(str);
		if (iter != m_mapSum.end())
		{
			if (c_pSum)
			{
				if (iter->second)
				{
					iter->second->m_usDown = c_pSum->m_usDown;
					iter->second->m_usNoChange = c_pSum->m_usNoChange;
					iter->second->m_usUp = c_pSum->m_usUp;
				}
				else
					iter->second = c_pSum;
			}
		}
		else
		{
			m_mapSum[str] = c_pSum;
		}
		return m_mapSum.size();
	}
	SummaryInfo * GetIndex(char * c_pIndex)
	{
		std::string str;
		str = c_pIndex;
		std::map<std::string,SummaryInfo *>::iterator iter;
		iter =m_mapSum.find(str);
		if (iter == m_mapSum.end())
			return NULL;
		return iter->second;
	}
	void MakeReq6001()
	{
		std::map<std::string,SummaryInfo *>::iterator iter;
		for(iter=m_mapSum.begin(); iter!=m_mapSum.end(); iter++)
		{
			PPACKETHEAD pHead;
			pHead = (PPACKETHEAD)(iter->second->m_acReq);
			pHead->m_commxHead.Prop = pHead->m_commxHead.MakeProp(0x03,CommxHead::ANSI,CommxHead::HOSTORDER);
			pHead->m_commxHead.SerialNo = 0;
			pHead->m_usFuncNo = 6001;//取指数的上涨，下跌和平盘家数
			strcpy(iter->second->m_acReq + 10,iter->first.c_str());//2BHL + H + 32S
			pHead->m_commxHead.SetLength(2 + 32);
			char buf[10];
			strncpy(buf,iter->first.c_str(),6);
			buf[6] = 0;
			iter->second->m_usCode = atoi(buf);
		}
	}

	void SendReq6001();
	void OnRep6001(unsigned int c_unId,char * c_pRep);
public:
	std::map<std::string,SummaryInfo *> m_mapSum;
};
//股票种类
class CStockKind
{
public:
	CStockKind(){};
	~CStockKind()
	{
		m_mapQuote.clear();
	}
public:
	bool	AddQuote(std::string c_strSymbol,Quote * c_pQuote);
	int		AddSymbols(char * c_pBuf,int c_nLen);
	Quote * GetQuote(std::string c_strSymbol)
	{
		if (m_mapQuote.find(c_strSymbol) != m_mapQuote.end())
			return m_mapQuote[c_strSymbol];
		else
			return NULL;
	}
	void	ClearSymbols()
	{
		m_mapQuote.clear();
	}
	int	SendReq6004();
	int OnReq6004(unsigned short c_usId,char * c_pRep);

	void Summary(unsigned int &c_unCjje,unsigned int &c_unCjsl)
	{
		std::map<std::string,Quote *>::iterator iter;
		unsigned int je,sl;
		je = 0;
		sl = 0;
		for(iter=m_mapQuote.begin(); iter!=m_mapQuote.end(); iter++)
		{
			if (iter->second)
			{
				je += iter->second->cjje;
				sl += iter->second->cjsl;
			}
		}
		c_unCjsl = sl;
		c_unCjje = je;
	}

public:
	unsigned short	m_usId;//编号
	std::string		m_strKindCode;//分类编码
	std::string		m_strKindName;//分类名称
	std::map<std::string,Quote *> m_mapQuote;
};
//证券分类
class CStockKindManager
{
public:
	CStockKindManager()
	{
		m_usId = 1;
	};
	~CStockKindManager()
	{
		std::map<std::string,CStockKind *>::iterator iter;
		for(iter=m_mapKind.begin(); iter!=m_mapKind.end(); iter++)
		{
			if (iter->second)
			{
				delete iter->second;
			}
		}
		m_mapKind.clear();
	};

public:
	bool AddStockKind(CStockKind * c_pItem)
	{
		m_rwLock.WaitWrite();
		std::map<std::string,CStockKind *>::iterator iter;
		iter = m_mapKind.find(c_pItem->m_strKindCode);
		if (iter!=m_mapKind.end())
		{
			c_pItem->m_usId = iter->second->m_usId;
			delete iter->second;
			m_mapKind[c_pItem->m_strKindCode] = c_pItem;
		}
		else
		{
			c_pItem->m_usId = m_usId++;
			m_mapKind[c_pItem->m_strKindCode] = c_pItem;
		}
		m_rwLock.EndWrite();
		return true;
	}
	bool AddStockKind(std::string c_strCode)
	{
		m_rwLock.WaitWrite();
		std::map<std::string,CStockKind *>::iterator iter;
		iter = m_mapKind.find(c_strCode);
		if (iter==m_mapKind.end())
		{			
			CStockKind * pKind;
			pKind = new CStockKind();
			pKind->m_strKindCode = c_strCode;
			pKind->m_usId = m_usId++;
			m_mapKind[c_strCode] = pKind;
			m_rwLock.EndWrite();
			return true;
		}
		m_rwLock.EndWrite();
		return false;
	}


	CStockKind * GetStockKind(char * c_pCode)
	{
		std::string str = c_pCode;
		std::map<std::string,CStockKind *>::iterator iter;
		iter = m_mapKind.find(c_pCode);
		if (iter!=m_mapKind.end())
		{
			return iter->second;
		}
		else
		{
			return NULL;
		}
	}
	void SendReq6004()
	{
		m_rwLock.WaitRead();
		std::map<std::string,CStockKind *>::iterator iter;
		for(iter=m_mapKind.begin(); iter!=m_mapKind.end(); iter++)
		{
			if (iter->first == "md_szwa" || iter->first == "md_shwa")
			{
				iter->second->SendReq6004();
			}
		}
		m_rwLock.EndRead();
	}
	void OnRep6004(unsigned short c_usId,char * c_pRep)
	{
		m_rwLock.WaitWrite();
		std::map<std::string,CStockKind *>::iterator iter;
		for(iter=m_mapKind.begin(); iter!=m_mapKind.end(); iter++)
		{
			if (iter->second->OnReq6004(c_usId,c_pRep))
				break;
		}
		m_rwLock.EndWrite();
	}
	bool Summary(char * c_pCode,unsigned int &c_unCjje,unsigned int &c_unCjsl)
	{
		CStockKind * pKind;
		m_rwLock.WaitRead();
		pKind = this->GetStockKind(c_pCode);
		if (pKind)
		{
			pKind->Summary(c_unCjje,c_unCjsl);
			m_rwLock.EndRead();
			return true;
		}
		m_rwLock.EndRead();
		return false;
	}

public:
	unsigned short m_usId;
	std::map<std::string,CStockKind *> m_mapKind;
	RWLock	m_rwLock;
};