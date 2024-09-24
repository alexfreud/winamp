#include <precomp.h>
#include "debuggerui.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS DebuggerUII
START_DISPATCH;
  CB(DEBUGGERUI_MESSAGELOOP, messageLoop);
  VCB(DEBUGGERUI_SETJITD, setJITD);
END_DISPATCH;
