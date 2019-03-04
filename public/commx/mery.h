/**@file  mery.h
 * @brief 共享内存管理
 @defgroup mery 共享内存管理
 */
#ifndef __COMMX_WINNT_BASE_MERY_H__
#define __COMMX_WINNT_BASE_MERY_H__

#include "commxapi.h"

//@{ ingroup mery

/**@brief 初始化共享内存池
@return
	-true 成功
	-false 失败
 */
COMMXAPI bool InitializeMemoryPool();

/**@brief 释放共享内存池


 */
COMMXAPI void ReleaseMemoryPool();

/**@brief 创建内存块

系统将从对内存中分配nGranularity*nNumOfBlock字节内存，并将其加入到内存管理链表
@param nGranularity 内存粒度，单位为byte
@param nNumOfBlock 分配数量
@return
 -true 成功
 -false 失败
 */
COMMXAPI bool CreateMemBlock(UINT nGranularity, UINT nNumOfBlock);

/**@brief 分配内存

@param nSize 要分配的大小，单位为byte
@param lpFile 分配点的源文件名
@param nLineNo 分配点的源代码行号
@return 分配好的指针，NULL表示分配失败
 */
COMMXAPI void* MallocInPool(UINT nSize
#ifdef _DEBUG
							,
							const char *lpFile,
							UINT nLineNo
#endif
							);

/**@brief 释放已分配的内存

@param lp 要释放的内存指针
@warning 本函数可以释放不是由MallocInPool分配出去的内存(不作任何处理)，但是这种内存还是要正确地释放才不会造成内存泄漏
 */
COMMXAPI void FreeToPool(void *lp);

/**@brief 打印内存池分配情况

debug版才能使用，release版中，这是一个空函数
 */
COMMXAPI void DumpMemPool();

#define MemoryPoolModuleName "MemoryPool"		/** 供REPORT使用的模块名称 [6/25/2009 xinl] */

#ifdef _DEBUG
#define mpnew(X) MallocInPool(X, __FILE__, __LINE__)
#else
#define mpnew(X) MallocInPool(X)
#endif // _DEBUG
#define mpdel(X) FreeToPool(X)

// @}

#endif
