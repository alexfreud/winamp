#include <precomp.h>

#include "svc_imggen.h"

#define CBCLASS svc_imageGeneratorI
START_DISPATCH;
  CB(TESTDESC, testDesc);
  CB(GENIMAGE, genImage);
  CB(OUTPUTCACHEABLE, outputCacheable);
END_DISPATCH;
#undef CBCLASS
