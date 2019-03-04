#ifndef _QUOTE_SERVER_DATA_STRUCT_
#define _QUOTE_SERVER_DATA_STRUCT_
#include "..\\protocol.h"
#pragma pack(1)
const	unsigned short	FARMHEART=0;		//QuoteFarm������
const	unsigned short	FARMNOTIFY=65000;	//����仯֪ͨ
const	unsigned short	INITDATA=0x0001;		//�г���ʼ��
const	unsigned short	TRACEDATA=0x0002;	//�ɽ���ϸ
const	unsigned short	REPORTDATA=0x0003;	//��ѡ�ɱ���
const	unsigned short	REALMINSDATA=0x0004;	//��������
const	unsigned short	HISKDAYDATA=0x0005;		//��ʷK����
const	unsigned short	HISKWEEKDATA=0x0006;		//��ʷK����
const	unsigned short	HISKMONTHDATA=0x0007;		//��ʷK����
const	unsigned short	REALMINSDATA2=0x0008;			//���շ�ʱ

const	unsigned short	LINETESTREQ=0x0000;//��·���������
const	unsigned short	LINETESTREP=0x0100;//��·����Ӧ���


const	unsigned short	REALPUSHREQ=0X1000;	//��������
const	unsigned int BUFFER_SIZE=102400;

typedef struct _PACKETHEAD
{
	CommxHead		m_commxHead;
	unsigned short	m_usFuncNo;//���ܱ��
	unsigned short	m_usReqCount;//�������
}PACKETHEAD,*PPACKETHEAD;

typedef struct _REQ_ITEM//������Ŀ
{
	unsigned short	m_unType;//����
	unsigned short	m_usStockCount;//֤ȯ����
	char		*	m_pStocks;//�б�
}REQ_ITEM,* PREQ_ITEM;

typedef struct _K_LINE
{
			unsigned int day;
			unsigned int open;
			unsigned int high;
			unsigned int low;
			unsigned int close;
			unsigned int volume;
			unsigned int amount;
}KLINE,*PKLINE;


//
//�Զ���ö���������ͣ�������ʶ�׽���IO��������
//
typedef enum _IO_OPERATION 
{
        IoAccept, //AcceptEx/accept
		IoRead,   //WSARecv/recv/ReadFile
		IoWrite,   //WSASend/send/WriteFile
		IoEnd,
		IoExtraSend,
		IoExtraSendEnd
}IO_OPERATION, *PIO_OPERATION;

#pragma pack()
#endif
