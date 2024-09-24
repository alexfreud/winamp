#include <precomp.h>
#include "svc_minibrowser.h"

#define CBCLASS svc_miniBrowserI
START_DISPATCH;
  CB(TESTGUID,            testGuid);
  CB(CREATEMINIBROWSER,   createMiniBrowser);
  VCB(DESTROYMINIBROWSER, destroyMiniBrowser);
END_DISPATCH;
#undef CBCLASS


