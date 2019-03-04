
/** @file		HashTable.h
 *	@brief		��ϣ�����ļ�
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
 *	@brief		��ϣ��Ĺ�ϣ����
 *
 *  ��ĳ���������ͼ����ϣֵ
 *  @param		K ��������
 */
template <class K>
struct Hash
{
/// ���ػ����У����붨���������͵Ĺ�ϣ����
///	static inline unsigned long GetHash(const K& value);
};

struct from_blizzard
{/// ����Ϊ��ѩ��˾��ϣ�㷨 //////////////////////////////////////////////////////
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
// ���¶����ڴ�ʹ�ò���

template <class K, class V>
struct CHashBase
{
	static unsigned long GetSize(unsigned int headlen, unsigned int nBuckets, unsigned int nNodes)
	{
		return headlen + (4 + nBuckets) * sizeof(unsigned long) + sizeof(NODE) * nNodes;
	}

	/** @class		NODE
	 *	@brief		��ϣ��ڵ�
	 *
	 *
	 */
	struct NODE
	{
		K key;					/**< key */
		V value;				/**< value */
		unsigned long next;		/**< ��һ�ڵ������ֵ */
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

	unsigned long *m_pulBucketCount;			/**< Ͱ�ĸ��� */
	unsigned long *m_pulNodeCount;				/**< �ڵ�ĸ��� */
	unsigned long *m_pulIndex;					/**< �ڵ���������һ�����ýڵ������ */
	unsigned long *m_pulElementCount;			/**< Ԫ�صĸ��� */
	unsigned long *m_pBucket;					/**< Ͱ���ڴ�ռ� */
	NODE *m_pNode;								/**< �ڵ��ڴ�ռ� */
};

////////////////////////////////////////////////////////////////////////////////
//���¶����ڴ�������
//ʹ�ö��ڴ�
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

		///�����ýڵ㴮�������������һ���ڵ��nextΪ0����ʾ�޺������ýڵ�
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
		memcpy(lpData, m_lpData, nSize);	//����ԭ���Ļ��������ݵ��µĻ�����

		//ɾ��ԭ�������ݻ���
		char *lpTmpData = m_lpData;
		m_lpData = lpData;
		delete []lpTmpData;

		//���³�ʼ����ϣ���������
		m_pulBucketCount = (unsigned long*)m_lpData;
		m_pulNodeCount = (unsigned long*)(&m_lpData[sizeof(unsigned long)]);
		m_pulIndex = (unsigned long*)(&m_lpData[2*sizeof(unsigned long)]);
		m_pulElementCount = (unsigned long*)(&m_lpData[3*sizeof(unsigned long)]);
		m_pBucket = (unsigned long*)(&m_lpData[4*sizeof(unsigned long)]);
		m_pNode = (NODE*)(&m_lpData[(4+(*m_pulBucketCount))*sizeof(unsigned long)]);

		*m_pulIndex = *m_pulNodeCount;
		*m_pulNodeCount = (*m_pulNodeCount) * 2;

		///�����ýڵ㴮�������������һ���ڵ��nextΪ0����ʾ�޺������ýڵ�
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

//ʹ���ڴ�ӳ���ļ�
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
		{//���ļ�
			memset(m_lpData, 0x00, nSize);
			memcpy(m_lpData, head, headlen);
			*m_pulIndex = 1;
			*m_pulBucketCount = nBuckets;
			*m_pulNodeCount = nNodes;

			///�����ýڵ㴮�������������һ���ڵ��nextΪ0����ʾ�޺������ýڵ�
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
		{//���ļ�
			memset(m_lpData, 0x00, nSize);
			memcpy(m_lpData, head, headlen);
			*m_pulIndex = 1;
			*m_pulBucketCount = nBuckets;
			*m_pulNodeCount = nNodes;

			///�����ýڵ㴮�������������һ���ڵ��nextΪ0����ʾ�޺������ýڵ�
			for (unsigned long i = *m_pulIndex; i < (*m_pulNodeCount) - 1; i++)
				m_pNode[i].next = i + 1;
		}

		return true;
	}

