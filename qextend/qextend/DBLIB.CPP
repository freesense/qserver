// Dblib.cpp: implementation of the CDblib class.
//
//////////////////////////////////////////////////////////////////////

//#ifdef __BORLANDC__
//#pragma warn +wucp
//#endif

#include "stdafx.h"
#include "errorlog.h"

#include "Dblib.h"
#include "time.h"
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>


#ifdef __BORLANDC__
#include <forms.hpp>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef DLL_DBLIB
p_dbgetuserdata CDblib::dbgetuserdata = NULL;
p_dbsetuserdata CDblib::dbsetuserdata = NULL;
#endif

HINSTANCE CDblib::hDLL = NULL;

void (* CDblib::MyMsgFunc)(CDbErrorInfo *) = NULL;

void CDblib::InstallMsgFunc(void (* MsgFunc)(CDbErrorInfo *))
{
    MyMsgFunc = MsgFunc;
}

INT SQLAPI CDblib::MyErrHandler(DBPROCESS *dbproc, INT severity, INT dberr, INT oserr, const char *dberrstr, const char *oserrstr)
{
//    FILE *fp;
//    time_t t;
//    struct tm * tmlocal;
    CDbErrorInfo * dbErrorInfo;
    CDbErrorInfo tempDbErrorInfo;

    if(MyMsgFunc != NULL)
    {
        memset(&tempDbErrorInfo, 0, sizeof(tempDbErrorInfo));
        tempDbErrorInfo.severity = severity;
        tempDbErrorInfo.dberr = dberr;
        tempDbErrorInfo.oserr = oserr;
        if(dberrstr)
            strncpy(tempDbErrorInfo.dberrstr, dberrstr, sizeof(tempDbErrorInfo.dberrstr));
        if(oserrstr)
            strncpy(tempDbErrorInfo.oserrstr, oserrstr, sizeof(tempDbErrorInfo.oserrstr));
        MyMsgFunc(&tempDbErrorInfo);
    }

    if(dbproc == NULL)
    {
        return(INT_CANCEL);
    }
    else
    {
        dbErrorInfo = (CDbErrorInfo *)dbgetuserdata(dbproc);
        if(dbErrorInfo != NULL && severity >= dbErrorInfo->severity)
        {
            dbErrorInfo->severity = severity;
            dbErrorInfo->dberr = dberr;
            dbErrorInfo->oserr = oserr;
            if(dberrstr)
                strncpy(dbErrorInfo->dberrstr, dberrstr, sizeof(dbErrorInfo->dberrstr) - 1);
            if(oserrstr)
                strncpy(dbErrorInfo->oserrstr, oserrstr, sizeof(dbErrorInfo->oserrstr) - 1);
        }
        if (oserr != DBNOERR || dberr != DBNOERR)
        {
//            t = ::time(&t);
//            tmlocal = ::localtime(&t);
//
//            fp = fopen(GetErrorLogFileName(), "a");
//            if(fp != NULL)
//            {
//                fprintf(fp, "%04d-%02d-%02d [%02d:%02d:%02d] ", tmlocal->tm_year + 1900, tmlocal->tm_mon + 1, tmlocal->tm_mday, tmlocal->tm_hour, tmlocal->tm_min, tmlocal->tm_sec);
//                if(oserrstr)
//                    fprintf(fp, "Operating-system error:%s\n", oserrstr);
//                if(dberrstr)
//                    fprintf(fp,"Dblibrary error:%s\n", dberrstr);
//                fclose(fp);
//            }
        }
        return(INT_CANCEL);
    }
#ifndef __BORLANDC__
    return(INT_CANCEL);
#endif
}

INT SQLAPI CDblib::MyMsgHandler(DBPROCESS *dbproc, DBINT msgno, INT msgstate, INT severity,
              const CHAR *msgtext, const CHAR * s1, const CHAR * s2, DBUSMALLINT dbus)
{
//    FILE *fp;
//    time_t t;
//    struct tm * tmlocal;
    CDbErrorInfo * dbErrorInfo;
    CDbErrorInfo tempDbErrorInfo;

    if(MyMsgFunc != NULL)
    {
        memset(&tempDbErrorInfo, 0, sizeof(tempDbErrorInfo));
        tempDbErrorInfo.severity = severity;
        tempDbErrorInfo.msgno = msgno;
        if(msgtext)
            strncpy(tempDbErrorInfo.msgtext, msgtext, sizeof(tempDbErrorInfo.msgtext));
        MyMsgFunc(&tempDbErrorInfo);
    }

    dbErrorInfo = (CDbErrorInfo *)dbgetuserdata(dbproc);
    if(dbErrorInfo != NULL && severity >= dbErrorInfo->severity)
    {
        dbErrorInfo->msgno = msgno;
        dbErrorInfo->severity = severity;
        sprintf(dbErrorInfo->msgtext, "message NO %ld, severity %ld: ", msgno, severity);
        if(msgtext)
            strncpy(dbErrorInfo->msgtext + strlen(dbErrorInfo->msgtext),
                    msgtext,
                    sizeof(dbErrorInfo->msgtext) - strlen(dbErrorInfo->msgtext) - 1);
    }

    if(severity > 10)
    {
//        t = ::time(&t);
//        tmlocal = ::localtime(&t);
//
//        fp = fopen(GetErrorLogFileName(), "a");
//        if(fp != NULL)
//        {
//            fprintf(fp, "%04d-%02d-%02d [%02d:%02d:%02d] ", tmlocal->tm_year + 1900, tmlocal->tm_mon + 1, tmlocal->tm_mday, tmlocal->tm_hour, tmlocal->tm_min, tmlocal->tm_sec);
//            fprintf(fp, "SQL Server Message: message NO %ld, severity %ld\n", msgno, severity);
//            fprintf(fp, "%s\n", msgtext);
//            fclose(fp);
//        }
    }
    return 0 ;
}

