#include "StdAfx.h"
#include <stdlib.h>
#include "RealData.h"
#include "qs_data_struct.h"
//#include "iomodel.h"
#include "farm.h"
#include "..\\..\\public\\data_struct.h"
#include "..\\plug.h"
#include "PlugManager.h"
#include "..\\..\\public\\commx\\synch.h"
#include "..\\..\\public\\commx\\report.h"


typedef int (*_SendReply)( SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);
typedef Quote * (*_GetSymbolQuote)(char * c_pSymbol);
typedef int (*_GetSymbolData)(char * c_pSymbol,char * c_pBuf,int c_iMaxLen);

typedef void (*_OnQuoteChanged)(const char *symbol, const int *notify); //4个int
typedef void (*_OnBusiness)(unsigned int c_ID ,char *c_pBuf);


#define MODULE_NAME "CS"
void PlugQuit(unsigned int c_unPlugID)
{
	g_pPlugs->OnPlugQuit(c_unPlugID);
}

Quote * GetSymbolQuote(char * c_pSymbol)
{
	Quote * pQuote;
	CQuoteData QuoteData(NULL);
	if (g_pDataFarm->m_hashIdx.cast(c_pSymbol,QuoteData,true) > 0)
	{
		//if (QuoteData.m_Idx.idxQuote > 0)
		//{
			pQuote = g_pDataFarm->GetQuote(QuoteData.m_Idx.idxQuote);
			return pQuote;
		//}
	}
	return NULL;
}

int SendReply(SMsgID * c_pMsgID,char * c_pBuf,int c_iLen)
{
	//cout<<"handle SendReply..." << c_iLen << endl;
	/*
	FILE * f;
	char acFile[256];
	sprintf(acFile,"log\\plus.%d.%d",c_pMsgID->m_unSocket,c_iLen);
	f=fopen(acFile,"wb");
	fwrite(c_pBuf,1,c_iLen,f);
	fclose(f);
	*/
	//if (g_pIo.FindClient(c_pMsgID->m_unID,c_pMsgID->m_unSocket))
	if (true)
	{
		//REPORT(MODULE_NAME,T("before SendReply[%d]\n",c_pMsgID->m_unSocket),RPT_INFO);
		PPER_IO_CONTEXT lpNode = g_pIo->GetIoFromLookaside();
		if (lpNode == NULL)
			return -1;
		lpNode->IoOperation = IoExtraSend;
		lpNode->m_nOverLen = 0;

		lpNode->m_timeCheck = 0;
		lpNode->m_timeLineTest = 0;
		lpNode->sClient = c_pMsgID->m_unSocket;
		lpNode->SocketType = SocketServer;
		int httplen = 0;
		if (c_pMsgID->m_unID > 0x10000000)
		{
			if (lpNode->m_nBufLen < ((unsigned int)c_iLen + HTTP_HEADER_SIZE))
			{
				if (lpNode->m_nBufLen > 0)
					free(lpNode->m_pBuf);
				lpNode->m_nBufLen = c_iLen + HTTP_HEADER_SIZE;
				lpNode->m_pBuf = (char *)malloc(c_iLen + HTTP_HEADER_SIZE);
			}
			char szHttp[8096];
			httplen=sprintf(szHttp, "HTTP/1.1 200 OK, Success\r\nContent-length: %d\r\ncontent-type: audio/mp3;charset=UTF-8\r\ncontent-source : licaiguanjia\r\n\r\n",c_iLen);
			memcpy(lpNode->m_pBuf,szHttp,httplen);
		}
		else
		{
			if (lpNode->m_nBufLen < (unsigned int)c_iLen)
			{
				if (lpNode->m_nBufLen > 0)
					free(lpNode->m_pBuf);
				lpNode->m_nBufLen = c_iLen;
				lpNode->m_pBuf = (char *)malloc(c_iLen);
			}
		}
		

		memcpy(lpNode->m_pBuf + httplen,c_pBuf,c_iLen);
		lpNode->wsaBuffer.buf = lpNode->m_pBuf;
		lpNode->wsaBuffer.len = c_iLen + httplen;
		lpNode->m_nOverLen = 0;
		lpNode->m_nRequireLen = c_iLen + httplen;		
		if (g_cfg.m_bDebug)
		REPORT(MODULE_NAME,T("SendReply [长度=%d][SOCKET=%d][ID=%d]\n",c_iLen + httplen,c_pMsgID->m_unSocket,lpNode->unId),RPT_INFO);
		g_pIo->ASYSendBuffer(lpNode);
	}
	else
		return -1;
	return 0;
}

