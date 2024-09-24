#include <precomp.h>
#include "bufferpaintwnd.h"
#include <tataki/canvas/bltcanvas.h>

// -----------------------------------------------------------------------
BufferPaintWnd::BufferPaintWnd() {
  canvas_w = -1;
  canvas_h = -1;
  render_canvas = NULL;
  invalidated = 1;
}

// -----------------------------------------------------------------------
BufferPaintWnd::~BufferPaintWnd() {
  delete render_canvas;
}

// -----------------------------------------------------------------------
int BufferPaintWnd::onInit() {
  BUFFERPAINTWND_PARENT::onInit();
  return 1;
}

// -----------------------------------------------------------------------
void BufferPaintWnd::bufferPaint() {
  updateCanvas();
  if (render_canvas != NULL)
    onBufferPaint(render_canvas, canvas_w, canvas_h);
}

void BufferPaintWnd::invalidateBuffer() {
  invalidated = 1;
  invalidate();
}

// -----------------------------------------------------------------------
void BufferPaintWnd::getBufferPaintSize(int *w, int *h) {
  RECT r;
  getClientRect(&r);
  if (w) *w = r.right-r.left;
  if (h) *h = r.bottom-r.top;
}

// -----------------------------------------------------------------------
void BufferPaintWnd::getBufferPaintSource(RECT *r) {
  ASSERT(r != NULL);
  r->left = 0;
  r->right = canvas_w;
  r->top = 0;
  r->bottom = canvas_h;
}

// -----------------------------------------------------------------------
void BufferPaintWnd::getBufferPaintDest(RECT *r) {
  ASSERT(r != NULL);
  getClientRect(r);
}

// -----------------------------------------------------------------------
int BufferPaintWnd::onPaint(Canvas *canvas) {

  BUFFERPAINTWND_PARENT::onPaint(canvas);

  if (invalidated) bufferPaint();
  invalidated = 0;

  RECT r;
  getBufferPaintDest(&r);
  RECT sr;
  getBufferPaintSource(&sr);

  render_canvas->/*getSkinBitmap()->*/stretchToRectAlpha(canvas, &sr, &r, getPaintingAlpha());

  return 1;
}

// -----------------------------------------------------------------------
int BufferPaintWnd::onResize() {
  if (!BUFFERPAINTWND_PARENT::onResize()) return 0;
  if (updateCanvas()) {
    invalidated = 1;
    invalidate();
  }
  return 1;
}

// -----------------------------------------------------------------------
int BufferPaintWnd::updateCanvas() {
  int w, h;
  getBufferPaintSize(&w, &h);

  if (wantEvenAlignment()) {
    if (w & 1) w++;
    if (h & 1) h++;
  }

  if (w == 0 || h == 0) return 0; 

  int newone = 0;

  if (canvas_w != w || canvas_h != h) {
		if (render_canvas)
			render_canvas->DestructiveResize(w, wantNegativeHeight() ? -h : h);
		else
	    render_canvas = new BltCanvas(w, wantNegativeHeight() ? -h : h, getOsWindowHandle());
    canvas_w = w;
    canvas_h = h;
    newone = 1;
    onNewBuffer(canvas_w, canvas_h);
  }

  return newone;
}
