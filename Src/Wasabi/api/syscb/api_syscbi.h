#ifndef __API_SYSCBI_IMPL_H
#define __API_SYSCBI_IMPL_H

/*<?<autoheader/>*/
#include "api_syscb.h"
#include "api_syscbx.h"

class SysCallback;
/*?>*/

class api_syscbI : public api_syscbX {
public:
  DISPATCH(20) int syscb_registerCallback(SysCallback *cb, void *param = NULL);
  DISPATCH(10) int syscb_deregisterCallback(SysCallback *cb);
  DISPATCH(30) int syscb_issueCallback(int eventtype, int msg, intptr_t p1=0, intptr_t p2=0);
};

/*[interface.footer.h]
// -- generated code - edit in api_syscbi.h

// {57B7A1B6-700E-44ff-9CB0-70B92BAF3959}
static const GUID syscbApiServiceGuid = 
{ 0x57b7a1b6, 0x700e, 0x44ff, { 0x9c, 0xb0, 0x70, 0xb9, 0x2b, 0xaf, 0x39, 0x59 } };

extern api_syscb *sysCallbackApi;
*/

#endif // __API_SYSCBI_IMPL_H
