#ifndef __TRUETYPEFONT_LINUX_H
#define __TRUETYPEFONT_LINUX_H

#include "../../../studio/services/svc_font.h"

#include "../../string.h"
#include "../../stack.h"
#include "../truetypefontdef.h"

class ifc_canvas;
class BltCanvas;

class TrueTypeFont_Linux : public svc_fontI {
 public:
  TrueTypeFont_Linux();
  virtual ~TrueTypeFont_Linux();

  virtual void textOut(ifc_canvas *c, int x, int y, const char *txt, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOut2(ifc_canvas *c, int x, int y, int w, int h, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutCentered(ifc_canvas *c, RECT *r, const char *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual int getTextWidth(ifc_canvas *c, const char *text, int size, int bold, int underline, int italic, int antialias);
  virtual int getTextHeight(ifc_canvas *c, const char *text, int size, int bold, int underline, int italic, int antialias);
  virtual int getTextHeight2(ifc_canvas *c, int size, int bold, int underline, int italic, int antialias);
  virtual void getTextExtent(ifc_canvas *c, const char *text, int *w, int *h, int size, int bold, int underline, int italic, int antialias);

  virtual void setFontId(const char *id) { font_id = id; }
  virtual const char *getFontId() { return font_id; }
  virtual int getScriptId() { return scriptid; }
  virtual void setScriptId(int id) { scriptid = id; }

  virtual int isBitmap();
  virtual void setFontFace(const char *face);
  virtual int addFontResource(FILE *f);
  virtual const char *getFaceName();

  virtual const wchar_t *getFontSvcName() { return "Linux"; }
  static const char *getServiceName() { return "Linux font renderer"; }

 protected:
  static char *filenameToFontFace(const wchar_t *filename);
  void prepareCanvas(ifc_canvas *c, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor);
  void restoreCanvas(ifc_canvas *c);

 protected:
  String font_id;
  int scriptid;

 private:
  ifc_canvas *prepareAntialias(ifc_canvas *c, int x, int y, const char *txt, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int w, int h);
  void completeAntialias(ifc_canvas *c);

  String face_name;
  String tmpfilename;
  int DColdstate;
  XFontStruct *font;
  Stack<fontslot*> fontstack;
  BltCanvas *antialias_canvas;
  int al_w, al_h, al_x, al_y, al_xo, al_yo, al_dw, al_dh;
  COLORREF al_mask;
};

#endif