	bool realloc()
	{//�ڴ�ӳ���ļ����Ȳ��ɶ�̬����
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
	HANDLE m_hFile;					//�ļ����
	HANDLE m_hMappingFile;			//ӳ����
#elif defined _POSIX_C_SOURCE
	int m_fd;						//�ļ�������
#endif

	char* m_lpData;					//ӳ����ڴ��׵�ַ
	unsigned long nBuckets;			//Ͱ��
	unsigned long nNodes;			//�ڵ���
};
////////////////////////////////////////////////////////////////////////////////

/** @class		CHashTable
 *	@brief		��ϣ��
 *
 *  - ʹ�����ƣ�Ͱ��Сһ��ȷ�����Ͳ��ɸ��ģ�����֮�⣬�ڵ���ɶ�̬��չ��
 *  - ����Լ��������Ĭ����1��ʼ��0��Ϊ�Ƿ�ֵ
 *  @param		K key������֧�ֿ������캯����operator==��operator=
 *  @param		V value
 *  @param		L Lock��һ�������������������Ͷ�д��
 *  @param		Allocator �ڴ�ʹ�ò���
 *  @param		HASH ��key�Ĺ�ϣֵ
 */
template <class K, class V, class L, class Allocator = HashAllocateNew<K, V>, class HASH = Hash<K> >
class CHashTable : public Allocator
{
	/** @class		LOCK
	 *	@brief		ͬ����
	 *
	 *  ���캯��������������������
	 */
	struct LOCK
	{
		public:
			enum LOCKTYPE {READ, WRITE};	/**< ��������ģʽ����������д�� */

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
			L *m_lock;				/**< ���湲����Դ��ָ�룬�Ա�������ʱʹ�� */
			int m_iLockSize;		/**< ��������ĳ��� */
			LOCKTYPE m_typeLocked;	/**< ������ */
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
	 *	@param		ulBucketCount Ͱ��С
	 *	@param		ulNodeCount �ڵ���
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
	/** @brief		����Ԫ��
	 *
	 *
	 *	@param		key
	 *	@param		value
	 *	@param		bRepeat	�Ƿ������ظ���Key
	 *	@return		����λ�õ�����
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
		{//��û���õ����Ͱ
			m_pBucket[nHash] = *m_pulIndex;
			ulIndex = *m_pulIndex;
		}
		else
		{//�Ѿ���Ԫ���õ������Ͱ
			while (1)
			{
				if (key == m_pNode[ulIndex].key && !bRepeat)
				{//Ҫ�����Ԫ���Ѿ����ڣ����Ҳ������ظ�Key����Ҫ���¸�ֵ
					m_pNode[ulIndex].value = value;
					return ulIndex;
				}

				if (!m_pNode[ulIndex].next)
				{//���Ͱ���Ԫ���Ѿ����ˣ�Ҫ����Ԫ��
					m_pNode[ulIndex].next = *m_pulIndex;
					ulIndex = *m_pulIndex;
					break;
				}
				ulIndex = m_pNode[ulIndex].next;
			}
		}

		*m_pulIndex = m_pNode[ulIndex].next;	//��һ�����õĽڵ�

		K empty;
		memcpy(&m_pNode[ulIndex].key, &empty, sizeof(K));
		m_pNode[ulIndex].key = key;

		m_pNode[ulIndex].value = value;
		m_pNode[ulIndex].next = 0;			//ͬ��һ��Ͱ�е���һ���ڵ�
		(*m_pulElementCount)++;

		return ulIndex;
	}

	/** @brief		ɾ��Ԫ��
	 *
	 *
	 *	@param		key
	 *	@return
	 *   - 0 û�����Ԫ��
	 *   - ���� ��Ԫ�ص�ԭ��������
	 */
	inline unsigned long DelElement(const K &key)
	{
		unsigned long nPrevious;
		unsigned long nHash = HASH::GetHash(key);
		nHash %= *m_pulBucketCount;
		LOCK lock(&m_pLock[nHash], LOCK::WRITE);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//��û���õ����Ͱ��Ҳ�Ͳ��������Ԫ��
			return 0;

		nPrevious = ulIndex;
		while (1)
		{
			if (key == m_pNode[ulIndex].key)
			{//�ҵ���Ҫɾ����Ԫ��
				m_pNode[nPrevious].next = m_pNode[ulIndex].next;
				if (ulIndex == m_pBucket[nHash])	//ɾ���������Ԫ��,�޸�Ͱ�����ʼֵ
					m_pBucket[nHash] = m_pNode[ulIndex].next;

				m_pNode[ulIndex].key.~K();
				m_pNode[ulIndex].value.~V();
				memset(&m_pNode[ulIndex], 0x00, sizeof(K) + sizeof(V));	//Ԫ������
				m_pNode[ulIndex].next = *m_pulIndex;
				*m_pulIndex = ulIndex;
				(*m_pulElementCount)--;
				break;
			}

			nPrevious = ulIndex;		//������������һ��Ԫ�ص�����ֵ
			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//û���ҵ�Ҫɾ����Ԫ��
		}

		return ulIndex;
	}

	/** @brief		����ĳ��Ԫ��
	 *
	 *
	 *	@param		key
	 *	@return		��Ԫ�ص�����
	 */
	inline unsigned long FindElement(const K &key)
	{
		unsigned long nHash = HASH::GetHash(key);
		nHash %= *m_pulBucketCount;
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//��û���õ����Ͱ��Ҳ�Ͳ��������Ԫ��
			return 0;

		while (1)
		{
			if (key == m_pNode[ulIndex].key)//�ҵ���Ԫ��
				return ulIndex;

			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//û���ҵ�Ԫ��
		}

		return 0;
	}

