//CltConEx.cpp
//
#include "stdafx.h"
#include "SocketServer.h"
#include "KLine_Exe.h"
#include "KLineFile.h"
#include "ClientConn.h"

#pragma pack(1)
struct tagPackHeader
{
	unsigned int nPackSize;
	unsigned int nDateCount;
};								      

struct tagClosePriceInfo
{
	unsigned int nDate;           //日期
	char szStockCode[15];		  //股票代码格式如：000001.sz
	unsigned int  nClosePrice;	  //收盘价格
};

static unsigned short	reverse_s(unsigned short	usData)
{
	return	(usData>>8)+((usData & 0xff)<<8);
}

static unsigned int	reverse_i(unsigned int	uiData)
{
	unsigned char	aucData[4];

	int i;
	for( i=0;i<4;i++)
		aucData[i]=uiData>>((3-i)*8);
	uiData=0;

	for(i=0;i<4;i++)
		uiData+=(aucData[i]<<(i*8));
	return	uiData;
}

#define BUF_SIZE       200
#define RECVBUF_SIZE   1024*100
#define RECORD_LEN     146

struct SMHeader
{
	unsigned int nSize;		//网络字节顺序
	unsigned int nReserve;
	unsigned short HostToNet(unsigned char*	pucData)
	{
		//转换成网络字节顺序
		unsigned short	usSize=0;
		((unsigned	int*)(pucData+usSize))[0]=reverse_i(nSize);
		usSize+=sizeof(unsigned	int);

		((unsigned	int*)(pucData+usSize))[0]=reverse_i(nReserve);
		usSize+=sizeof(unsigned	int);

		return usSize;
	}
};
//供客户端使用
struct	SMCodeInfo{
	unsigned	short	m_usMarketType;  //市场标识
	char				m_acCode[6];     //股票代码或者拼音简称
	char				m_acName[12];    //名称
	char				m_ggpj[12];      //个股评级
	
	unsigned	short	HostToNet(unsigned char*	pucData)
	{
		//转换成网络字节顺序
		unsigned short	usSize=0;
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketType);
		usSize+=sizeof(unsigned	short);
		memset(pucData+usSize, 0, 6);
		usSize+=6;
		memset(pucData+usSize, 0, 12);
		usSize+=12;
		memset(pucData+usSize, 0, 12);
		usSize+=12;
		return	usSize;
	}
};

//客户端请求数据结构
struct	SMRequestData{
	unsigned	short	m_usType;//请求类型
	unsigned	short	m_usMarketType;//市场标识
	unsigned	short	m_usIndex;//请求索引
	unsigned	short	m_usMobileType;//手机型号
	char				m_acRight[32];//权限描述符
	//请求证券总数,即结构CodeInfo的个数,在请求K线和成交明细数据时则表示K线和成交明细的个数
	unsigned	short	m_usSize;
	SMCodeInfo*		m_pstCode;//在请求K线和成交明细时将请求起始位置值赋给它
	
	unsigned	short	HostToNet(unsigned char*	pucData)
	{
		//转换成网络字节顺序
		unsigned short	usSize=0;
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usType);
		usSize+=sizeof(unsigned	short);
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketType);
		usSize+=sizeof(unsigned	short);
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usIndex);
		usSize+=sizeof(unsigned	short);
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMobileType);
		usSize+=sizeof(unsigned	short);
		memset(pucData+usSize, 0, 32);
		usSize+=32;
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usSize);
		usSize+=sizeof(unsigned	short);
		usSize+=4;
		
		return	usSize;
	}
};

struct _STAT
{
	unsigned short mkt_code;
	unsigned int symbol_code;
	char symbol_name[12];
	unsigned int close;
	unsigned int open;
	unsigned int max_price;
	unsigned int min_price;
	unsigned int avg_price;
	unsigned int volume;
	unsigned int sum;
	unsigned int buy_vol;
	unsigned int sell_vol;
	unsigned int vol_ratio;

	unsigned int deal_price;
	unsigned int deal_vol;

	unsigned int buy_price1;
	unsigned int buy_vol1;
	unsigned int buy_price2;
	unsigned int buy_vol2;
	unsigned int buy_price3;
	unsigned int buy_vol3;
	unsigned int buy_price4;
	unsigned int buy_vol4;
	unsigned int buy_price5;
	unsigned int buy_vol5;

