
/** @file		HashTable.h
 *	@brief		哈希表定义文件
 *
 *
 *	@author:	freesense@126.com
 */

#ifndef __HASHTABLE_XINLIANG_H__
#define __HASHTABLE_XINLIANG_H__

#pragma warning(disable:4786)

#include <assert.h>
#include <utility>
#include <string>
#ifdef _WIN32
	#ifndef _WINDOWS_
		#include <windows.h>
	#endif
#elif defined _POSIX_C_SOURCE
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <sys/mman.h>
#endif

/** @class		Hash<K>
 *	@brief		哈希表的哈希函数
 *
 *  对某种数据类型计算哈希值
 *  @param		K 数据类型
 */
template <class K>
struct Hash
{
/// 在特化类中，必须定义如下类型的哈希函数
///	static inline unsigned long GetHash(const K& value);
};

struct from_blizzard
{/// 以下为暴雪公司哈希算法 //////////////////////////////////////////////////////
	static unsigned long cryptTable[0x500];
	static bool bInitialized;

	static void prepareCryptTable();
	static unsigned long HashString(unsigned long dwHashType, const char *lpsz, unsigned int len = 0);
};

////////////////////////////////////////////////////////////////////////////////

template <>
struct Hash<unsigned int>
{
	static inline unsigned long GetHash(const unsigned int& value)
	{
		return value;
	}
};

template <>
struct Hash<std::string>
{
	static inline unsigned long GetHash(const std::string& value)
	{
		return from_blizzard::HashString(0, (char*)value.c_str());
	}
};

template <>
struct Hash<char*>
{
	static inline unsigned long GetHash(const char*& value)
	{
		return from_blizzard::HashString(0, (char*)value);
	}
};
////////////////////////////////////////////////////////////////////////////////
// 以下定义内存使用策略

template <class K, class V>
struct CHashBase
{
	static unsigned long GetSize(unsigned int headlen, unsigned int nBuckets, unsigned int nNodes)
	{
		return headlen + (4 + nBuckets) * sizeof(unsigned long) + sizeof(NODE) * nNodes;
	}

	/** @class		NODE
	 *	@brief		哈希表节点
	 *
	 *
	 */
	struct NODE
	{
		K key;					/**< key */
		V value;				/**< value */
		unsigned long next;		/**< 下一节点的索引值 */
	};

	CHashBase()
	{
		m_pulBucketCount = 0;
		m_pulNodeCount = 0;
		m_pulIndex = 0;
		m_pulElementCount = 0;
		m_pBucket = 0;
		m_pNode = 0;
	}

	virtual unsigned long bucket()
	{
		return *m_pulBucketCount;
	}

	virtual unsigned long node()
	{
		return *m_pulNodeCount;
	}

	virtual unsigned long size()
	{
		return *m_pulElementCount;
	}

	unsigned long *m_pulBucketCount;			/**< 桶的个数 */
	unsigned long *m_pulNodeCount;				/**< 节点的个数 */
	unsigned long *m_pulIndex;					/**< 节点向量中下一个可用节点的索引 */
	unsigned long *m_pulElementCount;			/**< 元素的个数 */
	unsigned long *m_pBucket;					/**< 桶的内存空间 */
	NODE *m_pNode;								/**< 节点内存空间 */
};

////////////////////////////////////////////////////////////////////////////////
//以下定义内存分配策略
//使用堆内存
template <class K, class V>
class HashAllocateNew : public CHashBase<K, V>
{
public:
	HashAllocateNew() : m_lpData(0) {}

