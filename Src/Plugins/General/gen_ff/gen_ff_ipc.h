#ifndef _GEN_FF_IPC_H
#define _GEN_FF_IPC_H

#include "ff_ipc.h"

void ff_ipc_getSkinColor(ff_skincolor *sc);
void ff_ipc_genSkinBitmap(ff_skinbitmap *sb);
HBITMAP ff_genwa2skinbitmap();
HWND ff_ipc_getContentWnd(HWND w);

class ColorThemeMonitor : public SkinCallbackI {
  public:
    ColorThemeMonitor();
    virtual ~ColorThemeMonitor();
    int skincb_onColorThemeChanged(const wchar_t *colortheme);
};

extern ColorThemeMonitor *colorThemeMonitor;

#endif