void CDblib::WindowMsgFunc(CDbErrorInfo * info)
{
#ifdef __BORLANDC__
    char buffer[1024];

    if(info->severity > 0 && info->msgno > 0)
    {
        sprintf(buffer, "severity = %d, msgno = %d\n\n%s\n\n", info->severity, info->msgno, info->msgtext);
        MessageBox(Screen->ActiveForm->Handle, buffer, "数据库访问错误", MB_OK | MB_ICONSTOP);
    }
    else if(info->severity > 0 && (info->dberr > 0 || info->oserr > 0))
    {
        sprintf(buffer, "severity = %d, dberr = %d, oserr = %d\n\n%s\n\n%s\n", info->severity, info->dberr, info->oserr, info->dberrstr, info->oserrstr);
        MessageBox(Screen->ActiveForm->Handle, buffer, "数据库访问错误", MB_OK | MB_ICONSTOP);
    }
#endif
}

CDblib::CDblib(bool bcpOK)
{
    m_bcpOK = bcpOK;

    LoadFunc();
    dbinit();
    dbsetlogintime(10);

    dberrhandle(CDblib::MyErrHandler);
    dbmsghandle(CDblib::MyMsgHandler);
    Init();
}

CDblib::~CDblib()
{
    Close();
}

bool CDblib::Init()
{
    m_dbmsOsType = otNT;
    m_dbmsVersion = dvSQL65;

    m_dbProcess = NULL;
    m_loginOK = false;
    memset(m_serverName, 0, sizeof(m_serverName));
    memset(m_loginID, 0, sizeof(m_loginID));
    memset(m_passWord, 0, sizeof(m_passWord));
    memset(m_appName, 0, sizeof(m_appName));
    memset(m_hostName, 0, sizeof(m_hostName));
    memset(m_databaseName, 0, sizeof(m_databaseName));
    m_rowCount = 0;
    m_colCount = 0;
    m_errorCode = 0;
    memset(m_errorMsg, 0, sizeof(m_errorMsg));
    memset(m_colInfo, 0, sizeof(m_colInfo));

    if(dbgetmaxprocs() <= 45)
        dbsetmaxprocs (45);

    return true;
}

bool CDblib::Login(const char *servername, const char *loginID, const char *passWord, const char *appname, const char *hostname)
{
    LOGINREC * loginRec;
    char version[81];

    loginRec = dblogin();

    DBSETLUSER(loginRec, (char *)loginID);
    DBSETLPWD(loginRec, (char *)passWord);
    DBSETLHOST(loginRec, (char *)hostname);
    DBSETLAPP(loginRec, (char *)appname);

    if(m_bcpOK)
        BCP_SETL(loginRec, TRUE);

    m_dbProcess = dbopen(loginRec, (char *)servername);
    dbfreelogin(loginRec);

    if(m_dbProcess == NULL)
    {
        m_errorCode = -1;
        strcpy(m_errorMsg, "连接SQL Server失败");
        m_loginOK = false;
        return false;
    }

    version[0] = 0;
    dbcmd(m_dbProcess, "select upper(substring(@@version, 1, 80))");
    dbsqlexec(m_dbProcess);
    while( dbresults(m_dbProcess ) != NO_MORE_RESULTS )
    {
        if(DBROWS(m_dbProcess) == SUCCEED)
        {
            dbbind(m_dbProcess, 1, NTBSTRINGBIND, 0, (BYTE *)version);
            while(dbnextrow(m_dbProcess) != NO_MORE_ROWS);
        }
    }
    if(strstr(version, "MICROSOFT SQL") == 0)
    {
        if(strstr(version, "/11.") == 0)
            m_dbmsVersion = dvSystem10;
        else
            m_dbmsVersion = dvSystem11;
    }
    else
    {
        if(strstr(version, "7.0"))
            m_dbmsVersion = dvSQL70;
		else if (strstr(version, "2000"))
			m_dbmsVersion = dvSQL2000;
        else
            m_dbmsVersion = dvSQL65;
    }

    if(strstr(version, "UNIX"))
        m_dbmsOsType = otUnix;
    else if(strstr(version, "NOVELL"))
        m_dbmsOsType = otNovell;

    memset(&m_dbErrorInfo, 0, sizeof(CDbErrorInfo));
    dbsetuserdata(m_dbProcess, &m_dbErrorInfo);

    if(m_dbmsVersion & dvSybase)
        dbcmd(m_dbProcess, "set transaction isolation level 1");
    else
        dbcmd(m_dbProcess, "set transaction isolation level read uncommitted");
    dbsqlexec(m_dbProcess);
    while(dbresults(m_dbProcess) != NO_MORE_RESULTS)
        while(dbnextrow(m_dbProcess) != NO_MORE_ROWS);

    if(m_databaseName[0])
        dbuse(m_dbProcess, m_databaseName);

    return true;
}

bool CDblib::Open(const char *servername, const char *loginID, const char *passWord, const char *appname, const char *hostname)
{
    if(m_loginOK == true)
        return true;

    if(Login(servername, loginID, passWord, appname, hostname) == false)
        return false;
        
    strcpy(m_serverName, servername);
    strcpy(m_loginID, loginID);
    strcpy(m_passWord, passWord);
	if (appname)
		strcpy(m_appName, appname);
	if (hostname)
		strcpy(m_hostName, hostname);
    strcpy(m_databaseName, dbname(m_dbProcess));

    m_errorCode = 0;
    strcpy(m_errorMsg, "连接SQL Server成功");
    m_loginOK = true;
    return true;
}

