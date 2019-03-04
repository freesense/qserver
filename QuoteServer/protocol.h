
#ifndef __PROTOCOL_QUOTE_H__
#define __PROTOCOL_QUOTE_H__
#include "windows.h"
/** @brief   ͨѶ��ͷ
*  @ingroup 
*  
*/
static unsigned short	reverse_s(unsigned short	usData)
{
	return	(usData>>8)+((usData & 0xff)<<8);
}

static unsigned int	reverse_i(unsigned int	uiData)
{
	unsigned char	aucData[4];

	int i;
	for( i=0;i<4;i++)
		aucData[i]=uiData>>((3-i)*8);
	uiData=0;

	for(i=0;i<4;i++)
		uiData+=(aucData[i]<<(i*8));
	return	uiData;
}

struct CommxHead
{
	typedef unsigned char CommxProp;		/**< �ض���ͨѶ������ */
	typedef unsigned char CommxFeed;		/**< �ض���ͨѶ��·��Ŀ�� */

	CommxProp Prop;						/**< ͨѶ������ */
	CommxFeed Feed;						/**< ͨѶ��·��Ŀ�� */
	unsigned short SerialNo;			/**< ҵ������кţ�ԭ��ԭȥ */
	unsigned long Length;				/**< ҵ������ȣ�������#CommxHead��ͷ */

	enum BYTE_ORDER {HOSTORDER = 0, NETORDER};		/**< ö���ֽ��� */
	enum ENCODING {ANSI = 0, UTF16LE, UTF8};		/**< ö�ٱ��뷽ʽ */

	/** @brief      ����ͨѶ������
	*
	*  11110000 | 00001110 | 00000001
	*  @param      ver��ͨѶ���汾��
	*  @param      encoding��ҵ������뷽ʽ��0��ʾANSI�ַ�����
	*  @param      byteorder��ҵ����ֽ��������ֽ��������ֽ���
	*  @retval     CommxProp��ͨѶ������
	*  @warning    
	*  @see        
	*  @ingroup    
	*  @exception  
	*/
	static CommxProp MakeProp(unsigned char ver, ENCODING encoding, BYTE_ORDER byteorder)
	{
		return (ver << 4) | ((encoding << 2) & 0x0e) | (byteorder & 0x01);
	}

	/** @brief      ��ð汾��
	*
	*  11110000
	*  @retval     unsigned char���汾��
	*  @warning    
	*  @see        
	*  @ingroup    
	*  @exception  
	*/
	unsigned char GetVersion()
	{
		return ((Prop & 0xf0)>>4);
	}

	/** @brief      ���ҵ������뷽ʽ
	*
	*  00001110
	*  @retval     ENCODING�����뷽ʽ
	*  @warning    
	*  @see        
	*  @ingroup    
	*  @exception  
	*/
	ENCODING GetEncoding()
	{
		return (ENCODING)((Prop & 0x0e)>>2);
	}

	/** @brief      ���ҵ����ֽ���
	*
	*  00000001
	*  @retval     BYTE_ORDER���ֽ���
	*  @warning    
	*  @see        
	*  @ingroup    
	*  @exception  
	*/
	BYTE_ORDER GetByteorder()
	{
		return (BYTE_ORDER)(Prop & 0x01);
	}
	unsigned long SetLength(unsigned long c_un)
	{
		Length = c_un;
		if (GetByteorder() == NETORDER)
		{
			Length = reverse_i(Length);
		}
		return Length;
	}

	unsigned long GetLength()
	{
		if (GetByteorder() == NETORDER)
		{
			unsigned long nRet = Length;
			return reverse_i(nRet);
		}
		return Length;
	}

	unsigned short GetUS(unsigned short c_us)
	{
		if (GetByteorder() == NETORDER)
		{
			unsigned short us = c_us;
			return reverse_s(us);
		}
		return c_us;
	}

	unsigned long GetUI(unsigned long c_un)
	{
		if (GetByteorder() == NETORDER)
		{
			unsigned long nRet = c_un;
			return reverse_i(nRet);
		}
		return c_un;
	}

