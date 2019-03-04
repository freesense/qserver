/**@file  mery.h
 * @brief �����ڴ����
 @defgroup mery �����ڴ����
 */
#ifndef __COMMX_WINNT_BASE_MERY_H__
#define __COMMX_WINNT_BASE_MERY_H__

#include "commxapi.h"

//@{ ingroup mery

/**@brief ��ʼ�������ڴ��
@return
	-true �ɹ�
	-false ʧ��
 */
COMMXAPI bool InitializeMemoryPool();

/**@brief �ͷŹ����ڴ��


 */
COMMXAPI void ReleaseMemoryPool();

/**@brief �����ڴ��

ϵͳ���Ӷ��ڴ��з���nGranularity*nNumOfBlock�ֽ��ڴ棬��������뵽�ڴ��������
@param nGranularity �ڴ����ȣ���λΪbyte
@param nNumOfBlock ��������
@return
 -true �ɹ�
 -false ʧ��
 */
COMMXAPI bool CreateMemBlock(UINT nGranularity, UINT nNumOfBlock);

/**@brief �����ڴ�

@param nSize Ҫ����Ĵ�С����λΪbyte
@param lpFile ������Դ�ļ���
@param nLineNo ������Դ�����к�
@return ����õ�ָ�룬NULL��ʾ����ʧ��
 */
COMMXAPI void* MallocInPool(UINT nSize
#ifdef _DEBUG
							,
							const char *lpFile,
							UINT nLineNo
#endif
							);

/**@brief �ͷ��ѷ�����ڴ�

@param lp Ҫ�ͷŵ��ڴ�ָ��
@warning �����������ͷŲ�����MallocInPool�����ȥ���ڴ�(�����κδ���)�����������ڴ滹��Ҫ��ȷ���ͷŲŲ�������ڴ�й©
 */
COMMXAPI void FreeToPool(void *lp);

/**@brief ��ӡ�ڴ�ط������

debug�����ʹ�ã�release���У�����һ���պ���
 */
COMMXAPI void DumpMemPool();

#define MemoryPoolModuleName "MemoryPool"		/** ��REPORTʹ�õ�ģ������ [6/25/2009 xinl] */

#ifdef _DEBUG
#define mpnew(X) MallocInPool(X, __FILE__, __LINE__)
#else
#define mpnew(X) MallocInPool(X)
#endif // _DEBUG
#define mpdel(X) FreeToPool(X)

// @}

#endif