bool CDblib::__Run(const char *CommandStr)
{
    int retryCount = 3;
	FILE *fp;
	time_t t;
	struct tm * tmlocal;

#ifndef __BORLANDC
    Cancel();
#endif

__deadlock:

    memset(&m_dbErrorInfo, 0, sizeof(CDbErrorInfo));

    if(dbcmd(m_dbProcess, (char *)CommandStr) == FAIL)
    {
        if(m_dbProcess == NULL || dbdead(m_dbProcess) == TRUE)
        {
            Login(m_serverName, m_loginID, m_passWord, m_appName, m_hostName);
        }
        m_errorCode = m_dbErrorInfo.msgno;
        strcpy(m_errorMsg, "执行存储过程失败");
        return false;
    }

    if(dbsqlexec(m_dbProcess) == FAIL)
    {
        if(m_dbProcess == NULL || dbdead(m_dbProcess) == TRUE)
        {
            Login(m_serverName, m_loginID, m_passWord, m_appName, m_hostName);
        }
        else if(m_dbErrorInfo.msgno == 1205)
        {
            Sleep(100);
            retryCount --;

           t = ::time(&t);
           tmlocal = ::localtime(&t);

           fp=fopen(GetErrorLogFileName(), "a");
		   if (fp!=NULL)
		   {
			   fprintf(fp, "%04d-%02d-%02d [%02d:%02d:%02d] ", tmlocal->tm_year + 1900, tmlocal->tm_mon + 1, tmlocal->tm_mday, tmlocal->tm_hour, tmlocal->tm_min, tmlocal->tm_sec);
			   fprintf(fp, "SQL Server Message: 1205\n");
			   fprintf(fp, CommandStr);
			   fclose(fp);
		   }

            goto __deadlock;
        }

        m_errorCode = m_dbErrorInfo.msgno;
        strcpy(m_errorMsg, "执行存储过程失败");
        Cancel();
        return false;
    }
    else
    {
        m_errorCode = 0;
        strcpy(m_errorMsg, "执行存储过程成功");
        if (dbresults(m_dbProcess) != NO_MORE_RESULTS)
           Bind();
    }
    return true;
}

bool CDblib::Run(const char *CommandStr, ...)
{
    char msg[2048];
    va_list arg_ptr;

    va_start(arg_ptr, CommandStr);
    vsprintf(msg, CommandStr, arg_ptr);
    va_end(arg_ptr);

    return __Run(msg);
}

bool CDblib::__Run20(const char *CommandStr)
{
    unsigned int len;
    char _sqlcmd[2048];
    char cznr1[256], cznr2[256], cznr3[256], cznr4[256];

    len = (unsigned int)strlen(CommandStr);
    if(len <= 250)
    {
      strcpy(cznr1, CommandStr);
      cznr2[0] = 0;
      cznr3[0] = 0;
      cznr4[0] = 0;
    }
    else if(len <= 500)
    {
      memcpy(cznr1, CommandStr, 250);
      cznr1[250] = 0;

      memcpy(cznr2, CommandStr + 250, len - 250);
      cznr2[250] = 0;

      cznr3[0] = 0;
      cznr4[0] = 0;
    }
    else if(len <= 750)
    {
      memcpy(cznr1, CommandStr, 250);
      cznr1[250] = 0;

      memcpy(cznr2, CommandStr + 250, 250);
      cznr2[250] = 0;

      memcpy(cznr3, CommandStr + 500, len - 500);
      cznr3[250] = 0;

      cznr4[0] = 0;
    }
    else if(len <= 1000)
    {
      memcpy(cznr1, CommandStr, 250);
      cznr1[250] = 0;

      memcpy(cznr2, CommandStr + 250, 250);
      cznr2[250] = 0;

      memcpy(cznr3, CommandStr + 500, 250);
      cznr3[250] = 0;

      memcpy(cznr4, CommandStr + 750, len - 750);
      cznr4[250] = 0;
    }
    else
    {
        strcpy(cznr1, "**********");
        strcpy(cznr2, "**********");
        strcpy(cznr3, "**********");
        strcpy(cznr4, "**********");
    }

    strcpy(_sqlcmd, CommandStr);
    strcat(_sqlcmd, ",\x22\x24");
    strcat(_sqlcmd, cznr1);
    strcat(_sqlcmd, "\x24\x22");
    strcat(_sqlcmd, ",\x22\x24");
    strcat(_sqlcmd, cznr2);
    strcat(_sqlcmd, "\x24\x22");
    strcat(_sqlcmd, ",\x22\x24");
    strcat(_sqlcmd, cznr3);
    strcat(_sqlcmd, "\x24\x22");
    strcat(_sqlcmd, ",\x22\x24");
    strcat(_sqlcmd, cznr4);
    strcat(_sqlcmd, "\x24\x22");

    if(__Run(_sqlcmd) == true)
    {
        while(dbresults(m_dbProcess) != NO_MORE_RESULTS)
        {
            if(DBROWS(m_dbProcess) == SUCCEED)
            {
                dbbind(m_dbProcess, 1, INTBIND, 0, (BYTE *)&m_errorCode);
                dbbind(m_dbProcess, 2, NTBSTRINGBIND, 0, (BYTE *)m_errorMsg);
                while(dbnextrow(m_dbProcess) != NO_MORE_ROWS);
                if (m_errorCode == 0)
                {
                    while(dbresults(m_dbProcess) != NO_MORE_RESULTS)
                    {
                        if(dbnumcols(m_dbProcess) > 0)
                        {
                            Bind();
                            break;
                        }
                    }

                    memset(m_data, 0, sizeof(m_data));
                }
                break;
            }
        }
    }
    else
      return false;

    return true;
}

bool CDblib::Run20(const char *CommandStr, ...)
{
    char msg[1024];
    va_list arg_ptr;

    memset(msg, '@', sizeof(msg));
    va_start(arg_ptr, CommandStr);
    vsprintf(msg, CommandStr, arg_ptr);
    va_end(arg_ptr);

    return __Run20(msg);
}