	//����Դ����Ч����
	int GetStr(char * c_pSrc,int c_nSrcLen,char * c_pDes,int c_nDataLen)
	{
		int nValidLen;
		if (GetEncoding() == ANSI)
		{
			nValidLen = strlen(c_pSrc);
			if (nValidLen > c_nSrcLen)
				nValidLen = c_nSrcLen;
			strncpy(c_pDes,c_pSrc,nValidLen);
			return nValidLen;//ֱ��ȡ����
		}
		else
		if (GetEncoding() == UTF16LE)
		{
			nValidLen = lstrlen((LPCWSTR)c_pSrc);
			if (nValidLen * 2 > c_nSrcLen)
				nValidLen = c_nSrcLen / 2;
			WideCharToMultiByte(CP_ACP,0,(LPCWSTR)c_pSrc,nValidLen,c_pDes,c_nDataLen,NULL,NULL);
			return nValidLen;
		}
		else
		if (GetEncoding() == UTF8)
		{
			nValidLen = strlen(c_pSrc);
			if (nValidLen > c_nSrcLen)
				nValidLen = c_nSrcLen;
			//��UTF8 to UNICODE
			int uncodeLen = MultiByteToWideChar(CP_UTF8, NULL, c_pSrc,nValidLen, NULL, 0);
			wchar_t* wszString = new wchar_t[uncodeLen + 1];
			::MultiByteToWideChar(CP_UTF8, NULL, c_pSrc, nValidLen, wszString, uncodeLen);
			//������'\0'
			wszString[uncodeLen] = '\0';

			//UNICODE to ANSI
			WideCharToMultiByte(CP_ACP,0,wszString,-1,c_pDes,c_nDataLen,NULL,NULL);
			delete [] wszString;
			return nValidLen;	
		}
		return -1;
	}

	//����Ŀ����Ч����
	int PutStr(char * c_pSrc,char * c_pDes,int c_nDataLen)
	{
		if (GetEncoding() == ANSI)
		{
			memset(c_pDes,0,c_nDataLen);
			strncpy(c_pDes,c_pSrc,c_nDataLen);
			return strlen(c_pDes)>c_nDataLen?c_nDataLen:strlen(c_pDes);
		}
		else
		if (GetEncoding() == UTF16LE)
		{
			//Ŀ������
			memset(c_pDes,0,c_nDataLen * 2);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,c_pSrc,
				c_nDataLen,(LPWSTR)(c_pDes),c_nDataLen);
			return lstrlen((LPWSTR)(c_pDes));
		}
		else
		if (GetEncoding() == UTF8)
		{
			memset(c_pDes,0,c_nDataLen * 3);
			//ANSI to UNICODE
			char * pTmp = new char[c_nDataLen * 2 + 2];
			memset(pTmp,0,c_nDataLen * 2 + 2);
			MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,c_pSrc,
				strlen(c_pSrc),(LPWSTR)(pTmp),c_nDataLen);
						
			//UNICODE to UTF8
			int u8Len = ::WideCharToMultiByte(CP_UTF8, NULL, (LPWSTR)(pTmp), wcslen((LPWSTR)(pTmp)), NULL, 0, NULL, NULL);
			 ::WideCharToMultiByte(CP_UTF8, NULL, (LPWSTR)(pTmp), wcslen((LPWSTR)(pTmp)), c_pDes, u8Len, NULL, NULL);
			//������'\0'
			c_pDes[u8Len] = '\0';
			delete [] pTmp;
			return strlen(c_pDes);
		}
		return -1;
	}

	bool IsValid()
	{
		if (GetVersion() != 0x03)
		{
			return false;
		}
		if (GetByteorder() != NETORDER && GetByteorder() != HOSTORDER)
		{
			return false;
		}
		if (GetEncoding() != ANSI && GetEncoding() != UTF8 && GetEncoding() != UTF16LE)
		{
			return false;
		}
		if (GetLength() < 6)
		{
			return false;
		}
		return true;
	}
};

#pragma pack()

#endif