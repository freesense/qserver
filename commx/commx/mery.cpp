
#include "stdafx.h"
#include <assert.h>
#include "../public/mery.h"
#include "../public/report.h"
#include "../public/synch.h"
#include "memmanager.h"

CMemManager *g_pMan = NULL;		/** 内存池对象 */
int refMan = 0;					/** 内存池初始化的引用计数 [7/23/2009 xinl] */
LockSingle lockMan;

bool InitializeMemoryPool()
{
	WGUARD(LockSingle, lockMan, grd);
	refMan += 1;
	if (g_pMan)
		return true;

	g_pMan = new CMemManager;
	return g_pMan ? true : false;
}

void ReleaseMemoryPool()
{
	WGUARD(LockSingle, lockMan, grd);
	refMan -= 1;
	if (refMan == 0)
	{
		delete g_pMan;
		g_pMan = NULL;
	}
	else if (refMan < 0)
		refMan = 0;
}

bool CreateMemBlock(UINT nGranularity, UINT nNumOfBlock)
{
	assert(g_pMan);
	return g_pMan->CreateMemBlock(nGranularity, nNumOfBlock);
}

void* MallocInPool(UINT nSize
#ifdef _DEBUG
				   ,
				   const char *lpFile,
				   UINT nLineNo
#endif
				   )
{
	assert(g_pMan);

#ifdef _DEBUG
	void *pv = g_pMan->Malloc(nSize, lpFile, nLineNo);
	return pv;
#else
	return g_pMan->Malloc(nSize);
#endif
}

void FreeToPool(void *lp)
{
	assert(g_pMan);
	g_pMan->Free(lp);
}

void DumpMemPool()
{
#ifdef _DEBUG
	g_pMan->Dump();
#endif // _DEBUG
}
