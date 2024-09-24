#ifndef _BLTCANVAS_H
#define _BLTCANVAS_H

#include "canvas.h"

class BltCanvas : public Canvas 
{
public:
  BltCanvas();
  BltCanvas(int width, int height, OSWINDOWHANDLE wnd);
  
  // override blit and stretchblit so we can use CGContextDrawLayerAtPoint/CGContextDrawLayerInRect
  virtual void blit(int srcx, int srcy, Canvas *dest, int dstx, int dsty, int dstw, int dsth);
	void blitToRect(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha = 255);

  virtual void stretchblit(int srcx, int srcy, int srcw, int srch, Canvas *dest, int dstx, int dsty, int dstw, int dsth);
  void stretchToRectAlpha(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha = 255);
    
	void DestructiveResize(int w, int h, int nb_bpp = 32); // resizes the bitmap, destroying the contents
  void fillBits(ARGB32 color); 

protected:
  CGLayerRef layer;
};

#endif
