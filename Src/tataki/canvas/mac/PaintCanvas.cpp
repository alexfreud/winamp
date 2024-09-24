#include "PaintCanvas.h"

PaintCanvas::PaintCanvas()
{
  qdcontext=0;
}

bool PaintCanvas::beginPaint(BaseWnd *wnd)
{
  HIWindowRef macWnd = wnd->getOsWindowHandle();

  qdcontext = GetWindowPort(macWnd);
  QDBeginCGContext(qdcontext, &context);
  
  return true;
}

PaintCanvas::~PaintCanvas()
{
  if (qdcontext)
      QDEndCGContext(qdcontext, &context);
}

WndCanvas::WndCanvas()
{
  qdcontext=0;
}

WndCanvas::~WndCanvas()
{
  if (qdcontext)
    QDEndCGContext(qdcontext, &context);
}
  
int WndCanvas::attachToClient(BaseWnd *basewnd)
{
  HIWindowRef macWnd = basewnd->getOsWindowHandle();
  
  qdcontext = GetWindowPort(macWnd);
  QDBeginCGContext(qdcontext, &context);
  return 1;
}
  

TextInfoCanvas::TextInfoCanvas(BaseWnd */*unused*/)
{
}

TextInfoCanvas::~TextInfoCanvas()
{
}
