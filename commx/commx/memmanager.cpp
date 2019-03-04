
/** @file		memmanager.cpp
 *	@brief		�ڴ�ع���ʵ���ļ�
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

/** @brief		��������ĳһ�ڴ����ȵ��ڴ����
 *
 *  
 *	@param		nGranularity �ڴ�����
 *	@param		nNumOfBlock ���ٿ��������ڴ�����
 *	@return
 *   - true �����ɹ�
 *   - false ����ʧ��
 */
bool CMemManager::CreateMemBlock(UINT &nGranularity, UINT &nNumOfBlock)
{
	if (nGranularity % 8)
		nGranularity = ((nGranularity / 8) + 1) * 8;

	//������ڴ�
	BYTE *pNewBuffer = new BYTE[nGranularity * nNumOfBlock];
	if (!pNewBuffer)
		return false;

	//���쵥������
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
	{//���ڴ������Ѿ�����
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
	{//���ڴ����Ȳ�����
		//�����ڴ���ƿ�
		OS_MEM *os_mem = new OS_MEM;
		if (!os_mem)
		{
			delete []pNewBuffer;
			return false;
		}

		//��ʼ���ڴ���ƿ���Ϣ
		os_mem->OSMemAddr = pNewBuffer;
		os_mem->OSMemFreeList = pNewBuffer;
		os_mem->OSMemBlkSize = nGranularity;
		os_mem->OSMemNBlks = nNumOfBlock;
		os_mem->OSMemNFree = nNumOfBlock;

		//�����ڴ���ƿ�map������������ָ�뱣�棬�Ա�����ʱ�ͷ��ڴ�
		if (m_mpOsMem.insert(std::make_pair(nGranularity, os_mem)).second)
			m_vcOsMem.push_back(pNewBuffer);
		else	//�ڴ���ƿ����ʧ�ܣ��ͷŵ��ڴ��Ϳ��ƿ��ָ��
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

/** @brief		�����ڴ�
 *
 *  ���Ҫ������ڴ�����������ڴ�鶼�Ѿ������ȥ��û�п����ڴ�飬
 *  ����������ͼ������ڴ���������ٷ���һ����ڴ棬�����ʽ������
 *  �ҵ���Ӧ���ڴ���ƿ���
 *	@param		nSize �����ڴ��С
 *	@return
 *   - 0 ����ʧ��
 *   - void* ������ڴ��ַ
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
		{//�޿��õĿ����ڴ�飬������ڴ����
			unsigned int nGranularity = pOsMem->OSMemBlkSize;
			unsigned int nSize = pOsMem->OSMemNBlks;

			//����Ҫע�⣺����һ���߳��Ѿ���ʼ�������ʱ���п��������߳�Ҳ��������
			//���õĿ����ڴ�飬Ҳ����ͼ���������Ҫ�������������ֻ���ɵ�һ���߳�
			//�������ڴ����ʱ�������������̵߳ȵ���һ���߳̽������������ʱ���ж�
			//�Ƿ��п��õĿ����ڴ�顣
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

/** @brief		�����ڴ�
 *
 *  ���պ�ö��ڴ��Ϊ�����ڴ棬�����¹ҵ��������ͷ
 *	@param		lp Ҫ���յ��ڴ��ַ,��#Malloc(UINT)�����ȥ
 *	@param		nSize ��#Malloc(UINT)����ʱ�Ĳ���
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

/** @brief		����ʵ�ʷ�����ڴ���С
 *
 *  ��������ڴ�ʱ����ô��ڵ��������С����ӽ����ڴ��Ĵ�С
 *	@param		nSize ���������ڴ��С
 *	@return		ʵ�ʷ�����ڴ���С
 */
UINT CMemManager::GetMemSize(UINT nSize)
{
	int i = 2;
	while (((UINT)1 << i) < nSize)
		i++;
	return ((UINT)1 << i);
}

/** @brief		�����ָ����С���Ӧ���ڴ���ƿ�ָ��
 *
 *  �������Ĵ�С�����ڴ��������ڴ��Ĵ�С��������������һ��
 *  �ڴ���������������С���µ��ڴ����
 *	@param		nSize ���������ڴ��С
 *	@return		�ڴ���ƿ�ָ��
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
		//û����ô����ڴ�飬��Ҫ�·���
		//����ֻ����һ�飬�����������õĻ����·���Ŀ��������ϵ���
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
