//CheckThread.h
//
#ifndef _CHECKTHREAD_H_
#define _CHECKTHREAD_H_

#include "ThreadEx.h"
#include "DownHttpData.h"
#include "FTPTranData.h"
#include <list>
#include "AdoDB.h"
#include "farm.h"
#include <string>
#include <map>

#pragma pack(push)
#pragma pack(1)
struct tagHQYinZi
{
	char    szStockCode[8];
	UINT    nDate;
	double	f0020;      //还权因子
};
#pragma pack(pop)
typedef std::list<tagHQYinZi*> CYinZiList;
typedef std::list<tagKLineInfo*> CKLineList;

#define WM_THREADEND       WM_USER + 120
#define WM_SAVE_FAILED     WM_USER + 121
#define WM_LIST_MSG WM_USER + 1234

#define CMD_NULL		               0x00
#define CMD_CHECK			           0x01
#define CMD_RE_CHECK		           0x02
#define CMD_SAVE                       0x03 
#define CMD_CHECK_ALL_FILE             0x04
#define CMD_CHECK_FILE_SIZE            0x05
#define CMD_CHECK_ALL_USE_HTTP_DATA    0x06
#define CMD_CHECK_SINGLE_USE_HTTP_DATA 0x07
#define CMD_UNRAR_KFILE                0x08
#define CMD_UPLOAD_HQKFILE             0x09
#define CMD_DOWNLOAD_HQKFILE           0x10
#define CMD_RAR_CURKFILE               0x11
#define CMD_WEIGHT                     0x12
#define CMD_COMBINATION                0x13


struct tagStockDataInfo
{
	char      strSymbol[20];
	char      strName[30];
	RINDEX*   pidx1;
	RINDEX*   pidx2;		
};

struct tagTickInfo
{
	unsigned int nTickCount;
	TickUnit*    pTickUnit;
};

typedef std::map<string, tagStockDataInfo*>  CDataInfoMap;
typedef std::map<unsigned int, MinUnit> CMinUnitMap;
typedef std::map<unsigned int, tagTickInfo*> CTickUnitMap;
///////////////////////////////////////////////////////////////////////////////
class CCheckThread : public CThreadEx
{
public:
	CCheckThread();
	virtual ~CCheckThread();

	void SetCtrlWnd(HWND hListCtrlWnd, HWND hListBoxWnd, HWND hDialogWnd, HWND hListFile);
	void CheckSingleFile(CString strFilePath);
	void RecheckFile();
	void SaveDataToFile();
	void StartCheckAllFile(CString strDirectory, BOOL bCheckLastOne = FALSE);
	void StartCheckFileSize(CString strFilePath);
	
	//使用网上数据校对，并更新错误数据
	void StartCheckAllFileUseHttpData(CString strFileDirectory);
	void StartCheckSingeleFileUseHttpData();
	void StartRarUpLoadKFile(BOOL bAuto = TRUE);
	void StartUnRarCurKLineFile();

	static UINT GetDayofWeek(UINT nDate);
	static BOOL IsValidDate(unsigned int nDate);
	BOOL UploadHQKLine();
	BOOL DownLoadHQKLine();
	void StartUploadHQKLineFile();
	void StartDownloadHQKLineFile();
	void StopUploadandDownload();
	void StartWeightThread(BOOL bRealHQ);
	void StopWeightThread();
	void StartCombinationDatFile();
	//virtual void Stop();

	long cast(std::string *pStr, RINDEX *pRindex, bool bQuitIfNoKey = true);
	int OnElement(std::string *pSymbol, RINDEX *pIdx);
	void StopCombinationDatFile();

protected:
	CKLineFile    m_file;
	HWND          m_hListCtrlWnd;
	HWND          m_hListBoxWnd;
	HWND          m_hDialogWnd;
	HWND          m_hListFileWnd;
	CDownHttpData m_DownHttpData;
	
	CFile         m_QuoteFile1;
	CFile         m_QuoteFile2;
	CFile         m_MinkFile1;
	CFile         m_MinkFile2;
	CFile         m_TickFile1;
	CFile         m_TickFile2;

