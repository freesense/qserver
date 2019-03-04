//CharTranslate.h
//

#pragma once
////////////////////////////////////////////////////////////
class CCharTranslate
{
public:
	static unsigned int PriceToDigital(char* pcValue, int iLen);
	static int SignedPriceToDigital(char*	pcValue,int iLen);
	static unsigned int VolumeToDigital100(char* pcValue, int iLen);
	static unsigned int VolumeToDigital10000(char* pcValue, int iLen);
	static unsigned int VolumeToDigital(char* pcValue, int iLen);
	static unsigned int SumPriceToDigital100(char* pcValue, int iLen);
	static unsigned int SumPriceToDigital1000(char* pcValue, int iLen);
	static int CharToDigital(char* pcValue, int iLen);
};