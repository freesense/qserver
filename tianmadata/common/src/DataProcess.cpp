//DataProcess.cpp
#include "stdafx.h"
#include "DataProcess.h"
#include "DataBaseEx.h"
#include "time.h"

extern _SendReply      pSendReply;
extern _GetSymbolQuote pGetSymbolQuote;
extern _PlugQuit       pPlugQuit;
extern _GetSymbolData  pGetSymbolData;
extern void AddLog(const char *pData, int nLen);
extern CDataBaseEx     g_database;

#define CODE_LEN       8
//////////////////////////////////////////////////////////////////
CDataProcess::CDataProcess()
{
	m_unPlugID = 0;
	m_pWorkThread = NULL;
	m_bRun = TRUE;
}

CDataProcess::~CDataProcess()
{
}

int CDataProcess::OnInit(unsigned int c_unPlugID,const char * c_pParam)
{
	//读取配置文件信息
	if (!InitCfg())
	{
		REPORT(MN, T("初始化配置信息失败\n"), RPT_ERROR);
		return -1;
	}

	if (!m_dataQueue.Init())
	{
		REPORT(MN, T("初始化数据请求队列失败\n"), RPT_ERROR);
		return -1;
	}

	if (!g_database.Open(m_cfg.m_strSerIP, m_cfg.m_strDBName, 
		m_cfg.m_strUserName, m_cfg.m_strPassword))
	{
		REPORT(MN, T("打开数据库失败\n"), RPT_ERROR);
		return -1;
	}  
	{//读取数据库信息到内存

		if (!m_stkInfo_CN.Open())
		{
			REPORT(MN, T("m_stkInfo_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_stkInfo_HK.Open())
		{
			REPORT(MN, T("m_stkInfo_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_iduInfo_CN.Open())
		{
			REPORT(MN, T("m_iduInfo_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_iduInfo_HK.Open())
		{
			REPORT(MN, T("m_iduInfo_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}

		if (!m_stkQind_CN.Open())
		{
			REPORT(MN, T("m_stkQind_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_stkQind_HK.Open())
		{
			REPORT(MN, T("m_stkQind_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}

		if (!m_mktIdx_CN.Open())
		{
			REPORT(MN, T("m_mktIdx_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}

		if (!m_mktIdx_HK.Open())
		{
			REPORT(MN, T("m_mktIdx_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_stkFgar_CN.Open())
		{
			REPORT(MN, T("m_stkFgar_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_stkFgar_HK.Open())
		{
			REPORT(MN, T("m_stkFgar_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_mktQind_CN.Open())
		{
			REPORT(MN, T("m_mktQind_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_mktQind_HK.Open())
		{
			REPORT(MN, T("m_mktQind_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_iduQind_CN.Open())
		{
			REPORT(MN, T("m_iduQind_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_iduQind_HK.Open())
		{
			REPORT(MN, T("m_iduQind_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_SubTab_CN.Open())
		{
			REPORT(MN, T("m_SubTab_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_SubTab_HK.Open())
		{
			REPORT(MN, T("m_SubTab_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_stkGgar_CN.Open())
		{
			REPORT(MN, T("m_stkGgar_CN.Open() failed\n"), RPT_ERROR);
			return -1;
		}
		if (!m_stkGgar_HK.Open())
		{
			REPORT(MN, T("m_stkGgar_HK.Open() failed\n"), RPT_ERROR);
			return -1;
		}

	}
	if (m_pWorkThread != NULL)
	{
		REPORT(MN, T("数据处理线程已经在运行\n"), RPT_ERROR);
		return -1;
	}
	m_pWorkThread = new CWorkThread(_WorkThread, this);
	if (m_pWorkThread == NULL)
	{
		REPORT(MN, T("创建线程失败，new CWorkThread() failed\n"), RPT_ERROR);
		return -1;
	}
	if (!m_pWorkThread->Start())
	{
		REPORT(MN, T("启动线程失败，m_pWorkThread->Start() failed\n"), RPT_ERROR);
		return -1;
	}

	//记录插件标识
	m_unPlugID = c_unPlugID;
	return 0;
}

void CDataProcess::OnRelease()
{
	m_bRun = FALSE;
	m_dataQueue.Release();

	if (m_pWorkThread != NULL)
	{
		m_pWorkThread->Stop();
		delete m_pWorkThread;
		m_pWorkThread = NULL;
	}

	{//关闭数据库表
		m_stkInfo_CN.Close();
		m_stkInfo_HK.Close();
		m_iduInfo_CN.Close();
		m_iduInfo_HK.Close();
		m_stkQind_CN.Close();
		m_stkQind_HK.Close();
		m_mktIdx_CN.Close();
		m_mktIdx_HK.Close();
		m_stkFgar_CN.Close();
		m_stkFgar_HK.Close();
		m_mktQind_CN.Close();
		m_mktQind_HK.Close();
		m_iduQind_CN.Close();
		m_iduQind_HK.Close();
		m_SubTab_CN.Close();
		m_SubTab_HK.Close();
		m_stkGgar_CN.Close();
		m_stkGgar_HK.Close();
	}
	g_database.Close();

	FreeIndex();
}

int CDataProcess::OnRequest(SMsgID* c_pMsgID, char* c_pBuf, int c_iLen)
{
	if (c_pMsgID == NULL || c_pBuf == NULL || c_iLen <= 0)
	{
		REPORT(MN, T("OnRequest()参数有误，c_pMsgID=0x%x c_pBuf=0x%x c_iLen=%d\n", c_pMsgID, c_pBuf, c_iLen), RPT_ERROR);
		return -1;
	}

	tagReturnMsgID* pMsgID = new tagReturnMsgID;
	if (pMsgID == NULL)
	{
		REPORT(MN, T("pMsgID = new tagReturnMsgID failed\n"), RPT_ERROR);
		return -1;
	}

	pMsgID->id.m_unID     = c_pMsgID->m_unID;
	pMsgID->id.m_unSocket = c_pMsgID->m_unSocket;
	pMsgID->pBuf = new char[c_iLen];
	if (pMsgID->pBuf == NULL)
	{
		REPORT(MN, T("分配内存[pMsgID->pBuf]失败\n"), RPT_ERROR);
		return -1;
	}
	memcpy(pMsgID->pBuf,c_pBuf,c_iLen);
	pMsgID->nBufSize = c_iLen;

	m_dataQueue.PushBack(pMsgID);
	return 0;
}

BOOL CDataProcess::InitCfg()
{
	char szFilePath[MAX_PATH];
	DWORD dwSize = GetModuleFileName(NULL, szFilePath, MAX_PATH);
	char* pEnd = strrchr(szFilePath, '\\');
	pEnd[0] = 0;
	strcat_s(szFilePath, MAX_PATH, "\\TianmaData.xml");
	m_cfg.Load(szFilePath);

	return TRUE;
}
/*
void CDataProcess::LogEvent(LPCTSTR lpFormat, ...)
{
char chMsg[200];
va_list pArg;
va_start(pArg, lpFormat);
vsprintf_s(chMsg, 200, lpFormat, pArg);
va_end(pArg);

////日志
AddLog(chMsg, (int)strlen(chMsg));
}	  */

UINT CDataProcess::_WorkThread(void* pParam)
{
	CDataProcess* pDataPro = (CDataProcess*)pParam;
	return pDataPro->WorkThread();
}

void CDataProcess::CheckDBCon()
{
	if (!g_database.IsConnectNormal("CN_STK01_Info"))
	{
		REPORT(MN, T("数据库连接断开，正在重连...\n"), RPT_WARNING);
		while(m_bRun)
		{
			if (g_database.Reconnect())
				break;
			Sleep(100);
		}
	}
}

void CDataProcess::RequeryDataFromDB()	
{
	//定时从数据库中取数据
	time_t timeCur;
	time(&timeCur);

	tm tmCur;
	localtime_s(&tmCur, &timeCur);
	int nCurTime = tmCur.tm_hour*100 + tmCur.tm_min;

	if (m_cfg.m_nUpdateTime < nCurTime)
		m_bUpdate = FALSE;

	if (m_cfg.m_nUpdateTime >= nCurTime && !m_bUpdate)
	{
		m_bUpdate = TRUE;
		//检测数据库连接，连接断开就一直重连
		CheckDBCon();

		if (!m_stkInfo_CN.Requery())
		{
			REPORT(MN, T("m_stkInfo_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_stkInfo_HK.Requery())
		{
			REPORT(MN, T("m_stkInfo_HK.Requery() failed\n"), RPT_ERROR);
		}
		if(!m_iduInfo_CN.Requery())
		{
			REPORT(MN, T("m_stkInfo_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_iduInfo_HK.Requery())
		{	
			REPORT(MN, T("m_iduInfo_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_stkQind_CN.Requery())
		{
			REPORT(MN, T("m_stkQind_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_stkQind_HK.Requery())
		{	
			REPORT(MN, T("m_stkQind_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_mktIdx_CN.Requery())
		{	
			REPORT(MN, T("m_mktIdx_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_mktIdx_HK.Requery())
		{
			REPORT(MN, T("m_mktIdx_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_stkFgar_CN.Requery())
		{
			REPORT(MN, T("m_stkFgar_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_stkFgar_HK.Requery())
		{
			REPORT(MN, T("m_stkFgar_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_mktQind_CN.Requery())
		{	
			REPORT(MN, T("m_mktQind_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_mktQind_HK.Requery())
		{
			REPORT(MN, T("m_mktQind_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_iduQind_CN.Requery())
		{
			REPORT(MN, T("m_iduQind_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_iduQind_HK.Requery())
		{
			REPORT(MN, T("m_iduQind_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_SubTab_CN.Requery())
		{
			REPORT(MN, T("m_SubTab_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_SubTab_HK.Requery())
		{
			REPORT(MN, T("m_SubTab_HK.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_stkGgar_CN.Requery())
		{
			REPORT(MN, T("m_stkGgar_CN.Requery() failed\n"), RPT_ERROR);
		}
		if (!m_stkGgar_HK.Requery())
		{
			REPORT(MN, T("m_stkGgar_HK.Requery() failed\n"), RPT_ERROR);
		}
		REPORT(MN, T("从数据库更新数据完成\n"), RPT_INFO);
	}
}

UINT CDataProcess::WorkThread()
{
	m_bRun = TRUE;
	m_bUpdate = FALSE;
	while(m_bRun)
	{
		RequeryDataFromDB();   //定时从数据库中取数据

		tagReturnMsgID* pMsgID = m_dataQueue.PopFront();
		if (pMsgID == NULL)
			continue;
		//处理
		char* pBufPos = pMsgID->pBuf;
		tagPackHead packHead = {0};
		memcpy(&packHead, pBufPos, sizeof(tagPackHead));
		pBufPos += sizeof(tagPackHead);

		if (packHead.comxHead.Length != pMsgID->nBufSize - sizeof(CommxHead))
		{
			REPORT(MN, T("接收数据大小[%d]与包头定义大小[%d]不一致\n", pMsgID->nBufSize - sizeof(CommxHead),packHead.comxHead.Length), RPT_ERROR);
			//释放	
			delete []pMsgID->pBuf;
			delete pMsgID;
			return 0xDEADDEAD;
		}

		for(int i=0; i<packHead.nReqCount; i++)
		{
			tagComInfo comInfo = {0};
			memcpy(&comInfo, pBufPos, sizeof(tagComInfo));
			REPORT(MN, T("正在处理业务[%d]\n", comInfo.nPacktype), RPT_INFO);
			switch(comInfo.nPacktype) //报文类型
			{
			case STOCK_INFO:    //获取所有证券信息
				{
					ProStockInfo(&pMsgID->id, &packHead, &comInfo);
					pBufPos += sizeof(tagComInfo);
					break;
				}
			case INDUSTRY_INFO: //获取行业分类信息
				{
					ProIndustryInfo(&pMsgID->id, &packHead, &comInfo);
					pBufPos += sizeof(tagComInfo);
					break;
				}
			case AREA_INFO:		//获取区域分类信息
				{
					pBufPos += sizeof(tagComInfo);
					break;
				}
			case INDEX_INFO:	//获取指数分类信息
				{
					IndexInfo(&pMsgID->id, &packHead, &comInfo);
					pBufPos += sizeof(tagComInfo);
					break;
				}
			case HQ_INDEX:		//获取股票行情类指标数据
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					unsigned short nStockCount = 0;
					memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
					pBufPos += sizeof(unsigned short);   //股票个数
					HQIndex(&pMsgID->id, &packHead, nStockCount, pBufPos);
					pBufPos += sizeof(tagMarketandStock)*nStockCount;  //nStockCount个市场类别和股票代码
					break;
				}
			case PAYOFF_COLLECT: //盈利预测汇总
				{
					pBufPos += sizeof(tagComInfo);
					char szStockCode[CODE_LEN] = {0};
					memcpy(szStockCode, pBufPos, CODE_LEN);
					pBufPos += CODE_LEN;
					PayOffCollect(&pMsgID->id, &packHead, &comInfo, szStockCode);
					break;
				}
			case PAYOFF_BILL: //盈利预测明细
				{
					pBufPos += sizeof(tagComInfo);
					char szStockCode[CODE_LEN] = {0};
					memcpy(szStockCode, pBufPos, CODE_LEN);
					pBufPos += CODE_LEN;
					PayOffBill(&pMsgID->id, &packHead, &comInfo, szStockCode);
					break;
				}
			case COMMENTARY_BILL: //投资评级明细
				{
					pBufPos += sizeof(tagComInfo);
					char szStockCode[CODE_LEN] = {0};
					memcpy(szStockCode, pBufPos, CODE_LEN);
					pBufPos += CODE_LEN;
					CommentaryBill(&pMsgID->id, &packHead, &comInfo, szStockCode);
					break;
				}
			case IDX_INDEX:	 //指数类指标
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					unsigned short nStockCount = 0;
					memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
					pBufPos += sizeof(unsigned short);   //指数个数
					IDXIndex(&pMsgID->id, &packHead, nStockCount, pBufPos);
					pBufPos += sizeof(tagMarketandStock)*nStockCount;  //nStockCount个市场类别和股票代码
					break;
				}
			case IDU_INDEX:  //行业类指标
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					unsigned short nStockCount = 0;
					memcpy(&nStockCount, pBufPos, sizeof(unsigned short));
					pBufPos += sizeof(unsigned short);   //行业个数
					IDUIndex(&pMsgID->id, &packHead, nStockCount, pBufPos);
					pBufPos += sizeof(tagMarketandStock)*nStockCount;  //nStockCount个市场类别和股票代码
					break;
				}
			case SUBTAB_INFO:  //科目表
				{
					pBufPos += sizeof(tagComInfo);
					unsigned short nTableNum = 0;
					memcpy(&nTableNum, pBufPos, sizeof(unsigned short));
					pBufPos += sizeof(unsigned short);
					SubjectTableInfo(&pMsgID->id, &packHead, &comInfo, nTableNum);
					break;
				}
			case COMMENTARY_COLLECT:  //评级汇总
				{
					pBufPos += sizeof(tagComInfo);
					char szStockCode[8];
					memcpy(szStockCode, pBufPos, 8);
					pBufPos += 8;
					ProCommentaryCollect(&pMsgID->id, &packHead, &comInfo, szStockCode);
					break;
				}
			case VERIFY_USERINFO:  //用户信息验证
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					char szUserID[16] = {0};
					char szPassword[32] = {0};
					memcpy(szUserID, pBufPos, 16);
					pBufPos += 16;
					memcpy(szPassword, pBufPos, 32);
					pBufPos += 32;
					VerifyUserInfo(&pMsgID->id, &packHead, &comInfo, szUserID, szPassword);
					break;
				}
			case MODIFY_PASSWORD:
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					char szUserID[16] = {0};
					char szOldPw[32] = {0};
					char szNewPw[32] = {0};
					memcpy(szUserID, pBufPos, 16);
					pBufPos += 16;
					memcpy(szOldPw, pBufPos, 32);
					pBufPos += 32;
					memcpy(szNewPw, pBufPos, 32);
					pBufPos += 32;
					ModifyPassword(&pMsgID->id, &packHead, &comInfo, szUserID, szOldPw, szNewPw);
					break;
				}
			case ANALYSER_INFO:  //分析师信息
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					ProAnalyserInfo(&pMsgID->id, &packHead, &comInfo);
					break;
				}
			case ANALYSER_REMARK: //分析师评论
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					char szAccount[20];
					memcpy(szAccount, pBufPos, 20);
					pBufPos += 20;
					char szSec_CD[8];
					memcpy(szSec_CD, pBufPos, 8);
					pBufPos += 8;
					char szIdu_CD[10];
					memcpy(szIdu_CD, pBufPos, 10);
					pBufPos += 10;
					ProAnalyserRemark(&pMsgID->id, &packHead, &comInfo, szAccount,
						szSec_CD, szIdu_CD);
					break;
				}
			case ADD_REMARK:
				{
					pBufPos += sizeof(unsigned short);   //报文类型
					tagAddRemark* pInfo = (tagAddRemark*)pBufPos;
					pBufPos += sizeof(tagAddRemark); 
					ProAddRemark(&pMsgID->id, &packHead, &comInfo, pInfo, pBufPos);
					pBufPos += pInfo->CommentLen;	//\0也要算在长度里面
					break;
				}
			case HIS_FINANCE_IDX:  //历史财务类指标
				{
					pBufPos += sizeof(tagComInfo);
					char szStockCode[8];
					memcpy(szStockCode, pBufPos, 8);
					pBufPos += 8;
					ProHisFinanceIndex(&pMsgID->id, &packHead, &comInfo, szStockCode);
					break;
				}
			case INDEX_HIS_DATA:  //指标历史数据
				{
					pBufPos += sizeof(tagComInfo);
					char szCode[8];
					memcpy(szCode, pBufPos, 8);
					pBufPos += 8;

					unsigned short nIdxMark = 0;
					memcpy(&nIdxMark, pBufPos, sizeof(unsigned short));
					pBufPos += sizeof(unsigned short);
					ProIndexHisData(&pMsgID->id, &packHead, &comInfo, szCode, nIdxMark);
					break;
				}
			case IDU_REPRE_DATA:  //行业表现数据
				{
					pBufPos += sizeof(tagComInfo);
					ProIduRepresentData(&pMsgID->id, &packHead, &comInfo);
					break;
				}
			case IDU_SORT_DATA:
				{
					pBufPos += sizeof(tagComInfo);

					unsigned short nFactorMark = 0;
					memcpy(&nFactorMark, pBufPos, sizeof(unsigned short));
					pBufPos += sizeof(unsigned short);
					ProIduSortData(&pMsgID->id, &packHead, &comInfo, nFactorMark);
					break;
				}
			case SELSTKMOD_INFO:
				{
					pBufPos += sizeof(unsigned short);
					ProSelStkModInfo(&pMsgID->id, &packHead, &comInfo);
					break;
				}
			case SELSTKMOD_DATA:
				{
					pBufPos += sizeof(unsigned short);

					char szCode[8];
					memcpy(szCode, pBufPos, 8);
					pBufPos += 8;

					ProSelStkModData(&pMsgID->id, &packHead, &comInfo, szCode);
					break;
				}
			case MKT_FACTOR_IDX:  //市场因素指标
				{
					pBufPos += sizeof(tagComInfo);
					MktFactorIdx(&pMsgID->id, &packHead, &comInfo);
					break;
				}
			case MKT_MIX_IDX:  //市场混合指标
				{
					pBufPos += sizeof(tagComInfo);
					MktMixIdx(&pMsgID->id, &packHead, &comInfo);
					break;
				}
			default:
				{
					pBufPos += sizeof(tagComInfo);
					REPORT(MN, T("不能处理的报文类型[%d]\n", comInfo.nPacktype), RPT_ERROR);
				}
			}
		}
		//释放	
		delete []pMsgID->pBuf;
		delete pMsgID;
	}
	return 0;
}

void CDataProcess::ProStockInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	int nRecordCount = 0; //从数据库获得股票总数
	//添加 数据库操作 代码(区分大陆和香港)
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);
	BOOL bMt_cn = FALSE;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		nRecordCount = m_stkInfo_CN.GetRecordCount();
		m_stkInfo_CN.MoveFirst();
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		nRecordCount = m_stkInfo_HK.GetRecordCount();
		m_stkInfo_HK.MoveFirst();
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		nRecordCount = 0;
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + sizeof(unsigned short) +
		sizeof(tagStockInfo)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //证券个数
		pSendBufPos += sizeof(unsigned short);

		//添加证券信息
		if (bMt_cn)	 //"CN" 大陆市场
		{
			for(int i=0; i<nRecordCount; i++)
			{
				tagStockInfo* pStockInfo = (tagStockInfo*)pSendBufPos;
				pSendBufPos += sizeof(tagStockInfo);

				if (!m_stkInfo_CN.GetRecordToBuffer((char*)pStockInfo))
					REPORT(MN, T("m_stkInfo_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_stkInfo_CN.MoveNext();
			}
		}
		else		 //"HK"	香港市场
		{
			for(int i=0; i<nRecordCount; i++)
			{
				tagStockInfo* pStockInfo = (tagStockInfo*)pSendBufPos;
				pSendBufPos += sizeof(tagStockInfo);

				if (!m_stkInfo_HK.GetRecordToBuffer((char*)pStockInfo))
					REPORT(MN, T("m_stkInfo_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_stkInfo_HK.MoveNext();
			}
		}
	}

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProIndustryInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	int nRecordCount = 0; //从数据库获得股票总数
	//添加 数据库操作 代码(区分大陆和香港)
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);
	BOOL bMt_cn = FALSE;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		nRecordCount = m_iduInfo_CN.GetRecordCount();
		m_iduInfo_CN.MoveFirst();
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		nRecordCount = m_iduInfo_HK.GetRecordCount();
		m_iduInfo_HK.MoveFirst();
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		nRecordCount = 0;
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + sizeof(unsigned short) +
		sizeof(tagIndustryInfo)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //证券个数
		pSendBufPos += sizeof(unsigned short);

		//添加证券信息
		if (bMt_cn)	 //"CN" 大陆市场
		{
			for(int i=0; i<nRecordCount; i++)
			{
				tagIndustryInfo* pIduInfo = (tagIndustryInfo*)pSendBufPos;
				pSendBufPos += sizeof(tagIndustryInfo);

				if (!m_iduInfo_CN.GetRecordToBuffer((char*)pIduInfo))
					REPORT(MN, T("m_iduInfo_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_iduInfo_CN.MoveNext();
			}
		}
		else		 //"HK"	香港市场
		{
			for(int i=0; i<nRecordCount; i++)
			{
				tagIndustryInfo* pIduInfo = (tagIndustryInfo*)pSendBufPos;
				pSendBufPos += sizeof(tagIndustryInfo);

				if (!m_iduInfo_HK.GetRecordToBuffer((char*)pIduInfo))
					REPORT(MN, T("m_iduInfo_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_iduInfo_HK.MoveNext();
			}
		}
	}

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::IndexInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	int nRecordCount = 0; //从数据库获得股票总数
	//添加 数据库操作 代码(区分大陆和香港)
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);
	BOOL bMt_cn = FALSE;
	int nDataSize = 0;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		nRecordCount = m_mktIdx_CN.GetRecordCount();
		m_mktIdx_CN.MoveFirst();
		bMt_cn = TRUE;
		nDataSize = m_mktIdx_CN.GetDataSize();
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		nRecordCount = m_mktIdx_HK.GetRecordCount();
		m_mktIdx_HK.MoveFirst();
		bMt_cn = FALSE;
		nDataSize = m_mktIdx_HK.GetDataSize();
	}
	else  //不能处理的市场
	{
		nRecordCount = 0;
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + sizeof(unsigned short) +
		nDataSize;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //证券个数
		pSendBufPos += sizeof(unsigned short);

		//添加证券信息
		tagMkt_Idx mkt_Idx;
		if (bMt_cn)	 //"CN" 大陆市场
		{
			for(int i=0; i<nRecordCount; i++)
			{

				memset(&mkt_Idx, 0, sizeof(tagMkt_Idx));
				if (!m_mktIdx_CN.GetRecordToBuffer((char*)&mkt_Idx))
					REPORT(MN, T("m_mktIdx_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);

				m_mktIdx_CN.MoveNext();

				int nStructSize = sizeof(tagMkt_Idx) - sizeof(char*);
				memcpy(pSendBufPos, &mkt_Idx, nStructSize);

				pSendBufPos += nStructSize;
				int nStockCodeSize;
				nStockCodeSize = mkt_Idx.nRecordCount * CODE_LEN;
				if (nStockCodeSize > 0)  //有数据才cpy
				{
					REPORT(MN, T("memcpy %d,Scsize=%d..%d]:\n", nStockCodeSize,pSendBufPos-pSendBuf), RPT_INFO);
					memcpy(pSendBufPos, mkt_Idx.pStockCode, nStockCodeSize);
					pSendBufPos += nStockCodeSize;
				}
			}
		}
		else		 //"HK"	香港市场
		{
			for(int i=0; i<nRecordCount; i++)
			{
				memset(&mkt_Idx, 0, sizeof(tagMkt_Idx));
				if (!m_mktIdx_HK.GetRecordToBuffer((char*)&mkt_Idx))
					REPORT(MN, T("m_mktIdx_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_mktIdx_HK.MoveNext();

				int nStructSize = sizeof(tagMkt_Idx) - sizeof(char*);
				memcpy(pSendBufPos, &mkt_Idx, nStructSize);
				pSendBufPos += nStructSize;
				int nStockCodeSize = mkt_Idx.nRecordCount * CODE_LEN;
				if (nStockCodeSize > 0)  //有数据才cpy
				{
					memcpy(pSendBufPos, mkt_Idx.pStockCode, nStockCodeSize);
					pSendBufPos += nStockCodeSize;
				}
			}
		}
	}

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

//股票行情类指标
void CDataProcess::HQIndex(SMsgID* pId, tagPackHead* pPackHead, unsigned short wStockCount,
						   const char* pMktStock)
{
	FreeIndex();
	char* pMktStockPos = (char*)pMktStock;

	for(int i=0; i<wStockCount; i++)
	{
		tagMarketandStock* pMktstk = new tagMarketandStock;
		if (pMktstk == NULL)
		{
			REPORT(MN, T("分配内存[pMktstk]失败\n"), RPT_ERROR);
			return;
		}
		memcpy(pMktstk, pMktStockPos, sizeof(tagMarketandStock));
		pMktStockPos +=	sizeof(tagMarketandStock);
		m_lstStk.push_back(pMktstk);
	}

	//创建缓冲区 一支股票可能对应多个行情类指标，初始化时设为10个
	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) + sizeof(unsigned short) +
		sizeof(tagHQIndex)*wStockCount*10;

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pMktstk]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	int nPackSize   = 0;
	int nStockCount = 0;
	char* pSendBufPos = pSendBuf;
	//memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);  //给报文头留出空间
	nPackSize += sizeof(tagPackHead);

	unsigned short nPackType = HQ_INDEX;
	memcpy(pSendBufPos, &nPackType, sizeof(unsigned short)); //报文类型
	pSendBufPos += sizeof(unsigned short);
	nPackSize += sizeof(unsigned short);

	//memcpy(pSendBufPos, &nStockCount, sizeof(unsigned short)); //股票个数
	pSendBufPos += sizeof(unsigned short);  //给股票个数留出空间
	nPackSize += sizeof(unsigned short);
	//
	BOOL bCNExist = FALSE;
	BOOL bHKExist = FALSE;
	DataIsExist(bCNExist, bHKExist);

	if (bCNExist)  //请求中包含大陆股票
	{
		m_stkQind_CN.MoveFirst();
		while(!m_stkQind_CN.IsEOF())
		{
			tagHQIndex* phqIndex = (tagHQIndex*)pSendBufPos;
			if (!m_stkQind_CN.GetRecordToBuffer((char*)phqIndex))
				REPORT(MN, T("m_stkQind_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);

			char szMktType[10] = {0};
			memcpy(szMktType, phqIndex->MarketType, 2);
			if (!StockIsExist(szMktType, (char*)phqIndex->Sec_cd))
			{
				m_stkQind_CN.MoveNext();
				continue;
			}

			if (nPackSize + (int)sizeof(tagHQIndex) > nSendBufSize) //缓冲区不够则重新分配
			{
				int nNewBufSize = nSendBufSize + sizeof(tagHQIndex)*wStockCount*10; //每支股票再增加10个记录的大小
				char* pNewBuf = new char[nNewBufSize];
				if (pNewBuf == NULL)
				{
					REPORT(MN, T("分配内存[pNewBuf]失败\n"), RPT_ERROR);
					return;
				}
				memcpy(pNewBuf, pSendBuf, nSendBufSize);
				delete []pSendBuf;
				pSendBuf = pNewBuf;
				pSendBufPos = pSendBuf + nPackSize;
				nSendBufSize = nNewBufSize;
			}

			pSendBufPos += sizeof(tagHQIndex);
			nPackSize += sizeof(tagHQIndex);
			nStockCount++;

			m_stkQind_CN.MoveNext();
		}
	}

	if (bHKExist)	//请求中包含香港股票	   
	{
		m_stkQind_HK.MoveFirst();
		while(!m_stkQind_HK.IsEOF())
		{
			tagHQIndex* phqIndex = (tagHQIndex*)pSendBufPos;;
			if (!m_stkQind_HK.GetRecordToBuffer((char*)phqIndex))
				REPORT(MN, T("m_stkQind_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);

			char szMktType[10] = {0};
			memcpy(szMktType, phqIndex->MarketType, 2);
			if (!StockIsExist(szMktType, (char*)phqIndex->Sec_cd))
			{
				m_stkQind_CN.MoveNext();
				continue;
			}

			if (nPackSize + (int)sizeof(tagHQIndex) > nSendBufSize) //缓冲区不够则重新分配
			{
				int nNewBufSize = nSendBufSize + sizeof(tagHQIndex)*wStockCount*10; //每支股票再增加10个记录的大小
				char* pNewBuf = new char[nNewBufSize];
				if (pNewBuf == NULL)
				{
					REPORT(MN, T("分配内存[pNewBuf]失败\n"), RPT_ERROR);
					return;
				}
				memcpy(pNewBuf, pSendBuf, nSendBufSize);
				delete []pSendBuf;
				pSendBuf = pNewBuf;
				pSendBufPos = pSendBuf + nPackSize;
				nSendBufSize = nNewBufSize;
			}

			pSendBufPos += sizeof(tagHQIndex);
			nPackSize += sizeof(tagHQIndex);
			nStockCount++;

			m_stkQind_HK.MoveNext();
		}
	}

	pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;

	if (nStockCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nPackSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
		memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
		pSendBufPos += sizeof(tagPackHead);
		memcpy(pSendBufPos, &nPackType, sizeof(unsigned short)); //报文类型
	}
	else
	{
		pPackHead->comxHead.Length = nPackSize - sizeof(CommxHead);
		memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
		pSendBufPos += sizeof(tagPackHead) + sizeof(unsigned short);
		memcpy(pSendBufPos, &nStockCount, sizeof(unsigned short)); //股票个数
	}

	pSendReply(pId, pSendBuf, nPackSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::FreeIndex()
{
	std::list<tagMarketandStock*>::iterator pos = m_lstStk.begin();
	while(pos != m_lstStk.end())
	{
		delete (*pos);
		pos++;
	}

	m_lstStk.clear();
}

//是否存在大陆市场股票和香港市场股票
void CDataProcess::DataIsExist(BOOL& bFindCN, BOOL& bFindHK) 
{
	bFindCN = bFindHK = FALSE;

	std::list<tagMarketandStock*>::iterator pos = m_lstStk.begin();
	while(pos != m_lstStk.end())
	{
		tagMarketandStock* pData = (tagMarketandStock*)(*pos);
		if (pData != NULL)
		{
			char szMarketType[10] = {0};
			memcpy(szMarketType, pData->MarketType, 2);
			if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
			{
				bFindCN = TRUE;
				if (bFindHK)
					break;
			}
			else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
			{
				bFindHK = TRUE;
				if (bFindCN)
					break;
			}
		}
		pos++;
	}
}

BOOL CDataProcess::StockIsExist(char* pMktType, char* pStockCode)
{
	std::list<tagMarketandStock*>::iterator pos = m_lstStk.begin();
	while(pos != m_lstStk.end())
	{
		tagMarketandStock* pData = (tagMarketandStock*)(*pos);
		if (pData != NULL)
		{
			char szMarketType[10] = {0};
			memcpy(szMarketType, pData->MarketType, 2);

			if (_stricmp(pStockCode, (const char*)pData->Sec_cd) == 0 &&
				_stricmp(pMktType, szMarketType) == 0)
				return TRUE;
		}
		pos++;
	}

	return FALSE;
}

void CDataProcess::PayOffCollect(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
								 char* pStockCode)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		nRecordCount = m_stkFgar_CN.GetStockRecordCount(pStockCode);
		m_stkFgar_CN.MoveFirst();
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		nRecordCount = m_stkFgar_HK.GetStockRecordCount(pStockCode);
		m_stkFgar_HK.MoveFirst();
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		nRecordCount = 0;
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + CODE_LEN +
		sizeof(unsigned short) + sizeof(tagPayOffCollect)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, pStockCode, CODE_LEN);   //股票代码
		pSendBufPos += CODE_LEN;

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		tagPayOffCollectEx Info;
		if (bMt_cn)	 //"CN" 大陆市场
		{
			while(!m_stkFgar_CN.IsEOF())
			{
				memset(&Info, 0, sizeof(tagPayOffCollect));
				if (!m_stkFgar_CN.GetRecordToBuffer((char*)&Info))
					REPORT(MN, T("m_stkFgar_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_stkFgar_CN.MoveNext();
				if (_stricmp(pStockCode, (const char*)Info.SEC_CD) != 0)
					continue;

				memcpy(pSendBufPos, &Info.collect, sizeof(tagPayOffCollect));
				pSendBufPos += sizeof(tagPayOffCollect);
			}
		}
		else		 //"HK"	香港市场
		{
			m_stkFgar_HK.MoveFirst();
			while(!m_stkFgar_HK.IsEOF())
			{
				memset(&Info, 0, sizeof(tagPayOffCollect));
				if (!m_stkFgar_HK.GetRecordToBuffer((char*)&Info))
					REPORT(MN, T("m_stkFgar_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_stkFgar_HK.MoveNext();
				if (_stricmp(pStockCode, (const char*)Info.SEC_CD) != 0)
					continue;

				memcpy(pSendBufPos, &Info.collect, sizeof(tagPayOffCollect));
				pSendBufPos += sizeof(tagPayOffCollect);
			}
		}
	}

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::PayOffBill(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
							  char* pStockCode)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		strSql  = "SELECT * FROM CN_STK05_Fbsc WHERE SEC_CD='";
		strSql += pStockCode;
		strSql += "' AND IS_VLD=1 ORDER BY For_Date DESC";
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		strSql  = "SELECT * FROM HK_STK05_Fbsc WHERE SEC_CD='";
		strSql += pStockCode;
		strSql += "' AND IS_VLD=1 ORDER BY For_Date DESC";
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	if (!g_database.OpenTableUseSql(strSql.c_str()))
	{
		REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
		CheckDBCon();
	}
	nRecordCount = g_database.GetRecordCount();

	if (nRecordCount > 200) //记录太多，只要最近200条
		nRecordCount = 200;

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + CODE_LEN +
		sizeof(unsigned short) + sizeof(tagPayOffBill)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, pStockCode, CODE_LEN);   //股票代码
		pSendBufPos += CODE_LEN;

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息

		for(int i=0; i<nRecordCount; i++)
		{
			tagPayOffBill* pinfo = (tagPayOffBill*)pSendBufPos;
			pSendBufPos += sizeof(tagPayOffBill);

			g_database.GetCollect("ORG_CL", pinfo->ORG_CL);
			g_database.GetCollect("IND_ID", (LPCTSTR)pinfo->IND_ID, 6);

			g_database.GetCollect("Rpt_Yr", (LPCTSTR)pinfo->Rpt_Yr, 4);
			g_database.GetCollect("Rpt_Prd", (LPCTSTR)pinfo->Rpt_Prd, 4);
			g_database.GetCollect("For_Value", pinfo->For_Value);
			g_database.GetCollect("Author", (LPCTSTR)pinfo->Author, 100);
			g_database.GetCollect("Dil_Value", pinfo->Dil_Value);

			SYSTEMTIME tm_For_Date;
			if (!g_database.GetCollect("For_Date", &tm_For_Date))
				pinfo->For_Date[0] = 0;
			else
			{
				int nDate = tm_For_Date.wYear*10000 + tm_For_Date.wMonth*100 + 
					tm_For_Date.wDay;
				char szDate[10] = {0};
				sprintf_s(szDate, 10, "%d", nDate);
				memcpy(pinfo->For_Date, szDate, 8);
			}

			SYSTEMTIME tm_Dil_Date;
			if (!g_database.GetCollect("Dil_Date", &tm_Dil_Date))
				pinfo->Dil_Date[0] = 0;
			else
			{
				int nDate = tm_Dil_Date.wYear*10000 + tm_Dil_Date.wMonth*100 + 
					tm_Dil_Date.wDay;
				char szDate[10] = {0};
				sprintf_s(szDate, 10, "%d", nDate);
				memcpy(pinfo->Dil_Date, &szDate, 8);
			}

			g_database.MoveNext();
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::CommentaryBill(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
								  char* pStockCode)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		strSql  = "SELECT * FROM CN_STK07_Gbsc WHERE SEC_CD='";
		strSql += pStockCode;
		strSql += "' AND IS_VLD=1 ORDER BY Gra_Date DESC";
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		strSql  = "SELECT * FROM HK_STK07_Gbsc WHERE SEC_CD='";
		strSql += pStockCode;
		strSql += "' AND IS_VLD=1 ORDER BY Gra_Date DESC";
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	if (!g_database.OpenTableUseSql(strSql.c_str()))
	{
		REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
		CheckDBCon();
	}
	nRecordCount = g_database.GetRecordCount();

	if (nRecordCount > 200) //记录太多，只要最近200条
		nRecordCount = 200;
	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + CODE_LEN +
		sizeof(unsigned short) + sizeof(tagCommentaryBill)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, pStockCode, CODE_LEN);   //股票代码
		pSendBufPos += CODE_LEN;

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息

		for(int i=0; i<nRecordCount; i++)
		{
			tagCommentaryBill* pinfo = (tagCommentaryBill*)pSendBufPos;
			pSendBufPos += sizeof(tagCommentaryBill);

			g_database.GetCollect("ORG_CL", pinfo->ORG_CL);
			g_database.GetCollect("Gra_Prd", (LPCTSTR)&pinfo->Gra_Prd, 1);
			g_database.GetCollect("Gra_Value", pinfo->Gra_Value);
			g_database.GetCollect("Tar_Price", pinfo->Tar_Price);
			g_database.GetCollect("Tar_Prd", (LPCTSTR)pinfo->Tar_Prd, 3);
			g_database.GetCollect("Author", (LPCTSTR)pinfo->Author, 100);
			g_database.GetCollect("Exr_Price", pinfo->Exr_Price);

			SYSTEMTIME tm_Gra_Date;
			if (!g_database.GetCollect("Gra_Date", &tm_Gra_Date))
				pinfo->Gra_Date[0] = 0;
			else
			{
				int nDate = tm_Gra_Date.wYear*10000 + tm_Gra_Date.wMonth*100 + 
					tm_Gra_Date.wDay;
				char szDate[10] = {0};
				sprintf_s(szDate, 10, "%d", nDate);
				memcpy(pinfo->Gra_Date, szDate, 8);
			}

			SYSTEMTIME tm_Exr_Date;
			if (!g_database.GetCollect("Exr_Date", &tm_Exr_Date))
				pinfo->Exr_Date[0] = 0;
			else
			{
				int nDate = tm_Exr_Date.wYear*10000 + tm_Exr_Date.wMonth*100 + 
					tm_Exr_Date.wDay;
				char szDate[10] = {0};
				sprintf_s(szDate, 10, "%d", nDate);
				memcpy(pinfo->Exr_Date, szDate, 8);
			}

			g_database.MoveNext();
		}
	}
	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}


void CDataProcess::IDXIndex(SMsgID* pId, tagPackHead* pPackHead, unsigned short wStockCount,
							const char* pMktStock)
{
	FreeIndex();
	char* pMktStockPos = (char*)pMktStock;

	for(int i=0; i<wStockCount; i++)
	{
		tagMarketandStock* pMktstk = new tagMarketandStock;
		if (pMktstk == NULL)
		{
			REPORT(MN, T("分配内存[pMktstk]失败\n"), RPT_ERROR);
			return;
		}
		memcpy(pMktstk, pMktStockPos, sizeof(tagMarketandStock));
		pMktStockPos +=	sizeof(tagMarketandStock);
		m_lstStk.push_back(pMktstk);
	}

	//创建缓冲区 一支股票可能对应多个行情类指标，初始化时设为10个
	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) + sizeof(unsigned short) +
		sizeof(tagIDXIndex)*wStockCount*10;
	//pComxHead->Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	int nPackSize   = 0;
	int nStockCount = 0;
	char* pSendBufPos = pSendBuf;
	//memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);  //给报文头留出空间
	nPackSize += sizeof(tagPackHead);

	unsigned short nPackType = IDX_INDEX;
	memcpy(pSendBufPos, &nPackType, sizeof(unsigned short)); //报文类型
	pSendBufPos += sizeof(unsigned short);
	nPackSize += sizeof(unsigned short);

	//memcpy(pSendBufPos, &nStockCount, sizeof(unsigned short)); //股票个数
	pSendBufPos += sizeof(unsigned short);  //给股票个数留出空间
	nPackSize += sizeof(unsigned short);
	//

	BOOL bCNExist = FALSE;
	BOOL bHKExist = FALSE;
	DataIsExist(bCNExist, bHKExist);

	if (bCNExist)  //请求中包含大陆股票
	{
		m_mktQind_CN.MoveFirst();
		while(!m_mktQind_CN.IsEOF())
		{
			tagIDXIndex* pindex = (tagIDXIndex*)pSendBufPos;

			if (!m_mktQind_CN.GetRecordToBuffer((char*)pindex))
				REPORT(MN, T("m_mktQind_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);

			char szMktType[10] = {0};
			memcpy(szMktType, pindex->MarketType, 2);
			if (!StockIsExist(szMktType, (char*)pindex->IDX_CD))
			{
				m_mktQind_CN.MoveNext();
				continue;
			}

			if (nPackSize + (int)sizeof(tagIDXIndex) > nSendBufSize) //缓冲区不够则重新分配
			{
				int nNewBufSize = nSendBufSize + sizeof(tagIDXIndex)*wStockCount*10; //每支股票再增加10个记录的大小
				char* pNewBuf = new char[nNewBufSize];
				if (pNewBuf == NULL)
				{
					REPORT(MN, T("分配内存[pNewBuf]失败\n"), RPT_ERROR);
					return;
				}
				memcpy(pNewBuf, pSendBuf, nSendBufSize);
				delete []pSendBuf;
				pSendBuf = pNewBuf;
				pSendBufPos = pSendBuf + nPackSize;
				nSendBufSize = nNewBufSize;
			}

			pSendBufPos += sizeof(tagIDXIndex);
			nPackSize += sizeof(tagIDXIndex);
			nStockCount++;

			m_mktQind_CN.MoveNext();
		}
	}

	if (bHKExist)	//请求中包含香港股票	   
	{
		m_mktQind_HK.MoveFirst();
		while(!m_mktQind_HK.IsEOF())
		{
			tagIDXIndex* pindex = (tagIDXIndex*)pSendBufPos;

			if (!m_mktQind_HK.GetRecordToBuffer((char*)pindex))
				REPORT(MN, T("m_mktQind_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);

			char szMktType[10] = {0};
			memcpy(szMktType, pindex->MarketType, 2);
			if (!StockIsExist(szMktType, (char*)pindex->IDX_CD))
			{
				m_mktQind_HK.MoveNext();
				continue;
			}

			if (nPackSize + (int)sizeof(tagIDXIndex) > nSendBufSize) //缓冲区不够则重新分配
			{
				int nNewBufSize = nSendBufSize + sizeof(tagIDXIndex)*wStockCount*10; //每支股票再增加10个记录的大小
				char* pNewBuf = new char[nNewBufSize];
				if (pNewBuf == NULL)
				{
					REPORT(MN, T("分配内存[pNewBuf]失败\n"), RPT_ERROR);
					return;
				}
				memcpy(pNewBuf, pSendBuf, nSendBufSize);
				delete []pSendBuf;
				pSendBuf = pNewBuf;
				pSendBufPos = pSendBuf + nPackSize;
				nSendBufSize = nNewBufSize;
			}

			pSendBufPos += sizeof(tagIDXIndex);
			nPackSize += sizeof(tagIDXIndex);
			nStockCount++;

			m_mktQind_HK.MoveNext();
		}
	}

	pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;

	if (nStockCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nPackSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
		memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
		pSendBufPos += sizeof(tagPackHead);
		memcpy(pSendBufPos, &nPackType, sizeof(unsigned short)); //报文类型
	}
	else
	{
		pPackHead->comxHead.Length = nPackSize - sizeof(CommxHead);
		memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
		pSendBufPos += sizeof(tagPackHead) + sizeof(unsigned short);
		memcpy(pSendBufPos, &nStockCount, sizeof(unsigned short)); //股票个数
	}

	pSendReply(pId, pSendBuf, nPackSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}


void CDataProcess::IDUIndex(SMsgID* pId, tagPackHead* pPackHead, unsigned short wStockCount,
							const char* pMktStock)
{
	FreeIndex();
	char* pMktStockPos = (char*)pMktStock;

	for(int i=0; i<wStockCount; i++)
	{
		tagMarketandStock* pMktstk = new tagMarketandStock;
		if (pMktstk == NULL)
		{
			REPORT(MN, T("分配内存[pMktstk]失败\n"), RPT_ERROR);
			return;
		}
		memcpy(pMktstk, pMktStockPos, sizeof(tagMarketandStock));
		pMktStockPos +=	sizeof(tagMarketandStock);
		m_lstStk.push_back(pMktstk);
	}

	//创建缓冲区  一支股票可能对应多个行情类指标，初始化时设为10个
	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) + sizeof(unsigned short) +
		sizeof(tagIduIndex)*wStockCount*10;
	//pComxHead->Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	int nPackSize   = 0;
	int nStockCount = 0;
	char* pSendBufPos = pSendBuf;
	//memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);  //给报文头留出空间
	nPackSize += sizeof(tagPackHead);

	unsigned short nPackType = IDU_INDEX;
	memcpy(pSendBufPos, &nPackType, sizeof(unsigned short)); //报文类型
	pSendBufPos += sizeof(unsigned short);
	nPackSize += sizeof(unsigned short);

	//memcpy(pSendBufPos, &nStockCount, sizeof(unsigned short)); //股票个数
	pSendBufPos += sizeof(unsigned short);  //给股票个数留出空间
	nPackSize += sizeof(unsigned short);
	//

	BOOL bCNExist = FALSE;
	BOOL bHKExist = FALSE;
	DataIsExist(bCNExist, bHKExist);

	if (bCNExist)  //请求中包含大陆股票
	{
		m_iduQind_CN.MoveFirst();
		while(!m_iduQind_CN.IsEOF())
		{
			tagIduIndex* pindex = (tagIduIndex*)pSendBufPos;

			if (!m_iduQind_CN.GetRecordToBuffer((char*)pindex))
				REPORT(MN, T("m_iduQind_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);

			char szMktType[10] = {0};
			memcpy(szMktType, pindex->MarketType, 2);
			if (!StockIsExist(szMktType, (char*)pindex->IDU_CD))
			{
				m_iduQind_CN.MoveNext();
				continue;
			}

			if (nPackSize + (int)sizeof(tagIduIndex) > nSendBufSize) //缓冲区不够则重新分配
			{
				int nNewBufSize = nSendBufSize + sizeof(tagIduIndex)*wStockCount*10; //每支股票再增加10个记录的大小
				char* pNewBuf = new char[nNewBufSize];
				if (pNewBuf == NULL)
				{
					REPORT(MN, T("分配内存[pNewBuf]失败\n"), RPT_ERROR);
					return;
				}
				memcpy(pNewBuf, pSendBuf, nSendBufSize);
				delete []pSendBuf;
				pSendBuf = pNewBuf;
				pSendBufPos = pSendBuf + nPackSize;
				nSendBufSize = nNewBufSize;
			}

			pSendBufPos += sizeof(tagIduIndex);
			nPackSize += sizeof(tagIduIndex);
			nStockCount++;

			m_iduQind_CN.MoveNext();
		}
	}

	if (bHKExist)	//请求中包含香港股票	   
	{
		m_iduQind_HK.MoveFirst();
		while(!m_iduQind_HK.IsEOF())
		{
			tagIduIndex* pindex = (tagIduIndex*)pSendBufPos;

			if (!m_iduQind_HK.GetRecordToBuffer((char*)pindex))
				REPORT(MN, T("m_iduQind_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);

			char szMktType[10] = {0};
			memcpy(szMktType, pindex->MarketType, 2);
			if (!StockIsExist(szMktType, (char*)pindex->IDU_CD))
			{
				m_iduQind_HK.MoveNext();
				continue;
			}

			if (nPackSize + (int)sizeof(tagIduIndex) > nSendBufSize) //缓冲区不够则重新分配
			{
				int nNewBufSize = nSendBufSize + sizeof(tagIduIndex)*wStockCount*10; //每支股票再增加10个记录的大小
				char* pNewBuf = new char[nNewBufSize];
				if (pNewBuf == NULL)
				{
					REPORT(MN, T("分配内存[pNewBuf]失败\n"), RPT_ERROR);
					return;
				}
				memcpy(pNewBuf, pSendBuf, nSendBufSize);
				delete []pSendBuf;
				pSendBuf = pNewBuf;
				pSendBufPos = pSendBuf + nPackSize;
				nSendBufSize = nNewBufSize;
			}

			pSendBufPos += sizeof(tagIduIndex);
			nPackSize += sizeof(tagIduIndex);
			nStockCount++;

			m_iduQind_HK.MoveNext();
		}
	}

	pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;

	if (nStockCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nPackSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
		memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
		pSendBufPos += sizeof(tagPackHead);
		memcpy(pSendBufPos, &nPackType, sizeof(unsigned short)); //报文类型
	}
	else
	{
		pPackHead->comxHead.Length = nPackSize - sizeof(CommxHead);
		memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
		pSendBufPos += sizeof(tagPackHead) + sizeof(unsigned short);
		memcpy(pSendBufPos, &nStockCount, sizeof(unsigned short)); //股票个数
	}

	pSendReply(pId, pSendBuf, nPackSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::SubjectTableInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
									unsigned short nTabNum)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		nRecordCount = m_SubTab_CN.MovePos(nTabNum);
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		nRecordCount = m_SubTab_HK.MovePos(nTabNum);
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		nRecordCount = 0;
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + sizeof(unsigned short) +
		sizeof(unsigned short) + sizeof(tagSubjectTable)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, &nTabNum, sizeof(unsigned short));   //科目表编码
		pSendBufPos += sizeof(unsigned short);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		tagSubjectTableEx Info;
		if (bMt_cn)	 //"CN" 大陆市场
		{
			for(int i=0; i<nRecordCount; i++)
			{
				memset(&Info, 0, sizeof(tagSubjectTableEx));
				if (!m_SubTab_CN.GetRecordToBuffer((char*)&Info))
					REPORT(MN, T("m_SubTab_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_SubTab_CN.MoveNext();
				if (nTabNum != Info.TableNum)
					continue;

				memcpy(pSendBufPos, &Info.table, sizeof(tagSubjectTable));
				pSendBufPos += sizeof(tagSubjectTable);
			}
		}
		else		 //"HK"	香港市场
		{
			for(int i=0; i<nRecordCount; i++)
			{
				memset(&Info, 0, sizeof(tagSubjectTableEx));
				if (!m_SubTab_HK.GetRecordToBuffer((char*)&Info))
					REPORT(MN, T("m_SubTab_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_SubTab_HK.MoveNext();
				if (nTabNum != Info.TableNum)
					continue;

				memcpy(pSendBufPos, &Info.table, sizeof(tagSubjectTable));
				pSendBufPos += sizeof(tagSubjectTable);
			}
		}
	}

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::VerifyUserInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
								  const char* pUserID, const char* pPassword)
{
	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) + 
		sizeof(tagVerifyUserInfo);
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);

	memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short)); //报文类型
	pSendBufPos += sizeof(unsigned short);

	//添加信息
	tagVerifyUserInfo* pInfo = (tagVerifyUserInfo*)pSendBufPos;
	pSendBufPos += sizeof(tagVerifyUserInfo);
	memset(pInfo, 0, sizeof(tagVerifyUserInfo));

	BOOL bRet = g_database.VerifyUserInfo(pUserID, pPassword, (const char*)pInfo->UserName,
		(const char*)pInfo->Role);
	if (bRet)
		memcpy(&pInfo->Result, "1", 1);
	else
		memcpy(&pInfo->Result, "0", 1);
	memcpy(pInfo->UserID, pUserID, 16);

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ModifyPassword(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
								  const char* pUserID, const char* pOldPw, const char* pNewPw)
{
	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) + 
		sizeof(tagModifyPassword);
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);

	memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short)); //报文类型
	pSendBufPos += sizeof(unsigned short);

	//添加信息
	tagModifyPassword* pInfo = (tagModifyPassword*)pSendBufPos;
	pSendBufPos += sizeof(tagModifyPassword);

	BOOL bRet = g_database.ModifyPassword(pUserID, pOldPw, pNewPw);
	if (bRet)
		memcpy(&pInfo->Result, "1", 1);
	else
		memcpy(&pInfo->Result, "0", 1);
	memcpy(pInfo->UserID, pUserID, 16);

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProCommentaryCollect(SMsgID* pId, tagPackHead* pPackHead, 
										tagComInfo* pComInfo, const char* pStockCode)
{
	int nRecordCount = 0; //从数据库获得股票总数
	//添加 数据库操作 代码(区分大陆和香港)
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);
	BOOL bMt_cn = FALSE;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		nRecordCount = m_stkGgar_CN.GetStockRecordCount(pStockCode);
		m_stkGgar_CN.MoveFirst();
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		nRecordCount = m_stkGgar_HK.GetStockRecordCount(pStockCode);
		m_stkGgar_HK.MoveFirst();
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		nRecordCount = 0;
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + 8 + sizeof(unsigned short) +
		sizeof(tagCommentaryCollect)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, pStockCode, 8);
		pSendBufPos += 8;

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //证券个数
		pSendBufPos += sizeof(unsigned short);

		//添加证券信息
		tagCommentaryCollectEx info;
		if (bMt_cn)	 //"CN" 大陆市场
		{
			while(!m_stkGgar_CN.IsEOF())
			{
				memset(&info, 0, sizeof(tagCommentaryCollectEx));
				if (!m_stkGgar_CN.GetRecordToBuffer((char*)&info))
					REPORT(MN, T("m_stkGgar_CN.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_stkGgar_CN.MoveNext();

				if (_stricmp(pStockCode, (const char*)info.SEC_CD) != 0)
					continue;

				memcpy(pSendBufPos, &info.collect, sizeof(tagCommentaryCollect));
				pSendBufPos += sizeof(tagCommentaryCollect);
			}
		}
		else		 //"HK"	香港市场
		{
			while(!m_stkGgar_HK.IsEOF())
			{
				memset(&info, 0, sizeof(tagCommentaryCollectEx));
				if (!m_stkGgar_HK.GetRecordToBuffer((char*)&info))
					REPORT(MN, T("m_stkGgar_HK.GetRecordToBuffer() failed\n"), RPT_ERROR);
				m_stkGgar_HK.MoveNext();

				if (_stricmp(pStockCode, (const char*)info.SEC_CD) != 0)
					continue;

				memcpy(pSendBufPos, &info.collect, sizeof(tagCommentaryCollect));
				pSendBufPos += sizeof(tagCommentaryCollect);
			}
		}
	}

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProAnalyserInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	int nRecordCount = 0; 
	string strSql = "SELECT * FROM ANA01_Info WHERE IS_VLD=1";

	if (!g_database.OpenTableUseSql(strSql.c_str()))
	{
		REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
		CheckDBCon();
	}
	nRecordCount = g_database.GetRecordCount();

	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) +
		sizeof(unsigned short) + sizeof(tagAnalyserInfo)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
		pSendBufPos += sizeof(unsigned short);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //记录个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息

		for(int i=0; i<nRecordCount; i++)
		{
			tagAnalyserInfo* pinfo = (tagAnalyserInfo*)pSendBufPos;
			pSendBufPos += sizeof(tagAnalyserInfo);

			g_database.GetCollect("ANA_Account", (LPCTSTR)pinfo->ANA_Account, 20);
			g_database.GetCollect("ANA_NM", (LPCTSTR)pinfo->ANA_NM, 20);
			g_database.GetCollect("Company", (LPCTSTR)pinfo->Company, 40);

			g_database.MoveNext();
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProAnalyserRemark(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
									 const char* pAccount, const char* pSec_CD, const char* pIdu_CD)
{
	int nRecordCount = 0; 
	string strSql = "SELECT * FROM ANA02_COMT WHERE IS_VLD=1";
	if (pAccount[0] != 0)
	{
		strSql += " AND ANA_Account='";
		strSql += pAccount;
		strSql += "'";
	}
	if (pSec_CD[0] != 0)
	{
		strSql += " AND SEC_CD='";
		strSql += pSec_CD;
		strSql += "'";
	}
	if (pIdu_CD[0] != 0)
	{
		strSql += " AND IDU_CD='";
		strSql += pIdu_CD;
		strSql += "'";
	}
	strSql += " ORDER BY UPDT_TM DESC";
	if (!g_database.OpenTableUseSql(strSql.c_str()))
	{
		REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
		CheckDBCon();
	}
	nRecordCount = g_database.GetRecordCount();
	if (nRecordCount > 100)	 //只需要最近100条记录
		nRecordCount = 100;

	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) +
		sizeof(unsigned short) + (sizeof(tagAnalyserRemark) + 1000) * nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = sizeof(tagPackHead) + sizeof(unsigned short) - 
			sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
		pSendBufPos += sizeof(unsigned short);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //记录个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		for(int i=0; i<nRecordCount; i++)
		{
			tagAnalyserRemark* pInfo = (tagAnalyserRemark*)pSendBufPos;
			pSendBufPos += sizeof(tagAnalyserRemark);

			g_database.GetCollect("ANA_Account", (LPCTSTR)pInfo->ANA_Account, 20);
			g_database.GetCollect("SEC_CD", (LPCTSTR)pInfo->SEC_CD, 8);
			g_database.GetCollect("IDU_CD", (LPCTSTR)pInfo->IDU_CD, 10);

			SYSTEMTIME tm_UPDT_TM;
			if (!g_database.GetCollect("UPDT_TM", &tm_UPDT_TM))
				pInfo->UPDT_TM[0] = 0;
			else
			{
				int nDate = tm_UPDT_TM.wYear*10000 + tm_UPDT_TM.wMonth*100 + 
					tm_UPDT_TM.wDay;
				sprintf_s((char*)pInfo->UPDT_TM, 9, "%d", nDate);
			}

			g_database.GetCollect("Comment", (LPCTSTR)pSendBufPos, 1000);
			int nLen = (int)strlen(pSendBufPos);
			pInfo->CommentLen = nLen + 1;
			pSendBufPos += pInfo->CommentLen;

			g_database.MoveNext();
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProAddRemark(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
								tagAddRemark* pInfo, const char* pComment)
{
	int nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) + 1;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);

	memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short)); //报文类型
	pSendBufPos += sizeof(unsigned short);

	//添加信息
	if(g_database.AddRemark(pInfo, pComment))
		memcpy(pSendBufPos, "1", 1);
	else
		memcpy(pSendBufPos, "0", 1);

	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}
void CDataProcess::ProHisFinanceIndex(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
									  const char* pStockCode)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	char szOrg_ID[20] = {0};
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		if (g_database.GetORG_ID(pStockCode, szOrg_ID))
		{
			strSql  = "SELECT * FROM CN_STK09_Fhind WHERE ORG_ID='";
			strSql += szOrg_ID;
			strSql += "' AND IS_VLD=1";
		}
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		if (g_database.GetORG_ID(pStockCode, szOrg_ID, FALSE))
		{
			strSql  = "SELECT * FROM HK_STK09_Fhind WHERE ORG_ID='";
			strSql += szOrg_ID;
			strSql += "' AND IS_VLD=1";
		}
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	if (!strSql.empty())
	{
		if (!g_database.OpenTableUseSql(strSql.c_str()))
		{
			REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
			CheckDBCon();
		}
		nRecordCount = g_database.GetRecordCount();
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + CODE_LEN +
		sizeof(unsigned short) + sizeof(tagHisFinanceIdx)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, pStockCode, CODE_LEN);   //股票代码
		pSendBufPos += CODE_LEN;

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		for(int i=0; i<nRecordCount; i++)
		{
			tagHisFinanceIdx* pInfo = (tagHisFinanceIdx*)pSendBufPos;

			g_database.GetCollect("IND_ID", (LPCTSTR)pInfo->IND_ID, 10);
			g_database.GetCollect("Rpt_Yr", (LPCTSTR)pInfo->Rpt_Yr, 4);
			g_database.GetCollect("Rpt_Prd", (LPCTSTR)pInfo->Rpt_Prd, 4);
			g_database.GetCollect("Ind_Value", pInfo->Ind_Value);

			g_database.MoveNext();
			pSendBufPos += sizeof(tagHisFinanceIdx);
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProIndexHisData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
								   const char* pCode, unsigned short nIdxMark)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		bMt_cn = TRUE;
		GetSqlString(TRUE, nIdxMark, strSql);
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		bMt_cn = FALSE;
		GetSqlString(FALSE, nIdxMark, strSql);
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	if (!strSql.empty())
	{
		strSql +=  pCode;
		strSql +=  "'";
		if (!g_database.OpenTableUseSql(strSql.c_str()))
		{
			REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
			CheckDBCon();
		}
		nRecordCount = g_database.GetRecordCount();
	}

	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + CODE_LEN +
		sizeof(unsigned short) + sizeof(unsigned short) + (8+20)*nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, pCode, CODE_LEN);   //股票代码
		pSendBufPos += CODE_LEN;

		memcpy(pSendBufPos, &nIdxMark, sizeof(unsigned short)); //指标标识
		pSendBufPos += sizeof(unsigned short);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		for(int i=0; i<nRecordCount; i++)
		{
			tagIdxHisData* pInfo = (tagIdxHisData*)pSendBufPos;

			SYSTEMTIME tm_UPDT_TM;
			string strField;
			if ((nIdxMark >= 1001 && nIdxMark <= 1016) ||
				(nIdxMark >= 2001 && nIdxMark <= 2037)  ||
				(nIdxMark >= 4001 && nIdxMark <= 4039)
				)
				strField = "PUB_DT";
			else
				strField = "UPDT_TM";

			if (g_database.GetCollect(strField, &tm_UPDT_TM))
			{
				int nDate = tm_UPDT_TM.wYear*10000 + tm_UPDT_TM.wMonth*100 + 
					tm_UPDT_TM.wDay;
				sprintf_s((char*)pInfo->Date, 9, "%d", nDate);
			}
			GetFieldValue((char*)pInfo->Value, nIdxMark);

			g_database.MoveNext();
			pSendBufPos += sizeof(tagIdxHisData);
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::GetSqlString(BOOL bMt_cn, unsigned short nIdxMark, string& strSql)
{
	if (nIdxMark >= 1001 && nIdxMark <= 1016)      //指数
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_MKT02_Qind WHERE IS_VLD=1 AND IDX_CD='";
		else
			strSql = "SELECT * FROM HK_MKT02_Qind WHERE IS_VLD=1 AND IDX_CD='";
	}
	else if (nIdxMark >= 1017 && nIdxMark <= 1020) //指数
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_MKT03_Fnind WHERE IS_VLD=1 AND IDX_CD='";
		else
			strSql = "SELECT * FROM HK_MKT03_Fnind WHERE IS_VLD=1 AND IDX_CD='";
	}
	else if (nIdxMark >= 1021 && nIdxMark <= 1030) //指数
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_MKT06_Factor WHERE IS_VLD=1 AND IDX_CD='";
		else
			strSql = "";
	}
	else if (nIdxMark >= 1031 && nIdxMark <= 1039) //指数
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_MKT07_Mix WHERE IS_VLD=1 AND IDX_CD='";
		else
			strSql = "";
	}
	else if (nIdxMark >= 2001 && nIdxMark <= 2037) //行业
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_IDU02_Qind WHERE IS_VLD=1 AND IDU_CD='";
		else
			strSql = "SELECT * FROM HK_IDU02_Qind WHERE IS_VLD=1 AND IDU_CD='";
	}
	else if (nIdxMark >= 2038 && nIdxMark <= 2086) //行业
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_IDU03_Fnind WHERE IS_VLD=1 AND IDU_CD='";
		else
			strSql = "SELECT * FROM HK_IDU03_Fnind WHERE IS_VLD=1 AND IDU_CD='";
	}
	else if (nIdxMark >= 2087 && nIdxMark <= 2099) //行业
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_IDU04_Anst WHERE IS_VLD=1 AND IDU_CD='";
		else
			strSql = "SELECT * FROM HK_IDU04_Anst WHERE IS_VLD=1 AND IDU_CD='";
	}
	else if (nIdxMark >= 4001 && nIdxMark <= 4039) //股票
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_STK02_Qind WHERE IS_VLD=1 AND SEC_CD='";
		else
			strSql = "SELECT * FROM HK_STK02_Qind WHERE IS_VLD=1 AND SEC_CD='";
	}
	else if (nIdxMark >= 4040 && nIdxMark <= 4094) //股票
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_STK03_Fnind WHERE IS_VLD=1 AND SEC_CD='";
		else
			strSql = "SELECT * FROM HK_STK03_Fnind WHERE IS_VLD=1 AND SEC_CD='";
	}
	else if (nIdxMark >= 4095 && nIdxMark <= 4113) //股票
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_STK04_Anst WHERE IS_VLD=1 AND SEC_CD='";
		else
			strSql = "SELECT * FROM HK_STK04_Anst WHERE IS_VLD=1 AND SEC_CD='";
	}
	else if (nIdxMark >= 4114 && nIdxMark <= 4122) //股票
	{
		if (bMt_cn)
			strSql = "SELECT * FROM CN_STK08_Ggar WHERE IS_VLD=1 AND SEC_CD='";
		else
			strSql = "SELECT * FROM HK_STK08_Ggar WHERE IS_VLD=1 AND SEC_CD='";
	}
}

//注意：港股字段名称和大陆不一样，要分别处理，目前还没有做处理
void CDataProcess::GetFieldValue(char* pValue, unsigned short nIdxMark)
{			
	double value = 0;
	switch(nIdxMark)
	{
	case INDEX_IDX_CM0201:
		{
			g_database.GetCollect("cm0201", pValue, 20);
			//g_database.GetCollect("cm0501", pValue, 20);
			break;
		}
	case INDEX_IDX_CM0202:
		{
			g_database.GetCollect("cm0202", pValue, 20);
			//g_database.GetCollect("cm0502", pValue, 20);
			break;
		}
	case INDEX_IDX_CM0203:
		{
			g_database.GetCollect("cm0203", pValue, 20);
			//g_database.GetCollect("cm0503", pValue, 20);
			break;
		}
	case INDEX_IDX_CM0211:
		{
			g_database.GetCollect("cm0211", value);
			//g_database.GetCollect("cm0511", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0212:
		{
			g_database.GetCollect("cm0212", value);
			//g_database.GetCollect("cm0512", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0213:
		{
			g_database.GetCollect("cm0213", value);
			//g_database.GetCollect("cm0513", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0214:
		{
			g_database.GetCollect("cm0214", value);
			//g_database.GetCollect("cm0514", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0215:
		{
			g_database.GetCollect("cm0215", value);
			//g_database.GetCollect("cm0515", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0216:
		{
			g_database.GetCollect("cm0216", value);
			//g_database.GetCollect("cm0516", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0217:
		{
			g_database.GetCollect("cm0217", value);
			//g_database.GetCollect("cm0517", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0221:
		{
			//g_database.GetCollect("cm0521", value);
			g_database.GetCollect("cm0221", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0222:
		{
			g_database.GetCollect("cm0222", value);
			//g_database.GetCollect("cm0522", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0223:
		{
			g_database.GetCollect("cm0223", value);
			//g_database.GetCollect("cm0523", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0224:
		{
			g_database.GetCollect("cm0224", value);
			//g_database.GetCollect("cm0524", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0225:
		{
			g_database.GetCollect("cm0225", value);
			//g_database.GetCollect("cm0525", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0226:
		{
			g_database.GetCollect("cm0226", value);
			//g_database.GetCollect("cm0526", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		/////////////////////////////////////////////////
	case INDEX_IDX_CM0301:
		{
			g_database.GetCollect("cm0301", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0302:
		{
			g_database.GetCollect("cm0302", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0303:
		{
			g_database.GetCollect("cm0303", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0304:
		{
			g_database.GetCollect("cm0304", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		//////////////////////////////////////////////////////////
	case INDEX_IDX_CM0601:
		{
			g_database.GetCollect("cm0601", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0602:
		{
			g_database.GetCollect("cm0602", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0603:
		{
			g_database.GetCollect("cm0603", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0604:
		{
			g_database.GetCollect("cm0604", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0605:
		{
			g_database.GetCollect("cm0605", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0606:
		{
			g_database.GetCollect("cm0606", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0607:
		{
			g_database.GetCollect("cm0607", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0608:
		{
			g_database.GetCollect("cm0608", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0609:
		{
			g_database.GetCollect("cm0609", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0610:
		{
			g_database.GetCollect("cm0610", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		/////////////////////////////////////////////////////////
	case INDEX_IDX_CM0701:
		{
			g_database.GetCollect("cm0701", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0702:
		{
			g_database.GetCollect("cm0702", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0703:
		{
			g_database.GetCollect("cm0703", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0704:
		{
			g_database.GetCollect("cm0704", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0705:
		{
			g_database.GetCollect("cm0705", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0706:
		{
			//g_database.GetCollect("cm0706", value); //是中文字符串
			value = 0;
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0707:
		{
			//g_database.GetCollect("cm0707", value);  //是中文字符串
			value = 0;
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0708:
		{
			//g_database.GetCollect("cm0708", value);	 //是中文字符串
			value = 0;
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDX_CM0709:
		{
			//g_database.GetCollect("cm0709", value);	//是中文字符串
			value = 0;
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		//////////////////////////////////////////////////////////
	case INDEX_IDU_CI0201:
		{
			g_database.GetCollect("ci0201", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0202:
		{
			g_database.GetCollect("ci0202", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0210:
		{
			g_database.GetCollect("ci0210", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0211:
		{
			g_database.GetCollect("ci0211", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0212:
		{
			g_database.GetCollect("ci0212", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0213:
		{
			g_database.GetCollect("ci0213", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0214:
		{
			g_database.GetCollect("ci0214", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0215:
		{
			g_database.GetCollect("ci0215", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0216:
		{
			g_database.GetCollect("ci0216", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0217:
		{
			g_database.GetCollect("ci0217", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0218:
		{
			g_database.GetCollect("ci0218", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0219:
		{
			g_database.GetCollect("ci0219", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0220:
		{
			g_database.GetCollect("ci0220", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0221:
		{
			g_database.GetCollect("ci0221", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0222:
		{
			g_database.GetCollect("ci0222", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0223:
		{
			g_database.GetCollect("ci0223", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0224:
		{
			g_database.GetCollect("ci0224", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0225:
		{
			g_database.GetCollect("ci0225", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0226:
		{
			g_database.GetCollect("ci0226", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0227:
		{
			g_database.GetCollect("ci0227", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0231:
		{
			g_database.GetCollect("ci0231", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0232:
		{
			g_database.GetCollect("ci0232", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0233:
		{
			g_database.GetCollect("ci0233", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0234:
		{
			g_database.GetCollect("ci0234", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0235:
		{
			g_database.GetCollect("ci0235", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0236:
		{
			g_database.GetCollect("ci0236", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0237:
		{
			g_database.GetCollect("ci0237", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0238:
		{
			g_database.GetCollect("ci0238", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0239:
		{
			g_database.GetCollect("ci0239", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0240:
		{
			g_database.GetCollect("ci0240", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0241:
		{
			g_database.GetCollect("ci0241", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0242:
		{
			g_database.GetCollect("ci0242", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0243:
		{
			g_database.GetCollect("ci0243", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0244:
		{
			g_database.GetCollect("ci0244", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0245:
		{
			g_database.GetCollect("ci0245", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0246:
		{
			g_database.GetCollect("ci0246", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0247:
		{
			g_database.GetCollect("ci0247", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		/////////////////////////////////////////////////////////
	case INDEX_IDU_CI0301:
		{
			g_database.GetCollect("ci0301", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0302:
		{
			g_database.GetCollect("ci0302", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0303:
		{
			g_database.GetCollect("ci0303", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0304:
		{
			g_database.GetCollect("ci0304", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0305:
		{
			g_database.GetCollect("ci0305", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0306:
		{
			g_database.GetCollect("ci0306", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0307:
		{
			g_database.GetCollect("ci0307", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0308:
		{
			g_database.GetCollect("ci0308", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0309:
		{
			g_database.GetCollect("ci0309", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0310:
		{
			g_database.GetCollect("ci0310", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0311:
		{
			g_database.GetCollect("ci0311", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0312:
		{
			g_database.GetCollect("ci0312", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0313:
		{
			g_database.GetCollect("ci0313", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0314:
		{
			g_database.GetCollect("ci0314", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0315:
		{
			g_database.GetCollect("ci0315", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0316:
		{
			g_database.GetCollect("ci0316", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0317:
		{
			g_database.GetCollect("ci0317", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0318:
		{
			g_database.GetCollect("ci0318", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0319:
		{
			g_database.GetCollect("ci0319", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0320:
		{
			g_database.GetCollect("ci0320", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0321:
		{
			g_database.GetCollect("ci0321", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0322:
		{
			g_database.GetCollect("ci0322", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0323:
		{
			g_database.GetCollect("ci0323", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0324:
		{
			g_database.GetCollect("ci0324", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0325:
		{
			g_database.GetCollect("ci0325", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0326:
		{
			g_database.GetCollect("ci0326", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0327:
		{
			g_database.GetCollect("ci0327", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0328:
		{
			g_database.GetCollect("ci0328", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0329:
		{
			g_database.GetCollect("ci0329", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0330:
		{
			g_database.GetCollect("ci0330", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0331:
		{
			g_database.GetCollect("ci0331", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0332:
		{
			g_database.GetCollect("ci0332", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0333:
		{
			g_database.GetCollect("ci0333", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0334:
		{
			g_database.GetCollect("ci0334", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0335:
		{
			g_database.GetCollect("ci0335", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0336:
		{
			g_database.GetCollect("ci0336", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0337:
		{
			g_database.GetCollect("ci0337", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0338:
		{
			g_database.GetCollect("ci0338", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0339:
		{
			g_database.GetCollect("ci0339", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0340:
		{
			g_database.GetCollect("ci0340", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0341:
		{
			g_database.GetCollect("ci0341", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0342:
		{
			g_database.GetCollect("ci0342", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0343:
		{
			g_database.GetCollect("ci0343", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0344:
		{
			g_database.GetCollect("ci0344", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0345:
		{
			g_database.GetCollect("ci0345", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0346:
		{
			g_database.GetCollect("ci0346", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0347:
		{
			g_database.GetCollect("ci0347", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0348:
		{
			g_database.GetCollect("ci0348", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0349:
		{
			g_database.GetCollect("ci0349", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		/////////////////////////////////////////////////////////
	case INDEX_IDU_CI0401:
		{
			g_database.GetCollect("ci0401", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0402:
		{
			g_database.GetCollect("ci0402", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0403:
		{
			g_database.GetCollect("ci0403", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0404:
		{
			g_database.GetCollect("ci0404", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0405:
		{
			g_database.GetCollect("ci0405", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0406:
		{
			g_database.GetCollect("ci0406", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0407:
		{
			g_database.GetCollect("ci0407", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0408:
		{
			g_database.GetCollect("ci0408", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0409:
		{
			g_database.GetCollect("ci0409", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0410:
		{
			g_database.GetCollect("ci0410", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0411:
		{
			g_database.GetCollect("ci0411", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0412:
		{
			g_database.GetCollect("ci0412", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_IDU_CI0413:
		{
			g_database.GetCollect("ci0413", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		/////////////////////////////////////////////////////////	
	case INDEX_STK_CS0201:
		{
			g_database.GetCollect("cs0201", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0202:
		{
			g_database.GetCollect("cs0202", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0203:
		{
			g_database.GetCollect("cs0203", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0209:
		{
			g_database.GetCollect("cs0209", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0210:
		{
			g_database.GetCollect("cs0210", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0211:
		{
			g_database.GetCollect("cs0211", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0212:
		{
			g_database.GetCollect("cs0212", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0213:
		{
			g_database.GetCollect("cs0213", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0214:
		{
			g_database.GetCollect("cs0214", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0215:
		{
			g_database.GetCollect("cs0215", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0216:
		{
			g_database.GetCollect("cs0216", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0217:
		{
			g_database.GetCollect("cs0217", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0218:
		{
			g_database.GetCollect("cs0218", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0219:
		{
			g_database.GetCollect("cs0219", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0220:
		{
			g_database.GetCollect("cs0220", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0221:
		{
			g_database.GetCollect("cs0221", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0222:
		{
			g_database.GetCollect("cs0222", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0223:
		{
			g_database.GetCollect("cs0223", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0224:
		{
			g_database.GetCollect("cs0224", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0225:
		{
			g_database.GetCollect("cs0225", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0226:
		{
			g_database.GetCollect("cs0226", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0227:
		{
			g_database.GetCollect("cs0227", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0231:
		{
			g_database.GetCollect("cs0231", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0232:
		{
			g_database.GetCollect("cs0232", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0233:
		{
			g_database.GetCollect("cs0233", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0234:
		{
			g_database.GetCollect("cs0234", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0235:
		{
			g_database.GetCollect("cs0235", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0236:
		{
			g_database.GetCollect("cs0236", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0237:
		{
			g_database.GetCollect("cs0237", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0238:
		{
			g_database.GetCollect("cs0238", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0239:
		{
			g_database.GetCollect("cs0239", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0240:
		{
			g_database.GetCollect("cs0240", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0241:
		{
			g_database.GetCollect("cs0241", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0242:
		{
			g_database.GetCollect("cs0242", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0243:
		{
			g_database.GetCollect("cs0243", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0244:
		{
			g_database.GetCollect("cs0244", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0245:
		{
			g_database.GetCollect("cs0245", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0246:
		{
			g_database.GetCollect("cs0246", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0247:
		{
			g_database.GetCollect("cs0247", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		////////////////////////////////////////////////
	case INDEX_STK_CS0301:
		{
			g_database.GetCollect("cs0301", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0302:
		{
			g_database.GetCollect("cs0302", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0303:
		{
			g_database.GetCollect("cs0303", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0311:
		{
			g_database.GetCollect("cs0311", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0312:
		{
			g_database.GetCollect("cs0312", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0313:
		{
			g_database.GetCollect("cs0313", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0314:
		{
			g_database.GetCollect("cs0314", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0315:
		{
			g_database.GetCollect("cs0315", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0316:
		{
			g_database.GetCollect("cs0316", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0317:
		{
			g_database.GetCollect("cs0317", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0318:
		{
			g_database.GetCollect("cs0318", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0319:
		{
			g_database.GetCollect("cs0319", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0320:
		{
			g_database.GetCollect("cs0320", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0321:
		{
			g_database.GetCollect("cs0321", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0322:
		{
			g_database.GetCollect("cs0322", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0323:
		{
			g_database.GetCollect("cs0323", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0324:
		{
			g_database.GetCollect("cs0324", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0325:
		{
			g_database.GetCollect("cs0325", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0326:
		{
			g_database.GetCollect("cs0326", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0327:
		{
			g_database.GetCollect("cs0327", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0328:
		{
			g_database.GetCollect("cs0328", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0329:
		{
			g_database.GetCollect("cs0329", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0330:
		{
			g_database.GetCollect("cs0330", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0331:
		{
			g_database.GetCollect("cs0331", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0332:
		{
			g_database.GetCollect("cs0332", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0333:
		{
			g_database.GetCollect("cs0333", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0334:
		{
			g_database.GetCollect("cs0334", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0335:
		{
			g_database.GetCollect("cs0335", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0336:
		{
			g_database.GetCollect("cs0336", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0337:
		{
			g_database.GetCollect("cs0337", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0338:
		{
			g_database.GetCollect("cs0338", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0339:
		{
			g_database.GetCollect("cs0339", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0340:
		{
			g_database.GetCollect("cs0340", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0341:
		{
			g_database.GetCollect("cs0341", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0342:
		{
			g_database.GetCollect("cs0342", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0343:
		{
			g_database.GetCollect("cs0343", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0344:
		{
			g_database.GetCollect("cs0344", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0345:
		{
			g_database.GetCollect("cs0345", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0346:
		{
			g_database.GetCollect("cs0346", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0347:
		{
			g_database.GetCollect("cs0347", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0348:
		{
			g_database.GetCollect("cs0348", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0349:
		{
			g_database.GetCollect("cs0349", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0350:
		{
			g_database.GetCollect("cs0350", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0351:
		{
			g_database.GetCollect("cs0351", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0352:
		{
			g_database.GetCollect("cs0352", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0353:
		{
			g_database.GetCollect("cs0353", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0354:
		{
			g_database.GetCollect("cs0354", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0355:
		{
			g_database.GetCollect("cs0355", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0356:
		{
			g_database.GetCollect("cs0356", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0357:
		{
			g_database.GetCollect("cs0357", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0358:
		{
			g_database.GetCollect("cs0358", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0359:
		{
			g_database.GetCollect("cs0359", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0360:
		{
			g_database.GetCollect("cs0360", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0361:
		{
			g_database.GetCollect("cs0361", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0362:
		{
			g_database.GetCollect("cs0362", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		//////////////////////////////////////////////////////
	case INDEX_STK_CS0401:
		{
			g_database.GetCollect("cs0401", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0402:
		{
			g_database.GetCollect("cs0402", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0403:
		{
			g_database.GetCollect("cs0403", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0404:
		{
			g_database.GetCollect("cs0404", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0405:
		{
			g_database.GetCollect("cs0405", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0406:
		{
			g_database.GetCollect("cs0406", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0407:
		{
			g_database.GetCollect("cs0407", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0408:
		{
			g_database.GetCollect("cs0408", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0409:
		{
			g_database.GetCollect("cs0409", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0411:
		{
			g_database.GetCollect("cs0411", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0412:
		{
			g_database.GetCollect("cs0412", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0413:
		{
			g_database.GetCollect("cs0413", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0414:
		{
			g_database.GetCollect("cs0414", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0415:
		{
			g_database.GetCollect("cs0415", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0416:
		{
			g_database.GetCollect("cs0416", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0417:
		{
			g_database.GetCollect("cs0417", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0418:
		{
			g_database.GetCollect("cs0418", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0419:
		{
			g_database.GetCollect("cs0419", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_CS0420:
		{
			g_database.GetCollect("cs0420", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
		///////////////////////////////////////////////
	case INDEX_STK_AVG_VALUE:
		{
			g_database.GetCollect("Avg_Value", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_TBU_NUM:
		{
			g_database.GetCollect("Tbu_Num", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_BUY_NUM:
		{
			g_database.GetCollect("Buy_Num", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_NEU_NUM:
		{
			g_database.GetCollect("Neu_Num", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_RED_NUM:
		{
			g_database.GetCollect("Red_Num", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_SEL_NUM:
		{
			g_database.GetCollect("Sel_Num", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_AVG_PRICE:
		{
			g_database.GetCollect("Avg_Price", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_HIG_PRICE:
		{
			g_database.GetCollect("Hig_Price", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	case INDEX_STK_LOW_PRICE:
		{
			g_database.GetCollect("Low_Price", value);
			sprintf_s(pValue, 20, "%04f", value);
			break;
		}
	}
}

void CDataProcess::ProIduRepresentData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	nRecordCount = 16;
	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) +
		sizeof(unsigned short) + sizeof(tagIduRepresentData) * nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //记录个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		for(int i=0; i<nRecordCount; i++)
		{
			tagIduRepresentData* pInfo = (tagIduRepresentData*)pSendBufPos;

			pInfo->nFactorMark = 1000 + i + 1;
			pInfo->field1 = 1;
			pInfo->field2 = 2;
			pInfo->field3 = 3;
			pInfo->field4 = 4;
			pInfo->field5 = 5;
			//g_database.MoveNext();
			pSendBufPos += sizeof(tagIduRepresentData);
		}
	}

	//g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProIduSortData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
								  unsigned short nFactorMark)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		bMt_cn = TRUE;
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	nRecordCount = 10;
	int nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) +
		sizeof(unsigned short) + sizeof(unsigned short) + 
		sizeof(tagIduSortData) * nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
		pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);
	}

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);

		memcpy(pSendBufPos, &nFactorMark, sizeof(unsigned short)); //因素标识
		pSendBufPos += sizeof(unsigned short);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //记录个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		for(int i=0; i<nRecordCount; i++)
		{
			tagIduSortData* pInfo = (tagIduSortData*)pSendBufPos;
			memset(pInfo, 0, sizeof(tagIduSortData));
			switch(i)
			{
			case 0:
				memcpy(pInfo->Idu_CD, "1010", strlen("1010"));
				break;
			case 1:
				memcpy(pInfo->Idu_CD, "101010", strlen("101010"));
				break;
			case 2:
				memcpy(pInfo->Idu_CD, "101020", strlen("101020"));
				break;
			case 3:
				memcpy(pInfo->Idu_CD, "101030", strlen("101030"));
				break;
			case 4:
				memcpy(pInfo->Idu_CD, "1510", strlen("1510"));
				break;
			case 5:
				memcpy(pInfo->Idu_CD, "151010", strlen("151010"));
				break;
			case 6:
				memcpy(pInfo->Idu_CD, "151020", strlen("151020"));
				break;
			case 7:
				memcpy(pInfo->Idu_CD, "151030", strlen("151030"));
				break;
			case 8:
				memcpy(pInfo->Idu_CD, "151040", strlen("151040"));
				break;
			case 9:
				memcpy(pInfo->Idu_CD, "151050", strlen("151050"));
				break;
			}
			pInfo->field1  = 1;
			pInfo->field2  = 2;
			pInfo->field3  = 3;
			pInfo->field4  = 4;
			pInfo->field5  = 5;
			pInfo->field6  = 6;
			pInfo->field7  = 7;
			pInfo->field8  = 8;
			pInfo->field9  = 9;
			pInfo->field10 = 10;
			//g_database.MoveNext();
			pSendBufPos += sizeof(tagIduSortData);
		}
	}

	//g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProSelStkModInfo(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	if (!g_database.OpenTableUseSql("SELECT * FROM MOD01_Info WHERE IS_VLD=1"))
	{
		REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
		CheckDBCon();
	}
	int nRecordCount = g_database.GetRecordCount();

	int nSendBufSize = 0;
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
	else
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) +
		sizeof(unsigned short) + sizeof(tagSelStkModInfo) * nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
		pSendBufPos += sizeof(unsigned short);

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //模型个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		for(int i=0; i<nRecordCount; i++)
		{
			tagSelStkModInfo* pInfo = (tagSelStkModInfo*)pSendBufPos;

			g_database.GetCollect("MOD_CD", (LPCTSTR)pInfo->MOD_CD, 8);
			g_database.GetCollect("MOD_NM", (LPCTSTR)pInfo->MOD_NM, 100);

			g_database.MoveNext();
			pSendBufPos += sizeof(tagSelStkModInfo);
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::ProSelStkModData(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo,
									const char* pCode)
{
	string strSql = "SELECT * FROM MOD02_Stk WHERE IS_VLD=1 AND MOD_CD='";
	strSql += pCode;
	strSql += "'";

	if (!g_database.OpenTableUseSql(strSql.c_str()))
	{
		REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
		CheckDBCon();
	}
	int nRecordCount = g_database.GetRecordCount();

	int nSendBufSize = 0;
	const int MOD_CD_LEN = 8;
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
	else
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short) + MOD_CD_LEN +
		sizeof(unsigned short) + sizeof(tagSelStkModData) * nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
		pSendBufPos += sizeof(unsigned short);

		memcpy(pSendBufPos, pCode, MOD_CD_LEN);
		pSendBufPos += MOD_CD_LEN;

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //股票个数
		pSendBufPos += sizeof(unsigned short);

		//添加信息
		for(int i=0; i<nRecordCount; i++)
		{
			tagSelStkModData* pInfo = (tagSelStkModData*)pSendBufPos;

			g_database.GetCollect("SEC_CD", (LPCTSTR)pInfo->SEC_CD, 8);
			g_database.GetCollect("MKT_CL", (LPCTSTR)pInfo->MKT_CL, 2);

			g_database.MoveNext();
			pSendBufPos += sizeof(tagSelStkModData);
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::MktFactorIdx(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		bMt_cn = TRUE;
		strSql = "SELECT * FROM CN_MKT06_Factor WHERE IS_VLD=1";
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	if (!strSql.empty())
	{
		if (!g_database.OpenTableUseSql(strSql.c_str()))
		{
			REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
			CheckDBCon();
		}
		nRecordCount = g_database.GetRecordCount();
	}

	int nSendBufSize = 0;
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
	else
		nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + sizeof(unsigned short) + 
		sizeof(tagMktFactorIdx) * nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);
		//添加信息

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		for(int i=0; i<nRecordCount; i++)
		{
			tagMktFactorIdx* pMktFacIdx = (tagMktFactorIdx*)pSendBufPos;
			pSendBufPos += sizeof(tagMktFactorIdx);

			g_database.GetCollect("IDX_CD", (LPCTSTR)pMktFacIdx->IDX_CD, 8);
			g_database.GetCollect("Dt_Prd", (LPCTSTR)pMktFacIdx->Dt_prd, 5);
			
			g_database.GetCollect("cm0601", pMktFacIdx->cm0601);
			g_database.GetCollect("cm0602", pMktFacIdx->cm0602);
			g_database.GetCollect("cm0603", pMktFacIdx->cm0603);
			g_database.GetCollect("cm0604", pMktFacIdx->cm0604);
			g_database.GetCollect("cm0605", pMktFacIdx->cm0605);

			g_database.GetCollect("cm0606", pMktFacIdx->cm0606);
			g_database.GetCollect("cm0607", pMktFacIdx->cm0607);
			g_database.GetCollect("cm0608", pMktFacIdx->cm0608);
			g_database.GetCollect("cm0609", pMktFacIdx->cm0609);
			g_database.GetCollect("cm0610", pMktFacIdx->cm0610);

			g_database.MoveNext();
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}

void CDataProcess::MktMixIdx(SMsgID* pId, tagPackHead* pPackHead, tagComInfo* pComInfo)
{
	int nRecordCount = 0; 
	char szMarketType[10] = {0};
	memcpy(szMarketType, pComInfo->MarketType, 2);

	BOOL bMt_cn = FALSE;
	string strSql;
	if (_stricmp(szMarketType, MT_CN) == 0)      //"CN" 大陆市场
	{
		bMt_cn = TRUE;
		strSql = "SELECT * FROM CN_MKT07_Mix WHERE IS_VLD=1";
	}
	else if (_stricmp(szMarketType, MT_HK) == 0) //"HK"	香港市场
	{
		bMt_cn = FALSE;
	}
	else  //不能处理的市场
	{
		REPORT(MN, T("不能处理的市场[%s]\n", szMarketType), RPT_ERROR);
	}

	if (!strSql.empty())
	{
		if (!g_database.OpenTableUseSql(strSql.c_str()))
		{
			REPORT(MN, T("OpenTableUseSql() failed\n"), RPT_ERROR);
			CheckDBCon();
		}
		nRecordCount = g_database.GetRecordCount();
	}

	int nSendBufSize = 0;
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
		nSendBufSize = sizeof(tagPackHead) + sizeof(unsigned short);
	else
		nSendBufSize = sizeof(tagPackHead) + sizeof(tagComInfo) + sizeof(unsigned short) + 
		sizeof(tagMktMixIdx) * nRecordCount;
	pPackHead->comxHead.Length = nSendBufSize - sizeof(CommxHead);

	char* pSendBuf = new char[nSendBufSize];
	if (pSendBuf == NULL)
	{
		REPORT(MN, T("分配内存[pSendBuf]失败\n"), RPT_ERROR);
		g_database.CloseTable();
		return;
	}
	memset(pSendBuf, 0, nSendBufSize);
	char* pSendBufPos = pSendBuf;
	pPackHead->nReqCount = 1;
	memcpy(pSendBufPos, pPackHead, sizeof(tagPackHead));
	pSendBufPos += sizeof(tagPackHead);
	if (nRecordCount <= 0) //如果没有数据，则只发送包头和报文类型，共14字节
	{
		memcpy(pSendBufPos, &pComInfo->nPacktype, sizeof(unsigned short));
	}
	else
	{
		memcpy(pSendBufPos, pComInfo, sizeof(tagComInfo));
		pSendBufPos += sizeof(tagComInfo);
		//添加信息

		memcpy(pSendBufPos, &nRecordCount, sizeof(unsigned short)); //指标个数
		pSendBufPos += sizeof(unsigned short);

		for(int i=0; i<nRecordCount; i++)
		{
			tagMktMixIdx* pMktMixIdx = (tagMktMixIdx*)pSendBufPos;
			pSendBufPos += sizeof(tagMktMixIdx);

			g_database.GetCollect("IDX_CD", (LPCTSTR)pMktMixIdx->IDX_CD, 8);
			
			g_database.GetCollect("cm0701", pMktMixIdx->cm0701);
			g_database.GetCollect("cm0702", pMktMixIdx->cm0702);
			g_database.GetCollect("cm0703", pMktMixIdx->cm0703);
			g_database.GetCollect("cm0704", pMktMixIdx->cm0704);
			g_database.GetCollect("cm0705", pMktMixIdx->cm0705);

			g_database.GetCollect("cm0706", pMktMixIdx->cm0706, 50);
			g_database.GetCollect("cm0707", pMktMixIdx->cm0707, 50);
			g_database.GetCollect("cm0708", pMktMixIdx->cm0708, 50);
			g_database.GetCollect("cm0709", pMktMixIdx->cm0709, 50);

			g_database.MoveNext();
		}
	}

	g_database.CloseTable();
	pSendReply(pId, pSendBuf, nSendBufSize);

	if (pSendBuf != NULL)
		delete []pSendBuf;
}