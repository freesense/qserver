/************************************************************************************
*
* 文件名  : AdoDB.h
*
* 文件描述: 数据库操作类
*
* 创建人  : dingjing, 2008-12-31
*
* 版本号  : 1.0
*
* 修改记录:
*
************************************************************************************/
#ifndef _ADODB_H_
#define _ADODB_H_
#include <string>
using std::string;

//预定义日志级别，取值范围0~255，多余的值留给用户自定义
#ifndef	RPT_DEBUG
#define RPT_DEBUG		0x10
#endif

#ifndef	RPT_INFO
#define RPT_INFO		0x20
#endif

#ifndef	RPT_WARNING
#define RPT_WARNING		0x40
#endif

#ifndef	RPT_ERROR
#define RPT_ERROR		0x80
#endif

#define DB_MSG_BUF_LEN 400          //LogEvent处理日志缓冲区大小
#define WM_LOGEVENT WM_USER + 109   //如果没有定义派生类，重定义虚函数LogEvent(), 否则发送此消息

#import "C:\Program Files\common files\system\ado\msado15.dll" no_namespace rename("EOF", "adoEOF")

class CDataBase
{
public: //构造析构
	CDataBase();
#ifdef _MFC_VER
	CDataBase(HWND hWnd);
#endif
	virtual ~CDataBase();
	
public: //属性
	_ConnectionPtr& GetConnection() { return m_pCon; }

	/********************************************************************************
	* 函数名: IsOpen
	* 功  能: 检测数据库是否打开
	* 返回值: 已打开返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL IsOpen();

	/********************************************************************************
	* 函数名: SetCursorLocation
	* 参  数:
	*		  CursorLocationEnum CursorLocation [IN] : 游标服务的位置
	*												   adUseNone        : 不使用游标服务。（此常量已作废并且只是为了向后兼容才出现。）
	*												   adUseServer      : 默认值。使用数据提供者或驱动程序提供的游标。这些游标有时很灵活，可以额外感知其他人对数据源所做的更改。但是，Microsoft Cursor Service for OLE DB 的某些功能（例如断开 Recordset 对象）不能用服务器端游标模拟，这些功能在此设置中将不可用。
	*												   adUseClient		: 使用本地游标库提供的客户端的游标。本地游标服务通常允许执行驱动程序提供的游标所不允许的许多功能，因此使用此设置可以充分利用即将启用的功能。为进行向后兼容，亦支持同义字 adUseClientBatch。
	*												   adUseClientBatch :
	* 功  能: 设置游标服务的位置
	********************************************************************************/
	void SetCursorLocation(CursorLocationEnum CursorLocation);

#ifdef _MFC_VER
	void SetMsgWnd(HWND hWnd){m_hWnd = hWnd;}
	HWND GetMsgWnd(){return m_hWnd;}
#endif

public:	//操作
	/********************************************************************************
	* 函数名: Open
	* 参  数:
	*		  LPCTSTR lpConStr [IN] : 连接字符串，包含连接信息
	*         long    nOptions [IN] : 连接选项
	*		                         adConnectUnspecified (默认)同步方式打开连接
	*                                adAsyncConnect		  异步方式打开连接
	* 功  能: 连接数据库
	* 返回值: 连接成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL Open(LPCTSTR lpConStr, long nOptions = adConnectUnspecified);
	
	/********************************************************************************
	* 函数名: Close
	* 功  能: 关闭数据库连接
	********************************************************************************/
	void Close();
	
	/********************************************************************************
	* 函数名: Execute
	* 参  数:
	*		  LPCTSTR lpSQL    [IN] : 要执行的SQL语句
	*         long    nOptions [IN] : 表示CommandText中内容的类型
	*                                 adCmdText   : 表明CommandText是文本命令
	*                                 adCmdTable  : 表明CommandText是一个表名
	*                                 adCmdProc   : 表明CommandText是一个存储过程
	*                                 adCmdUnknown: 未知
	* 功  能: 执行SQL语句
	* 返回值: 连接成功返回_RecordsetPtr指针, 否则NULL
	********************************************************************************/
	_RecordsetPtr Execute(LPCTSTR lpSQL, long nOptions = adCmdText);

	/********************************************************************************
	* 函数名: BeginTrans
	* 功  能: 开始事物
	* 返回值: 对于支持嵌套事务的数据库来说, 在已打开的事务中调用 BeginTrans 
	          方法将开始新的嵌套事务. 返回值将指示嵌套层次: 返回值为 1 表示已打开顶层
	          事务, 返回值为 2 表示已打开第二层事务, 依次类推
	********************************************************************************/
	long BeginTrans();

