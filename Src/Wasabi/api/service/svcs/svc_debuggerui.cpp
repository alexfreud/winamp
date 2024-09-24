#include <precomp.h>

#include "svc_debuggerui.h"

#define CBCLASS svc_debuggerUII
START_DISPATCH;
  CB(CREATEUI, createUI);
  VCB(DESTROYUI, destroyUI);
END_DISPATCH;
#undef CBCLASS
