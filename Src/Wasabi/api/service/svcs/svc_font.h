#ifndef _SVC_FONT_H
#define _SVC_FONT_H

#include <bfc/dispatch.h>
#include <bfc/std_file.h>
#include <stdio.h>
#include <api/service/services.h>
//#include <api/service/servicei.h>

class ifc_canvas;

#ifdef _WIN32
enum
{
  STDFONT_LEFT = DT_LEFT,
  STDFONT_RIGHT = DT_RIGHT,
  STDFONT_CENTER = DT_CENTER,
};
#else
#warning TODO: find good values for these
enum
{
  STDFONT_RIGHT = 1,
  STDFONT_CENTER = 2,
  STDFONT_LEFT = 4,
};
#endif

class NOVTABLE svc_font : public Dispatchable
{
public:
  static FOURCC getServiceType() { return WaSvc::FONTRENDER; }

  void textOut(ifc_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias);                           // abstract interface
  void textOut(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias);
  void textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias);
  void textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias);
  void textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias);
  void textOutCentered(ifc_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias);
  int getTextWidth(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias);
  int getTextHeight(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias);
  int getTextHeight(ifc_canvas *c, int size, int bold, int underline, int italic, int antialias);
  void getTextExtent(ifc_canvas *c, const wchar_t *text, int *w, int *h, int size, int bold, int underline, int italic, int antialias);

  void setFontId(const wchar_t *id);
  const wchar_t *getFontId();
  const wchar_t *getFaceName();
  int isBitmap();
  int getScriptId();
  void setScriptId(int id);

  void setFontFace(const wchar_t *face);
  int addFontResource(HANDLE f, const wchar_t *name);
  int addFontResource2(void *mem, int datalen, const wchar_t *name);

  const wchar_t *getFontSvcName();

protected:
  enum {
    TEXTOUT,
    TEXTOUT2,
    TEXTOUTELLIPSED,
    TEXTOUTWRAPPED,
    TEXTOUTWRAPPEDPATHED,
    TEXTOUTCENTERED,
    GETTEXTWIDTH,
    GETTEXTHEIGHT,
    GETTEXTHEIGHT2,
    GETTEXTEXTENT,
    SETFONTID,
    GETFONTID,
    GETFACENAME_,	// GETFACENAME is taken in win32
    ISBITMAP,
    GETSCRIPTID,
    SETSCRIPTID,
    SETFONTFACE,
    ADDFONTRESOURCE,
    ADDFONTRESOURCE2,
    GETFONTSVCNAME,
  };
};

inline void svc_font::textOut(ifc_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias) 
{
  _voidcall(TEXTOUT, c, x, y, txt, size, bold, opaque, underline, italic, color, bkcolor, xoffset, yoffset, antialias);
}

inline void svc_font::textOut(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias) 
{
  _voidcall(TEXTOUT2, c, x, y, w, h, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialias);
}

inline void svc_font::textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias) 
{
  _voidcall(TEXTOUTELLIPSED, c, x, y, w, h, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialias);
}

inline void svc_font::textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias) 
{
  _voidcall(TEXTOUTWRAPPED, c, x, y, w, h, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialias);
}

inline void svc_font::textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias) 
{
  _voidcall(TEXTOUTWRAPPEDPATHED, c, x, y, w, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialias);
}

inline void svc_font::textOutCentered(ifc_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias) 
{
  _voidcall(TEXTOUTCENTERED, c, r, txt, size, bold, opaque, underline, italic, align, color, bkcolor, xoffset, yoffset, antialias);
}

inline int svc_font::getTextWidth(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias) 
{
  return _call(GETTEXTWIDTH, (int)0, c, text, size, bold, underline, italic, antialias);
}

inline int svc_font::getTextHeight(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias) {
  return _call(GETTEXTHEIGHT, (int)0, c, text, size, bold, underline, italic, antialias);
}

inline int svc_font::getTextHeight(ifc_canvas *c, int size, int bold, int underline, int italic, int antialias) {
  return _call(GETTEXTHEIGHT, (int)0, c, size, bold, underline, italic, antialias);
}

inline void svc_font::getTextExtent(ifc_canvas *c, const wchar_t *text, int *w, int *h, int size, int bold, int underline, int italic, int antialias) {
  _voidcall(GETTEXTEXTENT, c, text, w, h, size, bold, underline, italic, antialias);
}

inline void svc_font::setFontId(const wchar_t *id) {
  _voidcall(SETFONTID, id);
}

inline const wchar_t *svc_font::getFontId() 
{
  return _call(GETFONTID, (const wchar_t *)0);
}

inline const wchar_t *svc_font::getFaceName() 
{
  return _call(GETFACENAME_, (const wchar_t *)0);
}

inline int svc_font::isBitmap() {
  return _call(ISBITMAP, (int)0);
}

inline int svc_font::getScriptId() {
  return _call(GETSCRIPTID, (int)0);
}

inline void svc_font::setScriptId(int id) {
  _voidcall(SETSCRIPTID, id);
}

inline void svc_font::setFontFace(const wchar_t *face) 
{
  _voidcall(SETFONTFACE, face);
}

inline int svc_font::addFontResource(HANDLE f, const wchar_t *name) {
  return _call(ADDFONTRESOURCE, (int)0, f, name);
}

inline int svc_font::addFontResource2(void *mem, int datalen, const wchar_t *name) {
  return _call(ADDFONTRESOURCE2, (int)0, mem, datalen, name);
}

inline const wchar_t *svc_font::getFontSvcName() {
  return _call(GETFONTSVCNAME, (const wchar_t *)0);
}






#endif // _SVC_FONT_H