bool CDblib::Run20Error(const char *CommandStr, ...)
{
#ifdef __BORLANDC__
    char msg[1024];
    va_list arg_ptr;

    memset(msg, '@', sizeof(msg));
    va_start(arg_ptr, CommandStr);
    vsprintf(msg, CommandStr, arg_ptr);
    va_end(arg_ptr);

    __Run20(msg);

    if(GetErrorCode() == 0)
    {
        return true;
    }
    else
    {
        MessageBox(Screen->ActiveForm->Handle, GetErrorMsg(), "系统提示信息", MB_OK | MB_ICONSTOP);
        return false;
    }
#else
    return false;
#endif
}

bool CDblib::Run20Message(const char *CommandStr, ...)
{
#ifdef __BORLANDC__
    char msg[1024];
    va_list arg_ptr;

    memset(msg, '@', sizeof(msg));
    va_start(arg_ptr, CommandStr);
    vsprintf(msg, CommandStr, arg_ptr);
    va_end(arg_ptr);

    __Run20(msg);

    if(GetErrorCode() == 0)
    {
        MessageBox(Screen->ActiveForm->Handle, GetErrorMsg(), "系统提示信息", MB_OK | MB_ICONINFORMATION);
        return true;
    }
    else
    {
        MessageBox(Screen->ActiveForm->Handle, GetErrorMsg(), "系统提示信息", MB_OK | MB_ICONSTOP);
        return false;
    }
#else
    return false;
#endif
}

bool CDblib::Bind()
{
    hasExtendColumn = false;
    m_colCount = (USHORT)dbnumcols(m_dbProcess);

    for(int i = 0; i < m_colCount; i++)
    {
//        dbcolinfo(m_dbProcess, CI_REGULAR, i + 1, i + 1, &col[i]);

        strcpy(m_colInfo[i].Name, dbcolname(m_dbProcess, i + 1));
        m_colInfo[i].Type = (SHORT)dbcoltype(m_dbProcess, i + 1);
        m_colInfo[i].MaxLength = dbcollen(m_dbProcess, i + 1);

        switch (m_colInfo[i].Type)
        {
            case SQLBIT:
            case SQLINT1:
                m_colInfo[i].MaxLength = 4;
                dbbind(m_dbProcess, i + 1, NTBSTRINGBIND, (DBINT)0, (BYTE *)m_data[i]);
                break;
            case SQLINT2:
                m_colInfo[i].MaxLength = 6;
                dbbind(m_dbProcess, i + 1, NTBSTRINGBIND, (DBINT)0, (BYTE *)m_data[i]);
                break;
            case SQLINT4:
                m_colInfo[i].MaxLength = 11;
                dbbind(m_dbProcess, i + 1, NTBSTRINGBIND, (DBINT)0, (BYTE *)m_data[i]);
                break;
            case SQLFLT4:
            case SQLFLT8:
            case SQLFLTN:
                m_colInfo[i].MaxLength = 16;
                dbbind(m_dbProcess, i + 1, NTBSTRINGBIND, (DBINT)0, (BYTE *)m_data[i]);
                break;
            case SQLMONEY4:
            case SQLMONEY:
            case SQLMONEYN:
            case SQLDECIMAL:
            case SQLNUMERIC:
                hasExtendColumn = true;
                m_colInfo[i].MaxLength = 16;
                dbbind(m_dbProcess, i + 1, FLT8BIND, (DBINT)0, (BYTE *)m_data[i]);
                break;
            case SQLDATETIME:
            case SQLDATETIMN:
            case SQLDATETIM4:
                hasExtendColumn = true;
                m_colInfo[i].MaxLength = 17;
                dbbind(m_dbProcess, i + 1, DATETIMEBIND, (DBINT)0, (BYTE *)m_data[i]);
                break;
            case SQLBINARY:
            case SQLVARBINARY:
                hasExtendColumn = true;
//                dbbind(m_dbProcess, i + 1, NTBSTRINGBIND, DBMAXCHAR, (BYTE *)m_data[i]);
                dbbind(m_dbProcess, i + 1, VARYBINBIND, DBMAXCHAR + 2, (BYTE *)m_data[i]);
                break;
            default:
                dbbind(m_dbProcess, i + 1, NTBSTRINGBIND, DBMAXCHAR, (BYTE *)m_data[i]);
                break;
        }
    }
    return true;
}

bool CDblib::MoveNext()
{
    double tempFloat;
    DBCHAR tempStr[DBMAXCHAR];
    DBINT ret;
    struct dbdaterec tempDate;

    ret = dbnextrow(m_dbProcess);
    if(ret == FAIL)
        return false;
    else if (ret == NO_MORE_ROWS)
    {
        while (dbresults(m_dbProcess) != NO_MORE_RESULTS)
        {
            memset(m_data, 0, sizeof(m_data));

            if(DBROWS(m_dbProcess)==SUCCEED)
            {
                Bind();
                ret = dbnextrow(m_dbProcess);
                if(ret == FAIL)
                    return false;
                else if(ret != NO_MORE_ROWS)
                    goto __fmt;
            }
        }
        if(dbchange(m_dbProcess))
            strcpy(m_databaseName, dbname(m_dbProcess));

        return false;
    }

__fmt:

    m_rowCount ++;

    if(hasExtendColumn == false)
        return true;

    for(int i = 0; i < m_colCount; i++)
    {
        switch(m_colInfo[i].Type)
        {
            case SQLMONEY4:
            case SQLMONEY:
            case SQLMONEYN:
                tempFloat = *(double *)m_data[i];
                sprintf(m_data[i], "%.3f", tempFloat);
                break;
            case SQLVARBINARY:
            case SQLBINARY:
//                dbconvert(m_dbProcess, m_colInfo[i].Type, ((DBVARYBIN *)m_data[i])->array, ((DBVARYBIN *)m_data[i])->len, SQLCHAR, tempStr, sizeof(tempStr));
                dbconvert(m_dbProcess, m_colInfo[i].Type, ((DBVARYBIN *)m_data[i])->array, min((int)(((DBVARYBIN *)m_data[i])->len), (DBMAXCHAR - 3) / 2), SQLCHAR, (BYTE *)tempStr, -1);
//                tempStr[((sizeof(m_data[i]) - 3) / 2) * 2] = 0;
                strcpy(m_data[i], "0x");
                strcat(m_data[i], tempStr);
                break;
            case SQLDATETIM4:
            case SQLDATETIME:
            case SQLDATETIMN:
                dbdatecrack(m_dbProcess, &tempDate, (DBDATETIME *) m_data[i]);
                sprintf(m_data[i], "%04d%02d%02d %02d:%02d:%02d",
                    tempDate.year, tempDate.month, tempDate.day,
                    tempDate.hour, tempDate.minute, tempDate.second);
                break;
        }
    }
    return true;
}

