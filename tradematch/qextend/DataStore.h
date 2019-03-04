#pragma once
#include<map>
#include<list>
//#include <hash_map> 
#include   <string> 

#include "../../public/data_struct.h"
#include "../../public/commx/synch.h"

#define CONSIGN_ADD			0
#define CONSIGN_CANCEL		1

#define SYMBOL_LEN			6

//ͳ����Ϣ
struct SumInfo
{
	int		m_nMaxConsign;//ί������
	int		m_nMaxBargain;//�ɽ�����
	int		m_nConsign;//ί������
	int		m_nBargain;//�ɽ�����
	LockProcess m_lock;	
	void	AddConsign(int c_nAdd)
	{
		m_lock.WaitWrite();
		m_nConsign += c_nAdd;
		m_lock.EndWrite();
	}
	void	AddBargain(int c_nAdd)
	{
		m_lock.WaitWrite();
		m_nBargain += c_nAdd;
		m_lock.EndWrite();
	}
	void	SetNew()
	{
		m_lock.WaitWrite();
		if (m_nConsign > m_nMaxConsign)
			m_nMaxConsign = m_nConsign;
		if (m_nBargain > m_nMaxBargain)
			m_nMaxBargain = m_nBargain;
		m_nBargain = 0;
		m_nConsign = 0;
		m_lock.EndWrite();
	}
};

//ί�нṹ
struct Consign
{
	int		m_nId;				//ί�е���
	char	m_acDate[30];		//ί��ʱ��
	int		m_nPrice;			//ί�м۸�
	int		m_nAmount;			//ί������
	int		m_nFlag;			//�����룬0���룬1����
	int		m_nType;			//ί�����,0ί�У�1����
	int		m_nCancelId;		//��������
	int		m_nUserId;			//�û�����
	char	m_acSymbol[SYMBOL_LEN+1];		//֤ȯ����

	char	m_acBgDate[30];		//bargainDate
	int		m_nStatus;			//��ǰ״̬
};
//���۵�Ԫ
struct PriceUnit
{
	int		m_nPrice;
	std::list<Consign *> m_listConsign;
};

class CConsignOfStock
{
public:
	CConsignOfStock(std::string c_strSymbol,int c_nMarket,RINDEX * c_pIdx);
	~CConsignOfStock(void);

public:
	void ResetQuote();
	//����һ������,0�ɹ�����,-1�۸񳬹���ͣ�۸�,-2�۸�͹���ͣ�۸�
	int AddConsignOld(Consign * c_pConsign);
	int AddConsign(Consign * c_pConsign);
	//����һ������,0�����ɹ�,-1ʧ��
	int CancelConsign(Consign * c_pConsign);
	int CheckPrice(unsigned int c_unPrice);//0����,-1���ڵ�ͣ��,1������ͣ��
	//�µĳɽ�����
	void TradeMatch();//������һ����һ���д��
	void OnNewTick(Quote *quote,TickUnit * tickunit);
	void OnDelete();//ɾ��
	int cast(std::string  * c_pKey,RINDEX * c_pValue,bool c_bFlag);
public:
	std::string m_strSymbol;	//֤ȯ����
	std::string m_strName;		//֤ȯ����
	int			m_nMarket;		//1�Ϻ�,2����
	RINDEX	  * m_pIdx;
	Quote	  * m_pQuote;		//
	bool		m_bCacl;		//������ͣ����ͣ�۸�
	unsigned int			m_unZtjg;		//��ͣ�۸�
	unsigned int			m_unDtjg;		//��ͣ�۸�
	unsigned int			m_unPrice;		//�ּ�
	unsigned int m_unZrsp;

	std::map<int,PriceUnit *> m_mapPriceUnitBuy;//ί�����
	std::map<int,PriceUnit *> m_mapPriceUnitSale;//ί������

	LockRW m_rwLock;
};

class CDataStore
{
public:
	CDataStore(void);
	~CDataStore(void);

public:
	void OnBegin();
	void AddSymbol(const char *symbol, int *pFlag);
	void UpdateSymbol(const char * symbol, int *pFlag, Quote *quote,TickUnit * tickunit);
	void DelSymbol(const char * symbol);
	void OnEnd();

	bool AddConsign(Consign * c_pConsign);//����һ��ί��
	void ResetQuote();
	void StockTradeMatch();
	void Clear();
public:
	LockProcess m_lock;	
	std::map<std::string ,CConsignOfStock *> m_mapStock;
};