	bool malloc(unsigned long nBuckets, unsigned long nNodes)
	{
		unsigned long nSize = HashAllocateNew<K, V>::GetSize(0, nBuckets, nNodes);
		m_lpData = new char[nSize];
		if (!m_lpData)
			return false;

		m_pulBucketCount = (unsigned long*)m_lpData;
		m_pulNodeCount = (unsigned long*)(&m_lpData[sizeof(unsigned long)]);
		m_pulIndex = (unsigned long*)(&m_lpData[2*sizeof(unsigned long)]);
		m_pulElementCount = (unsigned long*)(&m_lpData[3*sizeof(unsigned long)]);
		m_pBucket = (unsigned long*)(&m_lpData[4*sizeof(unsigned long)]);
		m_pNode = (NODE*)(&m_lpData[(4+nBuckets)*sizeof(unsigned long)]);

		memset(m_lpData, 0x00, nSize);
		*m_pulIndex = 1;
		*m_pulBucketCount = nBuckets;
		*m_pulNodeCount = nNodes;

		///将可用节点串成链，但是最后一个节点的next为0，表示无后续可用节点
		for (unsigned long i = *m_pulIndex; i < (*m_pulNodeCount) - 1; i++)
			m_pNode[i].next = i + 1;

		return true;
	}

	bool realloc()
	{
		unsigned long nSize = HashAllocateNew<K, V>::GetSize(0, *m_pulBucketCount, (*m_pulNodeCount) * 2);
		char* lpData = new char[nSize];
		if (!lpData)
			return false;
		memset(lpData, 0x00, nSize);

		nSize = HashAllocateNew<K, V>::GetSize(0, *m_pulBucketCount, *m_pulNodeCount);
		memcpy(lpData, m_lpData, nSize);	//拷贝原来的缓冲区内容到新的缓冲区

		//删除原来的数据缓冲
		char *lpTmpData = m_lpData;
		m_lpData = lpData;
		delete []lpTmpData;

		//重新初始化哈希表管理数据
		m_pulBucketCount = (unsigned long*)m_lpData;
		m_pulNodeCount = (unsigned long*)(&m_lpData[sizeof(unsigned long)]);
		m_pulIndex = (unsigned long*)(&m_lpData[2*sizeof(unsigned long)]);
		m_pulElementCount = (unsigned long*)(&m_lpData[3*sizeof(unsigned long)]);
		m_pBucket = (unsigned long*)(&m_lpData[4*sizeof(unsigned long)]);
		m_pNode = (NODE*)(&m_lpData[(4+(*m_pulBucketCount))*sizeof(unsigned long)]);

		*m_pulIndex = *m_pulNodeCount;
		*m_pulNodeCount = (*m_pulNodeCount) * 2;

		///将可用节点串成链，但是最后一个节点的next为0，表示无后续可用节点
		for (unsigned long i = *m_pulIndex; i < (*m_pulNodeCount) - 1; i++)
			m_pNode[i].next = i + 1;

			return true;
	}

	~HashAllocateNew()
	{
		delete []m_lpData;
	}

	int offset(char *lp)
	{
		return lp - m_lpData;
	}

	void* offset(int distance)
	{
		return m_lpData + distance;
	}

private:
	char* m_lpData;
};

//使用内存映射文件
template <class K, class V>
class HashAllocateMMapFile : public CHashBase<K, V>
{
public:
	HashAllocateMMapFile() : m_lpData(0), m_hFile(0), m_hMappingFile(0) {}

	bool malloc(unsigned long nBucketCount, unsigned long nNodeCount)
	{
		nBuckets = nBucketCount;
		nNodes = nNodeCount;
		return true;
	}

