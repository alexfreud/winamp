#include <precomp.h>
#ifndef NOCBMGR
//<?#include "<class data="implementationheader"/>"
#include "api_syscbi.h"
//?>

#include <api/api.h>
//#include <api/syscb/cbmgr.h>

api_syscb *sysCallbackApi = NULL;

int api_syscbI::syscb_registerCallback(SysCallback *cb, void *param) {
#ifdef WASABI_COMPILE_COMPONENTS
  WASABI_API_SYSCB->syscb_registerCallback(cb, param, WASABI_API_COMPONENT->getThisComponent());
#else
  WASABI_API_SYSCB->syscb_registerCallback(cb, param);
#endif
  return 1;
}

int api_syscbI::syscb_deregisterCallback(SysCallback *cb) {
#ifdef WASABI_COMPILE_COMPONENTS
  WASABI_API_SYSCB->syscb_deregisterCallback(cb, WASABI_API_COMPONENT->getThisComponent());
#else
  WASABI_API_SYSCB->syscb_deregisterCallback(cb);
#endif
  return 1;
}

int api_syscbI::syscb_issueCallback(int eventtype, int msg, int p1, int p2) {
  WASABI_API_SYSCB->syscb_issueCallback(eventtype, msg, p1, p2);
  return 1;
}

#endif