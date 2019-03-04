
#include "stdafx.h"
#include "../public/code_convert.h"

#ifdef _WIN32

int Unicode2MultiCharCoding(int eMultiCharCoding,
							LPCWSTR lpWideCharStr, int cchWideChar,
							LPSTR lpMultiByteStr, int cchMultiByte)
{
	return WideCharToMultiByte(eMultiCharCoding, 0, lpWideCharStr, cchWideChar, lpMultiByteStr, cchMultiByte, NULL, NULL);
}

int MultiCharCoding2Unicode(int eMultiCharCoding,
							LPCSTR lpMultiByteStr, int cchMultiByte,
							LPWSTR lpWideCharStr, int cchWideChar)
{
	return MultiByteToWideChar(eMultiCharCoding, 0, lpMultiByteStr, cchMultiByte, lpWideCharStr, cchWideChar);
}

int code_convert(int eCodingSrc, const char *pcArraySrc, int iSizeSrcArray, int eCodingDst, char *pcArrayDst, int iSizeDstArray)
{
	assert(NULL != pcArraySrc);
	assert(NULL != pcArrayDst);

	int bRet = -1;
	wchar_t *pwcTemp = new wchar_t[iSizeSrcArray + 1];
	memset(pwcTemp, 0, sizeof(wchar_t) * (iSizeSrcArray + 1));

	if (eCodingSrc == CONV_UTF16LE)
		bRet = Unicode2MultiCharCoding(eCodingDst, (wchar_t*)pcArraySrc, iSizeSrcArray, pcArrayDst, iSizeDstArray);
	else if (eCodingDst == CONV_UTF16LE)
		bRet = MultiCharCoding2Unicode(eCodingSrc, pcArraySrc, iSizeSrcArray, (wchar_t*)pcArrayDst, iSizeDstArray);
	else if (MultiCharCoding2Unicode(eCodingSrc, pcArraySrc, -1, pwcTemp, iSizeSrcArray + 1) != 0)
	{
		bRet = Unicode2MultiCharCoding(eCodingDst, pwcTemp, -1, pcArrayDst, iSizeDstArray);
		if (bRet == 0)
			bRet = -1;
	}

	delete []pwcTemp;
	return bRet;
}

#elif defined _POSIX_C_SOURCE

int code_convert(char *from_charset, const char *inbuf, int inlen, char *to_charset, char *outbuf, int outlen)
{
	char **pin = &inbuf;
	char **pout = &outbuf;

	iconv_t cd = iconv_open(to_charset, from_charset);
	if (cd == (iconv_t)(-1))
		return -1;
	memset(outbuf, 0, outlen);
	if (-1 == iconv(cd, pin, (size_t*)&inlen, pout, (size_t*)&outlen))
		return -1;
	iconv_close(cd);
	return outlen;
}

#endif
