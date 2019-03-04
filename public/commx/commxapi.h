
#ifndef __COMMX_DLL_API_H__
#define __COMMX_DLL_API_H__

#ifndef COMMXAPI
#	ifdef _DLL
#		define COMMXAPI __declspec(dllexport)	/**< ������������ */
#	else
#		define COMMXAPI __declspec(dllimport)	/**< ����������� */
#	endif
#endif

#include <string>

//���commxģ��İ汾��
COMMXAPI void CommxVer(char** pBuf);

/** @brief ��ÿ�ִ���ļ��ľ���·���������ļ���
 *
 *
 *  @return ���������ļ��ľ���·��
 */
COMMXAPI const char* GetExecutePath();

/** @brief ȥ���ַ�����β�Ŀհ��ַ�
 *
 *
 *  @param	src Դ�ַ���
 *  @return Ŀ���ַ���
 */
COMMXAPI const char * Trim(char *src);

COMMXAPI char* memstr(void* buf, unsigned int buflen, void* lpfind, unsigned int findlen);

/** @brief		�����ַ��������������
 *
 *  ����10���ƺ�16�����������
 *	@param		as �����ַ���
 *	@return		ʮ��������
 */
COMMXAPI int aint(const char *as);

COMMXAPI int code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen);

/** @brief		����һ�����ȵ������
 *
 *
 *	@param		lpBuf ��������ָ��
 *	@param		nLength ����
 */
COMMXAPI void GenRandom(char *lpBuf, unsigned int nLength);

/** @brief ����ϵͳʱ�䣬�û����й���Ա����rootȨ��!
 *
 *
 *  @param	secs ��1970��1��1��0ʱ��ʼ���������
 *  @return
 *   - 0	�ɹ�
 *   - -1	ʧ��,�����Ϊ�û�û��Ȩ������
 */
COMMXAPI int SetTime(unsigned long secs);

/** @brief ����/�����ļ���ĳһ����
 *
 *	@param nLock 0����1�Ӷ���2��д��
 *  @return
 *	 - true		�ɹ�
 *	 - false	ʧ��
 */
COMMXAPI bool lockfile(int fd, int offset, int where, int nLock, int range, int wait = -1);

COMMXAPI unsigned long GetNearPrime(unsigned long nNumber);

#ifdef _POSIX_C_SOURCE
COMMXAPI char* strlwr(char* str);
COMMXAPI void Sleep(int t);
#endif

#endif
