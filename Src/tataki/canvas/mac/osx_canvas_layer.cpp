#include <tataki/canvas/bltcanvas.h>

inline float QuartzBlue(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[0] / 255.f;
}

inline float QuartzGreen(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[1] / 255.f;
}

inline float QuartzRed(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[2] / 255.f;
}

inline float QuartzAlpha(RGB32 color)
{
  unsigned char *pixel = (unsigned char *)&color;
  return pixel[3] / 255.f;
}

BltCanvas::BltCanvas(int width, int height, OSWINDOWHANDLE wnd)
{
  CGrafPtr qdcontext = GetWindowPort(wnd);
  CGContextRef temp;
  QDBeginCGContext(qdcontext, &temp);
  CGSize size = CGSizeMake(width, height);
  layer = CGLayerCreateWithContext(temp, size, NULL);
  context = CGLayerGetContext(layer);
  QDEndCGContext(qdcontext, &temp);
}

void BltCanvas::blit(int srcx, int srcy, Canvas *dest, int dstx, int dsty, int dstw, int dsth)
{
  CGPoint point = CGPointMake(dstx-srcx, dsty-srcy);
  CGContextDrawLayerAtPoint(dest->getHDC(), point, layer);
}

void BltCanvas::blitToRect(api_canvas *canvas, RECT *src, RECT *dst, int alpha)
{
  CGContextRef dest = canvas->getHDC();
  CGContextSaveGState(dest);
  CGContextSetAlpha(dest, (float)alpha/255.f);
  // TODO: deal with width properly 
  CGRect rect = CGRectMake(dst->left - src->left, dst->top - src->top, dst->right - dst->left, dst->bottom - dst->top);
  CGContextDrawLayerInRect(dest, rect, layer);
  CGContextRestoreGState(dest);
}

void BltCanvas::stretchblit(int srcx, int srcy, int srcw, int srch, Canvas *dest, int dstx, int dsty, int dstw, int dsth)
{
  CGContextSaveGState(context);
  CGContextTranslateCTM(context, srcx, srcy);
  CGRect rect = CGRectMake(dstx, dsty, dstw, dsth);
  CGContextDrawLayerInRect(dest->getHDC(), rect, layer);
  CGContextRestoreGState(context);
}

void BltCanvas::stretchToRectAlpha(api_canvas *canvas, RECT *src, RECT *dst, int alpha)
{
  CGContextRef dest = canvas->getHDC();
  CGContextSaveGState(dest);
  CGContextSetAlpha(dest, (float)alpha/255.f);
// TODO: deal with width properly 
  CGRect rect = CGRectMake(dst->left - src->left, dst->top - src->top, dst->right - dst->left, dst->bottom - dst->top);
  CGContextDrawLayerInRect(dest, rect, layer);
  CGContextRestoreGState(dest);
}

void BltCanvas::DestructiveResize(int w, int h, int nb_bpp)
{
  CGSize size = CGSizeMake(w, h);
  CGLayerRef newlayer = CGLayerCreateWithContext(context, size, NULL);
  CGContextRelease(context);
  CGLayerRelease(layer);
  layer = newlayer;
  context = CGLayerGetContext(layer);
}

void BltCanvas::fillBits(ARGB32 color)
{
  CGContextSetRGBFillColor(context, 
                           QuartzRed(color), // red
                           QuartzGreen(color), // green
                           QuartzBlue(color), // blue
                           QuartzAlpha(color) // alpha
                           );
  
  CGContextFillRect(context, CGRectInfinite);
}