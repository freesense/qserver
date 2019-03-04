//TianmaDataExport.h
#ifndef _TIANMADATAEXPORT_H_
#define _TIANMADATAEXPORT_H_

#ifdef _DLL
#define TMDAPI __declspec(dllexport)
#else
#define TMDAPI __declspec(dllimport)
#endif
///////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct SMsgID
{
	unsigned int m_unID;
	unsigned int m_unSocket;
};

//��������ṹ
#define SYMBOL_LEN     16
#define STOCK_CODE_LEN 16
#define STOCK_NAME_LEN 16

#define REQ_QUOTE   	1		//��������
#define REQ_MINK	    2		//��������
#define REQ_TICK		3		//�ֱʳɽ�
#define REQ_HISK		4		//��ʷK��

struct SMinKReq
{
	unsigned int m_unDate;
};

struct STickReq
{
	unsigned int m_unDate;
};

struct SHisKReq
{
	unsigned int   m_unBeginDate;
	unsigned int   m_unEndDate;
	unsigned short m_usRight;   //1��ǰ��Ȩ�����Ȩ
};

struct SPlugReq
{
	unsigned int	m_unType;
	char			m_acSymbol[SYMBOL_LEN];
	union
	{
		SMinKReq	m_sMinKReq;
		STickReq	m_sTickReq;
		SHisKReq	m_sHisKReq;
	};
};

//����ṹ
struct Quote
{
	char szStockCode[STOCK_CODE_LEN];	//Ʒ�ִ���
	char szStockName[STOCK_NAME_LEN];	//��Ʊ���
	unsigned int zrsp;					//����
	unsigned int jrkp;					//��
	unsigned int ztjg;					//��ͣ��
	unsigned int dtjg;					//��ͣ��
	unsigned int syl1;					//��ӯ��1
	unsigned int syl2;					//��ӯ��2
	unsigned int zgjg;					//��߼۸�
	unsigned int zdjg;					//��ͼ۸�
	unsigned int zjjg;					//����ɽ���
	unsigned int zjcj;					//����ɽ���
	unsigned int cjsl;					//�ܳɽ���
	unsigned int cjje;					//�ɽ����
	unsigned int cjbs;					//�ɽ�����
	unsigned int BP1;
	unsigned int BM1;
	unsigned int BP2;
	unsigned int BM2;
	unsigned int BP3;
	unsigned int BM3;
	unsigned int BP4;
	unsigned int BM4;
	unsigned int BP5;
	unsigned int BM5;
	unsigned int SP1;
	unsigned int SM1;
	unsigned int SP2;
	unsigned int SM2;
	unsigned int SP3;
	unsigned int SM3;
	unsigned int SP4;
	unsigned int SM4;
	unsigned int SP5;
	unsigned int SM5;
	unsigned int day5pjzs;				//5��ƽ������
	unsigned int pjjg;					//����
	unsigned int wb;					//ί��
	unsigned int lb;					//����
	unsigned int np;					//����
	unsigned int wp;					//����
};

//�ֱʳɽ��ṹ��Ԫ
struct TickUnit
{
	unsigned int Time;					//ʱ�䣬��ʽ��HHMM
	unsigned int Price;					//�ɽ���
	unsigned int Volume;				//�ɽ���
	unsigned char Way;					//�ɽ�����-δ֪��-���̣�-����
};

//�ڴ����K�߽ṹ��Ԫ
struct MinUnit
{
	unsigned int Time;					//ʱ�䣬��ʽ��HHMM
	unsigned int OpenPrice;
	unsigned int MaxPrice;
	unsigned int MinPrice;
	unsigned int NewPrice;
	unsigned int Volume;
};

//��ʷK�ߵ�Ԫ
struct	SHisKUnit
{
		unsigned	int		m_uiDate;
		unsigned	int		m_uiOpenPrice;
		unsigned	int		m_uiMaxPrice;
		unsigned	int		m_uiMinPrice;
		unsigned	int		m_uiClosePrice;
		unsigned	int		m_uiVolume;
		unsigned	int		m_uiSum;//�ɽ����
		//unsigned	int		m_uEx;//���̸�unsigned int
};

