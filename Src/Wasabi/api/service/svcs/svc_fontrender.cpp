#include "svc_fontrender.h"

#define CBCLASS svc_fontRenderI
START_DISPATCH
  CB(ISNAMED, isNamed);
  VCB(INSTALLTRUETYPEFONT, installTrueTypeFont);
  VCB(INSTALLBITMAPFONT, installBitmapFont);
  VCB(UNINSTALLALL, uninstallAll);
  VCB(UNINSTALLBYSCRIPTID, uninstallByScriptId);
  CB(REQUESTSKINFONT, requestSkinFont);
  VCB(DISPATCHTEXTOUT, dispatchTextOut);
  CB(DISPATCHGETINFO, dispatchGetInfo);
  CB(USETRUETYPEOVERRIDE, useTrueTypeOverride);
  CB(GETTRUETYPEOVERRIDE, getTrueTypeOverride);
END_DISPATCH
#undef CBCLASS