	unsigned int sell_price1;
	unsigned int sell_vol1;
	unsigned int sell_price2;
	unsigned int sell_vol2;
	unsigned int sell_price3;
	unsigned int sell_vol3;
	unsigned int sell_price4;
	unsigned int sell_vol4;
	unsigned int sell_price5;
	unsigned int sell_vol5;
};

const	unsigned char	SH_Index		=01;
const	unsigned char	SH_Bond			=02;
const	unsigned char	SH_Fund			=03;
const	unsigned char	SH_AStock		=04;
const	unsigned char	SH_BStock		=05;
const	unsigned char	SH_Other		=06;

const	unsigned char	SZ_Index		=11;
const	unsigned char	SZ_Bond			=12;
const	unsigned char	SZ_Fund			=13;
const	unsigned char	SZ_AStock		=14;
const	unsigned char	SZ_BStock		=15;
const	unsigned char	SZ_Other		=16;

const	unsigned short	ST_Index		=0x01;//指数
const	unsigned short	ST_Bond			=0x02;//债券
const	unsigned short	ST_Fund			=0x03;//基金
const	unsigned short	ST_AStock		=0x04;//A股
const	unsigned short	ST_BStock		=0x05;//B股
const	unsigned short	ST_Other		=0x06;//其它

#define SEND_BUF_SIZE  1024*1024    
////////////////////////////////////////////////////////////////////////////
CCltConEx::CCltConEx()
{
	m_pSendBuf = new char[SEND_BUF_SIZE];
	m_nSendBufSize = SEND_BUF_SIZE;
	
	m_nFileCount = 0;
	m_nPackSize = 0;
	m_pSendBufPos = NULL;
	m_nRecordCount = 0;
}

CCltConEx::CCltConEx(SOCKET hCltSock, SOCKADDR_IN *pAddrClt)
:CClientConnect(hCltSock, pAddrClt)
{
	m_pSendBuf = new char[SEND_BUF_SIZE];
	m_nSendBufSize = SEND_BUF_SIZE;
	
	m_nFileCount = 0;
	m_nPackSize = 0;
	m_pSendBufPos = NULL;
	m_nRecordCount = 0;
}

CCltConEx::~CCltConEx()
{
	if (m_pSendBuf != NULL)
		delete []m_pSendBuf;
}

void CCltConEx::ParseData(const char *pData, int nLen)
{
	tagPackHeader packHdr = {0};
	memcpy(&packHdr, pData, sizeof(tagPackHeader));

	int nCorrectSize = packHdr.nPackSize;
	int nRecvSize = nLen - sizeof(tagPackHeader);
	if (nCorrectSize != nRecvSize)
	{
		REPORT(MN, T("数据包大小不正确, 正确大小:%d 接收大小:%d\n", nCorrectSize, nRecvSize), RPT_ERROR);
		return;
	}

	char* pDataPos = (char*)pData;
	pDataPos += sizeof(tagPackHeader);
	
	m_mapDate.clear();
	for(unsigned int i=0; i<packHdr.nDateCount; i++)
	{
		UINT nDate = 0;
		memcpy(&nDate, pDataPos, sizeof(UINT));
		pDataPos += sizeof(UINT);
		m_mapDate.insert(DatePari(nDate, 0));
	}

	CString strDayLinePath = theApp.m_strHQDayFilePath;
	m_nFileCount = 0;
	GetFileCount(strDayLinePath);
	int nNeedSize = m_nFileCount*packHdr.nDateCount*sizeof(tagClosePriceInfo) + sizeof(tagPackHeader);
	REPORT(MN, T("需要发送缓冲区大小[%d]\n", nNeedSize), RPT_INFO);
	if (nNeedSize > m_nSendBufSize)
	{
		if (m_pSendBuf != NULL)
			delete[]m_pSendBuf;
		m_pSendBuf = new char[nNeedSize];
		REPORT(MN, T("重新创建发送缓冲区大小\n", nNeedSize), RPT_INFO);
	}
	m_nSendBufSize = nNeedSize;

	m_nPackSize   = 0;
	m_pSendBufPos = m_pSendBuf;
	m_pSendBufPos += sizeof(tagPackHeader);	 //留出文件头空间
	m_nPackSize += sizeof(tagPackHeader);
	m_nRecordCount = 0;

	SetMapSecondValue(0);      //初始化日期访问量为0   
	
	FindAllFile(strDayLinePath); //查找本地日线文件
	FillTodayData();             //如果要今天的行情，就必须到行情服务器上获得
	
	tagPackHeader sendPackHdr = {0};
	sendPackHdr.nPackSize = m_nPackSize - sizeof(tagPackHeader);
	sendPackHdr.nDateCount  = m_nRecordCount;
	memcpy(m_pSendBuf, &sendPackHdr, sizeof(tagPackHeader));

	REPORT(MN, T("SendPackSize:[%d] RecordCount:[%d]\n", m_nPackSize, m_nRecordCount), RPT_INFO);
	if (!SendData(m_pSendBuf, m_nPackSize))
	{
		REPORT(MN, T("SendData() failed\n"), RPT_ERROR);
	}
}

