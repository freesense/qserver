
#ifndef __QUOTE_DATA_STRUCT_H__
#define __QUOTE_DATA_STRUCT_H__

#define STOCK_CODE_LEN	16
#define STOCK_NAME_LEN	16
#define STOCK_KIND_LEN	16
#define STOCK_PYDM_LEN	6
#define TICK_PERBLOCK	120
#define MINK_PERBLOCK	60
#define QUOTEHEAD		(STOCK_CODE_LEN+STOCK_NAME_LEN)
#define QUOTETAIL		(STOCK_KIND_LEN+sizeof(unsigned int)*4)

/// ��������Դ���͹��������ݵ��б�־
#define  FD_ZRSP                 1
#define  FD_JRKP                 2
#define  FD_ZJCJ                 3
#define  FD_CJSL                 4
#define  FD_CJJE                 5
#define  FD_CJBS                 6
#define  FD_ZGCJ                 7
#define  FD_ZDCJ                 8
#define  FD_SYL1                 9
#define  FD_SYL2                10
#define  FD_JSD1                11
#define  FD_JSD2                12
#define  FD_HYCC                13
#define  FD_SJW5                14
#define  FD_SSL5                15
#define  FD_SJW4                16
#define  FD_SSL4                17
#define  FD_SJW3                18
#define  FD_SSL3                19
#define  FD_SJW2                20
#define  FD_SSL2                21
#define  FD_SJW1                22
#define  FD_SSL1                23
#define  FD_BJW1                24
#define  FD_BSL1                25
#define  FD_BJW2                26
#define  FD_BSL2                27
#define  FD_BJW3                28
#define  FD_BSL3                29
#define  FD_BJW4                30
#define  FD_BSL4                31
#define  FD_BJW5                32
#define  FD_BSL5                33
#define  FD_XXJYDW				34
#define  FD_XXMGMZ				35
#define  FD_XXZFXL				36
#define  FD_XXLTGS				37
#define  FD_XXSNLR				38
#define  FD_XXBNLR				39
#define  FD_XXJSFL				40
#define  FD_XXYHSL				41
#define  FD_XXGHFL				42
#define  FD_XXMBXL				43
#define  FD_XXBLDW				44
#define  FD_XXSLDW				45
#define  FD_XXJGDW				46
#define  FD_XXJHCS				47
#define  FD_XXLXCS				48
#define  FD_XXXJXZ				49
#define  FD_XXZTJG				50
#define  FD_XXDTJG				51
#define  FD_XXZHBL				52
#define  FD_AVERAGE				53

#pragma pack(1)

//�����ļ�ͷ�ṹ
struct IndexHead
{
	unsigned char Version;				//�汾��
	unsigned short nBucketCount;		//Ͱ������
	unsigned short nSymbolCount;		//symbol����
	unsigned int nTickCount;			//tick�ṹ����
	unsigned int nMinkCount;			//����k������

	unsigned int nQuoteIndex;			//����Quote����
	unsigned int nTickIndex;			//����tick����
	unsigned int nMinkIndex;			//����mink����

	unsigned int nIndexOffset;			//������ƫ��
	unsigned int nPyOffset;				//ƴ������ƫ��
};

//�г�����������
struct MarketStatus
{
	char szMarket[STOCK_CODE_LEN];		//�г����룬sz,sh,hk...
	unsigned int dateOpen;				//��������
	unsigned int dateClose;				//��������
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
	int wb;								//ί��
	unsigned int lb;					//����
	unsigned int np;					//����
	unsigned int wp;					//����
	int zdf;							//�ǵ���
	int zf;								//���

	char szStockKind[STOCK_KIND_LEN];	//Ʒ�����������md_szag,......
	unsigned int flag[4];				//����仯��־
};

//�ֱʳɽ��ṹ
struct TickUnit
{
	unsigned int Time;					//ʱ�䣬��ʽ��HHMM
	unsigned int Price;					//�ɽ���
	unsigned int Volume;				//�ɽ���
	unsigned char Way;					//�ɽ�����0-δ֪��1-���̣�2-����
};

struct TickBlock
{
	TickUnit unit[TICK_PERBLOCK];
	unsigned int next;
};

//�ڴ����K�߽ṹ
struct MinUnit
{
	unsigned int Time;					//ʱ�䣬��ʽ��HHMM
	unsigned int OpenPrice;
	unsigned int MaxPrice;
	unsigned int MinPrice;
	unsigned int NewPrice;
	unsigned int Volume;
	unsigned int AvgPrice;				//����
};

struct MinBlock
{
	MinUnit unit[MINK_PERBLOCK];
	unsigned int next;
};

struct RINDEX
{
	int idxQuote;				//��������
	int idxTick;				//�ֱʳɽ�����
	int cntTick;				//�ֱʳɽ�����
	int idxMinK;				//����K������
	int cntMinK;				//����K�߸���
};

/**@brief ���̽ṹ

����k����ʷ�ļ�����ר�ŵ�dll��������˶��屾�ṹ��Ϊdll��������֮�����ݴ��ݵĻ�����λ
 */
struct dayk
{
	char szStockCode[STOCK_CODE_LEN];	//Ʒ�ִ���
	unsigned int day;					/** �������� [2/25/2010 xinl] */
	unsigned int open;					//���̼۸�
	unsigned int high;					//��߼�
	unsigned int low;					//��ͼ�
	unsigned int close;					//�������̼�
	unsigned int volume;				//���ճɽ���
	unsigned int amount;				//���ճɽ����
	double scale;						/** ���ոùɻ�Ȩ���� [2/25/2010 xinl] */
};

#pragma pack()

#endif
