// Dblib.h: interface for the CDblib class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __DBLIB_H
#define __DBLIB_H

#define DBNTWIN32
#define MAX_COLNUM 100
#define DBLIBN "DBLIB"
#define MAXDRIVE  _MAX_DRIVE
#define MAXDIR    _MAX_DIR
#define MAXFILE   _MAX_FNAME
#define MAXEXT    _MAX_EXT
#define fnsplit   _splitpath
#define fnmerge   _makepath

#include "../../public/mssql/sqlfront.h"
#include "../../public/mssql/sqldb.h"

#ifdef DLL_DBLIB
//定义DB-Libarary的函数指针
typedef  BYTE *     (*p_dbinit)(void);
typedef  RETCODE    (*p_dbsetlogintime)(INT);
typedef  LOGINREC*  (*p_dblogin)(void);
typedef  DBPROCESS* (*p_dbopen)(LOGINREC *,char *);
typedef  void       (*p_dbsetlname)(LOGINREC *,char *,int );
typedef  RETCODE    (*p_dbuse)(DBPROCESS *,char *);
typedef  RETCODE    (*p_dbcmd)(DBPROCESS *,char *);
typedef  RETCODE    (*p_dbcmdrow)(DBPROCESS *);
typedef  RETCODE    (*p_dbsqlexec)(DBPROCESS *);
typedef  RETCODE    (*p_dbsqlok)(DBPROCESS *);
typedef  RETCODE    (*p_dbsqlsend)(DBPROCESS *);
typedef  void       (*p_dbwinexit)(void);
typedef  RETCODE    (*p_dbresults)(DBPROCESS *);
typedef  RETCODE    (*p_dbnextrow)(DBPROCESS *);
typedef  RETCODE    (*p_dbbind)(DBPROCESS *,int,int,DBINT,BYTE *);
typedef  RETCODE    (*p_dbcancel)(DBPROCESS *);
typedef  RETCODE    (*p_dbclose)(DBPROCESS *);
typedef  void       (*p_dbexit)(void);
typedef  void       (*p_dbclrbuf)(DBPROCESS *,DBINT);
typedef  INT        (*p_dbcollen)(DBPROCESS *,int);
typedef  char *     (*p_dbcolname)(DBPROCESS *,int);
typedef  INT        (*p_dbcoltype)(DBPROCESS *,int);
typedef  INT        (*p_dbcolntype)(DBPROCESS *,int);
typedef  INT        (*p_dbcolutype)(DBPROCESS *,int);
typedef  DBINT      (*p_dbcount)(DBPROCESS *);
typedef  char *     (*p_dbdata)(DBPROCESS *,int);
typedef  DBUBOOL    (*p_dbdataready)(DBPROCESS *);
typedef  DBINT      (*p_dbdatlen)(DBPROCESS *,int);
typedef  RETCODE    (*p_dbdatecrack) (PDBPROCESS dbproc,LPDBDATEREC dateinfo,LPCDBDATETIME datetime );
typedef  DBUBOOL      (*p_dbdead)(DBPROCESS *);
typedef  DBERRHANDLE_PROC (*p_dberrhandle)(DBERRHANDLE_PROC);
typedef  DBMSGHANDLE_PROC (*p_dbmsghandle)(DBMSGHANDLE_PROC);
typedef  DBERRHANDLE_PROC (*p_dbprocerrhandle)(HANDLE *,DBERRHANDLE_PROC);
typedef  DBMSGHANDLE_PROC (*p_dbprocmsghandle)(HANDLE *,DBMSGHANDLE_PROC);
typedef  RETCODE    (*p_dbfcmd)(DBPROCESS *,char *,...);
typedef  DBINT      (*p_dbfirstrow)(DBPROCESS *);
typedef  DBINT      (*p_dblastrow)(DBPROCESS *);
typedef  BOOL       (*p_dbiscount)(DBPROCESS *);
typedef  INT        (*p_dbnumcols)(DBPROCESS *);
typedef  INT        (*p_dbnumorders)(DBPROCESS *);
typedef  INT        (*p_dbnumrets)(DBPROCESS *);
typedef  INT        (*p_dbordercol)(DBPROCESS *,INT);
typedef  RETCODE    (*p_dbrows)(DBPROCESS *);
typedef  STATUS     (*p_dbrowtype)(DBPROCESS *);
typedef  DBINT      (*p_dbretstatus)(DBPROCESS *);
typedef  int        (*p_dbserverenum)(USHORT,char *,USHORT,unsigned short * );
typedef  RETCODE    (*p_dbcolinfo)(DBPROCESS *, INT, DBINT, DBINT, DBCOL *);
typedef void        (*p_dbfreelogin)(PLOGINREC);
typedef LPVOID      (*p_dbgetuserdata)(DBPROCESS *);
typedef void        (*p_dbsetuserdata)(DBPROCESS *, LPVOID);
typedef SHORT       (*p_dbgetmaxprocs)(void);
typedef void        (*p_dbsetmaxprocs)(SHORT);
typedef LPCSTR      (*p_dbname)(DBPROCESS *);
typedef LPCSTR      (*p_dbchange)(DBPROCESS *);