void CCltConEx::FindAllFile(CString strPath)
{
	if (strPath.IsEmpty())
		return;

	CString strFolder = strPath;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			FindAllFile(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			CString strFileName = ffind.GetFilePath();   //只支持旧的文件格式
			CString strExe = strFileName.Right(3);
			if(strExe.CompareNoCase("day") == 0)
				FillDataToPack(strFileName);
		}
	}
}

void CCltConEx::GetFileCount(CString strPath)
{
	if (strPath.IsEmpty())
		return;

	CString strFolder = strPath;
	if(strFolder.Right(1) != "\\")
		strFolder += "\\";

	strFolder += "*.*";
	CFileFind   ffind;

	BOOL bRet = ffind.FindFile(strFolder);
	while(bRet)
	{
		bRet = ffind.FindNextFile();
		if (ffind.IsDirectory() && !ffind.IsDots()) //文件夹
		{
			CString strPath = ffind.GetFilePath(); //得到路径
			GetFileCount(strPath);
		}
		else if (!ffind.IsDirectory() && !ffind.IsDots())
		{
			m_nFileCount++;
		}
	}
}

void CCltConEx::FillDataToPack(CString strFileName)
{
	CKLineFile kfile;
	if (!kfile.Open(strFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING))
	{
		REPORT(MN, T("Open file [%s] failed\n", strFileName), RPT_ERROR);
		return;
	}

	UINT nDate = 0;
	UINT nClosePrice = 0;
	tagClosePriceInfo closePriceInfo = {0};
	UINT nStockType = 0;

	while(!kfile.IsEOF())
	{
		kfile.GetFieldValue(0, nDate);
		if (FindDate(nDate))
		{
			kfile.GetFieldValue(4, nClosePrice);
			
			CString strNewFileName = ChangeFileNameFormat(strFileName, nStockType);
			closePriceInfo.nDate = nDate;
			closePriceInfo.nClosePrice = nClosePrice;
			strcpy_s(closePriceInfo.szStockCode, 15, strNewFileName);

			memcpy(m_pSendBufPos, &closePriceInfo, sizeof(tagClosePriceInfo));
			m_pSendBufPos += sizeof(tagClosePriceInfo);
			m_nPackSize += sizeof(tagClosePriceInfo);
			m_nRecordCount++;
		}
		kfile.MoveNext();
	}
	kfile.Close();
}

