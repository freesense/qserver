// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// qextend.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

CWorkThread gWorkThread;
CWriteThread gWriteThread;
CInfoThread gInfoThread;
CMessageQueue<CommxHead*> gQueue;
CDataBaseEx gDBRead;
CDataBaseEx gDBWrite;
CDataStore gDataStore;
CDataFarm gFarm;
int gnTradeTime = 0;//����ʱ��
char gacTradeTime[4];//����ʱ��,������ָ֤���Ľ���ʱ��
bool gbIsTradeTime;//�Ƿ��ǽ���ʱ��
bool gbIsWorkTime;//�Ƿ��ǹ���ʱ��
CMessageQueue<Consign*> gConsignQueue;//ί�е���,�ȴ�д��ί�гɽ���
CMessageQueue<Consign*> gConsignQueueWait;//ί�е���,�ȴ�д��ί�гɽ���
bool gbExitFlag;//�˳�
SumInfo gSumInfo;//ͳ����Ϣ
bool	gbWorkDay;//�Ƿ��ǹ����գ������Ƿ���OnNewTick��������OnNewTick�����ɴ�ϳɹ�
bool	gbTradeTime;//����ʱ��