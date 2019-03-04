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
	CWorkThread* m_pWorkThread;       //���ݴ����߳�

	CDataQueue	 m_dataQueue;		  //���ݶ���
	BOOL		 m_bRun;              //�߳����б�־

	CCn_Stk01_Info  m_stkInfo_CN;     //��½��Ʊ������Ϣ��
	CHK_STK01_Info  m_stkInfo_HK;     //��۹�Ʊ������Ϣ��
	
	CCN_IDU01_Info  m_iduInfo_CN;	  //��½�г���ҵ������Ϣ
	CHK_IDU01_Info  m_iduInfo_HK;	  //����г���ҵ������Ϣ

	CCN_STK02_Qind  m_stkQind_CN;	  //��½�г���Ʊ������ָ��
	CHK_STK02_Qind  m_stkQind_HK;	  //����г���Ʊ������ָ��
	std::list<tagMarketandStock*>   m_lstStk; //��Ŵ�½������������顢ָ������ҵָ��Ĺ�Ʊ

	CCN_MKT01_Idx   m_mktIdx_CN;     //��½�г��г�ָ����Ϣ
	CHK_MKT01_Idx   m_mktIdx_HK;     //����г��г�ָ����Ϣ

	CCN_STK06_Fgar  m_stkFgar_CN;	 //��½�г�ӯ��Ԥ����ܱ�
	CHK_STK06_Fgar  m_stkFgar_HK;	 //����г�ӯ��Ԥ����ܱ�

	CCN_MKT02_Qind	m_mktQind_CN;    //��½�г�ָ����ָ��
	CHK_MKT02_Qind	m_mktQind_HK;    //����г�ָ����ָ��

	CCN_IDU02_Qind  m_iduQind_CN;    //��½�г���ҵ��ָ��
	CHK_IDU02_Qind  m_iduQind_HK;    //����г���ҵ��ָ��

	CCN_Subject_Table  m_SubTab_CN;  //��½�г���Ŀ��
	CHK_Subject_Table  m_SubTab_HK;  //����г���Ŀ��

	CCN_STK08_Ggar  m_stkGgar_CN;    //��½�г���������
	CHK_STK08_Ggar  m_stkGgar_HK;    //����г���������

	BOOL m_bUpdate;
};

/////////////////////////////////////////////////////////////////////////////
#endif