bool CDblib::FirstColInfo()
{
    return true;
}

bool CDblib::Use(const char * databaseName)
{
  m_errorCode = 0;
  strcpy(m_errorMsg, "执行存储过程成功");
  memset(&m_dbErrorInfo, 0, sizeof(CDbErrorInfo));

  dbuse(m_dbProcess, (char *)databaseName);
  m_errorCode = dbretstatus(m_dbProcess);
  if(m_errorCode == 0)
      strcpy(m_databaseName, dbname(m_dbProcess));
  return true;
}

bool CDblib::Cancel()
{
    if(m_dbmsVersion & dvSybase)
    {
        while( dbresults(m_dbProcess ) != NO_MORE_RESULTS )
            while(dbnextrow(m_dbProcess) != NO_MORE_ROWS);
    }
    else
        dbcancel(m_dbProcess);
    return false;
}

bool CDblib::Close()
{
    if(m_dbProcess)
        dbclose(m_dbProcess);
    Init();
    return true;
}

const SDBCOL * CDblib::GetColInfo(int index)
{
  if(m_errorCode != 0 || m_colCount <= 0 || index >= m_colCount)
    return (const SDBCOL *)NULL;
  return &m_colInfo[index];
}

const SDBCOL * CDblib::GetColInfo(const char * colName)
{
  int index;

  for(index = 0; index < m_colCount; index ++)
  {
      if(stricmp(m_colInfo[index].Name, colName) == 0)
          return GetColInfo(index);
  }

  return (const SDBCOL *)NULL;
}

char * CDblib::GetColData(int index)
{
  if(m_errorCode != 0 || m_colCount <= 0 || index >= m_colCount)
    return (char *)"";
  return m_data[index];
}

char * CDblib::GetColData(const char * colName)
{
  int index;

  for(index = 0; index < m_colCount; index ++)
  {
      if(stricmp(m_colInfo[index].Name, colName) == 0)
          return GetColData(index);
  }

  return (char *)"";
}

USHORT CDblib::GetServerList(CServerList * serverList)
{
    long ret;
    char * head = 0, * tail = 0;
    char allSrvName[1024];
    USHORT i;
    unsigned short srvNumber = 0;
    unsigned short validSrvNumber = 0;

    ret = dbserverenum(NET_SEARCH | LOC_SEARCH, allSrvName, 500, &srvNumber);
    if((ret == ENUM_SUCCESS || ret == MORE_DATA) && srvNumber > 0)
    {
        head = allSrvName;
        for(i = 0; i < srvNumber; i ++)
        {
            tail = strchr(head, 0);
            if(head[0] != 0)
            {
                memccpy(serverList[validSrvNumber].ServerName, head, (int)(tail - head + 1), 31);
                serverList[validSrvNumber].ServerName[MAXNAME - 1] = 0;
                validSrvNumber ++;
            }
            head = tail + 1;
        }
        return validSrvNumber;
    }
    return 0;
}

