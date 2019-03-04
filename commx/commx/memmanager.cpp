
/** @file		memmanager.cpp
 *	@brief		内存池管理实现文件
 *
 *  
 *	@author:	freesense@126.com
 */

#include "stdafx.h"
#include "../public/mery.h"
#include "../public/report.h"
#include "MemManager.h"
#include "math.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemManager::CMemManager()
{
#ifdef _DEBUG
	m_nTotalAllocated = 0;
#endif // _DEBUG
}

CMemManager::~CMemManager()
{
	typedef typeVecMem::iterator typeVecIter;
	for (typeVecIter vecIter = m_vcOsMem.begin(); vecIter != m_vcOsMem.end(); ++vecIter)
		delete [](*vecIter);
	m_vcOsMem.clear();

	typedef typeMapMem::iterator typeMapIter;
	for (typeMapIter mapIter = m_mpOsMem.begin(); mapIter != m_mpOsMem.end(); ++mapIter)
		delete mapIter->second;
	m_mpOsMem.clear();
}

/** @brief		单独创建某一内存粒度的内存分区
 *
 *  
 *	@param		nGranularity 内存粒度
 *	@param		nNumOfBlock 多少块这样的内存粒度
 *	@return
 *   - true 创建成功
 *   - false 创建失败
 */
bool CMemManager::CreateMemBlock(UINT &nGranularity, UINT &nNumOfBlock)
{
	if (nGranularity % 8)
		nGranularity = ((nGranularity / 8) + 1) * 8;

	//分配堆内存
	BYTE *pNewBuffer = new BYTE[nGranularity * nNumOfBlock];
	if (!pNewBuffer)
		return false;

	//构造单向链表
	void **ppLink = (void**)pNewBuffer;
	BYTE *pBlk = (BYTE*)pNewBuffer + nGranularity;
	for (UINT i = 0; i < (nNumOfBlock - 1); i++)
	{
		*ppLink = (void*)pBlk;
		ppLink = (void**)pBlk;
		pBlk = pBlk + nGranularity;
	}

	m_lockMap.lock();
	typeMapMem::iterator iter = m_mpOsMem.find(nGranularity);
	if (iter != m_mpOsMem.end())
	{//该内存粒度已经存在
		OS_MEM *pOsMem = iter->second;
		m_vcOsMem.push_back(pNewBuffer);
		m_lockMap.unlock();

		pOsMem->m_csLock.lock();

		*ppLink = pOsMem->OSMemFreeList;
		pOsMem->OSMemAddr = pNewBuffer;
		pOsMem->OSMemFreeList = pNewBuffer;
		pOsMem->OSMemNBlks += nNumOfBlock;
		pOsMem->OSMemNFree += nNumOfBlock;

		pOsMem->m_csLock.unlock();
	}
	else
	{//该内存粒度不存在
		//分配内存控制块
		OS_MEM *os_mem = new OS_MEM;
		if (!os_mem)
		{
			delete []pNewBuffer;
			return false;
		}

		//初始化内存控制块信息
		os_mem->OSMemAddr = pNewBuffer;
		os_mem->OSMemFreeList = pNewBuffer;
		os_mem->OSMemBlkSize = nGranularity;
		os_mem->OSMemNBlks = nNumOfBlock;
		os_mem->OSMemNFree = nNumOfBlock;

		//加入内存控制块map，并将缓冲区指针保存，以备析构时释放内存
		if (m_mpOsMem.insert(std::make_pair(nGranularity, os_mem)).second)
			m_vcOsMem.push_back(pNewBuffer);
		else	//内存控制块加入失败，释放掉内存块和控制块的指针
		{
			m_lockMap.unlock();
			delete os_mem;
			delete []pNewBuffer;
			return false;
		}
		m_lockMap.unlock();
	}

#ifdef _DEBUG
	m_nTotalAllocated += (nGranularity * nNumOfBlock);
#endif // _DEBUG

	return true;
}

/** @brief		分配内存
 *
 *  如果要分配的内存分区中所有内存块都已经分配出去，没有空闲内存块，
 *  本函数会试图扩大该内存分区，即再分配一大块内存，将其格式化，并
 *  挂到对应的内存控制块上
 *	@param		nSize 请求内存大小
 *	@return
 *   - 0 分配失败
 *   - void* 分配的内存地址
 */
