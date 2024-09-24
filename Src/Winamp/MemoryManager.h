#ifndef __MEMMGRAPI_H
#define __MEMMGRAPI_H

#include <api/memmgr/api_memmgr.h>

class MemoryManager : public api_memmgr
{
public:
	static const char *getServiceName() { return "Memory Manager"; }
	static const GUID getServiceGuid() { return memMgrApiServiceGuid; }
public:
	void *Malloc(size_t size);
	void Free(void *ptr);
	void *Realloc(void *ptr, size_t newsize);
	void MemChanged(void *ptr);

protected:
	RECVS_DISPATCH;
};

extern MemoryManager *memoryManager;

#endif
