#ifndef NULLSOFT_WASABI_SVC_FONTI_H
#define NULLSOFT_WASABI_SVC_FONTI_H

#include <api/service/svcs/svc_font.h>
//#include <tataki/canvas/canvas.h>
class ifc_canvas;
// implementor derives from this one
class NOVTABLE svc_fontI : public svc_font {
public:

  virtual void textOut(ifc_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias)=0;                           // abstract interface
  virtual void textOut2(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias)=0;
  virtual void textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias)=0;
  virtual void textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias)=0;
  virtual void textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias)=0;
  virtual void textOutCentered(ifc_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialias)=0;
  virtual int getTextWidth(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias)=0;
  virtual int getTextHeight(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialias)=0;
  virtual int getTextHeight2(ifc_canvas *c, int size, int bold, int underline, int italic, int antialias)=0;
  virtual void getTextExtent(ifc_canvas *c, const wchar_t *text, int *w, int *h, int size, int bold, int underline, int italic, int antialias)=0;

  virtual void setFontId(const wchar_t *id)=0;
  virtual const wchar_t *getFontId()=0;
  virtual const wchar_t *getFaceName()=0;
  virtual int isBitmap()=0;
  virtual int getScriptId()=0;
  virtual void setScriptId(int id)=0;

  virtual void setFontFace(const wchar_t *face)=0;
  virtual int addFontResource(OSFILETYPE f, const wchar_t *name)=0;
  virtual int addFontResource2(void *mem, int datalen, const wchar_t *name)=0;

  virtual const wchar_t * getFontSvcName()=0;

protected:
  RECVS_DISPATCH;
};

#endif