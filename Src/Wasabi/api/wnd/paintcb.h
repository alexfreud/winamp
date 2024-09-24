#ifndef _PAINTCB_H
#define _PAINTCB_H

#include <bfc/depview.h>
#include <bfc/dispatch.h>
#include <api/wnd/api_window.h>

class Canvas;
class api_region;

class PaintCallbackInfo : public Dispatchable {
  public:
    Canvas *getCanvas();
    api_region *getRegion();

  enum {
    PAINTCBINFO_GETCANVAS = 10,
    PAINTCBINFO_GETREGION = 20,
  };

};

inline Canvas *PaintCallbackInfo::getCanvas() {
  return _call(PAINTCBINFO_GETCANVAS, (Canvas *)NULL);
}

inline api_region *PaintCallbackInfo::getRegion() {
  return _call(PAINTCBINFO_GETREGION, (api_region *)NULL);
}

class PaintCallbackInfoI : public PaintCallbackInfo {
  public:
    PaintCallbackInfoI(Canvas *_canvas, api_region *_region) : canvas(_canvas), region(_region)  {}
    virtual ~PaintCallbackInfoI() {}

    virtual Canvas *getCanvas() { return canvas; }
    virtual api_region *getRegion() { return region; }

  private:

    Canvas *canvas;
    api_region *region;

  protected:
    RECVS_DISPATCH;
};

class PaintCallback : DependentViewerTPtr<ifc_window> {
public:
  PaintCallback() { wnd = NULL; };
  PaintCallback(ifc_window *w);
  virtual ~PaintCallback();

  virtual void monitorWindow(ifc_window *w);
  virtual int viewer_onEvent(ifc_window *item, int event, intptr_t param, void *ptr, size_t ptrlen);
  virtual int viewer_onItemDeleted(ifc_window *item);

  // override those
  virtual void onBeforePaint(PaintCallbackInfo *info) { }
  virtual void onAfterPaint(PaintCallbackInfo *info) { }
  virtual void onWindowDeleted(ifc_window *w)=0; // warning, pointer invalid
  virtual void onInvalidation(PaintCallbackInfo *info) { }

  enum {
    BEFOREPAINT = 10,
    AFTERPAINT = 20,
  };

private:
  ifc_window *wnd;
};

#endif
