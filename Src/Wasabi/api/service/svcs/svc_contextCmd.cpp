#include <precomp.h>

#include "svc_contextcmd.h"

#define CBCLASS svc_contextCmdI
START_DISPATCH;
  CB(TESTITEM, testItem);
  CB(GETSUBMENU, getSubMenu);
  CB(GETSUBMENUTEXT, getSubMenuText);
  CB(GETCOMMAND, getCommand);
  CB(GETENABLED, getEnabled);
  CB(GETCHECKED, getChecked);
  CB(GETSORTVAL, getSortVal);
  VCB(ONCOMMAND, onCommand);
END_DISPATCH;
#undef CBCLASS
