//UseLogData.cpp
#include "stdafx.h"
#include "UseLogData.h"
#include <assert.h>
#include "config.h"
//////////////////////////////////////////////////////////////////////
CUseLogData::CUseLogData(CFeedOwner *pServer)
{
	m_pServer = pServer;
	m_pWorkThread = new CWorkThread(_WorkThread, this);
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_bRun = TRUE;
	m_bPrevTime = 0;
}

CUseLogData::~CUseLogData()
{
	delete m_pWorkThread;
	CloseHandle(m_hEvent);
}

BOOL CUseLogData::Init()
{
	assert(m_pWorkThread != NULL);
	//预分配内存
	if (!CreateMemBlock(64, 10))
	{
		REPORT(MN, T("CreateMemBlock(64, 10) failed!\n"), RPT_ERROR);
		return FALSE;
	}
	if (!CreateMemBlock(1024, 20))
	{
		REPORT(MN, T("CreateMemBlock(1024, 20) failed!\n"), RPT_ERROR);
		return FALSE;
	}
	if (!CreateMemBlock(1024*100, 40))
	{
		REPORT(MN, T("CreateMemBlock(1024*100, 40) failed!\n"), RPT_ERROR);
		return FALSE;
	}
	if (!CreateMemBlock(1024*200, 40))
	{
		REPORT(MN, T("CreateMemBlock(1024*200, 40) failed!\n"), RPT_ERROR);
		return FALSE;
	}
	if (!CreateMemBlock(1024*500, 10))
	{
		REPORT(MN, T("CreateMemBlock(1024*500, 10) failed!\n"), RPT_ERROR);
		return FALSE;
	}
	//
	return m_pWorkThread->Start();
}

void CUseLogData::Release()
{
	assert(m_pWorkThread != NULL);
	m_bRun = FALSE;
	SetEvent(m_hEvent);
	m_pWorkThread->Stop();
}

unsigned int CUseLogData::_WorkThread(void* pParam)
{
	CUseLogData* pUseLogData = (CUseLogData*)pParam;
	pUseLogData->WorkThread();
	return 0xDEADDEAD;
}

void CUseLogData::WorkThread()
{
	m_bRun = TRUE;
	
	if (g_cfg.m_bSingleFile)
		REPORT(MN, T("循环类型：单个文件循环\n"), RPT_INFO);
	else
		REPORT(MN, T("循环类型：整个目录下的文件循环 开始日期：%d 结束日期：%d\n",
		g_cfg.m_nStartDate, g_cfg.m_nEndDate), RPT_INFO);

	while(m_bRun)
	{
		std::string strExecutePath = GetExecutePath();
		strExecutePath = strExecutePath.substr(0, strExecutePath.rfind("\\") + 1);
		std::string strRecLog = strExecutePath;
		strRecLog += g_cfg.m_strRecordPath;
		if (g_cfg.m_bSingleFile)
		{
			strRecLog += "\\";
			strRecLog += g_cfg.m_strFileName;
			ReadSingleFile(strRecLog);
		}
		else
		{
			ListAllFile(strRecLog);
		}

		WaitForSingleObject(m_hEvent, 3000);
	}
}

void CUseLogData::ListAllFile(string strPath)
{
	char szFile[MAX_PATH];
	WIN32_FIND_DATA FindFileData;

	strcpy(szFile, strPath.c_str());
	strcat(szFile, "\\*.log");

	HANDLE hFind = ::FindFirstFile(szFile, &FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)    
		return;

	while(TRUE)
	{
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(FindFileData.cFileName[0]!='.')
			{
				strcpy(szFile, strPath.c_str());
				strcat(szFile, "\\");
				strcat(szFile, FindFileData.cFileName);
				ListAllFile(szFile);
			}
		}
		else
		{
			std::string strFileName = strPath + "\\";
			strFileName += FindFileData.cFileName;
			char szFileName[100];
			strcpy_s(szFileName, 100, FindFileData.cFileName);
			szFileName[11] = 0;
			char* pDate = szFileName + 3;
			unsigned int nDate = atoi(pDate);
			if ( (nDate >= g_cfg.m_nStartDate && nDate <= g_cfg.m_nEndDate) ||
				g_cfg.m_nStartDate == 0 ||
				 g_cfg.m_nEndDate  == 0	)
				ReadSingleFile(strFileName);
		}

		if(!FindNextFile(hFind, &FindFileData))   
			break;
	}

	FindClose(hFind);
}

