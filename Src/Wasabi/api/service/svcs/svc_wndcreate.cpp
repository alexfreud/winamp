#include <precomp.h>

#include "svc_wndcreate.h"

#define CBCLASS svc_windowCreateI
START_DISPATCH;
  CB(TESTGUID,            testGuid);
  CB(CREATEWINDOWBYGUID,  createWindowByGuid);
  CB(TESTTYPE,            testType);
  CB(CREATEWINDOWOFTYPE,  createWindowOfType);
  CB(DESTROYWINDOW,       destroyWindow);
  CB(REFCOUNT,		  refcount);
END_DISPATCH;
#undef CBCLASS


