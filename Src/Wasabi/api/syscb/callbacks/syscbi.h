#ifndef _SYSCBI_H
#define _SYSCBI_H

//<?<autoheader/>
#include "syscb.h"
#include "syscbx.h"

//?> 

#include <bfc/dispatch.h>
#include <bfc/platform/platform.h>
//#include <bfc/common.h>

//derive from this one (see skincb.h for a good example)
class SysCallbackI : public SysCallbackX {
public:
  DISPATCH(101) FOURCC getEventType() { return syscb_getEventType(); }
  DISPATCH(200) int notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0) { return syscb_notify(msg, param1, param2); }

protected:
  NODISPATCH virtual FOURCC syscb_getEventType()=0;
  NODISPATCH virtual int syscb_notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0)=0;

// This is where you should edit the enum block
/*[interface]
public:
// -- begin generated - edit in syscbi.h
enum {	// event types
  NONE = 0,
  RUNLEVEL	= MK4CC('r','u','n','l'),	// system runlevel
  CONSOLE	= MK3CC('c','o','n'),	// debug messages
  SKINCB		= MK4CC('s','k','i','n'),	// skin unloading/loading
  DB		= MK2CC('d','b'),		// database change messages
  WINDOW		= MK3CC('w','n','d'),	// window events
  GC		= MK2CC('g','c'),		// garbage collection event
  POPUPEXIT	= MK4CC('p','o','p','x'), // popup exit 
  CMDLINE	= MK4CC('c','m','d','l'),	// command line sent (possibly from outside)
  SYSMEM		= MK4CC('s','y','s','m'),	// api->sysMalloc/sysFree
  SERVICE	= MK3CC('s','v','c'),
};
// -- end generated
*/

};

#endif // _SYSCB_I
