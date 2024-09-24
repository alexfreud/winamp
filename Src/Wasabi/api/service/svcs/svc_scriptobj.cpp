#include <precomp.h>

#include "svc_scriptobji.h"

#define CBCLASS svc_scriptObjectI
START_DISPATCH;
  CB(GETCONTROLLER, getController);
  VCB(ONREGISTER,   onRegisterClasses);
END_DISPATCH;
#undef CBCLASS
