#include <precomp.h>
#include "guistatuscb.h"

#define CBCLASS GuiStatusCallbackI
START_DISPATCH;
  CB(STATUS_GETDEP,       status_getDependencyPtr);
  VCB(STATUS_ONSETTEXT,   onSetStatusText);
  VCB(STATUS_ADDCTXTCMDS, onAddAppCmds);
  VCB(STATUS_REMCTXTCMDS, onRemoveAppCmds);
  VCB(STATUS_PUSHCOMPLETED, pushCompleted);
  VCB(STATUS_INCCOMPLETED, incCompleted);
  VCB(STATUS_SETCOMPLETED, setCompleted);
  VCB(STATUS_POPCOMPLETED, popCompleted);
END_DISPATCH;
#undef CBCLASS
