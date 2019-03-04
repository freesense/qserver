//condef.h
#ifndef _CONDEF_H_
#define _CONDEF_H_
#include "../../public/protocol.h"

#pragma warning(disable:4819)
#pragma warning(disable:4267)
#pragma warning(disable:4311)
#pragma warning(disable:4312)
////////////////////////////////////////////////////////////////////////////////////////

//��������
#define PT_REAL_MINS         0x0004		//��ʱ����
#define PT_QUOTEPRICE        0x0003		//���۱�
#define PT_HKDATA_DAY		 0x0005		//����
#define PT_HKDATA_WEEK		 0x0006		//����
#define PT_HKDATA_MONTH		 0x0007		//����
#define PT_SORTDATA		     0x1770		//���а�
#define PT_INIT  		     0x0001		//��ʼ��

#define CODE_LEN             6
#define NAME_LEN             12
#define NEWREALMIN           0x0700     //�·�ʱЭ��
#define REALMINSDATA         0x0400 //����
#define REPORTDATA           0x0200     //��ѡ�ɱ���
#define STATDATA             0x0300     //����ͳ����Ϣ
#define HISKDATA             0x0600     //��ʷK������
#define SORTDATA             0x0800     //���а�
#define REALMINSTAT          0x0B00     //�·�ʱͳ��Э��
#define REALMINOVERLAP       0x0F00     //���ӷ�ʱЭ��
#define INITMARKET           0x1000     //�г���ʼ��


const	unsigned short	LISTCODE=0x0A00;//�����о�
const	unsigned short	HK_Stock		=0x2004;
const	unsigned int	MULTIPLE=10000;//���ݵķŴ���
const	float			ROUND=(float)0.00005;//����ȡֵʱ��������

const	unsigned char	CH_CHAR=0x40;//Ϊ�ַ��ֶ�
const	unsigned char	CH_PRICE=0x10;//Ϊ�۸��ֶ�
const	unsigned char	CH_VOLUME=0x20;//Ϊ�ɽ����ֶ�
const	unsigned char	CH_FLOAT=0x04;//ΪС���ֶ�
const	unsigned char	CH_TIME=0x08;//Ϊʱ���ֶ�
const	unsigned char	CH_BARGIN=0x02;//Ϊ�ɽ��ֶ�
const	unsigned char	CH_NONE=0;//���ֶβ���

const	unsigned short	DAY2REALMIN=0x0D00;//���շ�ʱ����
const	unsigned int	MAX_STRUCTSIZE=102400;//�ṹ����ռ�õ����ռ�
//���а����ඨ��
const	unsigned char	CODE_SORT=0;//����˳�����а�
const	unsigned char	RISE_SORT=1;//�Ƿ����а�
const	unsigned char	SUM_SORT=4;//�ɽ�������а�
const	unsigned char	VOLUME_SORT=6;//�ɽ������а�
const	unsigned char	SWING_SORT=2;//������а�
const	unsigned char	VOLRATIO_SORT=3;//�������а�
const	unsigned char	BUYSELL_SORT=5;//ί�����а�
const	unsigned char	PRICE_SORT=7;//�ɽ������а�
const	unsigned char	PRICEASSET_SORT=8;//�о������а�
const	unsigned char	VOLSHARE_SORT=9;//���������а�
const	unsigned char	PRICEYIELD_SORT=10;//��ӯ�����а�

const	unsigned char	DAY_HISK=61;//��K��
const	unsigned char	WEEK_HISK=62;//��K��
const	unsigned char	MONTH_HISK=63;//��K��

const	unsigned short	ST_Index		=0x01;//ָ��
const	unsigned short	ST_Bond			=0x02;//ծȯ
const	unsigned short	ST_Fund			=0x03;//����
const	unsigned short	ST_AStock		=0x04;//A��
const	unsigned short	ST_BStock		=0x05;//B��
const	unsigned short	ST_Other		=0x06;//����

const	unsigned short	SH_Index		=0x1001;
const	unsigned short	SH_Bond			=0x1002;
const	unsigned short	SH_Fund			=0x1003;
const	unsigned short	SH_AStock		=0x1004;
const	unsigned short	SH_BStock		=0x1005;
const	unsigned short	SH_Other		=0x1006;

const	unsigned short	SZ_Index		=0x1011;
const	unsigned short	SZ_Bond			=0x1012;
const	unsigned short	SZ_Fund			=0x1013;
const	unsigned short	SZ_AStock		=0x1014;
const	unsigned short	SZ_BStock		=0x1015;
const	unsigned short	SZ_Other		=0x1016;
////////////////////////////////////////////////////////////////////
#pragma pack(push)
#pragma pack(1)

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

struct SMHeader
{
	unsigned int nSize;		//�����ֽ�˳��
	unsigned int nReserve;
	unsigned short HostToNet(unsigned char*	pucData)
	{
		//ת���������ֽ�˳��
		unsigned short	usSize=0;
		((unsigned	int*)(pucData+usSize))[0]=reverse_i(nSize);
		usSize+=sizeof(unsigned	int);

		((unsigned	int*)(pucData+usSize))[0]=reverse_i(nReserve);
		usSize+=sizeof(unsigned	int);

		return usSize;
	}
};

//���ͻ���ʹ��
struct	SMCodeInfo{
	unsigned	short	m_usMarketType;//�г���ʶ
	char				m_acCode[6];//��Ʊ�������ƴ�����
	char				m_acName[12];//����
	char				m_ggpj[12];//��������

