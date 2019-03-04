
/** @file		memmanager.h
 *	@brief		内存池管理头文件
 *	@ingroup mery
 *  
 *	@author:	freesense@126.com
 */

#if !defined(AFX_MEMMANAGER_H__EC9DD05D_ECFC_4D0D_B8C3_6FEC3257B5FF__INCLUDED_)
#define AFX_MEMMANAGER_H__EC9DD05D_ECFC_4D0D_B8C3_6FEC3257B5FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <vector>
#include "../public/synch.h"

/** @class	CMemManager	
 *	@brief	应用内存管理器	
 *
 *  本内存管理器预先分配一大块内存，将其分成若干分区分区，每一分区中包含若干粒相同
 *  大小的内存块，并使用单链表连接。分配时分配足够大小的某一块内存，将其从链表中删
 *  除，回收时将内存块重新挂接到单链表的链头
 @ingroup mery
 */
class CMemManager
{
	/** @class		OS_MEM
	 *	@brief		内存分区管理结构
	 *
	 *  
	 */
	struct OS_MEM
	{
		void			*OSMemAddr;			/**< 内存分区首地址 */
		void			*OSMemFreeList;		/**< 空闲内存块首地址 */
		UINT			OSMemBlkSize;		/**< 内存块大小 */
		UINT			OSMemNBlks;			/**< 内存分区中内存块总数 */
		UINT			OSMemNFree;			/**< 内存分区中空闲内存块个数 */
		LockSingle		m_csLock;			/**< 占用锁，表示本内存控制块已被占用 */
	};

#ifdef _DEBUG
	/**@brief 内存池统计信息
	
	@see Dump()
	 */
	struct ALLOC_STAT
	{
		OS_MEM *pOsMem;				/** 内存管理块地址 [6/25/2009 xinl] */
		char szFile[MAX_PATH];		/** 分配文件名 [6/25/2009 xinl] */
		UINT nLineNo;				/** 分配的源代码行号 [6/25/2009 xinl] */
	};
#else
	typedef OS_MEM* ALLOC_STAT;
#endif // _DEBUG

	typedef std::map<UINT, OS_MEM*> typeMapMem;	/**< 保存内存块大小和内存控制块指针的对应关系 */
	typedef std::vector<BYTE*> typeVecMem;		/**< 保存预分配的大块内存地址 */
	typedef std::map<void*, ALLOC_STAT> typeMapAlloc;	/**< 保存分配出去的内存地址与控制块之间的对应关系，以便回收 */

public:
	CMemManager();
	~CMemManager();

	/**@brief 
	
	@see CreateMemBlock()
	 */
	bool CreateMemBlock(UINT &nGranularity, UINT &nNumOfBlock);

#ifdef _DEBUG
	void* Malloc(UINT nSize, const char *lpFile, UINT nLineNo);
#else
	void* Malloc(UINT nSize);
#endif // _DEBUG
	void  Free(void *lp);

#ifdef _DEBUG
	void Dump();
#endif // _DEBUG

private:
	LockSingle m_lockMap;					/**< Map和Vector锁 */
	typeMapMem m_mpOsMem;					/**< 保存各内存分区控制块数据 */
	typeVecMem m_vcOsMem;					/**< 单独分配的内存块缓冲指针列表，仅用于释放内存 */

	LockSingle m_lockAlloc;
	typeMapAlloc m_mpAlloc;					/**< 已分配的内存指针 */

	inline UINT GetMemSize(UINT nSize);
	OS_MEM* GetOsMemOfSize(UINT nSize);

#ifdef _DEBUG
	UINT m_nTotalAllocated;
#endif // _DEBUG
};

///内存管理器指针
extern CMemManager *g_pMan;

#endif // !defined(AFX_MEMMANAGER_H__EC9DD05D_ECFC_4D0D_B8C3_6FEC3257B5FF__INCLUDED_)
