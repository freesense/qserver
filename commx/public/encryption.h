/**@file  encryption.h
 * @brief 综合加解密和压缩等算法
 */

#ifndef __ENCRYPTION_COMMX_H__
#define __ENCRYPTION_COMMX_H__

#include "commxapi.h"

/**@brief CRC32算法

对一段给定的内存序列，产生CRC32校验码
 */
class COMMXAPI CRC32
{
public:
	/**@brief 预备CRC32算法常量表
	
	该函数由应用在程序初始化时显式调用一次
	 */
	static void prepareCrc32Table();

	/**@brief 获得CRC32的int值
	
	@param InStr 需要计算CRC32值的数据指针
	@param len 计算CRC32的内存数据的长度
	@param CRC32值
	@return 整型CRC32值
	 */
	static unsigned long getCrc32Int(const char *InStr, int len);

	/**@brief 获得CRC32的标准字符串值
	
	将CRC32::getCrc32Int返回的Int值转换成标准的字符串值
	@param InStr 需要计算CRC32值的数据指针
	@param len 计算CRC32的内存数据的长度
	@param crc 出参，计算出的CRC32字符串拷贝在此，长度=8
	 */
	static void getCrc32(const char *InStr, int len, char *crc);

private:
	static unsigned long Crc32Table[256];		/** 预设的CRC32算法常量表 [7/13/2009 xinl] */
};

/**@brief MD5算法

对一段给定的内存序列，产生MD5指纹
 */
class COMMXAPI MD5
{
	typedef struct
	{
		ULONG i[2];
		ULONG buf[4];
		unsigned char in[64];
		unsigned char digest[16];
	} MD5_CTX;

	typedef void (WINAPI* PMD5Init) (MD5_CTX *);
	typedef void (WINAPI* PMD5Update) (MD5_CTX *, const unsigned char *, unsigned int);
	typedef void (WINAPI* PMD5Final ) (MD5_CTX *);

public:
	static char* getMd5(const char *str, unsigned int len, char *out);

private:
	static PMD5Init MD5Init;
	static PMD5Update MD5Update;
	static PMD5Final MD5Final;
};

/**@brief DES & 3DES 加解密算法


 */
class COMMXAPI DES
{
public:
	/**@brief 加解密
	
	@param Out 输出缓冲区。输出缓冲区(Out)的长度 >= ((datalen+7)/8)*8,即比datalen大的且是8的倍数的最小正整数
	@param In 输入缓冲区。In 可以= Out,此时加/解密后将覆盖输入缓冲区(In)的内容
	@param datalen 加密数据的长度
	@param Key 加密密钥
	@param keylen 当keylen>8时系统自动使用3次DES加/解密,否则使用标准DES加/解密.超过16字节后只取前16字节
	@param bEncrypt 加解密方向
	-true 加密
	-false 解密
	@return
	-true 成功
	-false 失败
	 */
	static bool Des(char *Out, char *In, long datalen, const char *Key, int keylen, bool bEncrypt);

private:
	typedef bool (*PSubKey)[16][48];

	static void des(char Out[8], char In[8], const PSubKey pSubKey, bool bEncrypt);//标准DES加/解密
	static void SetSubKey(PSubKey pSubKey, const char Key[8]);// 设置子密钥
	static void F_func(bool In[32], const bool Ki[48]);// f 函数
	static void S_func(bool Out[32], const bool In[48]);// S 盒代替
	static void Transform(bool *Out, bool *In, const char *Table, int len);// 变换
	static void Xor(bool *InA, const bool *InB, int len);// 异或
	static void RotateL(bool *In, int len, int loop);// 循环左移
	static void ByteToBit(bool *Out, const char *In, int bits);// 字节组转换成位组
	static void BitToByte(char *Out, const bool *In, int bits);// 位组转换成字节组

	static char IP_Table[64];			/** initial permutation IP [7/15/2009 xinl] */
	static char IPR_Table[64];			/** final permutation IP^-1  [7/15/2009 xinl] */
	static char E_Table[48];				/** expansion operation matrix [7/15/2009 xinl] */
	static char P_Table[32];			/** 32-bit permutation function P used on the output of the S-boxes [7/15/2009 xinl] */
	static char PC1_Table[56];			/** permuted choice table (key) [7/15/2009 xinl] */
	static char PC2_Table[48];			/** permuted choice key (table) [7/15/2009 xinl] */
	static char LOOP_Table[16];			/** number left rotations of pc1 [7/15/2009 xinl] */
	static char S_Box[8][4][16];		/** The (in)famous S-boxes [7/15/2009 xinl] */
};
#endif
