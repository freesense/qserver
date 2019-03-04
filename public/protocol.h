
#ifndef __PROTOCOL_QUOTE_H__
#define __PROTOCOL_QUOTE_H__

#pragma pack(1)

/** @brief   通讯包头
*  @ingroup 
*  
*/
struct CommxHead
{
	typedef unsigned char CommxProp;		/**< 重定义通讯包属性 */
	typedef unsigned char CommxFeed;		/**< 重定义通讯包路由目标 */

	CommxProp Prop;						/**< 通讯包属性 */
	CommxFeed Feed;						/**< 通讯包路由目标 */
	unsigned short SerialNo;			/**< 业务包序列号，原来原去 */
	unsigned long Length;				/**< 业务包长度，不包括#CommxHead包头 */

	enum BYTE_ORDER {HOSTORDER = 0, NETORDER};		/**< 枚举字节序 */
	enum ENCODING {ANSI = 0, UTF16LE, UTF8};		/**< 枚举编码方式 */

	/** @brief      生成通讯包属性
	*
	*  11110000 | 00001110 | 00000001
	*  @param      ver，通讯包版本号
	*  @param      encoding，业务包编码方式（0表示ANSI字符集）
	*  @param      byteorder，业务包字节序（网络字节序，主机字节序）
	*  @retval     CommxProp，通讯包属性
	*  @warning    
	*  @see        
	*  @ingroup    
	*  @exception  
	*/
	static CommxProp MakeProp(unsigned char ver, ENCODING encoding, BYTE_ORDER byteorder)
	{
		return (ver << 4) | ((encoding << 2) & 0x0e) | (byteorder & 0x01);
	}

	/** @brief      获得版本号
	*
	*  11110000
	*  @retval     unsigned char，版本号
	*  @warning    
	*  @see        
	*  @ingroup    
	*  @exception  
	*/
	unsigned char GetVersion()
	{
		return ((Prop & 0xf0) >> 4);
	}

	/** @brief      获得业务包编码方式
	*
	*  00001110
	*  @retval     ENCODING，编码方式
	*  @warning    
	*  @see        
	*  @ingroup    
	*  @exception  
	*/
	ENCODING GetEncoding()
	{
		return (ENCODING)((Prop & 0x0e) >> 2);
	}

	/** @brief      获得业务包字节序
	*
	*  00000001
	*  @retval     BYTE_ORDER，字节序
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