	bool mapmem(char *lpBuffer, void* head, unsigned int headlen)
	{
		unsigned long nSize = HashAllocateMMapFile<K, V>::GetSize(headlen, nBuckets, nNodes);
		m_lpData = lpBuffer;

		m_pulBucketCount = (unsigned long*)(&m_lpData[headlen]);
		m_pulNodeCount = (unsigned long*)(&m_lpData[headlen+sizeof(unsigned long)]);
		m_pulIndex = (unsigned long*)(&m_lpData[headlen+2*sizeof(unsigned long)]);
		m_pulElementCount = (unsigned long*)(&m_lpData[headlen+3*sizeof(unsigned long)]);
		m_pBucket = (unsigned long*)(&m_lpData[headlen+4*sizeof(unsigned long)]);
		m_pNode = (NODE*)(&m_lpData[headlen+(4+nBuckets)*sizeof(unsigned long)]);

		if (*m_pulBucketCount != 0)
			memcpy(head, m_lpData, headlen);
		else
		{//新文件
			memset(m_lpData, 0x00, nSize);
			memcpy(m_lpData, head, headlen);
			*m_pulIndex = 1;
			*m_pulBucketCount = nBuckets;
			*m_pulNodeCount = nNodes;

			///将可用节点串成链，但是最后一个节点的next为0，表示无后续可用节点
			for (unsigned long i = *m_pulIndex; i < (*m_pulNodeCount) - 1; i++)
				m_pNode[i].next = i + 1;
		}
		return true;
	}

	bool mapfile(const char* lpszFileName, void* head, unsigned int headlen)
	{
		unsigned long nSize = HashAllocateMMapFile<K, V>::GetSize(headlen, nBuckets, nNodes);

#ifdef _WIN32
		m_hFile = CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		m_hMappingFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, nSize, NULL);
		m_lpData = (char*)MapViewOfFile(m_hMappingFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#elif defined _POSIX_C_SOURCE
		m_fd = open(lpszFileName, O_RDWR|O_CREAT);
		m_lpData = (char*)mmap(0, nSize, PROT_READ|PROT_READ, MAP_SHARED, m_fd, 0);
#endif

		if (!m_lpData)
			return false;

		m_pulBucketCount = (unsigned long*)(&m_lpData[headlen]);
		m_pulNodeCount = (unsigned long*)(&m_lpData[headlen+sizeof(unsigned long)]);
		m_pulIndex = (unsigned long*)(&m_lpData[headlen+2*sizeof(unsigned long)]);
		m_pulElementCount = (unsigned long*)(&m_lpData[headlen+3*sizeof(unsigned long)]);
		m_pBucket = (unsigned long*)(&m_lpData[headlen+4*sizeof(unsigned long)]);
		m_pNode = (NODE*)(&m_lpData[headlen+(4+nBuckets)*sizeof(unsigned long)]);

		if (*m_pulBucketCount != 0)
			memcpy(head, m_lpData, headlen);
		else
		{//新文件
			memset(m_lpData, 0x00, nSize);
			memcpy(m_lpData, head, headlen);
			*m_pulIndex = 1;
			*m_pulBucketCount = nBuckets;
			*m_pulNodeCount = nNodes;

			///将可用节点串成链，但是最后一个节点的next为0，表示无后续可用节点
			for (unsigned long i = *m_pulIndex; i < (*m_pulNodeCount) - 1; i++)
				m_pNode[i].next = i + 1;
		}

		return true;
	}

	bool realloc()
	{//内存映射文件长度不可动态增加
		return false;
	}

	virtual unsigned long bucket()
	{
		return nBuckets;
	}

	virtual unsigned long node()
	{
		return nNodes;
	}

	~HashAllocateMMapFile()
	{
#ifdef _WIN32
		m_lpData = 0;
		CloseHandle(m_hMappingFile);
		CloseHandle(m_hFile);
#elif defined _POSIX_C_SOURCE
		munmap(m_lpData);
		close(m_fd);
		m_lpData = 0;
#endif
	}

	int offset(char *lp)
	{
		return lp - m_lpData;
	}

	void* offset(int distance)
	{
		return m_lpData + distance;
	}

private:
#ifdef _WIN32
	HANDLE m_hFile;					//文件句柄
	HANDLE m_hMappingFile;			//映射句柄
#elif defined _POSIX_C_SOURCE
	int m_fd;						//文件描述符
#endif

	char* m_lpData;					//映射的内存首地址
	unsigned long nBuckets;			//桶数
	unsigned long nNodes;			//节点数
};
////////////////////////////////////////////////////////////////////////////////

