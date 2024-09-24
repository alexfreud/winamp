#include <precomp.h>

#include "svc_droptarget.h"

#define CBCLASS svc_dropTargetI
START_DISPATCH;
  CB(TESTTARGET, testTarget);
  CB(GETDRAGINTERFACEFORTYPE, getDragInterfaceForType);
  CB(RELEASEDRAGINTERFACE, releaseDragInterface);
END_DISPATCH;
#undef CBCLASS