bool CDblib::LoadFunc()
{
#ifdef DLL_DBLIB
    if(!CDblib::hDLL)
    {
         CDblib::hDLL = LoadLibrary("NTWDBLIB.DLL");
         if (!CDblib::hDLL)
            {
                return false;
            }
    }

    //dbsetlogintime
    dbsetlogintime = (p_dbsetlogintime)GetProcAddress(CDblib::hDLL, "dbsetlogintime");
    if (!dbsetlogintime)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbmsghandle
    dbmsghandle = (p_dbmsghandle)GetProcAddress(CDblib::hDLL, "dbmsghandle");
    if (!dbmsghandle)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbinit
    dbinit = (p_dbinit)GetProcAddress(CDblib::hDLL, "dbinit");
    if (!dbinit)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dblogin
    dblogin = (p_dblogin)GetProcAddress(CDblib::hDLL, "dblogin");
    if (!dblogin)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbopen
    dbopen = (p_dbopen)GetProcAddress(CDblib::hDLL, "dbopen");
    if (!dbopen)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbsetlname
    dbsetlname = (p_dbsetlname)GetProcAddress(CDblib::hDLL, "dbsetlname");
    if (!dbsetlname)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbuse
    dbuse = (p_dbuse)GetProcAddress(CDblib::hDLL, "dbuse");
    if (!dbuse)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcmd
    dbcmd = (p_dbcmd)GetProcAddress(CDblib::hDLL, "dbcmd");
    if (!dbcmd)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcmdrow
    dbcmdrow = (p_dbcmdrow)GetProcAddress(CDblib::hDLL, "dbcmdrow");
    if (!dbcmdrow)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbsqlexec
    dbsqlexec = (p_dbsqlexec)GetProcAddress(CDblib::hDLL, "dbsqlexec");
    if (!dbsqlexec)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbsqlok
    dbsqlok = (p_dbsqlok)GetProcAddress(CDblib::hDLL, "dbsqlok");
    if (!dbsqlok)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbsqlsend
    dbsqlsend = (p_dbsqlsend)GetProcAddress(CDblib::hDLL, "dbsqlsend");
    if (!dbsqlsend)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //winexit
    dbwinexit = (p_dbwinexit)GetProcAddress(CDblib::hDLL, "dbwinexit");
    if (!dbwinexit)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbresults
    dbresults = (p_dbresults)GetProcAddress(CDblib::hDLL, "dbresults");
    if (!dbresults)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbnextrow
    dbnextrow = (p_dbnextrow)GetProcAddress(CDblib::hDLL, "dbnextrow");
    if (!dbnextrow)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbbind
    dbbind = (p_dbbind)GetProcAddress(CDblib::hDLL, "dbbind");
    if (!dbbind)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcancel
    dbcancel = (p_dbcancel)GetProcAddress(CDblib::hDLL, "dbcancel");
    if (!dbcancel)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbclose
    dbclose = (p_dbclose)GetProcAddress(CDblib::hDLL, "dbclose");
    if (!dbclose)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbclrbuf
    dbclrbuf = (p_dbclrbuf)GetProcAddress(CDblib::hDLL, "dbclrbuf");
    if (!dbclrbuf)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcollen
    dbcollen = (p_dbcollen)GetProcAddress(CDblib::hDLL, "dbcollen");
    if (!dbcollen)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcolname
    dbcolname = (p_dbcolname)GetProcAddress(CDblib::hDLL, "dbcolname");
    if (!dbcolname)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcoltype
    dbcoltype = (p_dbcoltype)GetProcAddress(CDblib::hDLL, "dbcoltype");
    if (!dbcoltype)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcolntype
    dbcolntype = (p_dbcolntype)GetProcAddress(CDblib::hDLL, "dbcolntype");
    if (!dbcolntype)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcolutype
    dbcolutype = (p_dbcolutype)GetProcAddress(CDblib::hDLL, "dbcolutype");
    if (!dbcolutype)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcount
    dbcount = (p_dbcount)GetProcAddress(CDblib::hDLL, "dbcount");
    if (!dbcount)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbdata
    dbdata = (p_dbdata)GetProcAddress(CDblib::hDLL, "dbdata");
    if (!dbdata)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbdataready
    dbdataready = (p_dbdataready)GetProcAddress(CDblib::hDLL, "dbdataready");
    if (!dbdataready)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbdatlen
    dbdatlen = (p_dbdatlen)GetProcAddress(CDblib::hDLL, "dbdatlen");
    if (!dbdatlen)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbdatecrack
    dbdatecrack = (p_dbdatecrack)GetProcAddress(CDblib::hDLL, "dbdatecrack");
    if (!dbdatecrack)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbdead
    dbdead = (p_dbdead)GetProcAddress(CDblib::hDLL, "dbdead");
    if (!dbdead)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dberrhandle
    dberrhandle = (p_dberrhandle)GetProcAddress(CDblib::hDLL, "dberrhandle");
    if (!dberrhandle)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }

    /*//dbprocerrhandle
    dbprocerrhandle = (p_db)GetProcAddress(CDblib::hDLL, "dbprocerrhandle");
    if (!dbprocerrhandle)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbprocmsghandle
    dbprocmsghandle = (p_db)GetProcAddress(CDblib::hDLL, "dbprocmsghandle");
    if (!dbprocmsghandle)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    } */
    //dbfcmd
    dbfcmd = (p_dbfcmd)GetProcAddress(CDblib::hDLL, "dbfcmd");
    if (!dbfcmd)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbfirstrow
    dbfirstrow = (p_dbfirstrow)GetProcAddress(CDblib::hDLL, "dbfirstrow");
    if (!dbfirstrow)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dblastrow
    dblastrow = (p_dblastrow)GetProcAddress(CDblib::hDLL, "dblastrow");
    if (!dblastrow)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
/*    //dbiscount
    dbiscount = (p_dbiscount)GetProcAddress(CDblib::hDLL, "dbiscount");
    if (!dbiscount)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }*/
    //dbnumcols
    dbnumcols = (p_dbnumcols)GetProcAddress(CDblib::hDLL, "dbnumcols");
    if (!dbnumcols)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbnumorders
    dbnumorders = (p_dbnumorders)GetProcAddress(CDblib::hDLL, "dbnumorders");
    if (!dbnumorders)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbnumrets
    dbnumrets = (p_dbnumrets)GetProcAddress(CDblib::hDLL, "dbnumrets");
    if (!dbnumrets)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbordercol
    dbordercol = (p_dbordercol)GetProcAddress(CDblib::hDLL, "dbordercol");
    if (!dbordercol)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbrows
    dbrows = (p_dbrows)GetProcAddress(CDblib::hDLL, "dbrows");
    if (!dbrows)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbrowtype
    dbrowtype = (p_dbrowtype)GetProcAddress(CDblib::hDLL, "dbrowtype");
    if (!dbrowtype)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbretstatus
    dbretstatus = (p_dbretstatus)GetProcAddress(CDblib::hDLL, "dbretstatus");
    if (!dbretstatus)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbexit
    dbexit = (p_dbexit)GetProcAddress(CDblib::hDLL, "dbexit");
    if (!dbexit)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbserverenum
    dbserverenum = (p_dbserverenum)GetProcAddress(CDblib::hDLL, "dbserverenum");
    if (!dbserverenum)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbcolinfo
    dbcolinfo = (p_dbcolinfo)GetProcAddress(CDblib::hDLL, "dbcolinfo");
    if (!dbcolinfo)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbmsghandle
    dbmsghandle = (p_dbmsghandle)GetProcAddress(CDblib::hDLL,"dbmsghandle");
    if (!dbmsghandle)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbfreelogin
    dbfreelogin = (p_dbfreelogin)GetProcAddress(CDblib::hDLL,"dbfreelogin");
    if (!dbfreelogin)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbgetuserdata
    CDblib::dbgetuserdata = (p_dbgetuserdata)GetProcAddress(CDblib::hDLL,"dbgetuserdata");
    if (!CDblib::dbgetuserdata)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbsetuserdata
    CDblib::dbsetuserdata = (p_dbsetuserdata)GetProcAddress(CDblib::hDLL,"dbsetuserdata");
    if (!CDblib::dbsetuserdata)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbgetmaxprocs
    dbgetmaxprocs = (p_dbgetmaxprocs)GetProcAddress(CDblib::hDLL,"dbgetmaxprocs");
    if (!dbgetmaxprocs)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbsetmaxprocs
    dbsetmaxprocs = (p_dbsetmaxprocs)GetProcAddress(CDblib::hDLL,"dbsetmaxprocs");
    if (!dbsetmaxprocs)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbname
    dbname = (p_dbname)GetProcAddress(CDblib::hDLL,"dbname");
    if (!dbname)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //dbchange
    dbchange = (p_dbchange)GetProcAddress(CDblib::hDLL,"dbchange");
    if (!dbchange)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }

    //bcp_init
    bcp_init = (p_bcp_init)GetProcAddress(CDblib::hDLL,"bcp_init");
    if (!bcp_init)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_exec
    bcp_exec = (p_bcp_exec)GetProcAddress(CDblib::hDLL,"bcp_exec");
    if (!bcp_exec)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_readfmt
    bcp_readfmt = (p_bcp_readfmt)GetProcAddress(CDblib::hDLL,"bcp_readfmt");
    if (!bcp_readfmt)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_writefmt
    bcp_writefmt = (p_bcp_writefmt)GetProcAddress(CDblib::hDLL,"bcp_writefmt");
    if (!bcp_writefmt)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_colfmt
    bcp_colfmt = (p_bcp_colfmt)GetProcAddress(CDblib::hDLL,"bcp_colfmt");
    if (!bcp_colfmt)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_bind
    bcp_bind = (p_bcp_bind)GetProcAddress(CDblib::hDLL,"bcp_bind");
    if (!bcp_bind)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_sendrow
    bcp_sendrow = (p_bcp_sendrow)GetProcAddress(CDblib::hDLL,"bcp_sendrow");
    if (!bcp_sendrow)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_batch
    bcp_batch = (p_bcp_batch)GetProcAddress(CDblib::hDLL,"bcp_batch");
    if (!bcp_batch)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
    //bcp_done
    bcp_done = (p_bcp_done)GetProcAddress(CDblib::hDLL,"bcp_done");
    if (!bcp_done)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }

    //dbconvert
    dbconvert = (p_dbconvert)GetProcAddress(CDblib::hDLL,"dbconvert");
    if (!dbconvert)
    {
        FreeLibrary(CDblib::hDLL);
        return false;
    }