typedef RETCODE     (*p_bcp_init)(PDBPROCESS dbproc, LPCSTR tblname, LPCSTR hfile, LPCSTR errfile, INT direction);
typedef RETCODE     (*p_bcp_exec)(PDBPROCESS dbproc, LPDBINT rows_copied);
typedef RETCODE     (*p_bcp_readfmt)(PDBPROCESS dbproc, LPCSTR filename);
typedef RETCODE     (*p_bcp_writefmt)(PDBPROCESS dbproc, LPCSTR filename);
typedef RETCODE     (*p_bcp_colfmt)(PDBPROCESS dbproc,
                                    INT file_column,
                                    BYTE file_type,
                                    INT file_prefixlen,
                                    DBINT file_collen,
                                    LPCBYTE file_term,
                                    INT file_termlen,
                                    INT table_column );
typedef RETCODE     (*p_bcp_bind) (PDBPROCESS dbproc,
                                    LPCBYTE varaddr,
                                    INT prefixlen,
                                    DBINT varlen,
                                    LPCBYTE terminator,
                                    INT termlen,
                                    INT type,
                                    INT table_column );
typedef RETCODE     (*p_bcp_sendrow)(PDBPROCESS dbproc);
typedef DBINT       (*p_bcp_batch)(PDBPROCESS dbproc);
typedef DBINT       (*p_bcp_done)(PDBPROCESS dbproc );

typedef INT         (*p_dbconvert)(PDBPROCESS dbproc,
                                    INT srctype,
                                    LPCBYTE src,
                                    DBINT srclen,
                                    INT desttype,
                                    LPBYTE dest,
                                    DBINT destlen );

#endif

typedef struct
{
    CHAR  Name[MAXCOLNAMELEN+1];
    SHORT Type;
    DBINT MaxLength;
    DBINT  Precision;
} SDBCOL;

typedef struct
{
    CHAR  Name[MAXCOLNAMELEN+1];
    SHORT Type;
    INT   Length;
    INT   Prec;
    INT   Scale;
    bool  Identity;
} CTableColInfo;

typedef struct
{
    CHAR ServerName[MAXNAME];
} CServerList;

typedef struct
{
    INT severity;
    DBINT msgno;
    INT dberr;
    INT oserr;
    CHAR msgtext[200];
    char dberrstr[200];
    char oserrstr[200];
} CDbErrorInfo;

struct tagSpaceInfo
{
    double totalDataSize;
    double usedDataSize;
    double totalLogSize;
    double usedLogSize;
};

class CDblib
{
public:
    enum DbmsOsType
    {
        otNT = 0,
        otUnix,
        otNovell
    };
    enum DbmsVersion
    {
        dvSQL65 = 1,
        dvSQL70 = 2,
		dvSQL2000 = 4,
        dvMicrosoft = dvSQL65 | dvSQL70 | dvSQL2000,