/** @class		CHashTable
 *	@brief		哈希表
 *
 *  - 使用限制：桶大小一旦确定，就不可更改，除此之外，节点域可动态扩展。
 *  - 索引约定：索引默认以1开始，0作为非法值
 *  @param		K key，必须支持拷贝构造函数，operator==，operator=
 *  @param		V value
 *  @param		L Lock，一般有两种锁，自由锁和读写锁
 *  @param		Allocator 内存使用策略
 *  @param		HASH 求key的哈希值
 */
template <class K, class V, class L, class Allocator = HashAllocateNew<K, V>, class HASH = Hash<K> >
class CHashTable : public Allocator
{
	/** @class		LOCK
	 *	@brief		同步锁
	 *
	 *  构造函数加锁，析构函数解锁
	 */
	struct LOCK
	{
		public:
			enum LOCKTYPE {READ, WRITE};	/**< 定义锁的模式：读锁或者写锁 */

			LOCK(L *pMutex, LOCKTYPE type, int iLockSize = -1)
			{
				m_lock = pMutex;
				m_iLockSize = iLockSize;

				if (-1 == iLockSize)
				{
					if (type == READ)
						m_lock->WaitRead();
					else
						m_lock->WaitWrite();
				}
				else
				{
					if (type == READ)
					{
						for (int i = 0; i < m_iLockSize; i++)
							m_lock[i].WaitRead();
					}
					else
					{
						for (int i = 0; i < m_iLockSize; i++)
							m_lock[i].WaitWrite();
					}
				}
			}

			~LOCK()
			{
				if (-1 == m_iLockSize)
				{
					if (m_typeLocked == READ)
						m_lock->EndRead();
					else
						m_lock->EndWrite();
				}
				else
				{
					if (m_typeLocked == READ)
					{
						for (int i = 0; i < m_iLockSize; i++)
							m_lock[i].EndRead();
					}
					else
					{
						for (int i = 0; i < m_iLockSize; i++)
							m_lock[i].EndWrite();
					}
				}
			}

		private:
			L *m_lock;				/**< 保存共享资源锁指针，以便于析构时使用 */
			int m_iLockSize;		/**< 保存加锁的长度 */
			LOCKTYPE m_typeLocked;	/**< 锁类型 */
	};

	bool realloc()
	{
		LOCK lock(&m_lockRealloc, LOCK::WRITE);
		if (*m_pulIndex == 0)
		{
			LOCK lock(m_pLock, LOCK::WRITE, bucket());
			return Allocator::realloc();
		}
		return true;
	}

public:
	CHashTable() : m_pLock(0) {}
	~CHashTable()
	{
		delete []m_pLock;
	}

	/** @brief
	 *
	 *
	 *	@param		ulBucketCount 桶大小
	 *	@param		ulNodeCount 节点数
	 */
	CHashTable(unsigned long ulBucketCount, unsigned long ulNodeCount)
	{
		if (!malloc(ulBucketCount, ulNodeCount))
			throw 0;
	}

	bool malloc(unsigned long nBuckets, unsigned long nNodes)
	{
		if (!Allocator::malloc(nBuckets, nNodes))
			return false;
		m_pLock = new L[bucket()];
		return true;
	}

public:
	/** @brief		增加元素
	 *
	 *
	 *	@param		key
	 *	@param		value
	 *	@param		bRepeat	是否允许重复的Key
	 *	@return		增加位置的索引
	 */
	inline unsigned long AddElement(const K &key, const V &value, bool bRepeat = false)
	{
		if (!realloc())
			return 0;

		unsigned long nHash = HASH::GetHash(key);
		nHash %= (*m_pulBucketCount);
		LOCK lock(&m_pLock[nHash], LOCK::WRITE);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)
		{//还没有用到这个桶
			m_pBucket[nHash] = *m_pulIndex;
			ulIndex = *m_pulIndex;
		}
		else
		{//已经有元素用到了这个桶
			while (1)
			{
				if (key == m_pNode[ulIndex].key && !bRepeat)
				{//要加入的元素已经存在，并且不允许重复Key，需要重新赋值
					m_pNode[ulIndex].value = value;
					return ulIndex;
				}

				if (!m_pNode[ulIndex].next)
				{//这个桶里的元素已经完了，要增加元素
					m_pNode[ulIndex].next = *m_pulIndex;
					ulIndex = *m_pulIndex;
					break;
				}
				ulIndex = m_pNode[ulIndex].next;
			}
		}