#endif
    return true;
}

DBINT CDblib::GetErrorCode()
{
    if (m_dbErrorInfo.severity > 10)
    {
        return m_dbErrorInfo.msgno;
    }
    else
    {
        return m_errorCode;
    }
}

const char * CDblib::GetErrorMsg()
{
    if (m_dbErrorInfo.severity > 10)
    {
        return m_dbErrorInfo.msgtext;
    }
    else
        return m_errorMsg;
}

const CDbErrorInfo * CDblib::GetDbErrorInfo()
{
    return &m_dbErrorInfo;
}

bool CDblib::GetSQLDataDir(char * systemPath)
{
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char file[MAXFILE];
    char ext[MAXEXT];

    if(m_dbmsVersion & dvSybase)
    {
        strcpy(systemPath, "/u/sybase/data/");
        return true;
    }
    else
    {
        Run("select phyname from master..sysdevices where name = 'master'");
        if(GetErrorCode() != 0)
            return false;

        while(MoveNext())
        {
            fnsplit(GetColData(0), drive, dir, file, ext);
            fnmerge(systemPath, drive, dir, file, ext);
            fnsplit(systemPath, drive, dir, file, ext);
            fnmerge(systemPath, drive, dir, NULL, NULL);
        }
    }
    return true;
}

bool CDblib::GetSQLBackupDir(char * systemPath)
{
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char file[MAXFILE];
    char ext[MAXEXT];

    systemPath[0] = 0;

    if(m_dbmsVersion & dvSybase)
    {
        strcpy(systemPath, "/u/sybase/backup/");
        return true;
    }
    else
    {
        Run("select phyname from master..sysdevices where name = 'master'");
        if(GetErrorCode() != 0)
            return true;

        while(MoveNext())
        {
            fnsplit(GetColData(0), drive, dir, file, ext);
            strcat(dir, "..\\BACKUP\\");
            fnmerge(systemPath, drive, dir, file, ext);
            fnsplit(systemPath, drive, dir, file, ext);
            fnmerge(systemPath, drive, dir, NULL, NULL);
        }
    }
    return true;
}

int CDblib::GetTableColInfo(const char * dbName, const char * tableName, CTableColInfo colInfo[], int maxColNum)
{
    char oldDatabaseName[MAXNAME];
    CTableColInfo temp;
    int colNum = 0;

    memset(&temp, 0, sizeof(temp));

    if(dbName)
    {
        strcpy(oldDatabaseName, dbname(m_dbProcess));
        dbuse(m_dbProcess, (char *)dbName);
    }
    
    Run("select name, type, length, prec, scale, status from syscolumns where id = object_id('%s') order by colid", tableName);
    while(MoveNext())
    {
        if(colNum >= maxColNum)
            continue;
        strncpy(temp.Name, GetColData(0), sizeof(temp.Name));
        temp.Type = (short)atoi(GetColData(1));
        temp.Length = atoi(GetColData(2));
        temp.Prec = atoi(GetColData(3));
        temp.Scale = atoi(GetColData(4));
        temp.Identity = (atoi(GetColData(5)) & 128) ? true : false;
        memcpy(&colInfo[colNum], &temp, sizeof(temp));
        colNum ++;
    }
    Cancel();

    if(dbName)
        dbuse(m_dbProcess, oldDatabaseName);

    return colNum;
}

bool CDblib::BcpInit(LPCSTR tblname, LPCSTR hfile, LPCSTR errfile, INT direction)
{
    return (bcp_init(m_dbProcess, tblname, hfile, errfile, direction) == SUCCEED) ? true : false;
}

