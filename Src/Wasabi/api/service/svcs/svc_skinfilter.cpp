#include <precomp.h>

#include "svc_skinfilter.h"

#define CBCLASS svc_skinFilterI
START_DISPATCH;
  CB(FILTERBITMAP, filterBitmap);
  CB(FILTERCOLOR, filterColor);
END_DISPATCH;
#undef CBCLASS