void CCltConEx::FillTodayData()
{
	tagClosePriceInfo closePriceInfo = {0};
	//本地没有找到的日期，如果是当天就到行情服务器上获取数据
	std::map<UINT, UINT>::iterator pos = m_mapDate.begin();
	BOOL bToday = FALSE;
	while(pos != m_mapDate.end())
	{
		if (pos->second == 0 && IsToday(pos->first))  //到服务器查找， 只能查找今天的
		{
			closePriceInfo.nDate = pos->first;
			bToday = TRUE;
			break;
		}
		pos++;
	}
	if (!bToday)
		return;

	CClientConn downData;
	BOOL bRet = downData.Connect(theApp.m_strHQServIP.GetBuffer(), theApp.m_nHQServPort);
	theApp.m_strHQServIP.ReleaseBuffer();
	
	if (!bRet)
	{
		REPORT(MN, T("连接服务器[%s:%d]失败\n", theApp.m_strHQServIP, theApp.m_nHQServPort), RPT_ERROR);
		return;
	}

	SMCodeInfo coseInfo = {0};
	SMRequestData requestData = {0};
	requestData.m_usType = 0x1600;      //所有股票行情报价协议
	requestData.m_usMarketType = 4100;  //

	char sendBuf[BUF_SIZE];
	int nPackSize = 0;
	strcpy_s(sendBuf, BUF_SIZE, "\r\n\r\n");
	nPackSize += 4;

	SMHeader hdr = {0};
	hdr.nSize = sizeof(SMRequestData)+sizeof(SMCodeInfo);
	nPackSize += hdr.HostToNet((unsigned char*)(sendBuf+nPackSize));
	nPackSize += requestData.HostToNet((unsigned char*)(sendBuf+nPackSize));
	nPackSize += coseInfo.HostToNet((unsigned char*)(sendBuf+nPackSize));

	bRet = downData.SendPacket(sendBuf, nPackSize);
	if (!bRet)
	{
		REPORT(MN, T("发送数据失败\n"), RPT_ERROR);
		return;
	}
	//
	char recvBuf[RECVBUF_SIZE];
	unsigned short nRecCount = 0;
	int nRecvCount = downData.RecvPacket(recvBuf, RECVBUF_SIZE); 
	if (nRecvCount == -1 || nRecvCount == 0)
		return;

	char* pRecvBufPos = strstr(recvBuf, "\r\n\r\n");
	if (pRecvBufPos == NULL)
	{
		REPORT(MN, T("在接收数据中没有找到\\r\\n\\r\\n\n"), RPT_WARNING);
		return;
	}
	pRecvBufPos += strlen("\r\n\r\n");
	SMHeader* psmHdr = (SMHeader*)pRecvBufPos;
	unsigned int nStructSize = reverse_i(psmHdr->nSize);
	REPORT(MN, T("数据包大小[%d]\n", nStructSize), RPT_INFO);

	pRecvBufPos += sizeof(SMHeader);
	pRecvBufPos += sizeof(unsigned char) + sizeof(unsigned short)*2;  // c2H
	memcpy(&nRecCount, pRecvBufPos, sizeof(unsigned short));
	nRecCount = reverse_s(nRecCount);
	pRecvBufPos += sizeof(unsigned short);  // H
	REPORT(MN, T("从行情服务器收到[%d]记录\n", nRecCount), RPT_INFO);
	if (nRecCount <= 0)
		return;
		
	//数据体
	unsigned short  nMarketType = 0;
	unsigned int    nStockCode  = 0;
	unsigned int    nClosePrice = 0;

	//判断接收缓冲区是否足够
	int nNeedSize = m_nPackSize + nRecCount*sizeof(tagClosePriceInfo);
	if (m_nSendBufSize < nNeedSize)
	{
		char* pNewBuf = new char[nNeedSize];
		memcpy(pNewBuf, m_pSendBuf, m_nPackSize);
		if (m_pSendBuf != NULL)
			delete []m_pSendBuf;

		m_pSendBuf = pNewBuf;
		m_pSendBufPos = m_pSendBuf + m_nPackSize;
		m_nSendBufSize = nNeedSize;
	}

	char szStat[RECORD_LEN];
	for(int i=0; i<nRecCount; i++)
	{
		_STAT* pStat = NULL;
		if (pRecvBufPos + sizeof(_STAT) > recvBuf + nRecvCount)   //如果不够一个结构体长度就复制一部分
		{
			int nLeaveSize = (int)(recvBuf + nRecvCount - pRecvBufPos);
			memcpy(szStat, pRecvBufPos, nLeaveSize);
			nRecvCount = downData.RecvPacket(recvBuf, RECVBUF_SIZE); 
			if (nRecvCount == -1 || nRecvCount == 0)
				return;

			pRecvBufPos = recvBuf;
			memcpy(szStat+nLeaveSize, pRecvBufPos, sizeof(_STAT)-nLeaveSize);
			pRecvBufPos += sizeof(_STAT)-nLeaveSize;
			pStat = (_STAT*)szStat;
		}
		else
		{
			pStat = (_STAT*)pRecvBufPos;
			pRecvBufPos += sizeof(_STAT);
		}
		
		nMarketType = reverse_s(pStat->mkt_code);
		nStockCode = reverse_i(pStat->symbol_code);
		switch(nMarketType)
		{
		case 4113: case 4114: case 4115: case 4116: case 4117: case 4118:  //深圳
			{
				if (nStockCode >= 399000) //指数
					sprintf_s(closePriceInfo.szStockCode, 15, "%06d.szidx", nStockCode);
				else
					sprintf_s(closePriceInfo.szStockCode, 15, "%06d.sz", nStockCode);
				break;
			}
		case 4097: case 4098: case 4099: case 4100: case 4101: case 4102:   //上海
			{
				if (nStockCode < 2000) //指数
					sprintf_s(closePriceInfo.szStockCode, 15, "%06d.shidx", nStockCode);
				else
					sprintf_s(closePriceInfo.szStockCode, 15, "%06d.sh", nStockCode);
				break;
			}
		default:
			{
				REPORT(MN, T("不可处理的市场代码[%d]\n", nMarketType), RPT_INFO);
				break;
			}
		}

		nClosePrice = reverse_i(pStat->deal_price);
        closePriceInfo.nClosePrice = nClosePrice;
		//DW("StockCode : %06d closePrice:%d", nStockCode, nClosePrice);
		memcpy(m_pSendBufPos, &closePriceInfo, sizeof(tagClosePriceInfo));
		m_pSendBufPos += sizeof(tagClosePriceInfo);
		m_nPackSize += sizeof(tagClosePriceInfo);
		m_nRecordCount++;
	} 
}