	/********************************************************************************
	* 函数名: CommitTrans
	* 功  能: 提交事物
	* 返回值: 成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL CommitTrans();

	/********************************************************************************
	* 函数名: RollbackTrans
	* 功  能: 取消事物
	* 返回值: 成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL RollbackTrans();

protected:
	/********************************************************************************
	* 函数名: LogEvent
	* 参  数:
	*		  unsigned short nLogType [IN] : 日志的类型
	*                                        RPT_DEBUG   : 表示是调试信息
	*										 RPT_INFO    : 表示是提示信息
	*										 RPT_WARNING : 表示是警告信息
	*                                        RPT_ERROR   : 表示是错误信息
	* 功  能: 处理日志
	********************************************************************************/
	virtual void LogEvent(unsigned short nLogType, const char* pFormat, ...);

	//Attributes
	_ConnectionPtr        m_pCon;

#ifdef _MFC_VER
	HWND        m_hWnd;  //接收消息的窗口
#endif

private:

};

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

class CRecordSet
{
public: //构造析构
	CRecordSet();
	CRecordSet(_ConnectionPtr pCon);
	CRecordSet(CDataBase* pDB);
	virtual ~CRecordSet();

public: //属性
	/********************************************************************************
	* 函数名: SetConnectionPtr
	* 参  数:
	*		  _ConnectionPtr pCon [IN] : 与_RecordsetPtr关联的_ConnectionPtr
	* 功  能: _RecordsetPtr指针与_ConnectionPtr指针关联
	********************************************************************************/
	void SetConnectionPtr(_ConnectionPtr pCon){m_pCon  = pCon;}

	/********************************************************************************
	* 函数名: GetAbsolutePosition
	* 功  能: 获取当前记录的位置
	* 返  回: 返回当前记录的序号
	********************************************************************************/
	long GetAbsolutePosition();

	/********************************************************************************
	* 函数名: SetAbsolutePosition
	* 参  数:
	*         int nPosition [IN] : 要设置的位置
	* 功  能: 设置当前记录的位置
	* 返  回: 成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL SetAbsolutePosition(int nPosition);

	/********************************************************************************
	* 函数名: GetRecordset
	* 功  能: 返回_RecordsetPtr指针m_pRset
	* 返  回: 返回m_pRset
	********************************************************************************/
	_RecordsetPtr& GetRecordset(){return m_pRset;}

public:	//操作
	/********************************************************************************
	* 函数名: Open
	* 参  数:
	*		  LPCTSTR lpSql             [IN] : 要执行的SQL语句
	*         CursorTypeEnum CursorType	[IN] : 光标类型
	*                                          adOpenUnspecified : 不作特别指定
    *                                          adOpenForwardOnly : 前滚静态光标, 这种光标只能向前浏览记录集，比如用MoveNext向前滚动,这种方式可以提高浏览速度, 但诸如BookMark,RecordCount,AbsolutePosition,AbsolutePage都不能使用
    *                                          adOpenKeyset      : 采用这种光标的记录集看不到其它用户的新增、删除操作，但对于更新原有记录的操作对你是可见的
    *                                          adOpenDynamic     : 动态光标, 所有数据库的操作都会立即在各用户记录集上反应出来
    *                                          adOpenStatic      : 静态光标, 它为你的记录集产生一个静态备份，但其它用户的新增、删除、更新操作对你的记录集来说是不可见的
	*         LockTypeEnum LockType 	[IN] : 锁定类型
	*										   adLockUnspecified     : 未指定 
	*										   adLockReadOnly        : 只读记录集
	*										   adLockPessimistic     : 悲观锁定方式, 数据在更新时锁定其它所有动作，这是最安全的锁定机制
	*										   adLockOptimistic      : 乐观锁定方式, 只有在你调用Update方法时才锁定记录。在此之前仍然可以做数据的更新、插入、删除等动作
	*										   adLockBatchOptimistic : 乐观分批更新, 编辑时记录不会锁定，更改、插入及删除是在批处理模式下完成。
	*         long lOption				[IN] : 参照上面Execute中lOption
	* 功  能: 打开数据表
	* 返回值: 成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL Open(LPCTSTR lpSql, CursorTypeEnum CursorType = adOpenDynamic, 
		LockTypeEnum LockType = adLockOptimistic, long lOption = adCmdText);

	/********************************************************************************
	* 函数名: Close
	* 功  能: 关闭数据表
	********************************************************************************/
	void Close();

	/********************************************************************************
	* 函数名: GetRecordCount
	* 功  能: 获取记录总数
	* 返回值: 返回记录总数
	********************************************************************************/
	long GetRecordCount();

	BOOL IsBOF();
	BOOL IsEOF();
	void MoveFirst();
	void MoveLast();
	void MoveNext();
	void MovePrevious();

	BOOL AddNew();
	BOOL Update();