	void NetToHost()//�ѽṹ��������ת��������˳��
	{
		m_usMarketType = reverse_s(m_usMarketType);
	}

	unsigned	short	NetToHost(unsigned char*	pucData)
	{
		//ת���������ֽ�˳��
		unsigned short	usSize=0;
		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketType);
		usSize+=sizeof(unsigned	short);

		int i;
		for( i=0;i<CODE_LEN;i++)
			(pucData+usSize)[i]=m_acCode[i];

		usSize+=CODE_LEN;

		for(i=0;i<NAME_LEN;i++)
			(pucData+usSize)[i]=m_acName[i];

		usSize += NAME_LEN;
		for(i=0;i<sizeof(m_ggpj);i++)
			(pucData+usSize)[i]=m_ggpj[i];

		return	usSize+/*NAME_LEN+*/sizeof(m_ggpj);
	}

	unsigned	short	HostToNet(unsigned char*	pucData)
	{
		//ת���������ֽ�˳��
		unsigned short	usSize=0;
		//		unsigned char	ucLen=6;

		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketType);
		usSize+=sizeof(unsigned	short);

		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		//		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize += NAME_LEN;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		return	usSize+/*NAME_LEN+*/sizeof(m_ggpj);
	}
};

//�ͻ����������ݽṹ
struct	SMRequestData{
	unsigned	short	m_usType;//��������
	unsigned	short	m_usMarketType;//�г���ʶ
	unsigned	short	m_usIndex;//��������
	unsigned	short	m_usMobileType;//�ֻ��ͺ�
	char				m_acRight[32];//Ȩ��������
	//����֤ȯ����,���ṹCodeInfo�ĸ���,������K�ߺͳɽ���ϸ����ʱ���ʾK�ߺͳɽ���ϸ�ĸ���
	unsigned	short	m_usSize;
	SMCodeInfo*		m_pstCode;//������K�ߺͳɽ���ϸʱ��������ʼλ��ֵ������

