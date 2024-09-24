#include "precomp.h"
#include "minibrowser.h"

#define CBCLASS MiniBrowserCallbackI
START_DISPATCH;
  CB(MINIBROWSER_ONBEFORENAVIGATE,     minibrowsercb_onBeforeNavigate);
  VCB(MINIBROWSER_ONDOCUMENTCOMPLETE,  minibrowsercb_onDocumentComplete);
END_DISPATCH;
#undef CBCLASS

#define CBCLASS MiniBrowserI
START_DISPATCH;
  CB(MINIBROWSER_GETROOTWND,           minibrowser_getRootWnd);
  CB(MINIBROWSER_NAVIGATEURL,          minibrowser_navigateUrl);
  CB(MINIBROWSER_BACK,                 minibrowser_back);
  CB(MINIBROWSER_FORWARD,              minibrowser_forward);
  CB(MINIBROWSER_HOME,                 minibrowser_home);
  CB(MINIBROWSER_REFRESH,              minibrowser_refresh);
  CB(MINIBROWSER_STOP,                 minibrowser_stop);
  VCB(MINIBROWSER_SETTARGETNAME,       minibrowser_setTargetName);
  CB(MINIBROWSER_GETTARGETNAME,        minibrowser_getTargetName);
  CB(MINIBROWSER_GETCURRENTURL,        minibrowser_getCurrentUrl);
  VCB(MINIBROWSER_ADDCB,               minibrowser_addCB);
  VCB(MINIBROWSER_SETHOME,             minibrowser_setHome);
  VCB(MINIBROWSER_SETSCROLLFLAG,       minibrowser_setScrollbarsFlag);
END_DISPATCH;
#undef CBCLASS

