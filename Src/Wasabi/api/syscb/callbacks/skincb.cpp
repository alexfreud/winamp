#include <precomp.h>
#include "skincb.h"
int SkinCallbackI::syscb_notify(int msg, intptr_t param1, intptr_t param2) {
  switch (msg) {
    case SkinCallback::UNLOADING: return skincb_onUnloading();
    case SkinCallback::RESET: return skincb_onReset();
    case SkinCallback::RELOAD: return skincb_onReload();
    case SkinCallback::BEFORELOADINGELEMENTS: return skincb_onBeforeLoadingElements();
    case SkinCallback::GUILOADED: return skincb_onGuiLoaded();
    case SkinCallback::LOADED: return skincb_onLoaded();
    case SkinCallback::COLORTHEMECHANGED: return skincb_onColorThemeChanged(WASABI_API_SKIN->colortheme_getColorSet());
    case SkinCallback::COLORTHEMESLISTCHANGED: return skincb_onColorThemesListChanged();
    case SkinCallback::CHECKPREVENTSWITCH: {
      int r = skincb_onCheckPreventSwitch((const wchar_t *)param1);
      if (r && param2)
        *(int *)param2 = r;
    }
  }
  return 0;
}
