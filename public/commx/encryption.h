/**@file  encryption.h
 * @brief �ۺϼӽ��ܺ�ѹ�����㷨
 */

#ifndef __ENCRYPTION_COMMX_H__
#define __ENCRYPTION_COMMX_H__

#include "commxapi.h"

/**@brief CRC32�㷨

��һ�θ������ڴ����У�����CRC32У����
 */
class COMMXAPI CRC32
{
public:
	/**@brief Ԥ��CRC32�㷨������
	
	�ú�����Ӧ���ڳ����ʼ��ʱ��ʽ����һ��
	 */
	static void prepareCrc32Table();

	/**@brief ���CRC32��intֵ
	
	@param InStr ��Ҫ����CRC32ֵ������ָ��
	@param len ����CRC32���ڴ����ݵĳ���
	@param CRC32ֵ
	@return ����CRC32ֵ
	 */
	static unsigned long getCrc32Int(const char *InStr, int len);

	/**@brief ���CRC32�ı�׼�ַ���ֵ
	
	��CRC32::getCrc32Int���ص�Intֵת���ɱ�׼���ַ���ֵ
	@param InStr ��Ҫ����CRC32ֵ������ָ��
	@param len ����CRC32���ڴ����ݵĳ���
	@param crc ���Σ��������CRC32�ַ��������ڴˣ�����=8
	 */
	static void getCrc32(const char *InStr, int len, char *crc);

private:
	static unsigned long Crc32Table[256];		/** Ԥ���CRC32�㷨������ [7/13/2009 xinl] */
};

/**@brief MD5�㷨

��һ�θ������ڴ����У�����MD5ָ��
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

/**@brief DES & 3DES �ӽ����㷨


 */
class COMMXAPI DES
{
public:
	/**@brief �ӽ���
	
	@param Out ��������������������(Out)�ĳ��� >= ((datalen+7)/8)*8,����datalen�������8�ı�������С������
	@param In ���뻺������In ����= Out,��ʱ��/���ܺ󽫸������뻺����(In)������
	@param datalen �������ݵĳ���
	@param Key ������Կ
	@param keylen ��keylen>8ʱϵͳ�Զ�ʹ��3��DES��/����,����ʹ�ñ�׼DES��/����.����16�ֽں�ֻȡǰ16�ֽ�
	@param bEncrypt �ӽ��ܷ���
	-true ����
	-false ����
	@return
	-true �ɹ�
	-false ʧ��
	 */
	static bool Des(char *Out, char *In, long datalen, const char *Key, int keylen, bool bEncrypt);

private:
	typedef bool (*PSubKey)[16][48];

	static void des(char Out[8], char In[8], const PSubKey pSubKey, bool bEncrypt);//��׼DES��/����
	static void SetSubKey(PSubKey pSubKey, const char Key[8]);// ��������Կ
	static void F_func(bool In[32], const bool Ki[48]);// f ����
	static void S_func(bool Out[32], const bool In[48]);// S �д���
	static void Transform(bool *Out, bool *In, const char *Table, int len);// �任
	static void Xor(bool *InA, const bool *InB, int len);// ���
	static void RotateL(bool *In, int len, int loop);// ѭ������
	static void ByteToBit(bool *Out, const char *In, int bits);// �ֽ���ת����λ��
	static void BitToByte(char *Out, const bool *In, int bits);// λ��ת�����ֽ���

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