BOOL CCltConEx::FindDate(UINT nDate)
{
	std::map<UINT, UINT>::iterator pos = m_mapDate.begin();
	while(pos != m_mapDate.end())
	{
		if (pos->first == nDate)
		{
			pos->second = 1;
			return TRUE;
		}
		pos++;
	}

	return FALSE;
}

BOOL CCltConEx::IsToday(UINT nDate)
{
	CTime tmCur = CTime::GetCurrentTime();
	UINT nCurDate = tmCur.GetYear()*10000 + tmCur.GetMonth()*100 + tmCur.GetDay();
	if (nDate == nCurDate)
		return TRUE;
	else
		return FALSE;
}

void CCltConEx::SetMapSecondValue(UINT nValue)
{
	std::map<UINT, UINT>::iterator pos = m_mapDate.begin();
	while(pos != m_mapDate.end())
	{
		pos->second = 0;
		pos++;
	}
}

CString CCltConEx::ChangeFileNameFormat(CString strFileName, UINT& nStockType)
{
	CString strName = CValidFunction::GetFileNameFromFilePath(strFileName);

	CString strStockType = strName.Left(2);
	BYTE bStockType = atoi(strStockType);

	CString strStockCode = strName.Mid(2, 6);
	int nStockCode = atoi(strStockCode);

	//DW(strStockCode);
	CString strFileNewName = strStockCode;
	switch(bStockType)
	{
	case SH_Index:  case SH_Bond:   case SH_Fund: 
	case SH_AStock: case SH_BStock: case SH_Other:
		{
			if (nStockCode < 2000) //指数
				strFileNewName += ".shidx";
			else
				strFileNewName += ".sh";
			break;
		}

	case SZ_Index:  case SZ_Bond:   case SZ_Fund: 
	case SZ_AStock: case SZ_BStock: case SZ_Other:
		{
			if (nStockCode >= 399000) //指数
				strFileNewName += ".szidx";
			else
				strFileNewName += ".sz";
			break;
		}
	default:
		{
		}
	}

	return strFileNewName;
}

/*void CCltConEx::LogEvent(const char* pFormat, ...)
{
	char chMsg[MSG_BUF_LEN];

	va_list pArg;
	va_start(pArg, pFormat);
	vsprintf_s(chMsg, MSG_BUF_LEN, pFormat, pArg);
	va_end(pArg);

	theApp.LogEvent(RPT_DEBUG, chMsg);
}*/