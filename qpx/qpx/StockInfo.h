//StockInfo.h
#ifndef _STOCKINFO_H_
#define _STOCKINFO_H_
#include "socketx.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CStockInfo
{
public:
	CStockInfo();
	virtual~CStockInfo();

	void Answer(char *lpData, unsigned int nLength);
	void UpdateStockInfo();
	char* GetStockCode(char *pSpell, int& nStockCount);
	unsigned short GetMktType(char* pStockCode);

protected:
	
	void SendInitRequest();
	void GetCurDateTime(unsigned int& nCurDate, unsigned int& nCurTime);

private:
	LockSingle      m_lockStockInfo;        //���ʹ�Ʊ��Ϣ��
    char*           m_pStockInfo;           //��Ź�Ʊ��Ϣ�Ļ�����ָ��
	unsigned int    m_nBufsize;				//��������С
	unsigned int    m_nStockCount;			//��Ʊ����

	unsigned int    m_nUpdateDate;          //���¹�Ʊ��Ϣ���ڣ����쿪��ʱ����һ��
};

extern CStockInfo g_StockInfo;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif