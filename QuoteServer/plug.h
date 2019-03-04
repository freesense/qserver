#pragma once
#include "..\\public\\data_struct.h"
#pragma pack(1)

struct SMsgID{
	unsigned int m_unID;
	unsigned int m_unSocket;
};

typedef	int (*_OnInit)(unsigned int c_unPlugID,const char * c_pParam);
typedef	int (*_OnRelease)();
typedef int (*_OnUpdateData)(char * c_pBuf,int c_iLen);
typedef int (*_OnRequest)(SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);
typedef int (*_SendReply)( SMsgID * c_pMsgID,char * c_pBuf,int c_iLen);
typedef Quote * (*_GetSymbolQuote)(char * c_pSymbol);
typedef void (*_Set_SendReplay)(_SendReply c_pFunc);
typedef void (*_Set_GetSymbolQuote)(_GetSymbolQuote c_pFunc);
typedef void (*_PlugQuit)(unsigned int c_unPlugID);
typedef void (*_Set_PlugQuit)(_PlugQuit c_pFunc);
typedef int (*_GetSymbolData)(char * c_pSymbol,char * c_pBuf,int c_iMaxLen);
typedef void (*_Set_GetSymbolData)(_GetSymbolData c_pFunc);

//Server���������仯֪ͨ�ص�����ԭ��
typedef void(*pfnOnQuoteChanged)(const char *symbol, const int *notify); //4��int
//Server�����ҵ������ص�����
typedef void(*pfnOnBusiness)(unsigned int,char *pHead);

typedef int  (*_FarmOnInitialize)(int iMode);
typedef void (*_FarmRelease)();
typedef void (*_FarmRegisterCallBack)(pfnOnQuoteChanged,pfnOnBusiness);
typedef bool (*_FarmOnRequest)(unsigned int,char *);

typedef void (*_GetModuleVer)(char **);//��ȡ�汾��Ϣ

//������qextend.dll����������
//extern "C" __declspec(dllexport) int Initialize(int iMode)
//extern "C" __declspec(dllexport) void Release()
//extern "C" __declspec(dllexport) void RegisterCallback(pfnOnQuoteChanged funcQuote, pfnOnBusiness funcBusiness)
//extern "C" __declspec(dllexport) bool OnRequest(unsigned int,CommxHead *pRequest)
//iMode��Server�ҽӵ�ʱ��̶�Ϊ1

#define SYMBOL_LEN 16
#define REQ_QUOTE	1		//��������
#define REQ_MINK	2		//��������
#define REQ_TICK		3		//�ֱʳɽ�
#define REQ_HISK		4	//	��ʷK��

struct SPlugMethod{
	_OnInit	m_OnInit;
	_OnRelease	m_OnRelease;
	_OnUpdateData	m_OnUpdateData;
	_OnRequest		m_OnRequest;
	_Set_SendReplay	m_Set_SendReplay;
	_Set_GetSymbolQuote	m_Set_GetSymbolQuote;
	_Set_PlugQuit		m_Set_PlugQuit;
	_Set_GetSymbolData	m_Set_GetSymbolData;

	_FarmOnInitialize	m_FarmOnInitialize;
	_FarmRelease		m_FarmRelease;
	_FarmRegisterCallBack m_FarmRegisterCallBack;
	_FarmOnRequest		m_FarmOnRequest;

	_GetModuleVer		m_GetModuleVer;
};
struct SMinKReq{
	unsigned int m_unDate;
};
struct STickReq{
	unsigned int m_unDate;
};
struct SHisKReq{
	unsigned int m_unBeginDate;
	unsigned int m_unEndDate;
	unsigned short m_usRight;//1��ǰ��Ȩ��2���Ȩ
	unsigned short	m_usType;//���ߣ����ߣ�����
};
struct SPlugReq{
	unsigned int	m_unType;
	unsigned short  m_usCount;//��Ʊ����
	union{
		SMinKReq	m_sMinKReq;
		STickReq	m_sTickReq;
		SHisKReq	m_sHisKReq;
	};
	char			m_acSymbol[SYMBOL_LEN];//��ʵ������m_usCount * SYMBOL_LEN
};
#pragma pack()