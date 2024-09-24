#include <precomp.h>
#include "svc_tooltips.h"

#define CBCLASS svc_toolTipsRendererI
START_DISPATCH;
  CB(SPAWNTOOLTIP,   spawnTooltip);
END_DISPATCH;
#undef CBCLASS


