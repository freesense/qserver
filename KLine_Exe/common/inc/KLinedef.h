//KLinedef.h
#ifndef _KLINEDEF_H_
#define _KLINEDEF_H_

////////////////////////////////////////////////////////////////////////////////////
#define MSG_BUF_LEN       1024 

#define  VERSION          0x03
#define  ROUTETARGET      0x00

#define STOCK_CODE_LEN    16
#define	STOCK_NAME_LEN    16

#define SEND_BUF_LEN      100

#define DT_HQ             1
#define DT_KLINE          2
#define DT_FBCJ           3

//������־�ļ���
#ifndef	RPT_DEBUG
#define RPT_DEBUG		0x10
#endif

#ifndef	RPT_INFO
#define RPT_INFO		0x20
#endif

#ifndef	RPT_WARNING
#define RPT_WARNING		0x40
#endif

#ifndef	RPT_ERROR
#define RPT_ERROR		0x80
#endif

#ifndef	RPT_CRITICAL
#define RPT_CRITICAL	0xf0
#endif

#ifndef	RPT_HEARTBEAT
#define RPT_HEARTBEAT   0xAA
#endif

#ifndef RPT_ADDI_INFO           //������Ϣ�����Ա����豸��״̬����Ϣ
#define	RPT_ADDI_INFO   0x90
#endif

#ifndef RPT_VERSION		        //ģ��İ汾��Ϣ
#define RPT_VERSION    0xAB
#endif

#ifndef RPT_RESTART		        //��������
#define RPT_RESTART    0xAC
#endif

//�����־λ���ⲿ��ʹ�û���㣬�����ֵ�����û��Զ���
#ifndef	RPT_IGNORE
#define RPT_IGNORE		0x00000100		/*����ʾ���κ�����ն���*/
#endif

#ifndef	RPT_NOPREFIX
#define RPT_NOPREFIX	0x00000200		/*������־ǰ��ĸ�����Ϣ*/
#endif

#ifndef	RPT_IMPORTANT
#define RPT_IMPORTANT	0x00000400		/*��Ҫ��Ϣ*/
#endif

#define HTTPFORMAT "http://hq.sinajs.cn/list=%s%s"







////////////////////////////////////////////////////////////////////////////////////
#endif