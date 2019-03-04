//CharTranslate.cpp
//
//#include "stdafx.h"
#include "CharTranslate.h"

//////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//将DBF中价格字段值转换为数值，这里的价格字段在转换后相当于放大了1000倍
//对于没有小数点的其它字段来说转换后没有放大
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
//将DBF中价格字段值转换为数值，这里的价格字段在转换后相当于放大了1000倍,有正负之分
//对于没有小数点的其它字段来说转换后没有放大
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
//将DBF字段中成交量字符串转换为数字再除以100
//pcValue为要转换的字符串
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
	iLen -= 2;////缩小100倍
	for(; i<iLen; i++)
		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';

	return	uiValue;
}

//////////////////////////////////////////////////////////////////////////
//将DBF字段中成交量字符串转换为数字再除以10000
//pcValue为要转换的字符串
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
	iLen -= 4;////缩小10000倍
	for(; i<iLen; i++)
		if(pcValue[i] >= '0' && pcValue[i] <= '9')
			uiValue = uiValue * 10 + pcValue[i] - '0';

	return	uiValue;
}

//////////////////////////////////////////////////////////////////////////
//将DBF字段中成交量字符串转换为数字
//pcValue为要转换的字符串
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
//将DBF字段中成交金额字符串转换为数字再除以100
//pcValue为要转换的字符串
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
			iLen = i;//表明有小数点存在
			break;
		}
	}

	iLen -= 2;//缩小100倍
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
			iLen = i;//表明有小数点存在
			break;
		}
	}

	iLen -= 3;//缩小1000倍
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
//将字符串pcValue转换为数字, 有正负之分
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