        dvSystem10 = 0x10000,
        dvSystem11 = 0x20000,
        dvSybase = dvSystem10 | dvSystem11
    };

    CDblib(bool bcpOK = false);
    virtual ~CDblib();

    bool Open(const char *servername, const char *loginID, const char *passWord, const char *appName = NULL, const char *hostname = NULL);
    bool __Run(const char * commandStr);
    bool Run(const char * formatString, ...);
    bool __Run20(const char * commandStr);
    bool Run20(const char * formatString, ...);
    bool Run20Error(const char * formatString, ...);
    bool Run20Message(const char * formatString, ...);
    bool MoveNext();
    bool Use(const char *databaseName);
    bool Cancel();
    bool Close();
    bool GetServerList(char * serverListName, USHORT * serverNum);
    DbmsOsType GetDbmsOsType() {return m_dbmsOsType;};
    DbmsVersion GetDbmsVersion() {return m_dbmsVersion;};
    bool GetSQLDataDir(char *);
    bool GetSQLBackupDir(char *);
    static void InstallMsgFunc(void (* MsgFunc)(CDbErrorInfo *));

    bool BcpInit(LPCSTR tblname, LPCSTR hfile, LPCSTR errfile, INT direction);
    bool BcpExec(LPDBINT rows_copied);
    bool BcpReadFmt(const char * fmt);
    bool BcpWriteFmt(const char * fmt);
    bool BcpColfmt(INT file_column,
                   BYTE file_type,
                   INT file_prefixlen,
                   DBINT file_collen,
                   LPCBYTE file_term,
                   INT file_termlen,
                   INT table_column );
    bool BcpBind( LPCBYTE varaddr,
                  INT prefixlen,
                  DBINT varlen,
                  LPCBYTE terminator,
                  INT termlen,
                  INT type,
                  INT table_column );
    bool BcpSendrow();
    DBINT BcpDone();
    DBINT BcpBatch();

private:
    bool Init();
    static INT SQLAPI MyErrHandler(DBPROCESS *dbproc, INT severity, INT dberr, INT oserr, const char *dberrstr, const char *oserrstr);
    static INT SQLAPI MyMsgHandler(DBPROCESS *dbproc, DBINT msgno, INT msgstate, INT severity, const CHAR *msgtext, const CHAR * s1, const CHAR * s2, DBUSMALLINT dbus);
    bool Login(const char *servername, const char *loginID, const char *passWord, const char *appname, const char *hostname);
    bool FirstColInfo();
    bool Bind();

    static void (* MyMsgFunc)(CDbErrorInfo *);

private:
    bool m_bcpOK;
    DBPROCESS * m_dbProcess;
    bool m_loginOK;
    USHORT m_colCount;
    ULONG m_rowCount;
    DBINT m_errorCode;
    DBCHAR m_errorMsg[81];
    CDbErrorInfo m_dbErrorInfo;

    SDBCOL m_colInfo[MAX_COLNUM];
    char m_data[MAX_COLNUM][DBMAXCHAR];
    DBCHAR m_serverName[MAXSERVERNAME + 1];
    DBCHAR m_loginID[MAXSERVERNAME + 1];
    DBCHAR m_passWord[MAXSERVERNAME + 1];
    DBCHAR m_appName[MAXSERVERNAME + 1];
    DBCHAR m_hostName[MAXSERVERNAME + 1];
    DBCHAR m_databaseName[DBMAXCHAR + 1];

    bool hasExtendColumn;