	/********************************************************************************
	* 函数名: Delete
	* 参  数:
	*		  AffectEnum AffectRecords [IN] : Delete 所影响的记录数
	*                                         AdAffectCurrent     : 默认, 仅删除当前记录
	*										  AdAffectGroup       : 删除满足当前 Filter 属性设置的记录, 要使用该选项, 必须将 Filter 属性设置为有效的预定义常量之一
	*										  adAffectAll         : 删除所有记录
	*										  adAffectAllChapters :	删除所有子集记录
	* 功  能: 删除记录
	* 返回值: 成功返回TRUE，否则FALSE
	********************************************************************************/
	BOOL Delete(AffectEnum AffectRecords = adAffectCurrent);

	/********************************************************************************
	* 函数名: Requery
	* 参  数:
	*		  long Options [IN] : 指示影响该操作选项的位屏蔽
	*                             AdAffectCurrent : 默认
	*							  AdAffectGroup   : 如果该参数设置为 adAsyncExecute, 则该操作将异步执行并在它结束时产生RecordsetChangeComplete 事件
	* 功  能: 更新记录
	* 返回值: 成功返回TRUE，否则FALSE
	********************************************************************************/
	BOOL Requery(long Options = adConnectUnspecified);

	/********************************************************************************
	* 函数名: SetFilter
	* 参  数:
	*		  LPCTSTR lpszFilter [IN] : 筛选条件
	* 功  能: 设置筛选条件
	* 返回值: 成功返回TRUE，否则FALSE
	********************************************************************************/
	BOOL SetFilter(LPCTSTR lpszFilter);

	/********************************************************************************
	* 函数名: SetSort
	* 参  数:
	*		  LPCTSTR lpszCriteria [IN] : 排序条件
	* 功  能: 设置排序条件
	* 返回值: 成功返回TRUE，否则FALSE
	********************************************************************************/
	BOOL SetSort(LPCTSTR lpszCriteria);

	void PutCollect(LPCSTR lpFieldName,  const _variant_t &value);
	void GetCollect(LPCSTR lpFieldName,  unsigned int     &nValue);
	BOOL GetCollect(LPCSTR lpFieldName,  SYSTEMTIME*      pValue);
	void GetCollect(LPCSTR lpFieldName,  LPCSTR lpValue,  int nMaxLen);
	void GetCollect(LPCSTR lpFieldName,  std::string      &strValue);
	void GetCollect(LPCSTR lpFieldName,  double           &Value);
	void GetCollect(LPCSTR lpFieldName,  int              &nValue);

protected:
	virtual void LogEvent(unsigned short nLogType, const char* pFormat, ...);

	//Attributes
	_RecordsetPtr    m_pRset;
	_ConnectionPtr   m_pCon;
	CDataBase*       m_pDB;

private:

};


//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
class CAccessDB : public CDataBase
{
public:	//构造析构
	CAccessDB();

#ifdef _MFC_VER
	CAccessDB(HWND hWnd);
#endif
	virtual ~CAccessDB();

public:	//属性

public:	//操作
	/********************************************************************************
	* 函数名: Open
	* 参  数:
	*		  LPCTSTR lpDBPath   [IN] : 数据库路径
	*		  LPCTSTR lpPassword [IN] : 数据库密码
	*         long    lOptions	 [IN] : 同上
	* 功  能: 连接数据库
	* 返回值: 连接成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL Open(LPCTSTR lpDBPath, LPCTSTR lpPassword = NULL, long lOptions = adConnectUnspecified);

protected:
	virtual void LogEvent(unsigned short nLogType, const char* pFormat, ...);

private:
};

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
class CSQLServerDB : public CDataBase
{
public:	//构造析构
	CSQLServerDB();
	virtual ~CSQLServerDB();

public:	//属性
	/********************************************************************************
	* 函数名: IsConnectNormal
	* 参  数:
	*		  string strTestTable [IN] : 用来测试连接是否正常的数据库表
	* 功  能: 判断连接是否正常
	* 返回值: 连接正常返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL IsConnectNormal(string strTestTable);

public:	//操作
	/********************************************************************************
	* 函数名: Open
	* 参  数:
	*		  string strSerIP    [IN] : 数据库服务器IP
	*		  string strDBName   [IN] : 数据库名
	*		  string strUserName [IN] : 用户名
	*		  string strPassword [IN] : 密码
	*         long    lOptions	 [IN] : 同上
	* 功  能: 连接数据库
	* 返回值: 连接成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL Open(string strSerIP, string strDBName, string strUserName, string strPassword,
		long lOptions = adConnectUnspecified);

	/********************************************************************************
	* 函数名: Reconnect
	* 参  数:
	* 功  能: 重新连接
	* 返回值: 连接成功返回TRUE, 否则FALSE
	********************************************************************************/
	BOOL Reconnect();

protected:

private: //属性
	string     m_strCon;   //用于重连
	long       m_lOptions; //用于重连

};

////////////////////////////////////////////////////////////////////////////////////
#endif