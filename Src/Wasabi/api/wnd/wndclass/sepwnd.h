#ifndef __SEPWND_H
#define __SEPWND_H

#include <tataki/bitmap/bitmap.h>
#include <api/wnd/virtualwnd.h>

#define SEP_UNKNOWN   -1
#define SEP_VERTICAL   0
#define SEP_HORIZONTAL 1

#define SEPWND_PARENT VirtualWnd

class SepWnd : public VirtualWnd {
public:
  SepWnd();
  ~SepWnd();
  virtual int onPaint(Canvas *c);
  virtual int setOrientation(int which);
  virtual int onInit();
  virtual void freeResources();
private:
  SkinBitmap *bitmap;
  int orientation;
};


#endif
