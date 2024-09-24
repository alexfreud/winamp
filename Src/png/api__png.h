#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH
#include <api/service/api_service.h>

extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memoryManager;
#define WASABI_API_MEMMGR memoryManager

#endif 