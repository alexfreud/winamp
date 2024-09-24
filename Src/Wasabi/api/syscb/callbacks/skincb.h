#ifndef _SKINCB_H
#define _SKINCB_H

#include <api/syscb/callbacks/syscbi.h>

namespace SkinCallback {
  enum {
    UNLOADING=100,	// beginning, haven't killed anything yet
    RESET=200,		// skin is gone
    RELOAD=300,		// stuff is loading
    BEFORELOADINGELEMENTS=350,
    GUILOADED=400,	// skin gui objects loaded from xml
    LOADED=500,		// all done, new skin in place
    CHECKPREVENTSWITCH=600,		// we're about to switch skin, wanna abort ? return 1 if so
    COLORTHEMECHANGED=700,		// color theme has been changed, trap this if you're not using automaticly themed widgets
    COLORTHEMESLISTCHANGED=710,		// color theme list has been modified, trap this if you're showing a list of the colorthemes and you want to mirror changes
  };
};

#define SKINCALLBACKI_PARENT SysCallbackI
class SkinCallbackI : public SKINCALLBACKI_PARENT {
public:
  virtual FOURCC syscb_getEventType() { return SysCallback::SKINCB; }

protected:
  // override these
  virtual int skincb_onUnloading() { return 0; }
  virtual int skincb_onReset() { return 0; }
  virtual int skincb_onReload() { return 0; }
  virtual int skincb_onBeforeLoadingElements() { return 0; }
  virtual int skincb_onGuiLoaded() { return 0; }
  virtual int skincb_onLoaded() { return 0; }
  virtual int skincb_onCheckPreventSwitch(const wchar_t *skinname) { return 0; }
  virtual int skincb_onColorThemeChanged(const wchar_t *newcolortheme) { return 0; }
  virtual int skincb_onColorThemesListChanged() { return 0; }

private:
  virtual int syscb_notify(int msg, intptr_t param1=0, intptr_t param2=0);
};

#endif
