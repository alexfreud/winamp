#include <precomp.h>
#include "oswndhost.h"

#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS OSWndHostI
START_DISPATCH;
  VCB(OSWNDHOST_OSWNDHOST_HOST, oswndhost_host);
  VCB(OSWNDHOST_OSWNDHOST_UNHOST, oswndhost_unhost);
  VCB(OSWNDHOST_OSWNDHOST_SETREGIONOFFSETS, oswndhost_setRegionOffsets);
END_DISPATCH;


