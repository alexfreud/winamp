#ifndef __BITMAPFONT_H
#define __BITMAPFONT_H

#include <api/font/svc_fonti.h>

//#include "font.h"

#include <bfc/platform/platform.h>
#include <bfc/ptrlist.h>
#include <bfc/stack.h>
#include <tataki/bitmap/autobitmap.h>
#include <bfc/string/StringW.h>

class Font;

class BitmapFont : public svc_fontI 
{
friend class Font;
 public:

  virtual void textOut(ifc_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased);
  virtual void textOut2(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased);
  virtual void textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased);
  virtual void textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased);
  virtual void textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased);
  virtual void textOutCentered(ifc_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased);
  virtual int getTextWidth(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased);
  virtual int getTextHeight(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased);
  virtual int getTextHeight2(ifc_canvas *c, int size, int bold, int underline, int italic, int antialiased);
  virtual void getTextExtent(ifc_canvas *c, const wchar_t *text, int *w, int *h, int size, int bold, int underline, int italic, int antialiased);
  virtual int isBitmap();

  virtual void setFontBitmap(const wchar_t *name_or_element, const wchar_t *path);
  virtual void setFontMetrics(int char_width, int char_height, int hor_spacing, int vert_spacing);
  virtual const wchar_t *getFaceName();

  virtual void setFontId(const wchar_t *id) { font_id = id; }
  virtual const wchar_t *getFontId() { return font_id; }
  virtual int getScriptId() { return scriptid; }
  virtual void setScriptId(int id) { scriptid = id; }
  virtual void setFontFace(const wchar_t *face) {}
  virtual int addFontResource(OSFILETYPE f, const wchar_t *name) { return 0; /*failure*/}
  virtual int addFontResource2(void *mem, int datalen, const wchar_t *name) { return 0; /*failure*/}

  virtual const wchar_t *getFontSvcName() { return L"Bitmap Font"; }

 protected:
  BitmapFont();
  virtual ~BitmapFont();

  AutoSkinBitmap *getCharTable();
  int getCharWidth();
  int getCharHeight();
  int getHorizontalSpacing();
  int getVerticalSpacing();
  void getXYfromChar(wchar_t ic, int *x, int *y);

 protected:
  StringW font_id;
  int scriptid;

 private:
  AutoSkinBitmap table;
  int char_width, char_height, hor_spacing, vert_spacing;
  static void do_textOut(BitmapFont *font, ifc_canvas *c, int x, int y, int x2, int y2, const wchar_t *text, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, int style);
  static int getWordLength(const wchar_t *p);
  static wchar_t *makeLine(const wchar_t *t, BitmapFont *font, int line, int physwidth, int style);
};

#endif
