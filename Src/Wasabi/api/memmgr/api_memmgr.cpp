#include <precomp.h>
#include "api_memmgr.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS api_memmgrI
START_DISPATCH;
  CB(API_MEMMGR_SYSMALLOC, sysMalloc);
  VCB(API_MEMMGR_SYSFREE, sysFree);
  CB(API_MEMMGR_SYSREALLOC, sysRealloc);
  VCB(API_MEMMGR_SYSMEMCHANGED, sysMemChanged);
END_DISPATCH;
