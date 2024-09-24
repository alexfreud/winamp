#ifndef NULLSOFT_WASABI_OSX_BITMAP_CGIMAGE_H
#define NULLSOFT_WASABI_OSX_BITMAP_CGIMAGE_H

#include <bfc/platform/platform.h>
#include <tataki/canvas/ifc_canvas.h>
#include <api/wnd/ifc_bitmap.h>

/*
 TODO:
 need some kind of updateBits() so that the underlying image can be updated to reflect changes 
 */
class SkinBitmap : public ifc_bitmap
{
public:
  SkinBitmap(ARGB32 *bits, int w, int h); // added by benski, use if you have raw image bits
  SkinBitmap(const wchar_t *elementname, int cached = 1);
  ~SkinBitmap();
  int getWidth();
  int getHeight();
  int getFullWidth(); // aka pitch
  
  // blits
  void blit(ifc_canvas *canvas, int x, int y);
  void blitAlpha(ifc_canvas *canvas, int x, int y, int alpha = 255);
  // stretch blits
  void stretchToRect(ifc_canvas *canvas, RECT *r);
  void stretchToRectAlpha(ifc_canvas *canvas, RECT *r, int alpha = 255);
  void stretchToRectAlpha(ifc_canvas *canvas, RECT *src, RECT *dst, int alpha = 255);
// tiled blits  
  void blitTile(ifc_canvas *canvas, RECT *dest, int xoffs = 0, int yoffs = 0, int alpha = 255);
  
  ARGB32 getPixel(int x, int y);
public: // ifc_bitmap implementations
  OSBITMAPHANDLE GetBitmap() { return image; }
  uint8_t *getBits();
  void UpdateBits(uint8_t *bits);
  
private:
  CGImageRef image;
  CGContextRef imageContext;
  void *bits;
  
protected:
    RECVS_DISPATCH;
};

#endif