#ifndef _PAINTSET_H
#define _PAINTSET_H

#include <wasabicfg.h>

#include <api/wnd/paintsets.h>
#include <tataki/canvas/ifc_canvas.h>

int paintset_present(int set);
#ifdef WASABI_COMPILE_IMGLDR
int paintset_renderPaintSet(int type, ifc_canvas *c, const RECT *r, int alpha=255, int checkonly=FALSE);
#ifdef WASABI_COMPILE_FONTS
void paintset_renderTitle(const wchar_t *title, ifc_canvas *canvas, const RECT *r, int alpha=255, int dostreaks=TRUE, int doborder=TRUE);
#endif
#endif
void paintset_reset();

#endif
