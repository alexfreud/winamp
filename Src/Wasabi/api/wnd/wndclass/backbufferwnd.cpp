#include <precomp.h>
#include "backbufferwnd.h"
#include <tataki/canvas/bltcanvas.h>
#include <api/api.h>
#include <tataki/region/region.h>

// -----------------------------------------------------------------------
BackBufferWnd::BackBufferWnd() {
  backbuffer = 0;
  canvas_w = -1;
  canvas_h = -1;
  back_buffer = NULL;
}               

// -----------------------------------------------------------------------
BackBufferWnd::~BackBufferWnd() {
  delete back_buffer;
}

//------------------------------------------------------------------------
BltCanvas *BackBufferWnd::getBackBuffer() {
  return back_buffer;
}

// -----------------------------------------------------------------------
int BackBufferWnd::onPaint(Canvas *canvas) {

  BBWND_PARENT::onPaint(canvas);

  if (!canvas) return 1;

  RECT r;
  getClientRect(&r);

  if (back_buffer && r.right-r.left > 0 && r.bottom -r.top > 0) {

    int w = r.right-r.left;
    int h = r.bottom-r.top;

    if (canvas_w != w || canvas_h != h) {
      delete back_buffer;
      back_buffer = new BltCanvas(w, h, getOsWindowHandle());
      canvas_w = w;
      canvas_h = h;
    }

#ifdef _WIN32
    RegionI reg;
    canvas->getClipRgn(&reg);
    back_buffer->selectClipRgn(&reg);
#else
#warning port me
#endif
    canvas->blit(r.left, r.top, back_buffer, 0, 0, w, h);
    back_buffer->selectClipRgn(NULL);
  }

  return 1;
}

int BackBufferWnd::onSiblingInvalidateRgn(api_region *r, ifc_window *who, int who_idx, int my_idx) {
  if (who_idx >= my_idx || !wantBackBuffer()) return 0;

  RECT rr;
  getClientRect(&rr);

  api_region *_r = getRegion();
  RegionI *__r=NULL;

  if (!_r) {
    __r = new RegionI();
    _r = __r;
    _r->setRect(&rr);
  } else {
    _r->offset(rr.left, rr.top);
  }

  int intersect = _r->doesIntersectRgn(r);
  if (intersect)
    r->addRegion(_r);

  delete __r;

  return intersect;
}

