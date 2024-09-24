#include <precomp.h>

#include "svc_xmlprov.h"

#define CBCLASS svc_xmlProviderI
START_DISPATCH;
  CB(TESTDESC, testDesc);
  CB(GETXMLDATA, getXmlData);
END_DISPATCH;
#undef CBCLASS