void CUseLogData::ReadSingleFile(string strFilePath)
{
	REPORT(MN, T("正在发送%s...\n", strFilePath.c_str()), RPT_INFO);

	HANDLE hFile = CreateFile(strFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE
		, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL)
		return;

	m_bPrevTime = 0;
	while(m_bRun)
	{
		unsigned int nTime = 0;
		DWORD dwReaded = 0;
		BOOL bRet = ReadFile(hFile, &nTime, sizeof(unsigned int), &dwReaded, NULL);
		if (!bRet || dwReaded != sizeof(unsigned int))
		{
			if (bRet && dwReaded == 0)
				REPORT(MN, T("ReadFile end\n"), RPT_INFO);
			else
				REPORT(MN, T("ReadFile() failed bRet:%d dwReaded:%d\n", bRet, dwReaded), RPT_WARNING);
			CloseHandle(hFile);
			return;
		}
		//判断是否是原始数据文件
		if (nTime < 0 || nTime >= 24*60*60)
		{
			REPORT(MN, T("不是有效的原始数据"), RPT_WARNING);
			CloseHandle(hFile);
			return;
		}

		//4个字节的保留位
		unsigned long nReverse = 0;
		bRet = ReadFile(hFile, &nReverse, sizeof(unsigned long), &dwReaded, NULL);
		if (!bRet || dwReaded != sizeof(unsigned long))
		{
			REPORT(MN, T("ReadFile() failed bRet:%d dwReaded:%d\n", bRet, dwReaded), RPT_WARNING);
			CloseHandle(hFile);
			return;
		}

		CommxHead head = {0};
		bRet = ReadFile(hFile, &head, sizeof(CommxHead), &dwReaded, NULL);
		if (!bRet || dwReaded != sizeof(CommxHead))
		{
			REPORT(MN, T("ReadFile() failed bRet:%d dwReaded:%d\n", bRet, dwReaded), RPT_WARNING);
			CloseHandle(hFile);
			return;
		}

		int nBufSize = sizeof(unsigned long) + sizeof(CommxHead) + head.Length;
		char* pData = (char*)mpnew(nBufSize);
		if (pData == NULL)
		{
			REPORT(MN, T("mpnew(%d) failed", nBufSize), RPT_WARNING);
			CloseHandle(hFile);
			return;
		}
		char* pDataPos = pData;
		memcpy(pDataPos, &nReverse, sizeof(unsigned long));
		pDataPos +=	sizeof(unsigned long);

		memcpy(pDataPos, &head, sizeof(CommxHead));
		pDataPos +=	sizeof(CommxHead);

		bRet = ReadFile(hFile, pDataPos, head.Length, &dwReaded, NULL);
		if (!bRet || dwReaded != head.Length)
		{
			REPORT(MN, T("ReadFile() failed bRet:%d dwReaded:%d head.Length:%d\n", bRet,
				dwReaded, head.Length), RPT_WARNING);
			CloseHandle(hFile);
			return;
		}

		if (m_bPrevTime != 0)
		{
			double timespan = 0;
			if (g_cfg.m_nInterval != 0)
				timespan = ((nTime-m_bPrevTime)/(double)g_cfg.m_nInterval)*1000;
			WaitForSingleObject(m_hEvent, (DWORD)timespan);
		}
		m_bPrevTime = nTime;


		nBufSize -=	sizeof(unsigned long);
		m_pServer->Quote(pData, nBufSize, NULL);
		REPORT(MN, T("Send data size:%d\n", nBufSize), RPT_INFO);
	}
	CloseHandle(hFile);
}










