//CharTranslate.cpp
//
//#include "stdafx.h"
#include "CharTranslate.h"

//////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//��DBF�м۸��ֶ�ֵת��Ϊ��ֵ������ļ۸��ֶ���ת�����൱�ڷŴ���1000��
//����û��С����������ֶ���˵ת����û�зŴ�
/////////////////////////////////////////////////////////////////////////
unsigned int CCharTranslate::PriceToDigital(char*	pcValue,int iLen)
{
	unsigned int	uiValue=0;
	int i=0;
	for(; i<iLen; i++)
		if(pcValue[i]!=' ')
			break;

	for(; i<iLen && pcValue[i]!=0 && pcValue[i]!=' '; i++)
	{
		if(pcValue[i]=='.')
		{
			int j=0;
			for(i++; i<iLen && pcValue[i]!=' ' && j<3; i++)
				if(pcValue[i] >= '0' && pcValue[i] <= '9')
				{
					uiValue = uiValue * 10 + pcValue[i] - '0';
					j++;
				}
			
			for(; j<3; j++)
				uiValue *= 10;
			break;
		}

		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';
	}

	return	uiValue;
}

/////////////////////////////////////////////////////////////////////////
//��DBF�м۸��ֶ�ֵת��Ϊ��ֵ������ļ۸��ֶ���ת�����൱�ڷŴ���1000��,������֮��
//����û��С����������ֶ���˵ת����û�зŴ�
/////////////////////////////////////////////////////////////////////////
int CCharTranslate::SignedPriceToDigital(char*	pcValue,int iLen)
{
	int	uiValue=0;
	int i=0;
	for(; i<iLen; i++)
		if(pcValue[i]!=' ')
			break;

	bool bNegative = false;
	for(; i<iLen && pcValue[i]!=0 && pcValue[i]!=' '; i++)
	{
		if(pcValue[i]=='-')
		{
			bNegative = true;
			continue;
		}

		if(pcValue[i]=='.')
		{
			int j=0;
			for(i++; i<iLen && pcValue[i]!=' ' && j<3; i++,j++)
				if(pcValue[i] >= '0' && pcValue[i] <= '9')
					uiValue = uiValue * 10 + pcValue[i] - '0';

			for(; j<3; j++)
				uiValue *= 10;
			break;
		}

		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';
	}

	if(bNegative)
		return -uiValue;

	return	uiValue;
}

//////////////////////////////////////////////////////////////////////////
//��DBF�ֶ��гɽ����ַ���ת��Ϊ�����ٳ���100
//pcValueΪҪת�����ַ���
//////////////////////////////////////////////////////////////////////////
unsigned int CCharTranslate::VolumeToDigital100(char* pcValue, int iLen)
{
	unsigned int uiValue = 0;
	for(; iLen>0; iLen--)
		if(pcValue[iLen-1] != ' ')
			break;

	int i=0;
	for(; i<iLen; i++)
		if(pcValue[i]!=' ')
			break;
	iLen -= 2;////��С100��
	for(; i<iLen; i++)
		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';

	return	uiValue;
}

//////////////////////////////////////////////////////////////////////////
//��DBF�ֶ��гɽ����ַ���ת��Ϊ�����ٳ���10000
//pcValueΪҪת�����ַ���
//////////////////////////////////////////////////////////////////////////
unsigned int CCharTranslate::VolumeToDigital10000(char* pcValue, int iLen)
{
	unsigned int uiValue = 0;
	for(; iLen>0; iLen--)
		if(pcValue[iLen-1] != ' ')
			break;

	int i=0;
	for(; i<iLen; i++)
		if(pcValue[i]!=' ')
			break;
	iLen -= 4;////��С10000��
	for(; i<iLen; i++)
		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';

	return	uiValue;
}

//////////////////////////////////////////////////////////////////////////
//��DBF�ֶ��гɽ����ַ���ת��Ϊ����
//pcValueΪҪת�����ַ���
//////////////////////////////////////////////////////////////////////////
unsigned int CCharTranslate::VolumeToDigital(char* pcValue, int iLen)
{
	unsigned int uiValue = 0;
	for(; iLen>0; iLen--)
		if(pcValue[iLen-1] != ' ')
			break;

	int i=0;
	for(; i<iLen; i++)
		if(pcValue[i]!=' ')
			break;
	
	for(; i<iLen; i++)
		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';

	return	uiValue;
}
//////////////////////////////////////////////////////////////////////////
//��DBF�ֶ��гɽ�����ַ���ת��Ϊ�����ٳ���100
//pcValueΪҪת�����ַ���
//////////////////////////////////////////////////////////////////////////
unsigned int CCharTranslate::SumPriceToDigital100(char* pcValue, int iLen)
{
	for(; iLen>0; iLen--)
		if(pcValue[iLen-1]!=' ')
			break;

	int i = iLen - 1;
	for(; i>=0; i--)
	{
		if(pcValue[i] == '.')
		{
			iLen = i;//������С�������
			break;
		}
	}

	iLen -= 2;//��С100��
	unsigned int	uiValue=0;
	for(i=0; i<iLen; i++)
		if(pcValue[i] != ' ')
			break;

	for(; i<iLen; i++)
		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';

	return	uiValue;
}

unsigned int CCharTranslate::SumPriceToDigital1000(char* pcValue, int iLen)
{
	for(; iLen>0; iLen--)
		if(pcValue[iLen-1]!=' ')
			break;

	int i = iLen - 1;
	for(; i>=0; i--)
	{
		if(pcValue[i] == '.')
		{
			iLen = i;//������С�������
			break;
		}
	}

	iLen -= 3;//��С1000��
	unsigned int	uiValue=0;
	for(i=0; i<iLen; i++)
		if(pcValue[i] != ' ')
			break;

	for(; i<iLen; i++)
		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';

	return	uiValue;
}

//////////////////////////////////////////////////////////////////////////
//���ַ���pcValueת��Ϊ����, ������֮��
//////////////////////////////////////////////////////////////////////////
int CCharTranslate::CharToDigital(char* pcValue, int iLen)
{
	int uiValue = 0;
	for(; iLen>0; iLen--)
		if(pcValue[iLen-1] != ' ')
			break;

	int i=0;
	for(; i<iLen; i++)
		if(pcValue[i]!=' ')
			break;

	bool bNegative = false;
	for(; i<iLen; i++)
	{
		if(pcValue[i]=='-')
		{
			bNegative = true;
			continue;
		}

		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';
	}

	if(bNegative)
		return -uiValue;
	else
		return	uiValue;
}