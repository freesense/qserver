
#ifndef __COMMX_DLL_API_H__
#define __COMMX_DLL_API_H__

#ifndef COMMXAPI
#	ifdef _DLL
#		define COMMXAPI __declspec(dllexport)	/**< 定义引出符号 */
#	else
#		define COMMXAPI __declspec(dllimport)	/**< 定义引入符号 */
#	endif
#endif

#include <string>

//获得commx模块的版本号
COMMXAPI void CommxVer(char** pBuf);

/** @brief 获得可执行文件的绝对路径，包括文件名
 *
 *
 *  @return 程序运行文件的绝对路径
 */
COMMXAPI const char* GetExecutePath();

/** @brief 去掉字符串首尾的空白字符
 *
 *
 *  @param	src 源字符串
 *  @return 目的字符串
 */
COMMXAPI const char * Trim(char *src);

COMMXAPI char* memstr(void* buf, unsigned int buflen, void* lpfind, unsigned int findlen);

/** @brief		根据字符串获得整型数据
 *
 *  区分10进制和16进制两种情况
 *	@param		as 数字字符串
 *	@return		十进制数字
 */
COMMXAPI int aint(const char *as);

COMMXAPI int code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen);

/** @brief		产生一定长度的随机流
 *
 *
 *	@param		lpBuf 随机流存放指针
 *	@param		nLength 长度
 */
COMMXAPI void GenRandom(char *lpBuf, unsigned int nLength);

/** @brief 设置系统时间，用户需有管理员或者root权限!
 *
 *
 *  @param	secs 从1970年1月1日0时开始计算的秒数
 *  @return
 *   - 0	成功
 *   - -1	失败,多半因为用户没有权限引起
 */
COMMXAPI int SetTime(unsigned long secs);

/** @brief 锁定/解锁文件的某一部分
 *
 *	@param nLock 0解锁1加读锁2加写锁
 *  @return
 *	 - true		成功
 *	 - false	失败
 */
COMMXAPI bool lockfile(int fd, int offset, int where, int nLock, int range, int wait = -1);

COMMXAPI unsigned long GetNearPrime(unsigned long nNumber);

#ifdef _POSIX_C_SOURCE
COMMXAPI char* strlwr(char* str);
COMMXAPI void Sleep(int t);
#endif

#endif
