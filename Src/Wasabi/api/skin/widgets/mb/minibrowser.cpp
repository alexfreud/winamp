#include <precomp.h>
#include "minibrowser.h"

#define CBCLASS MiniBrowserCallbackI
START_DISPATCH;
  CB(MINIBROWSER_ONBEFORENAVIGATE,     minibrowsercb_onBeforeNavigate);
  VCB(MINIBROWSER_ONDOCUMENTCOMPLETE,  minibrowsercb_onDocumentComplete);
  VCB(MINIBROWSER_ONDOCUMENTREADY,  minibrowsercb_onDocumentReady);
  VCB(MINIBROWSER_ONNAVIGATEERROR,  minibrowsercb_onNavigateError);
	VCB(MINIBROWSER_ONMEDIALINK,  minibrowsercb_onMediaLink);
	CB(MINIBROWSER_MESSAGETOMAKI,  minibrowsercb_messageToMaki);
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
	VCB(MINIBROWSER_SCRAPE, minibrowser_scrape);
	VCB(MINIBROWSER_SETCANCELIEERRORPAGE, minibrowser_setCancelIEErrorPage);
	VCB(MINIBROWSER_GETDOCUMENTTITLE, minibrowser_getDocumentTitle);
	CB(MINIBROWSER_MESSAGETOJS,		minibrowser_messageToJS);
END_DISPATCH;
#undef CBCLASS

