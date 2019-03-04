#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif
#include "stdafx.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>

#include "errorlog.h"

class TErrorlog
{
public:
    TErrorlog();

    char m_errorlog[1024];
};

TErrorlog::TErrorlog()
{
    char temp[MAXPATH];
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char tempdir[MAXDIR];
    char file[MAXFILE];
    char ext[MAXEXT];
    int i;

#ifdef __BORLANDC__
    int fh;
    struct ftime f;
    struct date d;
#else
    time_t t;
    struct tm tmlocal, f_tmlocal;
    struct _stat buf;
#endif

    m_errorlog[0] = 0;

#ifdef __BORLANDC__
    fnsplit(Application->ExeName.c_str(), drive, dir, file, ext);
#else
    _splitpath(_pgmptr, drive, dir, file, ext);
#endif

    strcpy(tempdir, dir);
    strcpy(ext, ".log");

    for(i = 0; i < 2; i ++)
    {
        strcpy(dir, tempdir);
        if(i == 0)
            strcat(dir, "..\\log\\");

        fnmerge(m_errorlog, drive, dir, NULL, NULL);
        if(access(m_errorlog, 0) == 0)
            break;

        if(i == 1)
            return;
    }

    strcat(m_errorlog, file);
    strcat(m_errorlog, ext);

#ifdef __BORLANDC__
    getdate(&d);

    fh = ::open(m_errorlog, O_BINARY | O_RDONLY);
    if(fh == -1)
        return;

    getftime(fh, &f);
    close(fh);

    if(((int)(f.ft_year + 1980) == (int)d.da_year)
       && ((int)f.ft_month == (int)d.da_mon)
       && ((int)f.ft_day == (int)d.da_day))
        return;

    sprintf(ext, ".%d", f.ft_day);
    fnmerge(temp, drive, dir, file, ext);
#else
    t = ::time(&t);
    tmlocal = *(::localtime(&t));

    if (_stat(m_errorlog, &buf)==0)
	{
		f_tmlocal = *(::localtime(&(buf.st_ctime)));

		if((f_tmlocal.tm_year == tmlocal.tm_year) && (f_tmlocal.tm_mon == tmlocal.tm_mon) && (f_tmlocal.tm_mday == tmlocal.tm_mday))
			return;

		sprintf(ext, ".%d", f_tmlocal.tm_mday);
		_makepath(temp, drive, dir, file, ext);
	}
	else
		return ;
#endif

    unlink(temp);
    rename(m_errorlog, temp);;
}

static TErrorlog errorlog;

const char * GetErrorLogFileName()
{
    return errorlog.m_errorlog;
}

#ifdef __BORLANDC__
#pragma package(smart_init)
#endif
