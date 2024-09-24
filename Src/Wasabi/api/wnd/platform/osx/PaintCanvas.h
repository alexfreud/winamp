#ifndef NULLSOFT_WASABI_OSX_PAINTCANVAS_H
#define NULLSOFT_WASABI_OSX_PAINTCANVAS_H

#include <tataki/canvas/canvas.h>
#include <api/wnd/basewnd.h>

class PaintCanvas : public Canvas
{
public:
  PaintCanvas();
  ~PaintCanvas();
  bool beginPaint(BaseWnd *wnd);
protected:
  CGrafPtr qdcontext;
};

class PaintBltCanvas : public PaintCanvas
{
public:
  bool beginPaintNC(BaseWnd *wnd)
  {
    return beginPaint(wnd);
  }
};
#warning port PaintBltCanvas
class WndCanvas : public Canvas 
{
public:
  WndCanvas();
  virtual ~WndCanvas();
  
  // address client area
  int attachToClient(BaseWnd *basewnd);

private:
  CGrafPtr qdcontext;
};

class TextInfoCanvas : public Canvas 
{
public:
  TextInfoCanvas(BaseWnd *baseWnd);
  virtual ~TextInfoCanvas();
};
#endif