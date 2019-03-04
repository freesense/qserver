
#ifndef __CHARACTER_CODING_H__
#define __CHARACTER_CODING_H__

/* This document is captured from http://www.gnu.org/software/libiconv/documentation/libiconv/iconv_open.3.html
The values permitted for fromcode and tocode and the supported combinations are system dependent.
For the libiconv library, the following encodings are supported, in all combinations.

European languages
ASCII, ISO-8859-{1,2,3,4,5,7,9,10,13,14,15,16}, KOI8-R, KOI8-U, KOI8-RU,
CP{1250,1251,1252,1253,1254,1257}, CP{850,866}, Mac{Roman,CentralEurope,Iceland,Croatian,Romania},
Mac{Cyrillic,Ukraine,Greek,Turkish}, Macintosh

Semitic languages
ISO-8859-{6,8}, CP{1255,1256}, CP862, Mac{Hebrew,Arabic}

Japanese
EUC-JP, SHIFT_JIS, CP932, ISO-2022-JP, ISO-2022-JP-2, ISO-2022-JP-1

Chinese
EUC-CN, HZ, GBK, CP936, GB18030, EUC-TW, BIG5, CP950, BIG5-HKSCS, BIG5-HKSCS:2001, BIG5-HKSCS:1999, ISO-2022-CN, ISO-2022-CN-EXT

Korean
EUC-KR, CP949, ISO-2022-KR, JOHAB

Armenian
ARMSCII-8

Georgian
Georgian-Academy, Georgian-PS

Tajik
KOI8-T
Kazakh
PT154, RK1048

Thai
TIS-620, CP874, MacThai

Laotian
MuleLao-1, CP1133

Vietnamese
VISCII, TCVN, CP1258

Platform specifics
HP-ROMAN8, NEXTSTEP

Full Unicode
UTF-8
UCS-2, UCS-2BE, UCS-2LE
UCS-4, UCS-4BE, UCS-4LE
UTF-16, UTF-16BE, UTF-16LE
UTF-32, UTF-32BE, UTF-32LE
UTF-7
C99, JAVA

Full Unicode, in terms of uint16_t or uint32_t
(with machine dependent endianness and alignment)
UCS-2-INTERNAL, UCS-4-INTERNAL

Locale dependent, in terms of char or wchar_t
(with machine dependent endianness and alignment, and with semantics depending on the OS and the current LC_CTYPE locale facet)
char, wchar_t

When configured with the option --enable-extra-encodings, it also provides support for a few extra encodings:
European languages
CP{437,737,775,852,853,855,857,858,860,861,863,865,869,1125}

Semitic languages
CP864
Japanese
EUC-JISX0213, Shift_JISX0213, ISO-2022-JP-3
Chinese
BIG5-2003 (experimental)

Turkmen
TDS565

Platform specifics
ATARIST, RISCOS-LATIN1
*/

#include "stdafx.h"
#include "commxapi.h"
#include <assert.h>

#ifdef _WIN32

#define CONV_UTF8		65001
#define CONV_GBK		936
#define CONV_BIG5		950
#define CONV_UTF16LE	0

int COMMXAPI code_convert(int eCodingSrc, const char *pcArraySrc, int iSizeSrcArray, int eCodingDst, char *pcArrayDst, int iSizeDstArray);

#elif defined _POSIX_C_SOURCE

#define CONV_UTF8		"utf-8"
#define CONV_GBK		"gbk"
#define CONV_BIG5		"big5"
#define CONV_UTF16LE	"utf_16_le"

int COMMXAPI code_convert(char *from_charset, const char *inbuf, int inlen, char *to_charset, char *outbuf, int outlen);

#endif

#define ICONV(CODING_FROM, SRC, SRCLEN, CODING_TO, DEST, DESTLEN) \
	code_convert((CODING_FROM), (SRC), (SRCLEN), (CODING_TO), (DEST), (DESTLEN))

#endif
