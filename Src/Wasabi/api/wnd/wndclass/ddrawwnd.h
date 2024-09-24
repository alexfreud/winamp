#ifndef __DDRAWWND_H
#define __DDRAWWND_H

#include <ddraw.h>
#include "../bfc/basewnd.h"

class DDSurfaceCanvas;
class DDrawWnd;
class api_region;

#define DDRAWWND_PARENT BaseWnd

class NOVTABLE DDrawWnd : public DDRAWWND_PARENT {

public:

  DDrawWnd();
  virtual ~DDrawWnd();

  virtual int virtualBeforePaint(api_region *r);
  virtual int virtualAfterPaint(api_region *r);

  virtual void virtualCanvasCommit(Canvas *canvas, RECT *r, double ratio);

  virtual Canvas *createFrameBuffer(int w, int h);
  virtual void deleteFrameBuffer(Canvas *canvas);

  virtual int onInit();

  virtual void invalidate();
  virtual void invalidateRect(RECT *r);
  virtual void invalidateRgn(api_region *rgn);
  virtual void validate();
  virtual void validateRect(RECT *r);
  virtual void validateRgn(api_region *rgn);

private:

  void initDDraw();
  void startThread();
  void stopThread();

  LPDIRECTDRAW m_lpDD;
  LPDIRECTDRAWSURFACE m_lpRenderSurf, m_lpPrimSurf;

  DDSurfaceCanvas *fb_canvas;
  int w, h;
  LPDIRECTDRAWCLIPPER lpClipper;
  static int allow_dd;
  static int sleep_val;
  static CRITICAL_SECTION cs;
  static HANDLE thread;
  static int quitthread;
  static PtrList<DDrawWnd> ddlist;

  static unsigned int WINAPI renderThread(void *);
};

#endif

