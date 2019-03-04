
#include "stdafx.h"
#include "../public/commxapi.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
	#include <io.h>
	#include <sys/locking.h>
	#include <share.h>
#elif defined _POSIX_C_SOURCE
	#include <sys/time.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <iconv.h>
#endif

#pragma warning(disable:4267 4244)

#define MAX_PATH_LENGTH		1024						/**< 文件路径最大长度 */

void CommxVer(char** pBuf)
{
	*pBuf = (char*)gVer;
}

unsigned long GetNearPrime(unsigned long nNumber)
{
	static const unsigned long PRIME_LIST[] =
	{
		7ul,		 23ul,		  53ul,         97ul,         193ul,
		389ul,		 769ul,  	  1543ul,       3079ul,       6151ul,
		12289ul,	 24593ul,	  49157ul,      98317ul,      196613ul,
		393241ul,    786433ul,	  1572869ul,    3145739ul,    6291469ul,
		12582917ul,  25165843ul,  50331653ul,   100663319ul,  201326611ul,
		402653189ul, 805306457ul, 1610612741ul, 3221225473ul, 4294967291ul
	};
	static int nCount = sizeof(PRIME_LIST) / sizeof(unsigned long);

	for (int nIndex = 0; nIndex < nCount; nIndex++)
	{
		if (nNumber <= PRIME_LIST[nIndex])
			return PRIME_LIST[nIndex];
	}
	return PRIME_LIST[nCount - 1];
}

const char* GetExecutePath()
{
	static char szPath[MAX_PATH_LENGTH + 1];
	memset(szPath, 0x00, sizeof(szPath));
#ifdef _WIN32
	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileName(hModule, (LPTSTR)szPath, MAX_PATH_LENGTH);
#elif defined _POSIX_C_SOURCE
	char szLink[MAX_PATH_LENGTH + 1];
	sprintf(szLink, "/proc/%d/exe", getpid());
	int iLength = readlink(szLink, szPath, MAX_PATH_LENGTH);
	if (iLength != -1)
		szPath[iLength] = 0x00;
#endif
	return szPath;
}

const char * Trim(char *src)
{
	std::string s = src;
	unsigned int nBegin = s.find_first_not_of(" \r\n\t");
	unsigned int nEnd = s.find_last_not_of(" \r\n\t") + 1;
	if (nEnd == s.length())
		nEnd = std::string::npos;
	s = s.substr(nBegin, nEnd);
	strcpy((char*)src, s.c_str());
	return src;
}

char* memstr(void* buf, unsigned int buflen, void* lpfind, unsigned int findlen)
{
	unsigned int nPos = 0;
	while (1)
	{
		char* lpTmp = (char*)memchr(&((char*)buf)[nPos], ((char*)lpfind)[0], buflen - nPos);
		if (!lpTmp)
			return NULL;
		if (0 == memcmp(lpTmp, lpfind, findlen))
			return lpTmp;
		nPos = lpTmp - (char*)buf + 1;
		if (nPos >= buflen)
			return NULL;
	}
}

int aint(const char *as)
{
	char *ptmp = strlwr((char*)as);
	if (!strchr(ptmp, 'x') && !strchr(ptmp, 'X'))
		return atoi(ptmp);

	int len = strlen(ptmp), ir = 0, a;
	for (int i = len - 1; i >= 0; i--)
	{
		if (ptmp[i] >= '0' && ptmp[i] <= '9')
			a = ptmp[i] - '0';
		else if (ptmp[i] >= 'a' && ptmp[i] <= 'f')
			a = ptmp[i] - 'a' + 10;
		else
			return ir;
		ir += a * (int)(pow((double)16, (double)(len - i - 1)));
	}

	return ir;
}

int code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen)
{
#ifdef _WIN32
	return -1;
#elif defined _POSIX_C_SOURCE
	char **pin = &inbuf;
	char **pout = &outbuf;

	iconv_t cd = iconv_open(to_charset,from_charset);
	if (cd == (iconv_t)(-1))
		return -1;
	memset(outbuf, 0, outlen);
	if (-1 == iconv(cd, pin, (size_t*)&inlen, pout, (size_t*)&outlen))
		return -1;
	iconv_close(cd);
	return 0;
#endif
}

void GenRandom(char *lpBuf, unsigned int nLength)
{
	int count = nLength / sizeof(int);

	time_t t;
	time(&t);
	srand((unsigned int)t);

	int rnd;
	for (int i = 0; i < count; i++)
	{
		rnd = rand();
		memcpy(&lpBuf[i*sizeof(int)], &rnd, sizeof(rnd));
	}
}

int SetTime(unsigned long secs)
{
#ifdef _WIN32
	struct tm *t = localtime((time_t*)&secs);
	SYSTEMTIME st;
	st.wYear = t->tm_year + 1900;
	st.wMonth = t->tm_mon + 1;
	st.wDay = t->tm_mday;
	st.wDayOfWeek = t->tm_wday;
	st.wHour = t->tm_hour;
	st.wMinute = t->tm_min;
	st.wSecond = t->tm_sec;
	srand(secs);
	st.wMilliseconds = rand() % 1000;
	if (SetSystemTime(&st))
		return 0;
	return -1;
#else
#endif
}

/** @brief 锁定/解锁文件的某一部分
 *
 *	@param nLock 0解锁1加读锁2加写锁
 *  @return
 *	 - true		成功
 *	 - false	失败
 */
bool lockfile(int fd, int offset, int where, int nLock, int range, int wait)
{
	bool ret = false;
	time_t t1, t2;
	time(&t1);
	t2 = t1;

#ifdef _WIN32
	int filepos = _tell(fd);
	_lseek(fd, offset, where);
	while (!ret && (wait < 0 || difftime(t2, t1) < wait))
	{
		ret = _locking(fd, nLock ? _LK_NBLCK : _LK_UNLCK, range) == 0 ? true : false;
		time(&t2);
	}
	_lseek(fd, filepos, SEEK_SET);
#elif defined _POSIX_C_SOURCE
	flock lk;
	if (nLock == 0)
		lk.l_type = F_UNLCK;
	else if (nLock == 1)
		lk.l_type = F_RDLCK;
	else
		lk.l_type = F_WRLCK;
	lk.l_start = offset;
	lk.l_whence = where;
	lk.l_len = range;
	lk.l_pid = 0;
	while (!ret && (wait < 0 || difftime(t2, t1) < wait))
	{
		ret = fcntl(fd, F_SETLK, &lk) == 0 ? true : false;
		time(&t2);
	}
#endif

	return ret;
}

/////////////////////////////////////////////////////////////////////////
#ifdef _POSIX_C_SOURCE
char* strlwr(char* str)
{
	char* orig = str;
	for (; *str != 0x00; str++)
		*str = tolower(*str);
	return orig;
}

void Sleep(int t)
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = t*1000;
	::select(0, NULL, NULL, NULL, &timeout);
}
#endif