void* CMemManager::Malloc(UINT nSize
#ifdef _DEBUG
						  ,
						  const char *lpFile,
						  UINT nLineNo
#endif
						  )
{
	void *pBlk;

	OS_MEM *pOsMem = GetOsMemOfSize(nSize);
	if (!pOsMem)
		return 0;

	while (1)
	{
		pOsMem->m_csLock.lock();
		if (!pOsMem->OSMemNFree)
		{//无可用的空闲内存块，扩大该内存分区
			unsigned int nGranularity = pOsMem->OSMemBlkSize;
			unsigned int nSize = pOsMem->OSMemNBlks;

			//这里要注意：当第一个线程已经开始扩大分区时，有可能其他线程也发现了无
			//可用的空闲内存块，也会试图扩大分区。要避免这种情况，只能由第一个线程
			//在扩大内存分区时上锁，让其他线程等到第一个线程将分区扩充完毕时再判断
			//是否有可用的空闲内存块。
			try
			{
				if (!CreateMemBlock(nGranularity, nSize))
				{
					pOsMem->m_csLock.unlock();
					return 0;
				}
			}
			catch (...)
			{
				pOsMem->m_csLock.unlock();
				return 0;
			}
			pOsMem->m_csLock.unlock();
			continue;
		}

		pBlk = pOsMem->OSMemFreeList;
		pOsMem->OSMemFreeList = *(void**)pBlk;
		pOsMem->OSMemNFree--;
		pOsMem->m_csLock.unlock();
		break;
	}

#ifdef _DEBUG
	ALLOC_STAT as;
	as.pOsMem = pOsMem;
	std::string s(lpFile);
	s = s.substr(s.rfind("\\")+1);
	strcpy(as.szFile, s.c_str());
	as.nLineNo = nLineNo;
#endif // _DEBUG

	m_lockAlloc.lock();
#ifdef _DEBUG
	m_mpAlloc[pBlk] = as;
#else
	m_mpAlloc[pBlk] = pOsMem;
#endif
	m_lockAlloc.unlock();
	return pBlk;
}

/** @brief		回收内存
 *
 *  回收后该段内存成为空闲内存，并重新挂到链表的链头
 *	@param		lp 要回收的内存地址,由#Malloc(UINT)分配出去
 *	@param		nSize 在#Malloc(UINT)分配时的参数
 */
void CMemManager::Free(void *lp)
{
	OS_MEM *pOsMem = 0;
	m_lockAlloc.lock();
	typeMapAlloc::iterator iter = m_mpAlloc.find(lp);
	if (iter != m_mpAlloc.end())
	{
#ifdef _DEBUG
		pOsMem = iter->second.pOsMem;
#else
		pOsMem = iter->second;
#endif // _DEBUG
		m_mpAlloc.erase(iter);
	}
	m_lockAlloc.unlock();

	if (pOsMem == NULL)
		return;

	pOsMem->m_csLock.lock();
	*(void**)lp = pOsMem->OSMemFreeList;
	pOsMem->OSMemFreeList = lp;
	pOsMem->OSMemNFree++;
	pOsMem->m_csLock.unlock();
}

/** @brief		计算实际分配的内存块大小
 *
 *  请求分配内存时，获得大于等于请求大小的最接近的内存块的大小
 *	@param		nSize 请求分配的内存大小
 *	@return		实际分配的内存块大小
 */
UINT CMemManager::GetMemSize(UINT nSize)
{
	int i = 2;
	while (((UINT)1 << i) < nSize)
		i++;
	return ((UINT)1 << i);
}

/** @brief		获得与指定大小相对应的内存控制块指针
 *
 *  如果请求的大小超过内存池中最大内存块的大小，本函数将构造一块
 *  内存块足以容纳请求大小的新的内存分区
 *	@param		nSize 请求分配的内存大小
 *	@return		内存控制块指针
 */
CMemManager::OS_MEM* CMemManager::GetOsMemOfSize(UINT nSize)
{
	OS_MEM *pOsMem = 0;
	m_lockMap.lock();
	typeMapMem::iterator iter = m_mpOsMem.lower_bound(nSize);
	if (iter != m_mpOsMem.end())
		pOsMem = iter->second;
	m_lockMap.unlock();

	if (!pOsMem)
	{
		//没有这么大的内存块，需要新分配
		//首先只分配一块，随后如果不够用的话，新分配的块数会向上递增
		UINT nGranularity = GetMemSize(nSize);
		UINT nBlock = 1;
		if (!CreateMemBlock(nGranularity, nBlock))
			return 0;
		return GetOsMemOfSize(nSize);
	}

	return pOsMem;
}

#ifdef _DEBUG
void CMemManager::Dump()
{
	typedef std::map<UINT, UINT> typeStatistics;
	typeStatistics tmpStat;
	REPORT(MemoryPoolModuleName, T("Dump MemoryPool: %u bytes\n", m_nTotalAllocated), RPT_INFO);

	m_lockAlloc.lock();
	for (typeMapAlloc::iterator iter = m_mpAlloc.begin(); iter != m_mpAlloc.end(); ++iter)
	{
		REPORT(MemoryPoolModuleName, T("  Addr:0x%08x - Size:% 8d - Trace:%s(%d)\n",
			iter->first, iter->second.pOsMem->OSMemBlkSize, iter->second.szFile, iter->second.nLineNo), RPT_INFO|RPT_NOPREFIX);
		typeStatistics::iterator is = tmpStat.find(iter->second.pOsMem->OSMemBlkSize);
		if (is == tmpStat.end())
			tmpStat[iter->second.pOsMem->OSMemBlkSize] = 1;
		else
			is->second += 1;
	}
	m_lockAlloc.unlock();

	REPORT(MemoryPoolModuleName, T("--- MemoryPool Statistics ---\n"), RPT_INFO|RPT_NOPREFIX);
	for (typeStatistics::iterator is = tmpStat.begin(); is != tmpStat.end(); ++is)
		REPORT(MemoryPoolModuleName, T("% 8d(bytes): % 8d(block)\n", is->first, is->second), RPT_INFO|RPT_NOPREFIX);

	REPORT(MemoryPoolModuleName, T("MemoryPool Dump End\n"), RPT_INFO);
}
#endif // _DEBUG