	//��ͬһ��Ͱ��ģ��������Func�������㷨
	template <class Func>
	inline unsigned long FindBucket(const K &key, Func _func)
	{
		unsigned long count = 0;
		unsigned long nHash = HASH::GetHash(key);
		nHash %= *m_pulBucketCount;
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//��û���õ����Ͱ��Ҳ�Ͳ��������Ԫ��
			return 0;

		while (1)
		{
			count++;
			_func.OnBucket(m_pNode[ulIndex].key, m_pNode[ulIndex].value);
			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//û���ҵ�Ԫ��
		}

		return count;
	}

	/** @brief		���ĳ��Ԫ��
	 *
	 *
	 *	@param		key
	 *	@param		value ���Σ��ҵ���Ԫ��ֵ���������
	 *	@return
	 *   - 0 û�����Ԫ��
	 *   - ���� ��Ԫ���ڽڵ��ڴ���е�����
	 */
	inline unsigned long GetElement(const K &key, V &value) const
	{
		unsigned long nHash = HASH::GetHash(key);
		nHash %= (*m_pulBucketCount);
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//��û���õ����Ͱ��Ҳ�Ͳ��������Ԫ��
			return 0;

		while (1)
		{
			if (key == m_pNode[ulIndex].key)
			{//�ҵ���Ԫ��
				value = m_pNode[ulIndex].value;
				return ulIndex;
			}

			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//û���ҵ�Ԫ��
		}

		return 0;
	}

	inline V* GetElement(const K &key) const
	{
		unsigned long nHash = HASH::GetHash(key);
		nHash %= (*m_pulBucketCount);
		LOCK lock(&m_pLock[nHash], LOCK::READ);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//��û���õ����Ͱ��Ҳ�Ͳ��������Ԫ��
			return 0;

		while (1)
		{
			if (key == m_pNode[ulIndex].key)//�ҵ���Ԫ��
				return &m_pNode[ulIndex].value;

			ulIndex = m_pNode[ulIndex].next;
			if (!ulIndex)
				break;		//û���ҵ�Ԫ��
		}

		return 0;
	}

	/** @brief		��ÿһ����ϣ��Ԫ������
	*
	*  �º��������Ԫ��ָ��
	*	@param		func ����º������º�������-1��ʾ��ֹö�ٹ�ϣ��Ԫ�أ�1��ʾ���ӷ��ؼ�����0��ʾ�����ӷ��ؼ���������ö�ټ���
	*	@return		���������Ԫ������
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

			if (!ulIndex)	//��û���õ����Ͱ��Ҳ�Ͳ��������Ԫ��
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

	/** @brief		�Թ�ϣ��Ԫ������
	 *
	 *  �º��������Ԫ��ָ��
	 *	@param		key
	 *	@param		func ����º���
	 *	@param		bQuitIfNoKey Key������ʱ�Ƿ����Key���ٽ�������
	 *	@return
	 *   - 0 û�����Ԫ��
	 *   - ���� ��Ԫ���ڽڵ��ڴ���е�����
	 */
	template <class F>
	inline long cast(const K &key, F &func, bool bQuitIfNoKey = true)
	{
		bool bFlag = true;		//����Ƿ�������һ��Ԫ��
		unsigned long nHash = HASH::GetHash(key);
		nHash %= (*m_pulBucketCount);
		LOCK lock(&m_pLock[nHash], LOCK::WRITE);

		unsigned long ulIndex = m_pBucket[nHash];
		if (!ulIndex)	//��û���õ����Ͱ��Ҳ�Ͳ��������Ԫ��
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
				{//���Ͱ���Ԫ���Ѿ�����
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
			*m_pulIndex = m_pNode[ulIndex].next;	//��һ�����õĽڵ�

			K empty;
			memcpy(&m_pNode[ulIndex].key, &empty, sizeof(K));
			m_pNode[ulIndex].key = key;

			m_pNode[ulIndex].next = 0;			//ͬ��һ��Ͱ�е���һ���ڵ�
			(*m_pulElementCount)++;
		}

		if (-1 == func.cast(&m_pNode[ulIndex].key, &m_pNode[ulIndex].value, bFlag))
			return -1;

		return ulIndex;
	}

	L *m_pLock;												/**< �ڴ��� */
	L m_lockRealloc;										/*�ط����ڴ�ʱ�Ķ�ռ��*/

private:
	//�������ƿ���
	CHashTable(const CHashTable&);
	CHashTable& operator = (const CHashTable&);
};

#endif