	tagStockDataInfo* m_pStockDataInfo;
	BOOL          m_bRunCombDatFile;

	virtual void Run();
private:
	CString       m_strFilePath; //文件的路径
	CString       m_strFileName; //文件名

	CString       m_strDirectory;
	unsigned int  m_nErrCount;
	unsigned int  m_nCheckedCount;
    UINT          m_nCmd;

	int           m_nCheckedFile;
	int           m_nErrFile;

	BOOL          m_bCheckLastOne;   //是否只检测最近一条记录
	BOOL          m_bCheckFileSize;  //是否检测文件大小
	
	CString       m_strCheckSizeFilePath; //检测文件大小目录
	CFTPTranData* m_pFtp;

	BOOL          m_bRealHQ; //实时还权
	BOOL          m_bStartWeight;
	CYinZiList    m_lstYinZi;
	CKLineFile    m_KFileCur;
	CKLineFile    m_KFileHis;
	string        m_strCurKLine;  //文件夹名称
	string        m_strHisKLine;
	CSQLServerDB   m_sqlDB;
	CString       m_strCurKFilePath;
	CString       m_strHisKFilePath;

	BOOL ProcessRecord(tagKLineInfo *pKLInfo, BOOL bShowRecord, UINT nFileType);
	void AddLog(int nIndex, BOOL bListInfo, LPCTSTR pFormat, ...);
	void RecheckListCtrlFile();
	void CheckSingleFile(BOOL bShowRecord = TRUE);
	void CheckAllFile(CString strFile);
	void SaveFile();
	void CheckFileSize(CString strFile);
	void CutFile(CString strFile);
	void CheckAllFileUseHttpData(CString strFile);
	void CheckSingleFileUseHttpData();
	void CheckAllWeekFile(CString strPath);
	void CheckWeekFile(CString strFile, BOOL bCheckLastOne);
	BOOL GetWeekData(tagKLineInfo *pklInfo, CString strFile);
	void CheckAllMonFile(CString strPath);
	void CheckMonFile(CString strFile, BOOL bCheckLastOne);
	BOOL GetMonData(tagKLineInfo *pklInfo, CString strFile);
// 	BOOL RarCurKLineFile();
// 	BOOL UnRarCurKLineFile();
	BOOL GetRarSoftWarePath(CString& strPath);
	void KillRarProcess();
	void StartWeightThread();
	void AddMsg(UINT nPackType, LPCSTR pFormat, ...);
	BOOL BackupKLine();
	BOOL QueryWeight();
	BOOL GetWekMonLine(CString strDayLine);
	BOOL GetSingleWekMonLine(CString strDayFile);
	void UpdateDayLineToWeight();
	void UpdateAllFile(CString strFile);
	void UpdateSingleFile(CString strFile, CString strFileName);
	BOOL IsSameWeek(unsigned int uiNow, unsigned int uiPrev);
	void StartWeight(tagHQYinZi* pWeight, BOOL bCurKLine = TRUE);
	void CombinationDatFile();
	void ReleaseMemory();
	void OpenDataFile();
	void CloseDataFile();
	void GetQuote(RINDEX *pRindex1, RINDEX *pRindex2, Quote** pQuote);
	void GetMink(RINDEX *pRindex1, RINDEX *pRindex2, RINDEX *pRindex);
	void GetTick(RINDEX *pRindex1, RINDEX *pRindex2, RINDEX *pRindex);
	BOOL ReadFile1();
	BOOL ReadFile2();

public:
	int         m_nQuoteFile1;
	int         m_nMinFile1;
	int         m_nTickFile1;
	
	UINT        m_nMinK;
	UINT        m_nTick;
	UINT        m_nSymbol;

	CDataFarm*   m_pdataFarm;

	IndexHead   m_indexHead1;
	IndexHead   m_indexHead2;

	CString     m_strPath1;
	CString     m_strPath2;
	CString     m_strPath3;

	UINT        m_nMarketCount1;
	UINT        m_nMarketCount2;
	UINT        m_nMarketCount3;

	CDataInfoMap  m_mapDataInfo;

	BOOL        m_bIsDataFile1;
};

////////////////////////////////////////////////////////////////////////////////
#endif