		*m_pulIndex = m_pNode[ulIndex].next;	//下一个可用的节点

		K empty;
		memcpy(&m_pNode[ulIndex].key, &empty, sizeof(K));
		m_pNode[ulIndex].key = key;

		m_pNode[ulIndex].value = value;
		m_pNode[ulIndex].next = 0;			//同在一个桶中的下一个节点
		(*m_pulElementCount)++;

		return ulIndex;
	}

	/** @brief		删除元素
	 *
	 *
	 *	@param		key
	 *	@return
	 *   - 0 没有这个元素
	 *   - 其它 该元素的原来的索引
	 */
	inline unsigned long DelElement(const K &key)
	{
		unsigned long nPrevious;
		unsigned long nHash = HASH::GetHash(key);
		nHash %= *m_pulBucketCount;
		LOCK lock(&m_pLock[nHash], LOCK::WRITE);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//还没有用到这个桶，也就不存在这个元素
			return 0;

		nPrevious = ulIndex;
		while (1)
		{
			if (key == m_pNode[ulIndex].key)
			{//找到了要删除的元素
				m_pNode[nPrevious].next = m_pNode[ulIndex].next;
				if (ulIndex == m_pBucket[nHash])	//删除链表的首元素,修改桶里的起始值
					m_pBucket[nHash] = m_pNode[ulIndex].next;

				m_pNode[ulIndex].key.~K();
				m_pNode[ulIndex].value.~V();
				memset(&m_pNode[ulIndex], 0x00, sizeof(K) + sizeof(V));	//元素清零
				m_pNode[ulIndex].next = *m_pulIndex;
				*m_pulIndex = ulIndex;
				(*m_pulElementCount)--;
				break;
			}

			nPrevious = ulIndex;		//保存链表中上一个元素的索引值
			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//没有找到要删除的元素
		}

		return ulIndex;
	}

	/** @brief		搜索某个元素
	 *
	 *
	 *	@param		key
	 *	@return		该元素的索引
	 */
	inline unsigned long FindElement(const K &key)
	{
		unsigned long nHash = HASH::GetHash(key);
		nHash %= *m_pulBucketCount;
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//还没有用到这个桶，也就不存在这个元素
			return 0;

		while (1)
		{
			if (key == m_pNode[ulIndex].key)//找到了元素
				return ulIndex;

			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//没有找到元素
		}

		return 0;
	}

	//在同一个桶中模糊搜索，Func是搜索算法
	template <class Func>
	inline unsigned long FindBucket(const K &key, Func _func)
	{
		unsigned long count = 0;
		unsigned long nHash = HASH::GetHash(key);
		nHash %= *m_pulBucketCount;
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//还没有用到这个桶，也就不存在这个元素
			return 0;

		while (1)
		{
			count++;
			_func.OnBucket(m_pNode[ulIndex].key, m_pNode[ulIndex].value);
			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//没有找到元素
		}

		return count;
	}

	/** @brief		获得某个元素
	 *
	 *
	 *	@param		key
	 *	@param		value 出参，找到的元素值存放在这里
	 *	@return
	 *   - 0 没有这个元素
	 *   - 其它 该元素在节点内存块中的索引
	 */
	inline unsigned long GetElement(const K &key, V &value) const
	{
		unsigned long nHash = HASH::GetHash(key);
		nHash %= (*m_pulBucketCount);
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//还没有用到这个桶，也就不存在这个元素
			return 0;

		while (1)
		{
			if (key == m_pNode[ulIndex].key)
			{//找到了元素
				value = m_pNode[ulIndex].value;
				return ulIndex;
			}

			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//没有找到元素
		}

		return 0;
	}

	inline V* GetElement(const K &key) const
	{
		unsigned long nHash = HASH::GetHash(key);
		nHash %= (*m_pulBucketCount);
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//还没有用到这个桶，也就不存在这个元素
			return 0;

		while (1)
		{
			if (key == m_pNode[ulIndex].key)//找到了元素
				return &m_pNode[ulIndex].value;

			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//没有找到元素
		}

		return 0;
	}

	/** @brief		对每一个哈希表元素运算
	*
	*  仿函数将获得元素指针
	*	@param		func 运算仿函数，仿函数返回-1表示中止枚举哈希表元素，1表示增加返回计数，0表示不增加返回计数，但是枚举继续
	*	@return		参与运算的元素数量
	*/
	template <class F>
	inline unsigned long for_each(F &func)
	{
		unsigned long nCount = 0;
		int ir = 0;
		for (unsigned long i = 0; i < *m_pulBucketCount && ir != -1; i++)
		{
			LOCK lock(&m_pLock[i], LOCK::WRITE);
			unsigned long ulIndex = m_pBucket[i];

			if (!ulIndex)	//还没有用到这个桶，也就不存在这个元素
				continue;

			while (1)
			{
				ir = func.OnElement(&m_pNode[ulIndex].key, &m_pNode[ulIndex].value);
				if (ir == 1)
					nCount++;
				else if (ir == -1)
					break;
				ulIndex = m_pNode[ulIndex].next;
				if (ulIndex == 0)
					break;
			}
		}
		return nCount;
	}

	/** @brief		对哈希表元素运算
	 *
	 *  仿函数将获得元素指针
	 *	@param		key
	 *	@param		func 运算仿函数
	 *	@param		bQuitIfNoKey Key不存在时是否插入Key，再进行运算
	 *	@return
	 *   - 0 没有这个元素
	 *   - 其它 该元素在节点内存块中的索引
	 */
	template <class F>
	inline long cast(const K &key, F &func, bool bQuitIfNoKey = true)
	{
		bool bFlag = true;		//标记是否新增了一个元素
		unsigned long nHash = HASH::GetHash(key);
		nHash %= (*m_pulBucketCount);
		LOCK lock(&m_pLock[nHash], LOCK::WRITE);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//还没有用到这个桶，也就不存在这个元素
		{
			if (bQuitIfNoKey || !realloc())
				return 0;

			m_pBucket[nHash] = *m_pulIndex;
			ulIndex = *m_pulIndex;
		}
		else
		{
			while (1)
			{
				if (key == m_pNode[ulIndex].key)
				{
					bFlag = false;
					break;
				}

				if (!m_pNode[ulIndex].next)
				{//这个桶里的元素已经完了
					if (bQuitIfNoKey || !realloc())
						return 0;
					else
					{
						m_pNode[ulIndex].next = *m_pulIndex;
						ulIndex = *m_pulIndex;
						break;
					}
				}
				ulIndex = m_pNode[ulIndex].next;
			}
		}

		if (bFlag)
		{
			*m_pulIndex = m_pNode[ulIndex].next;	//下一个可用的节点

			K empty;
			memcpy(&m_pNode[ulIndex].key, &empty, sizeof(K));
			m_pNode[ulIndex].key = key;

			m_pNode[ulIndex].next = 0;			//同在一个桶中的下一个节点
			(*m_pulElementCount)++;
		}

		if (-1 == func.cast(&m_pNode[ulIndex].key, &m_pNode[ulIndex].value, bFlag))
			return -1;

		return ulIndex;
	}

	L *m_pLock;												/**< 内存锁 */
	L m_lockRealloc;										/*重分配内存时的独占锁*/

private:
	//不允许复制拷贝
	CHashTable(const CHashTable&);
	CHashTable& operator = (const CHashTable&);
};

#endif
