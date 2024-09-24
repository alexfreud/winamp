#include <precomp.h>

#include "wkc.h"

#define CBCLASS WasabiKernelControllerI
START_DISPATCH;
  CB(TESTCOMPONENT, testComponent);
  CB(TESTSCRIPT, testScript);
  CB(TESTSKIN, testSkin);
  CB(TESTSKINFILE, testSkinFile);
END_DISPATCH;
#undef CBCLASS
