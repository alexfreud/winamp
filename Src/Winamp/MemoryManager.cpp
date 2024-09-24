/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/
#include "main.h"
#include "MemoryManager.h"
#include "api.h"

#include <api/syscb/callbacks/syscb.h>
#include <api/syscb/callbacks/sysmemcb.h>

void *MemoryManager::Malloc(size_t size)
{
	if (size <= 0) return NULL;
	void *ret = calloc(size, 1);
	if (ret != NULL)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SYSMEM, SysMemCallback::ONMALLOC, (intptr_t)ret, size);
	return ret;
}

void MemoryManager::Free(void *ptr)
{
	if (ptr != NULL)
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SYSMEM, SysMemCallback::ONFREE, (intptr_t)ptr);
	free(ptr);
}

void *MemoryManager::Realloc(void *ptr, size_t newsize)
{
	void *ptrs[2] = {ptr, 0};
	void *new_memory = realloc(ptr, newsize);
	if (new_memory)
		ptrs[1] = new_memory;
	else
	{
		new_memory = calloc(newsize, 1);
		if (new_memory)
		{
			memcpy(new_memory, ptr, _msize(ptr));
			ptrs[1] = new_memory;
		}
	}

	if (!(ptrs[0] == NULL && ptrs[1] == NULL))
		WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SYSMEM, SysMemCallback::ONREALLOC, (intptr_t)ptrs, newsize);

	return new_memory;
}

void MemoryManager::MemChanged(void *ptr)
{
	if (ptr == NULL) return ;
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::SYSMEM, SysMemCallback::ONCHANGE, (intptr_t)ptr);
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MemoryManager
START_DISPATCH;
  CB(API_MEMMGR_SYSMALLOC, Malloc);
  VCB(API_MEMMGR_SYSFREE, Free);
  CB(API_MEMMGR_SYSREALLOC, Realloc);
  VCB(API_MEMMGR_SYSMEMCHANGED, MemChanged);
END_DISPATCH;
