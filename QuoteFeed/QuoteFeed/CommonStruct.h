//CommonStruct.h
//
#pragma once
//////////////////////////////////////
#define  VERSION              0x03
#define  ENCODING             0x00
#define  BYTEORDER            0x00
#define  ROUTETARGET          0x00

#define  STOCK_TYPE_NORMAL    0x00
#define  STOCK_TYPE_NEW       0x01
#define  STOCK_TYPE_DELETE    0x02

//���������
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

//�Ϻ������
//ע�⣺û�� S12 S14 S20
#define  FD_S3                   1
#define  FD_S4                   2
#define  FD_S5                   3
#define  FD_S6                   4
#define  FD_S7                   5
#define  FD_S8                   6
#define  FD_S9                   7
#define  FD_S10                  8
#define  FD_S11                  9
#define  FD_S13                 10
#define  FD_S15                 11
#define  FD_S16                 12
#define  FD_S17                 13
#define  FD_S18                 14
#define  FD_S19                 15
#define  FD_S21                 16
#define  FD_S22                 17
#define  FD_S23                 18
#define  FD_S24                 19
#define  FD_S25                 20
#define  FD_S26                 21
#define  FD_S27                 22
#define  FD_S28                 23
#define  FD_S29                 24
#define  FD_S30                 25
#define  FD_S31                 26
#define  FD_S32                 27
#define  FD_S33                 28

//������Ϣ��
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

#define  PACK_HEADER_LEN         8
#define  SZ_MAX_FIELD            33
#define  SH_MAX_FIELD            28
#define  SZXX_MAX_FIELD          19
#define  STOCK_NAME_LEN          17
/*************************************************************************
1.����ͷ�̶�Ϊ8���ֽ�
2.���������㷨��Prop = ver << 4 | (encoding << 2) & 0x0e | byteorder & 0x01
  Ver��      Э��汾�ţ�Ŀǰ�ݶ�Ϊ0x03���޸�Э��ʱҪ��Ӧ���޸�����ֶΡ�
  Encoding�� �ַ������뷽ʽ��0-ANSI/GBK��1-UTF8��2-UTF16LE��
  Byteorder���ֽ���0-������1-������
  ����ͷ�ͱ������Ķ�������ѭencoding��byteorder����ĸ�ʽ��
3.·��Ŀ��Ŀǰ�ݶ�Ϊ0x00��
4.���ı�����������ã�����ƽ̨��Ӧ�������ø���ֵΪ���󷽵ı�š���0��ʼ����
5.���ĳ��Ȳ���������ͷ���ȡ�
*************************************************************************/
#pragma pack(push)
#pragma pack(1)
typedef struct tagPackHeader
{
	tagPackHeader()
	{
		byProp        = VERSION << 4 | (ENCODING << 2) & 0x0e | BYTEORDER & 0x01;
		byRouteTarget = ROUTETARGET;
		wPackNum      = 0; 
		ulPackSize    = 0;
	}

	unsigned char    byProp;          //��������
	unsigned char    byRouteTarget;   //·��Ŀ��
	unsigned short   wPackNum;		  //���ı��
	unsigned long    ulPackSize;      //���Ĵ�С������������ͷ
}PACKHEADER;
	  
struct tagPackInfo     //��ͨ��Ʊ�跢�͵���Ϣ
{
	unsigned short   wStockType;      //��Ʊ����(0x00��ͨ�� 0x01������ 0x02ɾ��)      
	unsigned short   wTime;			  //ʱ�䣨��0�㿪ʼ���㣬�Է�Ϊ��λ	��
	unsigned short   wFieldCount;	  //�������ݸ���,���ٸ��ֶ��跢��
	char             szMarketType[6]; //�г�����(sz sh)
	char             szStockCode[8];  //��Ʊ����
};

struct tagNewPackInfo  //������Ʊ�跢�͵���Ϣ
{
	tagPackInfo      packinfo;
	char             szStockName[STOCK_NAME_LEN]; //��Ʊ����
};

struct tagFieldInfo
{
	unsigned short   wFieldType;      //�ֶ����ͣ���˳���� 1-���� 2-��...
	unsigned int     nFieldValue;	  //�ֶ�ֵ
};
#pragma pack(pop)