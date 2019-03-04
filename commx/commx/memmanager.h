
/** @file		memmanager.h
 *	@brief		�ڴ�ع���ͷ�ļ�
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
 *	@brief	Ӧ���ڴ������	
 *
 *  ���ڴ������Ԥ�ȷ���һ����ڴ棬����ֳ����ɷ���������ÿһ�����а�����������ͬ
 *  ��С���ڴ�飬��ʹ�õ��������ӡ�����ʱ�����㹻��С��ĳһ���ڴ棬�����������ɾ
 *  ��������ʱ���ڴ�����¹ҽӵ����������ͷ
 @ingroup mery
 */
class CMemManager
{
	/** @class		OS_MEM
	 *	@brief		�ڴ��������ṹ
	 *
	 *  
	 */
	struct OS_MEM
	{
		void			*OSMemAddr;			/**< �ڴ�����׵�ַ */
		void			*OSMemFreeList;		/**< �����ڴ���׵�ַ */
		UINT			OSMemBlkSize;		/**< �ڴ���С */
		UINT			OSMemNBlks;			/**< �ڴ�������ڴ������ */
		UINT			OSMemNFree;			/**< �ڴ�����п����ڴ����� */
		LockSingle		m_csLock;			/**< ռ��������ʾ���ڴ���ƿ��ѱ�ռ�� */
	};

#ifdef _DEBUG
	/**@brief �ڴ��ͳ����Ϣ
	
	@see Dump()
	 */
	struct ALLOC_STAT
	{
		OS_MEM *pOsMem;				/** �ڴ������ַ [6/25/2009 xinl] */
		char szFile[MAX_PATH];		/** �����ļ��� [6/25/2009 xinl] */
		UINT nLineNo;				/** �����Դ�����к� [6/25/2009 xinl] */
	};
#else
	typedef OS_MEM* ALLOC_STAT;
#endif // _DEBUG

	typedef std::map<UINT, OS_MEM*> typeMapMem;	/**< �����ڴ���С���ڴ���ƿ�ָ��Ķ�Ӧ��ϵ */
	typedef std::vector<BYTE*> typeVecMem;		/**< ����Ԥ����Ĵ���ڴ��ַ */
	typedef std::map<void*, ALLOC_STAT> typeMapAlloc;	/**< ��������ȥ���ڴ��ַ����ƿ�֮��Ķ�Ӧ��ϵ���Ա���� */

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
	LockSingle m_lockMap;					/**< Map��Vector�� */
	typeMapMem m_mpOsMem;					/**< ������ڴ�������ƿ����� */
	typeVecMem m_vcOsMem;					/**< ����������ڴ�黺��ָ���б��������ͷ��ڴ� */

	LockSingle m_lockAlloc;
	typeMapAlloc m_mpAlloc;					/**< �ѷ�����ڴ�ָ�� */

	inline UINT GetMemSize(UINT nSize);
	OS_MEM* GetOsMemOfSize(UINT nSize);

#ifdef _DEBUG
	UINT m_nTotalAllocated;
#endif // _DEBUG
};

///�ڴ������ָ��
extern CMemManager *g_pMan;

#endif // !defined(AFX_MEMMANAGER_H__EC9DD05D_ECFC_4D0D_B8C3_6FEC3257B5FF__INCLUDED_)