typedef int (*_SendReply)( SMsgID* c_pMsgID, char* c_pBuf, int c_iLen);
typedef Quote* (*_GetSymbolQuote)(char* c_pSymbol);
typedef int (*_GetSymbolData)(char* c_pReq, char* c_pBuf, int c_iMaxLen, int c_iDataType);
typedef void (*_PlugQuit)(unsigned int c_unPlugID);

/**********************************************************************
Function  : ��ʼ��
c_unPlugID:	ƽ̨������˲����ID��
return    : 0��ʾ�ɹ�������ʧ��
***********************************************************************/
extern "C" TMDAPI int _OnInit(unsigned int c_unPlugID,const char * c_pParam);

/**********************************************************************
Function  : �������
c_unPlugID:	
return    : 
***********************************************************************/
extern "C" TMDAPI void _OnRelease();

/**********************************************************************
Function  : ��Ӧ���ݸ��´���(Ҫ���첽��������ڵõ����ݺ���������)
c_pBuf    :	�������ݸ��µ���Ŀ
c_iLen    :	c_pBuf����Ч����
return    : 0��ʾ�ɹ�������ʧ��
***********************************************************************/
extern "C" TMDAPI int _OnUpdateData(char * c_pBuf,int c_iLen);

/**********************************************************************
Function  : ��Ӧ�ⲿ����(�첽������������)
c_pMsgID  :	�����ʶ�������Ҫ���ƴ˽ṹ���ݣ���������ָ�룬
            �ڴ�����ɺ���Ӧ��ʱ��Ҫ���������ͻ�
c_pBuf    :	��������
c_iLen    :	c_pBuf����Ч����
return    : 0��ʾ�ɹ�������ʧ��
***********************************************************************/
extern "C" TMDAPI int _OnRequest(SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);

/**********************************************************************
Function  : ����Ӧ��
c_pFunc   :	����ָ��
            typedef int (*_SendReply)( SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);
			c_pMsgID: �����ʶ����OnRequestʱ����������
            c_pBuf  : �������ݣ�ָ��һ���������ݵ�Ԫ
            c_iLen  : �������ݳ���
return    :
***********************************************************************/
extern "C" TMDAPI void _Set_SendReplay(_SendReply c_pFunc);

/**********************************************************************
Function  : ��ȡ��������
c_pFunc   :	����ָ��
            typedef Quote* (*_GetSymbolQuote)(char* c_pSymbol);
			c_pSymbol: ����Ĺ�Ʊ����
			return   : ����NULL��ʾȡʧ�ܣ�������һ����Ч��Quoteָ��
return    :
***********************************************************************/
extern "C" TMDAPI void _Set_GetSymbolQuote(_GetSymbolQuote c_pFunc);

/**********************************************************************
Function  : ��ȡ����
c_pFunc   :	����ָ��
            typedef int (*_GetSymbolData)(char* c_pReq, char* c_pBuf,
			int c_iMaxLen, int c_iDataType);
			c_pReq     : ��������ṹ�������������͡���Ʊ���뼰����Ĳ�����
			             �˽ṹ����ʵ��������в��䣬�ĵ����˵���˽ṹ
			c_pBuf     : �������ݻ���������ʽ����I[�����Ԫ]����I��ʾ�ж���
			             �������Ԫ����֪�����鵥Ԫ����ʱ���Ƶ�Ԫ����ʱ�ɽ�
					     ��Ԫ��K�ߵ�Ԫ��δ֪�ȴ�����
			c_iMaxLen  : c_pBuf����󳤶�
			c_iDataType: ��������
return     : 0��ʾ�ɹ�������ʧ��
***********************************************************************/
extern "C" TMDAPI void _Set_GetSymbolData(_GetSymbolData c_pFunc);

/**********************************************************************
Function  : ֪ͨ�˳�
c_pFunc   :	����ָ��
            typedef void (*_PlugQuit)(unsigned int c_unPlugID);
			c_unPlugID: ƽ̨������˲����ID��
return    :
***********************************************************************/
extern "C" TMDAPI void _Set_PlugQuit(_PlugQuit c_pFunc);

extern "C" TMDAPI void GetModuleVer(char** pBuf);
///////////////////////////////////////////////////////////////////////
#endif