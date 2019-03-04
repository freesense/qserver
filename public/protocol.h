
#ifndef __PROTOCOL_QUOTE_H__
#define __PROTOCOL_QUOTE_H__

#pragma pack(1)

/** @brief   ͨѶ��ͷ
*  @ingroup 
*  
*/
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
		return ((Prop & 0xf0) >> 4);
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
		return (ENCODING)((Prop & 0x0e) >> 2);
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
};

struct FeedHead
{
	unsigned short nFeedType;
	unsigned short nTime;
	unsigned short nCount;
	char szMarketCode[6];
	char szSymbolCode[8];
};

#pragma pack()

#endif