    DbmsOsType m_dbmsOsType;
    DbmsVersion m_dbmsVersion;

public:
    const char * GetServerName();
    const char * GetLoginID();
    USHORT GetColCount();
    const SDBCOL * GetColInfo(int);
    const SDBCOL * GetColInfo(const char *);
    int GetTableColInfo(const char * dbName, const char * tableName, CTableColInfo [], int maxColNum);
    char * GetColData(int);
    char * GetColData(const char *);
    ULONG GetRowCount();
    DBINT GetErrorCode();
    const char * GetErrorMsg();
    const CDbErrorInfo * GetDbErrorInfo();
    USHORT GetServerList(CServerList * serverList);

    RETCODE DBResult(){return dbresults(m_dbProcess);};
    STATUS DBNextrow(){return dbnextrow(m_dbProcess);};

    bool GetSpaceUsed(const char * dbName, tagSpaceInfo &);

private:
    static HINSTANCE hDLL;
    bool LoadFunc();

#ifdef DLL_DBLIB
protected:
    p_dbinit           dbinit;
    p_dbsetlogintime   dbsetlogintime;
    p_dblogin          dblogin;
    p_dbopen           dbopen;
    p_dbsetlname       dbsetlname;
    p_dbuse            dbuse;
    p_dbcmd            dbcmd;
    p_dbcmdrow         dbcmdrow;
    p_dbsqlexec        dbsqlexec;
    p_dbsqlok          dbsqlok;
    p_dbsqlsend        dbsqlsend;
    p_dbwinexit        dbwinexit;
    p_dbresults        dbresults;
    p_dbnextrow        dbnextrow;
    p_dbbind           dbbind;
    p_dbcancel         dbcancel;
    p_dbclose          dbclose;
    p_dbclrbuf         dbclrbuf;
    p_dbcollen         dbcollen;
    p_dbcolname        dbcolname;
    p_dbcoltype        dbcoltype;
    p_dbcolntype       dbcolntype;
    p_dbcolutype       dbcolutype;
    p_dbcount          dbcount;
    p_dbdata           dbdata;
    p_dbdataready      dbdataready;
    p_dbdatlen         dbdatlen;
    p_dbdatecrack      dbdatecrack;
    p_dbdead           dbdead;
    p_dberrhandle      dberrhandle;
    p_dbmsghandle      dbmsghandle;
    //p_dbprocerrhandle  dbprocerrhandle;
    //p_dbprocmsghandle  dbprocmsghandle;
    p_dbfcmd           dbfcmd;
    p_dbfirstrow       dbfirstrow;
    p_dblastrow        dblastrow;
//    p_dbiscount        dbiscount;
    p_dbnumcols        dbnumcols;
    p_dbnumorders      dbnumorders;
    p_dbnumrets        dbnumrets;
    p_dbordercol       dbordercol;
    p_dbrows           dbrows;
    p_dbrowtype        dbrowtype;
    p_dbretstatus      dbretstatus;
    p_dbexit           dbexit;
    p_dbserverenum     dbserverenum;
    p_dbcolinfo        dbcolinfo;
    p_dbfreelogin      dbfreelogin;
    static p_dbgetuserdata    dbgetuserdata;
    static p_dbsetuserdata    dbsetuserdata;
    p_dbgetmaxprocs    dbgetmaxprocs;
    p_dbsetmaxprocs    dbsetmaxprocs;
    p_dbname           dbname;
    p_dbchange         dbchange;

    p_bcp_init         bcp_init;
    p_bcp_exec         bcp_exec;
    p_bcp_readfmt      bcp_readfmt;
    p_bcp_writefmt     bcp_writefmt;
    p_bcp_colfmt       bcp_colfmt;
    p_bcp_bind         bcp_bind;
    p_bcp_sendrow      bcp_sendrow;
    p_bcp_batch        bcp_batch;
    p_bcp_done         bcp_done;

    p_dbconvert        dbconvert;
#endif
};

inline const char * CDblib::GetServerName()
{
    return m_serverName;
}

inline const char * CDblib::GetLoginID()
{
    return m_loginID;
}

inline USHORT CDblib::GetColCount()
{
    return m_colCount;
}

inline ULONG CDblib::GetRowCount()
{
    return m_rowCount;
}

#endif
