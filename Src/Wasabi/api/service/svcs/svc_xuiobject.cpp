#include <precomp.h>

#include "svc_xuiobject.h"

#define CBCLASS svc_xuiObjectI
START_DISPATCH;
  CB(XUI_TESTTAG,        testTag);
//  CB(XUI_INSTANTIATE,    instantiate);
  CB(XUI_INSTANTIATEWITHPARAMS,    instantiate);
  VCB(XUI_DESTROY,       destroy);
END_DISPATCH;
#undef CBCLASS
