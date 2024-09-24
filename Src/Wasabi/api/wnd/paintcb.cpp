#include "precomp.h"
#include "paintcb.h"
#include "api_window.h"

#define CBCLASS PaintCallbackInfoI
START_DISPATCH;
  CB(PAINTCBINFO_GETCANVAS, getCanvas);
  CB(PAINTCBINFO_GETREGION, getRegion);
END_DISPATCH;

PaintCallback::PaintCallback(ifc_window *w) {
  monitorWindow(w);
}

PaintCallback::~PaintCallback() {
  if (wnd != NULL) viewer_delViewItem(wnd);
}

void PaintCallback::monitorWindow(ifc_window *w) {
  if (wnd != NULL) {
    viewer_delViewItem(wnd);
    wnd = NULL;
  }
  if (w != NULL) {
    viewer_addViewItem(w);
    wnd = w;
  }
}

int PaintCallback::viewer_onItemDeleted(ifc_window *item) {
  ASSERT(item == wnd);//jic
  onWindowDeleted(wnd);
  wnd = NULL;
  return 1;
}

int PaintCallback::viewer_onEvent(ifc_window *item, int event, intptr_t param, void *ptr, size_t ptrlen) {
  PaintCallbackInfo *info = reinterpret_cast<PaintCallbackInfo *>(ptr);
  switch (event) {
    case ifc_window::Event_ONPAINT:
      if (param == BEFOREPAINT)
        onBeforePaint(info);
      else
        onAfterPaint(info);
    break;
    case ifc_window::Event_ONINVALIDATE:
      onInvalidation(info);
    break;
  }
  return 1;
}