bool CDblib::BcpExec(LPDBINT rows_copied)
{
    return (bcp_exec(m_dbProcess, rows_copied) == SUCCEED) ? true : false;
}

bool CDblib::BcpReadFmt(const char * fmt)
{
    return (bcp_readfmt(m_dbProcess, fmt) == SUCCEED) ? true : false;
}

bool CDblib::BcpWriteFmt(const char * fmt)
{
    return (bcp_writefmt(m_dbProcess, fmt) == SUCCEED) ? true : false;
}

bool CDblib::BcpColfmt(INT file_column,
               BYTE file_type,
               INT file_prefixlen,
               DBINT file_collen,
               LPCBYTE file_term,
               INT file_termlen,
               INT table_column )
{
    RETCODE ret;

    ret = bcp_colfmt(m_dbProcess, file_column,
                     file_type,
                     file_prefixlen,
                     file_collen,
                     file_term,
                     file_termlen,
                     table_column );

    return (ret == SUCCEED) ? true : false;
}

bool CDblib::BcpBind( LPCBYTE varaddr,
                      INT prefixlen,
                      DBINT varlen,
                      LPCBYTE terminator,
                      INT termlen,
                      INT type,
                      INT table_column )
{
    RETCODE ret;

    ret = bcp_bind(m_dbProcess,
                   varaddr,
                   prefixlen,
                   varlen,
                   terminator,
                   termlen,
                   type,
                   table_column );

    return (ret == SUCCEED) ? true : false;                   
}

bool CDblib::BcpSendrow()
{
    return (bcp_sendrow(m_dbProcess) == SUCCEED) ? true : false;
}

DBINT CDblib::BcpBatch()
{
    return bcp_batch(m_dbProcess);
}

DBINT CDblib::BcpDone()
{
    return bcp_done(m_dbProcess);
}

bool CDblib::GetSpaceUsed(const char * dbName, tagSpaceInfo & info)
{
    const char * commandStrSQL70Data =
        "dbcc showfilestats\n";

    const char * commandStrSQL70Log =
        "dbcc sqlperf(logspace)\n";

    const char * commandStrSQL65 =
        "set nocount on\n"
        "declare @d1 int, @d2 int, @d3 int, @d4 int\n"
        "select @d1 = sum(size * 2) from master..sysusages where dbid = db_id() and segmap in (3, 7)\n"
        "select @d2 = sum(used * 2) from sysindexes where id != 8\n"
        "select @d3 = sum(size * 2) from master..sysusages where dbid = db_id() and segmap  = 4\n"
        "select @d4 = sum(used * 2) from sysindexes where id = 8\n"
        "select @d1, @d2, @d3, @d4";

    const char * commandStrSystem11 =
        "set nocount on\n"
        "declare @pagekb int, @d1 int, @d2 int, @d3 int, @d4 int\n"
        "select @pagekb = (low / 1024) from master..spt_values where number = 1 and type = 'E'\n"
        "select @d1 = sum(size * @pagekb) from master..sysusages where dbid = db_id() and segmap in (3, 7)\n"
        "select @d2 = sum(curunreservedpgs(db_id(), lstart,unreservedpgs) * @pagekb) from master..sysusages where dbid = db_id() and segmap in (3, 7)\n"
        "select @d3 = sum(size * @pagekb) from master..sysusages where dbid = db_id() and segmap  = 4\n"
        "select @d4 = sum(curunreservedpgs(db_id(), lstart,unreservedpgs) * @pagekb) from master..sysusages where dbid = db_id() and segmap = 4\n"
        "select @d1, @d1 - @d2, @d3, @d3 - @d4";

    const char * commandStrSystem10 =
        "set nocount on\n"
        "declare @d1 int, @d2 int, @d3 int, @d4 int\n"
        "select @d1 = sum(size * 2) from master..sysusages where dbid = db_id() and segmap in (3, 7)\n"
        "select @d2 = sum(unreservedpgs  * 2) from master..sysusages where dbid = db_id() and segmap in (3, 7)\n"
        "select @d3 = sum(size * 2) from master..sysusages where dbid = db_id() and segmap  = 4\n"
        "select @d4 = sum(unreservedpgs  * 2) from master..sysusages where dbid = db_id() and segmap = 4\n"
        "select @d1, @d1 - @d2, @d3, @d3 - @d4";

    info.totalDataSize = 0;
    info.usedDataSize = 0;
    info.totalLogSize = 0;
    info.usedLogSize = 0;

    Use(dbName);
    if(GetErrorCode() != 0)
        return false;

    if(GetDbmsVersion() & CDblib::dvSQL70)
    {
        Run(commandStrSQL70Data);
        while(MoveNext())
        {
            info.totalDataSize += atof(GetColData(2)) * 64.00 / 1024.00;
            info.usedDataSize += atof(GetColData(3)) * 64.00 / 1024.00;
        }

        Run(commandStrSQL70Log);
        while(MoveNext())
        {
            if(strcmp(dbName, GetColData(0)) == 0)
            {
                info.totalLogSize = atof(GetColData(1));
                info.usedLogSize = info.totalLogSize * atof(GetColData(2)) / 100;
            }
        }
    }
    else
    {
        if(GetDbmsVersion() & CDblib::dvSystem11)
            Run(commandStrSystem11);
        else if(GetDbmsVersion() & CDblib::dvSystem10)
            Run(commandStrSystem10);
        else
            Run(commandStrSQL65);

        while(MoveNext())
        {
            info.totalDataSize = atof(GetColData(0)) / 1024.00;
            info.usedDataSize = atof(GetColData(1)) / 1024.00;
            info.totalLogSize = atof(GetColData(2)) / 1024.00;
            info.usedLogSize = atof(GetColData(3)) / 1024.00;
            if(info.totalLogSize <= 0)
            {
                info.usedDataSize += info.usedLogSize;
                info.usedLogSize = 0;
            }
        }
    }
    return true;
}
