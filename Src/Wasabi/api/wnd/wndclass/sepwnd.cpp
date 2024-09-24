#include <precomp.h>

#include <tataki/bitmap/bitmap.h>
#include <api/wnd/basewnd.h>
#include "sepwnd.h"
#include <tataki/canvas/canvas.h>
#include <api/wnd/PaintCanvas.h>

SepWnd::SepWnd() {
  bitmap = NULL;
  orientation = SEP_UNKNOWN;
}

SepWnd::~SepWnd() {
  if (bitmap) delete bitmap;
}

int SepWnd::onPaint(Canvas *canvas) {

  PaintBltCanvas paintcanvas;
  if (canvas == NULL) {
    if (!paintcanvas.beginPaintNC(this)) return 0;
    canvas = &paintcanvas;
  }


  if (!bitmap) {
    switch (orientation) {
      case SEP_VERTICAL:
        bitmap = new SkinBitmap(L"studio.FrameVerticalDivider");
        break;
      case SEP_HORIZONTAL:
        bitmap = new SkinBitmap(L"studio.FrameHorizontalDivider");
        break;
      case SEP_UNKNOWN:
        return 1;
    }
    ASSERT(bitmap != NULL);
  }
  RECT r;
  getClientRect(&r);
  RenderBaseTexture(canvas, r);
  if (bitmap) {
    bitmap->stretchToRectAlpha(canvas, &r, 128);
  }
  return 1;
}

int SepWnd::setOrientation(int which) {
  orientation = which;
  return 1;  
}

int SepWnd::onInit() {
  SEPWND_PARENT::onInit();
  return 1;
}

void SepWnd::freeResources() {
  SEPWND_PARENT::freeResources();
  if (bitmap) delete bitmap;
  bitmap = NULL;
}