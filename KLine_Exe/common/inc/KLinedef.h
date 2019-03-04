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

//定义日志的级别
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

#ifndef RPT_ADDI_INFO           //附加信息，可以报告设备的状态等信息
#define	RPT_ADDI_INFO   0x90
#endif

#ifndef RPT_VERSION		        //模块的版本信息
#define RPT_VERSION    0xAB
#endif

#ifndef RPT_RESTART		        //重启程序
#define RPT_RESTART    0xAC
#endif

//特殊标志位，这部分使用或计算，多余的值留给用户自定义
#ifndef	RPT_IGNORE
#define RPT_IGNORE		0x00000100		/*不显示在任何输出终端中*/
#endif

#ifndef	RPT_NOPREFIX
#define RPT_NOPREFIX	0x00000200		/*忽略日志前面的辅助信息*/
#endif

#ifndef	RPT_IMPORTANT
#define RPT_IMPORTANT	0x00000400		/*重要信息*/
#endif

#define HTTPFORMAT "http://hq.sinajs.cn/list=%s%s"







////////////////////////////////////////////////////////////////////////////////////
#endif