int GetSymbolData(char * c_pReq,char * c_pBuf,int c_iMaxLen)
{
	SPlugReq * pReq;
	Quote * pQuote;
	int iRetLen;
	CRealData RealData;
	pReq = (SPlugReq *)c_pReq;

	switch(pReq->m_unType)
	{
	case REQ_QUOTE:
		if ((unsigned int)c_iMaxLen < (sizeof(unsigned int) + sizeof(Quote) * pReq->m_usCount))
			return -101;
		//可以取多个
		for(int k=0; k<pReq->m_usCount; k++)
		{
			pQuote = GetSymbolQuote(pReq->m_acSymbol + k * SYMBOL_LEN);
			if (pQuote == NULL)
			{
				//cout<<"fail:type= "<<pReq->m_unType<<" symbol=="<<(pReq->m_acSymbol + k * SYMBOL_LEN)<<endl;
				return -1;
			}
			memcpy(c_pBuf + sizeof(unsigned int) + sizeof(Quote) * k,(char *)pQuote,sizeof(Quote));
		}
		
		*((unsigned int *)c_pBuf) = pReq->m_usCount;
		iRetLen = sizeof(unsigned int) + sizeof(Quote) * pReq->m_usCount;
		break;
	case REQ_MINK:		
		iRetLen = RealData.GetMinKData(pReq->m_acSymbol,c_pBuf + sizeof(unsigned int),c_iMaxLen - sizeof(unsigned int));
		if (iRetLen <= 0)
			return -1;
		*((unsigned int *)c_pBuf) = iRetLen;
		iRetLen *= sizeof(MinUnit);
		iRetLen += sizeof(unsigned int);
		break;
	case REQ_TICK:
		iRetLen = RealData.GetTraceData(pReq->m_acSymbol,c_pBuf + sizeof(unsigned int),c_iMaxLen - sizeof(unsigned int));
		if (iRetLen <= 0)
			return -1;
		*((unsigned int *)c_pBuf) = iRetLen;
		iRetLen *= sizeof(TickUnit);
		iRetLen += sizeof(unsigned int);
		break;
		break;
	case REQ_HISK:
		iRetLen = RealData.GetHisKData(pReq->m_acSymbol,pReq->m_sHisKReq.m_unBeginDate,pReq->m_sHisKReq.m_unEndDate, pReq->m_sHisKReq.m_usType,pReq->m_sHisKReq.m_usRight,c_pBuf + sizeof(unsigned int),c_iMaxLen - sizeof(unsigned int));
		if (iRetLen <= 0)
			return -1;
		iRetLen *= sizeof(KLINE);
		iRetLen += sizeof(unsigned int);
		break;
	default:
		//cout<<"default symbol=="<<pReq->m_acSymbol<<endl;
		return -1;
	}
	return iRetLen;
}

void OnQuoteChanged(const char *symbol, const int *notify) //4个int
{
	//REPORT(MODULE_NAME,T("OnQuoteChanged %s\n",symbol),RPT_INFO);
	g_pIo->OnFarmUpdateData((char *)symbol);
	return;
}

//void OnBusiness(unsigned int,CommxHead *pHead)
void OnBusiness(unsigned int c_ID ,char *c_pBuf)
{
	//REPORT(MODULE_NAME,T("before OnBusiness %d\n",c_ID),RPT_DEBUG);
	if (c_ID > 0xF0000000)
	{
		g_IndexSummary.OnRep6001(c_ID,c_pBuf);
		return;
	}
	PPACKETHEAD pHead = (PPACKETHEAD)c_pBuf;

	if (pHead->m_usFuncNo == 6004)
	{
		//REPORT(MODULE_NAME,T("[%d][%s]\n",c_ID,c_pBuf+10),RPT_INFO);
		g_StockKindManager.OnRep6004(c_ID,c_pBuf);
		return;
	}
	unsigned int unID = c_ID;
	if (unID > 0x10000000)//http协议
	{
		unID -= 0x10000000;
	}
	g_pIo->m_rwLockConnect.WaitWrite();
	PPER_IO_CONTEXT pNode = g_pIo->FindIO(unID);
	if (pNode)
	{
		SMsgID MsgID;
		//PPACKETHEAD pHead = (PPACKETHEAD)c_pBuf;
		MsgID.m_unID = c_ID;
		MsgID.m_unSocket = pNode->sClient;
		g_pIo->m_rwLockConnect.EndWrite();
		if (g_cfg.m_bDebug)
		REPORT(MODULE_NAME,T("插件OnBusiness [COMMXHEAD:长度=%d,SEQ=%d][FUNCNO=%d][ID=%d][SOCKET=%d]\n",pHead->m_commxHead.GetLength() + 8,pHead->m_commxHead.GetUS(pHead->m_commxHead.SerialNo),pHead->m_commxHead.GetUS(pHead->m_usFuncNo),c_ID,MsgID.m_unSocket),RPT_DEBUG);
		SendReply(&MsgID,(char *)pHead,pHead->m_commxHead.GetLength() + 8);
		
	}
	else
		g_pIo->m_rwLockConnect.EndWrite();
	//REPORT(MODULE_NAME,T("OnBusiness over %d\n",c_ID),RPT_INFO);
}