#ifndef __TRUETYPEFONT_WN32_H
#define __TRUETYPEFONT_WN32_H

#include <api/font/svc_fonti.h>

#include <bfc/stack.h>
#include <api/font/truetypefontdef.h>
#include <tataki/canvas/bltcanvas.h>
class ifc_canvas;
class BltCanvas;

class TrueTypeFont_Win32 : public svc_fontI {
 public:
  TrueTypeFont_Win32();
  virtual ~TrueTypeFont_Win32();

  virtual void textOut(ifc_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOut2(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual void textOutCentered(ifc_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias);
  virtual int getTextWidth(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias);
  virtual int getTextHeight(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias);
  virtual int getTextHeight2(ifc_canvas *c, int size, int bold, int underline, int italic, int antialias);
  virtual void getTextExtent(ifc_canvas *c, const wchar_t *text, int *w, int *h, int size, int bold, int underline, int italic, int antialias);

  virtual void setFontId(const wchar_t *id) { font_id = id; }
  virtual const wchar_t *getFontId() { return font_id; }
  virtual int getScriptId() { return scriptid; }
  virtual void setScriptId(int id) { scriptid = id; }

  virtual int isBitmap();
  virtual void setFontFace(const wchar_t *face);
  virtual int addFontResource(OSFILETYPE f, const wchar_t *name);
  virtual int addFontResource2( void *data, int datalen, const wchar_t *name );
  virtual const wchar_t *getFaceName();

  virtual const wchar_t *getFontSvcName() { return L"Win32 TextOut"; }
  static const char *getServiceName() { return "Win32 Truetype font renderer"; }

  static wchar_t *filenameToFontFace(const wchar_t *filename);

 protected:
  void prepareCanvas(BltCanvas *canvas, int size, int bold, int opaque, int underline, int italic, int antialiased);
  void restoreCanvas(BltCanvas *canvas, ifc_canvas *dest, int w, int h, COLORREF color, COLORREF bkcolor, int antialiased, int x=0, int y=0);
	HFONT MakeFont(int size, int bold, int underline, int italic, int antialiased);

 protected:
  StringW font_id;
  int scriptid;

 private:
  StringW face_name;
  StringW tmpfilename;

  HFONT font, oldFont;

};

#endif