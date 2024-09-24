#ifndef __FREETYPEFONT_H
#define __FREETYPEFONT_H
#include <api/font/svc_fonti.h>
#include <tataki/blending/blending.h>
#include <bfc/string/StringW.h>

// begin relative to freetype include directory
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
// end relative to freetype include directory

class ifc_canvas;
class SkinBitmap;

class FreeTypeFont : public svc_fontI
{
 public:
  FreeTypeFont();
  virtual ~FreeTypeFont();

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

  virtual int isBitmap();
  virtual const wchar_t *getFaceName() { return facename; }
  virtual void setFontFace(const wchar_t *face) { facename = face; }

  virtual void setFontId(const wchar_t *id) { font_id = id; }
  virtual const wchar_t *getFontId() { return font_id; }
  virtual int getScriptId() { return scriptid; }
  virtual void setScriptId(int id) { scriptid = id; }

  virtual const wchar_t *getFontSvcName() { return L"Freetype"; }
  static const char *getServiceName() { return "Freetype font renderer"; }

 protected:
  
  int addFontResource( OSFILETYPE font, const wchar_t *name );
  int addFontResource2( void *data, int datalen, const wchar_t *name );
  
  void prepareCanvas(ifc_canvas *c, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, const wchar_t *txt, int width = -1, int height = -1 );
  void restoreCanvas(ifc_canvas *c, int x, int y );
  int drawChar( int x, int y, unsigned long c, COLORREF color, int antialias);
  int getAscent();

 protected:
  StringW font_id;
  StringW facename;
  int scriptid;

 private:
  void updateCharmap();
  void drawText(int x, int y, const wchar_t *txt, int len, COLORREF color, int antialias);
  int tweakSize(const wchar_t *face, int size);
  wchar_t *filenameToFontFace(const wchar_t *pszFile);
  inline unsigned __int8 overlay(unsigned __int8 c, unsigned __int8 b, int amount, int r, int z) 
	{
    if (curboldstrength == 2 && z == r-1) {
      if (c < b) 
        return Blenders::BLEND_ADJ1(c, b, 0xB0);
      return MIN(255, c+b);
    }
    return MIN(255, c+b);
  }
  FT_Library flib;
  char *fontbuffer;
  int fontbufferlen;
  FT_Face font;
  FT_Face font_bold;
  FT_Face font_italic;
  FT_Face font_bolditalic;
  SkinBitmap *blt;
  int lastchar;
  int curboldstrength;
  int cursize;
  int direction;
  int last_encoding;
  CfgItem *optionsitem;
};

#endif//__FREETYPEFONT_H
