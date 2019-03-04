//DataProcess.h
#ifndef _DATAPROCESS_H_
#define _DATAPROCESS_H_
#include "TMDdef.h"
#include "../../public/protocol.h"
#include "Log.h"
#include "WorkThread.h"
#include "DataQueue.h"
#include <list>

#include "CN_STK01_Info.h"
#include "HK_STK01_Info.h"
#include "CN_IDU01_Info.h"
#include "HK_IDU01_Info.h"
#include "CN_STK02_Qind.h"
#include "HK_STK02_Qind.h"
#include "CN_MKT01_Idx.h"
#include "HK_MKT01_Idx.h"
#include "CN_STK06_Fgar.h"
#include "HK_STK06_Fgar.h"
#include "CN_MKT02_Qind.h"
#include "HK_MKT02_Qind.h"
#include "CN_IDU02_Qind.h"
#include "HK_IDU02_Qind.h"
#include "CN_Subject_Table.h"
#include "HK_Subject_Table.h"
#include "CN_STK08_Ggar.h"
#include "HK_STK08_Ggar.h"
#include "xmlparser.h"
/////////////////////////////////////////////////////////////////////////////
class CDataProcess
{
public:
	CDataProcess();
	virtual ~CDataProcess();

	int OnInit(unsigned int c_unPlugID,const char * c_pParam);
	void OnRelease();
	int OnRequest(SMsgID* c_pMsgID, char* c_pBuf, int c_iLen);

	//void LogEvent(LPCTSTR lpFormat, ...);
	BOOL InitCfg();
protected:
	static UINT WINAPI _WorkThread(void* pParam);
	
	UINT WorkThread();
	void ProStockInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);
	void ProIndustryInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);
	void IndexInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);
	void HQIndex(SMsgID* pId, tagPackHead* pPackHead, unsigned short wStockCount,
		const char* pMktStock);		 
	void FreeIndex();
	void DataIsExist(BOOL& bFindCN, BOOL& bFindHK);
	BOOL StockIsExist(char* pMktType, char* pStockCode);
	void PayOffCollect(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		char* pStockCode);
	void PayOffBill(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		char* pStockCode);
	void CommentaryBill(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		char* pStockCode);
	void IDXIndex(SMsgID* pId, tagPackHead* pPackHead, unsigned short wStockCount,
		const char* pMktStock);
	void IDUIndex(SMsgID* pId, tagPackHead* pPackHead, unsigned short wStockCount,
		const char* pMktStock);
	void SubjectTableInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		unsigned short nTabNum);
	void VerifyUserInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		const char* pUserID, const char* pPassword);
	void ModifyPassword(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		const char* pUserID, const char* pOldPw, const char* pNewPw);
	void ProCommentaryCollect(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		const char* pStockCode);
	void ProAnalyserInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);
	void ProAnalyserRemark(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		const char* pAccount, const char* pSec_CD, const char* pIdu_CD);
	void ProAddRemark(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		 tagAddRemark* pInfo, const char* pComment);
	void ProHisFinanceIndex(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		const char* pStockCode);
	void ProIndexHisData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		const char* pCode, unsigned short nIdxMark);
	void GetSqlString(BOOL bMt_cn, unsigned short nIdxMark, std::string& strSql);
	void GetFieldValue(char* pValue, unsigned short nIdxMark);
	void ProIduRepresentData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);
	void ProIduSortData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		unsigned short nFactorMark);
	void ProSelStkModInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);
	void ProSelStkModData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
		const char* pCode);
	void MktFactorIdx(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);
	void MktMixIdx(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo);

	void CheckDBCon();
	void RequeryDataFromDB();

private:
	unsigned int m_unPlugID;
	TiXmlParser  m_cfg;
	CWorkThread* m_pWorkThread;       //数据处理线程

	CDataQueue	 m_dataQueue;		  //数据队列
	BOOL		 m_bRun;              //线程运行标志

	CCn_Stk01_Info  m_stkInfo_CN;     //大陆股票基本信息表
	CHK_STK01_Info  m_stkInfo_HK;     //香港股票基本信息表
	
	CCN_IDU01_Info  m_iduInfo_CN;	  //大陆市场行业基本信息
	CHK_IDU01_Info  m_iduInfo_HK;	  //香港市场行业基本信息

	CCN_STK02_Qind  m_stkQind_CN;	  //大陆市场股票行情类指标
	CHK_STK02_Qind  m_stkQind_HK;	  //香港市场股票行情类指标
	std::list<tagMarketandStock*>   m_lstStk; //存放大陆和香港请求行情、指数、行业指标的股票

	CCN_MKT01_Idx   m_mktIdx_CN;     //大陆市场市场指数信息
	CHK_MKT01_Idx   m_mktIdx_HK;     //香港市场市场指数信息

	CCN_STK06_Fgar  m_stkFgar_CN;	 //大陆市场盈利预测汇总表
	CHK_STK06_Fgar  m_stkFgar_HK;	 //香港市场盈利预测汇总表

	CCN_MKT02_Qind	m_mktQind_CN;    //大陆市场指数类指标
	CHK_MKT02_Qind	m_mktQind_HK;    //香港市场指数类指标

	CCN_IDU02_Qind  m_iduQind_CN;    //大陆市场行业类指标
	CHK_IDU02_Qind  m_iduQind_HK;    //香港市场行业类指标

	CCN_Subject_Table  m_SubTab_CN;  //大陆市场科目表
	CHK_Subject_Table  m_SubTab_HK;  //香港市场科目表

	CCN_STK08_Ggar  m_stkGgar_CN;    //大陆市场评级汇总
	CHK_STK08_Ggar  m_stkGgar_HK;    //香港市场评级汇总

	BOOL m_bUpdate;
};

/////////////////////////////////////////////////////////////////////////////
#endif