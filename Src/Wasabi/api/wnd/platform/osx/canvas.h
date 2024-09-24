#ifndef NULLSOFT_WASABI_CANVAS_H
#define NULLSOFT_WASABI_CANVAS_H

#include <Carbon/Carbon.h>
#include <tataki/canvas/ifc_canvas.h>
#include <bfc/platform/platform.h>
#include <api/service/svcs/svc_font.h> // for STDFONT_* stuff.  should make a std_font thingy later
#include <bfc/std.h> // for WASABI_DEFAULT_FONTNAMEW
class BaseWnd;
class api_region;

class Canvas : public ifc_canvas
{
public:
  Canvas() :context(0), wnd(0) {}
  Canvas(CGContextRef _context) : context(_context), wnd(0) {}
  Canvas(CGrafPtr _context);
  HDC getHDC();
  void fillRect(const RECT *r, RGB32 color);
  void fillRgn(api_region *r, RGB32 color);
  void setBaseWnd(BaseWnd *_wnd) { wnd=_wnd; }
  void selectClipRgn(api_region *r);

  virtual void blit(int srcx, int srcy, Canvas *dest, int dstx, int dsty, int dstw, int dsth);
  virtual void stretchblit(int srcx, int srcy, int srcw, int srch, Canvas *dest, int dstx, int dsty, int dstw, int dsth);

  void textOut(int x, int y, const wchar_t *txt, const Wasabi::FontInfo *fontInfo);
  
  static float getSystemFontScale() { return 1.0f; }
  
  int getTextWidth(const wchar_t *text, const Wasabi::FontInfo *fontInfo);
  int getTextHeight(const wchar_t *text, const Wasabi::FontInfo *fontInfo);
  int getTextHeight(const Wasabi::FontInfo *fontInfo)
  {
    return getTextHeight(L"M", fontInfo);
  }
  void getTextExtent(const wchar_t *text, int *w, int *h, const Wasabi::FontInfo *fontInfo);
  void textOutCentered(RECT *r, const wchar_t *txt, const Wasabi::FontInfo *fontInfo);
  void textOut(int x, int y, int w, int h, const wchar_t *txt, const Wasabi::FontInfo *fontInfo);
  void textOutEllipsed(int x, int y, int w, int h, const wchar_t *txt, const Wasabi::FontInfo *fontInfo);

  void drawSysObject(const RECT *r, int sysobj, int alpha=255);
protected:
  RECVS_DISPATCH;

  CGContextRef context; 
  BaseWnd *wnd; // TODO: not 100% sure we'll need this.  win32 version has it so we'll keep it for now
};

class BaseCloneCanvas : public Canvas 
{
public:
  BaseCloneCanvas(ifc_canvas *cloner=NULL);
  virtual ~BaseCloneCanvas();
  
  int clone(ifc_canvas *cloner);
};

namespace DrawSysObj {
  enum {
    BUTTON, BUTTON_PUSHED, BUTTON_DISABLED,
    OSBUTTON, OSBUTTON_PUSHED, OSBUTTON_DISABLED,
    OSBUTTON_CLOSE, OSBUTTON_CLOSE_PUSHED, OSBUTTON_CLOSE_DISABLED,
    OSBUTTON_MINIMIZE, OSBUTTON_MINIMIZE_PUSHED, OSBUTTON_MINIMIZE_DISABLED,
    OSBUTTON_MAXIMIZE, OSBUTTON_MAXIMIZE_PUSHED, OSBUTTON_MAXIMIZE_DISABLED,
  };
};

#endif