	unsigned	short	NetToHost(unsigned char*	pucData)
	{
		//ת���������ֽ�˳��
		unsigned short	usSize=0;
		unsigned short* pusData=&m_usType;

		int i;
		for( i=0;i<4;i++)
		{
			((unsigned short*)pucData)[i]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		memcpy(pucData+usSize,m_acRight,32);
		usSize+=32;
		((unsigned short*)(pucData+usSize))[0]=reverse_s(m_usSize);
		usSize+=sizeof(unsigned	short);
/*
		//�������ʹ��m_pstCode�Ļ���Ī����������ƫ��2�ֽ�
		//�������ֱ�ӷ������󻺳����������ȷ��SMCodeInfo����
		char *pTemp = ((char*)&m_usSize) + sizeof(unsigned short);
//		((unsigned	int*)(pucData+usSize))[0]=reverse_i((unsigned int)&pTemp);
//		SMCodeInfo*	pstMCodeInfo=(SMCodeInfo*)(((char*)&m_pstCode)+sizeof(SMCodeInfo*));
		pTemp += sizeof(void*);
		SMCodeInfo*	pstMCodeInfo=(SMCodeInfo*)pTemp;
*/
		((unsigned	int*)(pucData+usSize))[0]=reverse_i((unsigned int)m_pstCode);
		SMCodeInfo*	pstMCodeInfo=(SMCodeInfo*)(((unsigned int)&m_pstCode)+sizeof(SMCodeInfo*));

		usSize=sizeof(SMRequestData);
		unsigned short	usUnits=reverse_s(m_usSize);

		if(0xffffffff != (unsigned int)m_pstCode)
			usUnits=1;

		for(i=0;i<usUnits;i++)
			usSize+=pstMCodeInfo[i].NetToHost(pucData+usSize);

		return	usSize;
	}

	unsigned	short	HostToNet(unsigned char*	pucData)
	{
		//ת���������ֽ�˳��
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

	void NetToHost()//�ѽṹ��������ת��������˳��
	{
		m_usType       = reverse_s(m_usType);
		m_usMarketType = reverse_s(m_usMarketType);
		m_usIndex      = reverse_s(m_usIndex);
		m_usMobileType = reverse_s(m_usMobileType);
		m_usSize       = reverse_s(m_usSize);
	}
};

//����֤ȯ�ɽ����ݣ����ڻ��۸��ߺͳɽ�����
struct	SRealMinsUnit{//�Է���Ϊ��λ����
	unsigned	short	m_usTime;//�����һ�ο���ʱ��
	unsigned	int		m_uiNewPrice;
	unsigned	int		m_uiVolume;
	unsigned	int		m_uiAverage;//ƽ���۸�
	/*unsigned	char	m_ucStructSize;//�ṹ����
	unsigned	int		m_usTime;//�����һ�ο���ʱ��
	unsigned	int		m_uiNewPrice;
	unsigned	int		m_uiUpMargin;//�ǵ�
	unsigned	int		m_uiVolume;
	unsigned	int		m_uiAverage;//ƽ���۸�
	unsigned	int		m_uiVolRatio;//����ֵ
	unsigned	int		m_uiSellVol;//ί������
	unsigned	int		m_uiBuyVol;//ί������
	unsigned	int		m_uiKeepVol;//�ֲ���,�������ڻ��г�
	*/
	unsigned	short	HostToNet(unsigned char*	pucData, unsigned short usType)
	{//ת���������ֽ�˳��
		if (usType == REALMINSDATA || usType == DAY2REALMIN)
		{			
			char* pBufPos = (char*)pucData;
			memcpy(pBufPos, &m_usTime, sizeof(unsigned	short));
			pBufPos += sizeof(unsigned	short);
			memcpy(pBufPos, &m_uiNewPrice, sizeof(unsigned	int));
			pBufPos += sizeof(unsigned	int);
			memcpy(pBufPos, &m_uiVolume, sizeof(unsigned	int));
			pBufPos += sizeof(unsigned	int);
			memcpy(pBufPos, &m_uiAverage, sizeof(unsigned	int));
			pBufPos += sizeof(unsigned	int);

			unsigned short	usSize = sizeof(unsigned	short) + sizeof(unsigned int)*3;
			return	usSize;
			/*pucData[0]=sizeof(SRealMinsUnit);
			unsigned short	usSize=sizeof(unsigned	char);
			unsigned int*	puiData=&m_usTime;
			// ???
			if(m_uiVolume < 0)
				m_uiVolume = 0;
			// ???
			
			int i;
			for( i=0;i<9;i++)
			{
				((unsigned	int*)(pucData+usSize))[0]=reverse_i(puiData[i]);
				usSize+=sizeof(unsigned	int);
			}
			return	usSize;*/
		}
		else if (usType == REALMINOVERLAP)
		{
			unsigned short usTime = (unsigned short)m_usTime;
			((unsigned short*)pucData)[0] = reverse_s(usTime);
			((unsigned int*)(pucData+2))[0] = reverse_i(m_uiNewPrice);
			return sizeof(unsigned int) + 2;
		}
		else
		{
			unsigned short usTime = (unsigned short)m_usTime;
			((unsigned short*)pucData)[0] = reverse_s(usTime);

			((unsigned int*)(pucData+2))[0] = reverse_i(m_uiNewPrice);
			((unsigned int*)(pucData+2))[1] = reverse_i(m_uiVolume);
			((unsigned int*)(pucData+2))[2] = reverse_i(m_uiAverage);
			return sizeof(unsigned int) * 3 + 2;
		}
	}
	static unsigned short	GetBufNeedSize(unsigned short usSize, unsigned short usType)
	{
		if (usType == NEWREALMIN || usType == DAY2REALMIN)
			return usSize * 37;  //37��SRealMinsUnit�Ĵ�С
		return 0;
	}
};

//���ӳɽ�����
struct	SRealMinsData{
	unsigned	char	m_ucStructSize;//�ṹ����
	unsigned	short	m_usType;//��Ӧ���ͣ����������ݰ�����һ��
	unsigned	short	m_usIndex;//�������������������ݰ�����һ��
	unsigned	short	m_usMarketType;//�г���ʶ
	char				m_acName[12];//����
	char				m_acCode[6];//����
	char				m_ggpj[12];
	unsigned	int		m_uiPrevClose;//���ռ�
	unsigned	short	m_usFirstOpen;//����ʱ��;
	unsigned	short	m_usFirstClose;//����ʱ��;
	unsigned	short	m_usSecondOpen;//����ʱ��;
	unsigned	short	m_usSecondClose;//����ʱ��;
	unsigned	short	m_usMinsUnits;//�ṹRealDealUnit�ĸ���
	unsigned	char	m_ucUnits;//��Աm_pcTitle��Ԫ�ظ���
	unsigned	char*	m_pcTitle;
	SRealMinsUnit*		m_pstMinsUnit;
	unsigned	short	HostToNet(unsigned char*	pucData, unsigned short usType)
	{//ת���������ֽ�˳��
		char*	pcTitle[]={"ʱ��","����","�ǵ�","��ֵ","����","","","","","",};
		unsigned char	aucDescribe[]={CH_TIME,CH_PRICE,CH_PRICE,CH_PRICE,CH_VOLUME,
			CH_PRICE,CH_FLOAT,CH_VOLUME,CH_VOLUME,CH_NONE};
		pucData[0]=sizeof(SRealMinsData);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;
		int i;
		for( i=0;i<3;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize+=NAME_LEN;
		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		usSize+=sizeof(m_ggpj);
		((unsigned	int*)(pucData+usSize))[0]=reverse_i(m_uiPrevClose);
		usSize+=sizeof(unsigned	int);
		pusData=&m_usFirstOpen;

		for(i=0;i<5;i++)
		{	((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
		usSize+=sizeof(unsigned	short);
		}

		m_ucUnits=10;
		(pucData+usSize)[0]=m_ucUnits;
		usSize+=sizeof(unsigned	char);

		for(i=0;i<m_ucUnits;i++)
		{
			ucLen=(unsigned char)strlen(pcTitle[i]);
			//			code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
				ucLen,(LPWSTR)(pucData+usSize),ucLen);
			usSize+=ucLen;
			pucData[usSize]='|';
			pucData[usSize+1]=aucDescribe[i];
			usSize+=2;
		}

		for(i=0;i<m_usMinsUnits;i++)
		{
			usSize+=m_pstMinsUnit[i].HostToNet(pucData+usSize, usType);
		}

		return	usSize;
	}
	unsigned short	GetSize(unsigned short	usSize)
	{
		return	40+sizeof(SRealMinsData)+usSize*sizeof(SRealMinsUnit);
	}

	static unsigned short	GetBufNeedSize(unsigned short usSize, unsigned short usType)
	{
		if (usType == NEWREALMIN  || usType == DAY2REALMIN)
			return 98 + usSize * sizeof(SRealMinsUnit);  //98��SRealMinsData�Ĵ�С 	 */
		return 0;  
	}
};

//����Ʒ��K�����ݽṹ
struct	SHisKUnit{
	unsigned	int		m_uiDate;
	unsigned	int		m_uiOpenPrice;
	unsigned	int		m_uiMaxPrice;
	unsigned	int		m_uiMinPrice;
	unsigned	int		m_uiClosePrice;
	unsigned	int		m_uiVolume;
	unsigned	int		m_uiSum;//�ɽ����(�ֲ���)

	unsigned	short	HostToNet(unsigned char*	pucData)
	{//ת���������ֽ�˳��
		pucData[0]=sizeof(SHisKUnit)-sizeof(unsigned int)+sizeof(unsigned char);
		unsigned short	usSize=sizeof(unsigned char);
		unsigned int*	puiData=&m_uiDate;

		int i;
		for( i=0;i<6;i++)
		{
			((unsigned	int*)(pucData+usSize))[0]=reverse_i(puiData[i]);
			usSize+=sizeof(unsigned	int);
		}

		return	usSize;
	}
};

//��ʷK������
struct	SHisKData{
	unsigned	char	m_ucStructSize;//�ṹ����
	unsigned	short	m_usType;//��Ӧ���ͣ����������ݰ�����һ��
	unsigned	short	m_usIndex;//�������������������ݰ�����һ��
	unsigned	short	m_usMarketType;//�г���ʶ
	char				m_acName[12];//����
	char				m_acCode[6];//����
	char				m_ggpj[12];
	unsigned	char	m_ucUnits;//��Աm_pcTitle���ַ�����
	unsigned	char*	m_pcTitle;
	unsigned	short	m_usSize;//�ṹHisKUnit�ĸ���
	SHisKUnit*			m_pstHisKUnit;

	unsigned	short	HostToNet(unsigned char*	pucData)
	{//ת���������ֽ�˳��
		char*	pcTitle[]={"ʱ��","����","���","���","����","�ɽ���",};
		unsigned char	aucDescribe[]={CH_TIME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,CH_VOLUME,};
		pucData[0]=sizeof(SHisKData);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i=0;
		for(;i<3;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize+=NAME_LEN;
		ucLen=CODE_LEN;
		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		usSize+=sizeof(m_ggpj);
		m_ucUnits=6;
		pucData[usSize]=m_ucUnits;
		usSize+=sizeof(unsigned char);

		for(i=0;i<m_ucUnits;i++)
		{
			ucLen=(unsigned char)strlen(pcTitle[i]);
			//			code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
				ucLen,(LPWSTR)(pucData+usSize),ucLen);
			usSize+=ucLen;
			pucData[usSize]='|';
			pucData[usSize+1]=aucDescribe[i];
			usSize+=2;
		}

		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usSize);
		usSize+=sizeof(unsigned	short);

		for(i=0;i<m_usSize;i++)
			usSize+=m_pstHisKUnit[i].HostToNet(pucData+usSize);

		return	usSize;
	}
	unsigned short	GetSize(unsigned short	usSize)
	{
		return	26+sizeof(SHisKData)+usSize*sizeof(SHisKUnit);
	}

	unsigned short	GetBufNeedSize(unsigned short	usSize)
	{
		return 84 + usSize * 25;  //84��SHisKData�Ĵ�С 25��SHisKUnit�Ĵ�С
	}
};

// ���а�����
struct	SSortUnit{
	unsigned	char	m_ucStructSize;//�ṹ����
	unsigned short m_usMarketCode;
	char				m_acName[12];//����
	char				m_acCode[6];//����
	char				m_ggpj[12];
	unsigned	int		m_uiPrevClose;//���ռ�
	unsigned	int		m_uiNewPrice;//���¼�
	unsigned	int		m_uiUpMargin;//�Ƿ�
	unsigned	int		m_uiVolume;//�ɽ�������
	unsigned	int		m_uiCurVol;//�ɽ�������
	unsigned	int		m_uiSum;//�ɽ����(�ֲ���)
	unsigned	int		m_uiBuyPrice;//ί���
	unsigned	int		m_uiSellPrice;//ί����
	unsigned	int		m_uiOpenPrice;//����
	unsigned	int		m_uiMaxPrice;//��߼�
	unsigned	int		m_uiMinPrice;//��ͼ�
	unsigned	int		m_uiVolRatio;//����(����)
	int					m_iEntrustRatio;//ί��(ƽ��)
	unsigned	int		m_uiPriceYield;//��Ӧ��
	unsigned	int		m_uiPriceAsset;//�о���
	unsigned	int		m_uiVolShare;//������
	unsigned	short	HostToNet(unsigned char*	pucData,unsigned char	ucType)
	{//ת���������ֽ�˳��
		unsigned short	usSize=sizeof(m_ucStructSize);

		((unsigned	short*)(pucData+usSize))[0]=reverse_s(m_usMarketCode);
		usSize += sizeof(unsigned short);

		//		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
		//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);

		usSize+=NAME_LEN;
		//		ucLen=CODE_LEN;
		//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);

		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));

		usSize+= sizeof(m_ggpj);
		unsigned int*	puiData=(unsigned	int*)(pucData+usSize);
		int	nIndex=0;
		puiData[nIndex++]=m_uiPrevClose;
		switch(ucType)
		{
			// �������а��е����� �� title  �еĵڶ����п�ʼ��Ӧ
		case CODE_SORT: // ����˳�����а�
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiUpMargin;//�Ƿ�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//���
			break;
			// ���Ӧ�� title
			//"����"
			//"����"
			//"�ɽ�"
			//"�Ƿ�"
			//"����"
			//"����"
			//"���"
			//"���"
			//"����"
			//"����"
			//"���"
		case RISE_SORT: // �Ƿ����а�
			//puiData[nIndex++]=(int)((((float)m_uiNewPrice-m_uiPrevClose)/m_uiPrevClose+ROUND)*MULTIPLE);//�Ƿ�
			puiData[nIndex++]=m_uiUpMargin;
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//�ǵ�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			break;
		case SUM_SORT: // �ɽ�������а�
			puiData[nIndex++]=m_uiSum;//���
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//�ǵ�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			break;
		case VOLUME_SORT: // �ɽ������а�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//�ǵ�
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//���
			break;
		case SWING_SORT: // ������а�
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//���
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//�ǵ�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			break;
		case VOLRATIO_SORT: // �������а�
			puiData[nIndex++]=m_uiVolRatio;//����
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//�ǵ�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			break;
		case BUYSELL_SORT: // ί�����а�
			puiData[nIndex++]=m_iEntrustRatio;//ί��
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//�ǵ�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			break;
		case PRICE_SORT: // �ɽ������а�
			puiData[nIndex++]=m_uiNewPrice;//�ɽ�
			puiData[nIndex++]=m_uiOpenPrice;//����
			puiData[nIndex++]=(int)((((float)m_uiMaxPrice-m_uiMinPrice)/m_uiPrevClose+ROUND)*MULTIPLE);//���
			puiData[nIndex++]=m_uiNewPrice-m_uiPrevClose;//�ǵ�
			puiData[nIndex++]=m_uiVolume;//����
			puiData[nIndex++]=m_uiCurVol;//����
			puiData[nIndex++]=m_uiMaxPrice;//���
			puiData[nIndex++]=m_uiMinPrice;//���
			puiData[nIndex++]=m_uiBuyPrice;//����
			puiData[nIndex++]=m_uiSellPrice;//����
			break;
		}

		int i;
		for( i=0;i<nIndex;i++)
			puiData[i]=reverse_i(puiData[i]);

		usSize+=(unsigned short)sizeof(unsigned	int)*nIndex;
		pucData[0]=(unsigned char)(sizeof(unsigned char)+nIndex*sizeof(unsigned int)+CODE_LEN*2+NAME_LEN+sizeof(m_ggpj));
		return	usSize;
	}

	static unsigned short GetBufNeedSize(unsigned char	ucType)
	{
		if(CODE_SORT == ucType)
			return 83;  // 83��SSortUnit�Ĵ�С
		else
			return 0;
	}
};

struct	SSortData{
	unsigned	char	m_ucStructSize;//�ṹ����
	unsigned	short	m_usType;//��Ӧ���ͣ����������ݰ�����һ��
	unsigned	short	m_usIndex;//�������������������ݰ�����һ��
	unsigned	short	m_usSize;//�ṹSortUnit�ĸ���
	unsigned	short	m_usTotal;//�����Ʊ������Ŀ
	unsigned	char	m_ucUnits;//��Աm_pcTitle��Ԫ�ظ���
	unsigned	char*	m_pcTitle;
	SSortUnit*			m_pstSortUnit;
	unsigned	short	HostToNet(unsigned char*	pucData,unsigned char	ucType)
	{//ת���������ֽ�˳��
		pucData[0]=sizeof(SHisKData)-sizeof(SSortUnit*);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i;
		for( i=0;i<4;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		m_ucUnits=11;
		(pucData+usSize)[0]=m_ucUnits;
		usSize+=sizeof(unsigned	char);

		unsigned char	ucLen;

		for(i=0;i<m_ucUnits;i++)
		{
			switch(ucType)
			{
			case CODE_SORT://����˳�����а�
				{	char*	pcTitle[]={"����","����","�ɽ�","�Ƿ�","����",
					"����","���","���","����","����","���",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_PRICE,CH_PRICE,CH_FLOAT,CH_VOLUME,
					CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,CH_FLOAT,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case RISE_SORT://�Ƿ����а�
				{	char*	pcTitle[]={"����","�Ƿ�","����","�ɽ�","�ǵ�",
					"����","����","���","���","����","����",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case SUM_SORT://�ɽ�������а�
				{	char*	pcTitle[]={"����","���","����","�ɽ�","�ǵ�",
					"����","����","���","���","����","����",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case VOLUME_SORT://�ɽ������а�
				{	char*	pcTitle[]={"����","����","����","�ɽ�","�ǵ�",
					"����","���","���","����","����","���",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_FLOAT,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case SWING_SORT://������а�
				{	char*	pcTitle[]={"����","���","����","�ɽ�","�ǵ�",
					"����","����","���","���","����","����",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case VOLRATIO_SORT://�������а�
				{	char*	pcTitle[]={"����","����","����","�ɽ�","�ǵ�",
					"����","����","���","���","����","����",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case BUYSELL_SORT://ί�����а�
				{	char*	pcTitle[]={"����","ί��","����","�ɽ�","�ǵ�",
					"����","����","���","���","����","����",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			case PRICE_SORT://�ɽ������а�
				{	char*	pcTitle[]={"����","�ɽ�","����","���","�ǵ�",
					"����","����","���","���","����","����",};
				unsigned char	aucDescribe[]={CH_CHAR,CH_PRICE,CH_PRICE,CH_FLOAT,CH_PRICE,
					CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,};
				ucLen=(unsigned char)strlen(pcTitle[i]);
				//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],ucLen,(LPWSTR)(pucData+usSize),ucLen);
				usSize+=ucLen;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];}
				break;
			}

			usSize+=2;
		}

		for(i=0;i<m_usSize;i++)
			usSize+=m_pstSortUnit[i].HostToNet(pucData+usSize,ucType);

		return	usSize;
	}

	unsigned short	GetSize(unsigned char	ucSize)
	{
		return	sizeof(SSortData)+ucSize*sizeof(SSortUnit);
	}

	static unsigned short GetBufNeedSize(unsigned char	ucSize, unsigned char	ucType)
	{
		if(CODE_SORT == ucType)
			return 76 + ucSize*83;  //76�ǽṹ��SSortData�Ĵ�С 83��SSortUnit�Ĵ�С
		else
			return 0;
	}
};

//������ڲ�ʹ��
struct	SRequestSort{
	unsigned	short	m_usType;//��������
	unsigned	short	m_usMarketType;//�г���ʶ
	unsigned	short	m_usIndex;//��������
	unsigned	short	m_usSize;//�����֤ȯ����
	unsigned	short	m_usFirst;//��������ʼ���
};

//�ͻ����������а�����
struct	SMRequestSort{
	unsigned	short	m_usType;//��������
	unsigned	short	m_usMarketType;//�г���ʶ
	unsigned	short	m_usIndex;//��������
	unsigned	short	m_usMobileType;//�ֻ��ͺ�
	char				m_acRight[32];//Ȩ��������
	unsigned	short	m_usSize;//�����֤ȯ����
	unsigned	short	m_usFirst;//��������ʼ���
	unsigned	short	NetToHost(unsigned char*	pucData)
	{//ת���������ֽ�˳��
		unsigned short	usSize=0;
		unsigned short*	pusData=&m_usType;

		int i;
		for( i=0;i<4;i++)
		{
			pusData[i]=reverse_s(((unsigned	short*)(pucData+usSize))[0]);
			usSize+=sizeof(unsigned	short);
		}

		memcpy(m_acRight,pucData+usSize,32);
		usSize+=32;
		m_usSize=reverse_s(((unsigned	short*)(pucData+usSize))[0]);
		usSize+=sizeof(unsigned	short);
		m_usFirst=reverse_s(((unsigned	short*)(pucData+usSize))[0]);
		usSize+=sizeof(unsigned	short);
		return	usSize;
	}
};

//������ڲ�ʹ��
struct	SCodeInfo{
	unsigned	short	m_usMarketType;//�г���ʶ
	unsigned	int		m_uiCode;//֤ȯ����
	char				m_acCode[6];//��Ʊ����
	char				m_acName[12];//����
	unsigned	int		m_uiPrevClose;//���ռ�
	unsigned	int 	m_ui5DayVol;//����ƽ������
	unsigned	int 	m_uiUpLimit;//��ͣ������,�������ڹ�ծ��ʾӦ����Ϣ
	unsigned	int		m_uiDownLimit;//��ͣ������
	unsigned	int		m_uiShares;//��ͨ����
	short				m_sAsset;//ÿ�ɾ��ʲ�
	short				m_sYield;//ÿ������
	char				m_ggpj[12];//��������
//	unsigned	int		m_uiExRightsPrevClose;//���ռ�
};

//������ڲ�ʹ��
struct	SRequestData{
	unsigned	short	m_usType;//��������
	unsigned	short	m_usMarketType;//�г���ʶ
	unsigned	short	m_usIndex;//��������
	unsigned	short	m_usSize;//����֤ȯ����,���ṹCodeInfo�ĸ���
	unsigned	int		m_uiFirst;//�������ݽṹ�ĵ���ʼ
	SCodeInfo*			m_pstCode;
};

//�������ֻ��ͻ������͵�ʵʱ����
struct	SRealUnit{
	unsigned	char	m_ucStructSize;//�ṹ����
	unsigned	int*	m_puiData;
	unsigned	short	HostToNet(unsigned char*	pucData,unsigned short	usUnits)
	{
		//ת���������ֽ�˳��
		unsigned short	usSize=sizeof(unsigned char);
		pucData[0]=usUnits*sizeof(unsigned int)+sizeof(unsigned char);

		for(int i=0;i<usUnits;i++)
		{	((unsigned	int*)(pucData+usSize))[0]=reverse_i(m_puiData[i]);
		usSize+=sizeof(unsigned	int);
		}

		return	usSize;
	}

	unsigned short	GetSize(unsigned char	ucSize)
	{
		return	sizeof(unsigned char)+ucSize*sizeof(unsigned int);
	}
};

//����֤ȯ�ɽ�ͳ����Ϣ
struct	SStatData{
	unsigned	char	m_ucStructSize;//�ṹ����
	unsigned	short	m_usType;//��Ӧ���ͣ����������ݰ�����һ��
	unsigned	short	m_usIndex;//�������������������ݰ�����һ��
	unsigned	short	m_usMarketType;//�г���ʶ
	char				m_acName[12];//����
	char				m_acCode[6];//����
	char				m_ggpj[12];
	unsigned	int		m_uiPrevClose;//���ռ�
	unsigned	char	m_ucUnits;//��Աm_pcTitle���ַ�����
	unsigned	char*	m_pcTitle;
	SRealUnit			m_stRealUnit;
	unsigned	short	HostToNet(unsigned char*	pucData)
	{//ת���������ֽ�˳��
		pucData[0]=sizeof(SStatData);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i,j;
		for( i=0;i<3;i++)
		{	((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
		usSize+=sizeof(unsigned	short);
		}

		unsigned char	ucLen=NAME_LEN;
		memset(pucData+usSize,0,NAME_LEN);
//		code_convert("GBK", "UTF-16LE", (char*)m_acName, NAME_LEN, (char*)pucData+usSize, NAME_LEN);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acName,
			NAME_LEN,(LPWSTR)(pucData+usSize),NAME_LEN);
		usSize+=NAME_LEN;
//		code_convert("GBK", "UTF-16LE", (char*)m_acCode, CODE_LEN, (char*)pucData+usSize, CODE_LEN*2);
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_acCode,
			CODE_LEN,(LPWSTR)(pucData+usSize),CODE_LEN*2);
		usSize+=CODE_LEN*2;
		MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,m_ggpj,
			sizeof(m_ggpj),(LPWSTR)(pucData+usSize),sizeof(m_ggpj));
		usSize+=sizeof(m_ggpj);
		((unsigned	int*)(pucData+usSize))[0]=reverse_i(m_uiPrevClose);
		usSize+=sizeof(unsigned	int);

		if(ST_Index != (m_usMarketType & 0x0f))
		{
			char*	pcTitle[]={"�ǵ�","�ǵ���","����","����","���","���",
				"��ͣ","��ͣ","����","����","ί��","����","���","����","����",
				"��ӯ��","������","����","","����","","����","","����","",
				"��һ","","�ɽ�","","��һ","","���","","����","","����","","����","",};

			//���ڳ�Ա����m_stNowData�����������Ա������ʼ������������������ɫ���۸񣬳ɽ�����С��
			unsigned char	aucDescribe[]={CH_PRICE,CH_FLOAT,CH_PRICE,CH_PRICE,CH_PRICE,CH_PRICE,
				CH_PRICE,CH_PRICE,CH_PRICE,CH_VOLUME,CH_FLOAT,CH_FLOAT,CH_PRICE,CH_VOLUME,CH_VOLUME,
				CH_FLOAT,CH_FLOAT,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,
				CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_BARGIN,CH_VOLUME,
				CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,CH_PRICE,CH_VOLUME,
				CH_PRICE,CH_VOLUME,};

			m_ucUnits=39;
			(pucData+usSize)[0]=m_ucUnits;
			usSize+=sizeof(unsigned	char);

			for( i=0;i<m_ucUnits;i++)
			{
				ucLen=strlen(pcTitle[i]);
//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucLen);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
				ucLen,(LPWSTR)(pucData+usSize),ucLen);
			usSize+=ucLen;
			pucData[usSize]='|';
			pucData[usSize+1]=aucDescribe[i];
			usSize+=2;
			}
		}
		else
		{
			char*	pcTitle[]={"A�ɳɽ�","B�ɳɽ�","����ɽ�","ָ��","�ǵ�","����",
				"�ܳɽ���","�ܳɽ���","���ָ��","���ָ��","���Ǽ���","�µ�����",};

			//���ڳ�Ա����m_stNowData�����������Ա������ʼ������������������ɫ���۸񣬳ɽ�����С��
			unsigned char	aucDescribe[]={CH_VOLUME,CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_FLOAT,
				CH_VOLUME,CH_VOLUME,CH_PRICE,CH_PRICE,CH_VOLUME,CH_VOLUME,};
			m_ucUnits=12;
			(pucData+usSize)[0]=m_ucUnits;
			usSize+=sizeof(unsigned	char);

			unsigned char	ucInvert;
			for( i=0;i<m_ucUnits;i++)
			{
				ucLen=strlen(pcTitle[i]);
				ucInvert=ucLen;
				for( j=0;j<ucLen;j++)
				{
					if(pcTitle[i][j]>0)
						ucInvert++;
				}

//				code_convert("GBK", "UTF-16LE", (char*)pcTitle[i], ucLen, (char*)pucData+usSize, ucInvert);
				MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,pcTitle[i],
					ucLen,(LPWSTR)(pucData+usSize),ucInvert);
				usSize+=ucInvert;
				pucData[usSize]='|';
				pucData[usSize+1]=aucDescribe[i];
				usSize+=2;
			}
		}

		usSize+=m_stRealUnit.HostToNet(pucData+usSize,m_ucUnits);
		return	usSize;
	}

	unsigned short	GetSize(unsigned char	ucType)
	{//ת���������ֽ�˳��
		if(ST_Index != ucType)
			return	202+sizeof(SStatData)+m_stRealUnit.GetSize(39);
		else
			return	108+sizeof(SStatData)+m_stRealUnit.GetSize(12);

		return	m_stRealUnit.GetSize(ucType);
	}
};

// ���۽ṹ����ʾ��ۡ������������ۡ�����
struct	SVolPrice
{
	unsigned	int		m_uiPrice;//�۸�
	unsigned	int		m_uiVolume;//��
};

struct SINDEXOVERLAP
{
	unsigned int m_uiClose;
	unsigned char m_ucUnits;
	SRealMinsUnit *m_pstMinsUnit;

	unsigned short HostToNet(unsigned char*	pucData, unsigned short usType)
	{
		unsigned short usLen = 0;
		((unsigned int*)pucData)[0] = reverse_i(m_uiClose);
		usLen += 4;
		pucData[usLen] = m_ucUnits;
		usLen += 1;
		SRealMinsUnit *prmu = (SRealMinsUnit*)((char*)&m_ucUnits + 1);
		for (int i = 0; i < m_ucUnits; i++)
			usLen += prmu[i].HostToNet(pucData+usLen, usType);
		return usLen;
	};

	unsigned short GetSize(unsigned short usSize)
	{
		return sizeof(int)+sizeof(char)+usSize*sizeof(SRealMinsUnit);
	};
};

//������֤��Ʊ�����Ƿ�Ϸ����߷��ض����Ʊ����
struct	SCodeList{
	unsigned	char	m_ucStructSize;//�ṹ����
	unsigned	short	m_usType;//��Ӧ���ͣ����������ݰ�����һ��
	unsigned	short	m_usIndex;//�������������������ݰ�����һ��
	unsigned	char	m_ucSize;//�ṹSCodeInfo�ĸ���
	SMCodeInfo*			m_pstCodeInfo;
	unsigned	short	HostToNet(unsigned char*	pucData)
	{//ת���������ֽ�˳��
		pucData[0]=sizeof(SHisKData)-sizeof(SSortUnit*);
		unsigned short	usSize=sizeof(unsigned	char);
		unsigned short*	pusData=&m_usType;

		int i;
		for( i=0;i<2;i++)
		{
			((unsigned	short*)(pucData+usSize))[0]=reverse_s(pusData[i]);
			usSize+=sizeof(unsigned	short);
		}

		(pucData+usSize)[0]=m_ucSize;
		usSize+=sizeof(m_ucSize);
		char *pTemp = ((char*)&m_usType) + 2 * sizeof(unsigned short)  + sizeof(unsigned char) + sizeof(SMCodeInfo*);
		SMCodeInfo* pstCodeInfo= (SMCodeInfo*)pTemp;
//			(SMCodeInfo*)(&m_ucSize+sizeof(m_ucSize)+sizeof(SMCodeInfo*));

		for(i=0;i<m_ucSize;i++)
			usSize+=pstCodeInfo[i].HostToNet(pucData+usSize);

		return	usSize;
	}

	unsigned	short	GetSize(unsigned char	ucSize)
	{
		return	sizeof(SHisKData)+ucSize*sizeof(SMCodeInfo);
	}
};
////////////////////////////////////////////////////////////////////////////////////////
//��ʱ����
struct tagRealMins
{
	unsigned int	 nTime;           //ʱ�䣬�Է�Ϊ��λ
	unsigned int	 nOpenPrice;	  //���̼۸�
	unsigned int	 nMaxPrice;		  //��߼۸�
	unsigned int	 nMinPrice;		  //��ͼ۸�
	unsigned int	 nNewPrice;       //���¼۸�
	unsigned int	 nVolume;		  //�ɽ�����
	unsigned int     AvgPrice;				//����
};

//����ͳ��
struct tagQuotedPrice
{
	char szCode[16];				  //֤ȯ����
	char szName[16];                  //֤ȯ����
	unsigned int	nZrsp;            //��������
	unsigned int	nJrkp;			  //����
	unsigned int	nZtjg;            //��ͣ�۸�

	unsigned int	nDtjg;            //��ͣ�۸�
	unsigned int	nSyl1;            //��ӯ��1
	unsigned int	nSyl2;            //��ӯ��2
	unsigned int	nZgjg;            //��߼۸�
	unsigned int	nZdjg;            //��ͼ۸�

	unsigned int	nZjjg;            //����۸�
	unsigned int	nZjcj;            //����ɽ�
	unsigned int	nCjsl;            //�ɽ�����
	unsigned int	nCjje;            //�ɽ����
	unsigned int	nCjbs;            //�ɽ�����

	unsigned int	nBjw1;            //���һ
	unsigned int	nBsl1;            //����һ
	unsigned int	nBjw2;            //��۶�
	unsigned int	nBsl2;            //������
	unsigned int	nBjw3;            //�����

	unsigned int	nBsl3;            //������
	unsigned int	nBjw4;            //�����
	unsigned int	nBsl4;            //������
	unsigned int	nBjw5;            //�����
	unsigned int	nBsl5;            //������

	unsigned int	nSjw1;            //����һ
	unsigned int	nSsl1;            //����һ
	unsigned int	nSjw2;            //���۶�
	unsigned int	nSsl2;            //������
	unsigned int	nSjw3;            //������

	unsigned int	nSsl3;            //������
	unsigned int	nSjw4;            //������
	unsigned int	nSsl4;            //������
	unsigned int	nSjw5;            //������
	unsigned int	nSsl5;            //������

	unsigned int	n5rpjzs;          //����ƽ������
	unsigned int	nPjjg;            //����
	unsigned int	nWb;			  //ί��
	unsigned int    nLb;			  //����
	unsigned int	nWp;			  //����
	unsigned int	nNp;			  //����
	
	unsigned int	nZd;              //�ǵ�
	unsigned int	nZdf;             //�ǵ���
	unsigned int	nZf;              //���
};

struct tagKLineInfo
{
	unsigned int     nDate;			  //����
	unsigned int     nOpenPrice;	  //���̼۸�
	unsigned int     nMaxPrice;		  //��߼۸�
	unsigned int     nMinPrice;		  //��ͼ۸�
	unsigned int     nClosePrice;	  //���̼۸�
	unsigned int     nVolume;		  //�ɽ�����
	unsigned int     nSum;			  //�ɽ����
};

struct tagSaveData   //������ź�����ָ��
{
	unsigned short   SerialNo;
	char*            pData;
};

struct tagStockInfo
{
	char             szCode[16];	 //֤ȯ����
	char             szName[16];	 //֤ȯ���
	char             szSpell[4];	 //ƴ��
	unsigned int     nZrsp;			 //��������
};
////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)
#endif