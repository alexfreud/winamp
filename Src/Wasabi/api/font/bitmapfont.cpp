#include "precomp.h"
// ============================================================================================================================================================
// Font abstract class + statics to install TT fonts and Bitmap fonts
// ============================================================================================================================================================
#include "bitmapfont.h"
#include <api/wnd/fontdef.h>
#include <api/config/items/cfgitem.h>
#ifdef WASABI_COMPILE_SKIN
#include <api/skin/skin.h>
#endif
#ifdef WA3COMPATIBILITY
#endif

#include <tataki/canvas/ifc_canvas.h>
#include <tataki/region/api_region.h>
#include <api/skin/skinparse.h>

// ============================================================================================================================================================
// BitmapFont implementation. 
// ============================================================================================================================================================

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
BitmapFont::BitmapFont() : scriptid(0), char_width(0), char_height(0), hor_spacing(0), vert_spacing(0) {
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
BitmapFont::~BitmapFont() {

}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::isBitmap() {
  return 1;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
const wchar_t *BitmapFont::getFaceName() {
  return getFontId();
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::textOut(ifc_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased) {
  do_textOut(this, c, x+xoffset, y+yoffset, -1, -1, txt, size, bold, opaque, underline, italic, STDFONT_LEFT, color, WA_FONT_TEXTOUT_NORMAL);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::textOut2(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased) {
  do_textOut(this, c, x+xoffset, y+yoffset, x+xoffset+w, y+yoffset+h, txt, size, bold, opaque, underline, italic, align, color, WA_FONT_TEXTOUT_RECT);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::textOutEllipsed(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased) {
  do_textOut(this, c, x+xoffset, y+yoffset, x+xoffset+w, y+yoffset+h, txt, size, bold, opaque, underline, italic, align, color, WA_FONT_TEXTOUT_ELLIPSED);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::textOutWrapped(ifc_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased) {
  do_textOut(this, c, x+xoffset, y+yoffset, x+xoffset+w, y+yoffset+h, txt, size, bold, opaque, underline, italic, align, color, WA_FONT_TEXTOUT_WRAPPED);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::textOutWrappedPathed(ifc_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased) {
  do_textOut(this, c, x+xoffset, y+yoffset, x+xoffset+w, -1, txt, size, bold, opaque, underline, italic, align, color, WA_FONT_TEXTOUT_WRAPPEDPATHED);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::textOutCentered(ifc_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, ARGB32 bkcolor, int xoffset, int yoffset, int antialiased) {
  do_textOut(this, c, r->left, r->top, r->right-r->left, r->bottom-r->top, txt, size, bold, opaque, underline, italic, align, color, WA_FONT_TEXTOUT_CENTERED);
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getTextWidth(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased) 
{
  return wcslen(text) * char_width + wcslen(text)*hor_spacing;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getTextHeight(ifc_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased) 
{
  return char_height;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getTextHeight2(ifc_canvas *c, int size, int bold, int underline, int italic, int antialiased) 
{
  return char_height;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::getTextExtent(ifc_canvas *c, const wchar_t *text, int *w, int *h, int size, int bold, int underline, int italic, int antialiased) 
{
  if (w) *w = getTextWidth(c, text, size, bold, underline, italic, antialiased);
  if (h) *h = char_height;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::setFontBitmap(const wchar_t *name_or_element, const wchar_t *path) 
{
  StringW pathfile;
  if (!wcschr(name_or_element, L':')) 
	{
    pathfile = path;
		pathfile.AddBackslash();
  }
  pathfile.cat(name_or_element);
  if (!WACCESS(pathfile, 0))
    table = pathfile;
  else
    table = name_or_element;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::setFontMetrics(int _char_width, int _char_height, int _hor_spacing, int _vert_spacing) {
  char_width = _char_width;
  char_height = _char_height;
  hor_spacing = _hor_spacing;
  vert_spacing = _vert_spacing;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getHorizontalSpacing() {
  return hor_spacing;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getVerticalSpacing() {
  return vert_spacing;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getCharWidth() {
  return char_width;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getCharHeight() {
  return char_height;
}

AutoSkinBitmap *BitmapFont::getCharTable() {
  return &table;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::getXYfromChar(wchar_t ic, int *x, int *y)
{
	int c,c2=0;
	switch (ic)
	{
		case L'\u00B0': /*°*/ ic = L'0'; break;
		case L'\u00C6':/*Æ*/  ic = L'A'; break;
	//	case '\u00C1':/*Á*/  ic = L'A'; break;
	//	case '\u00C2': ic = L'A'; break;
    case L'\u00C7': /*Ç*/ ic = L'C'; break;
    case L'\u00C9':/*É*/ ic = L'E'; break;
      
		case L'\u00E0': /*à*/ case L'\u00E1': /*á*/ case L'\u00E2': /*â*/ ic = L'a'; break;
    case L'\u00E6':/*æ*/ ic = L'a'; break;
    case L'\u00E7': /*ç*/ ic = L'c'; break;
		case L'\u00E8': /*è*/ case L'\u00E9': /*é*/ case L'\u00EB': /*ë*/case L'\u00EA':/*ê*/  ic = L'e'; break;
    case L'\u00EC':/*ì*/ case L'\u00ED':/*í*/  case L'\u00EE':/*î*/ case L'\u00EF':/*ï*/ ic = L'i'; break;

#ifdef _WIN32
		case L'ó':/*ó*/ case L'ò':/*ò*/ case L'ô':/*ô*/ ic = L'o'; break;
		case L'ú':/*ú*/ case L'ù':/*ù*/ case L'û':/*û*/ ic = L'u'; break;
		case L'ÿ':/*ÿ*/ ic = L'y'; break;
		case L'Ü':/*Ü*/ ic = L'U'; break;
#else
#warning change these to \u
#endif
    case L'\u00D1':/*Ñ*/ ic = L'N'; break;
    case L'\u00F1':/*ñ*/ ic = L'n'; break;
    case L'\u00FC': /*ü*/ ic = L'u'; break;
    case L'\u0192':/*ƒ*/ ic = L'f'; break;
		default: break;
	} // quick relocations
	if (ic <= L'Z' && ic >= L'A') c = (ic-'A');
	else if (ic <= L'z' && ic >= L'a') c = (ic-'a');
	 else if (ic == L' ') c = 30;
    else {
		c2 += char_height;
		if (ic == L'\1') c=10;
	    else if (ic == L'.') c = 11;
  		else if (ic <= L'9' && ic >= L'0') c = ic - L'0';
  		else if (ic == L':') c = 12;
		else if (ic == L'(') c = 13;
  		else if (ic == L')') c = 14;
	    else if (ic == L'-') c = 15;
		else if (ic == L'\'' || ic=='`') c = 16;
		else if (ic == L'!') c = 17;
		else if (ic == L'_') c = 18;
		else if (ic == L'+') c = 19;
		else if (ic == L'\\') c = 20;
		else if (ic == L'/') c = 21;
		else if (ic == L'[' || ic == L'{' || ic == L'<') c = 22;
		else if (ic == L']' || ic == L'}' || ic == L'>') c = 23;
		else if (ic == L'~' || ic == L'^') c = 24;
		else if (ic == L'&') c = 25;
		else if (ic == L'%') c = 26;
		else if (ic == L',') c = 27;
		else if (ic == L'=') c = 28;
		else if (ic == L'$') c = 29;
		else if (ic == L'#') c = 30;
		else 
		{
			c2 += char_height;
#ifdef _WIN32
			if (ic == L'Å' || ic == L'å') c = 0;
			else if (ic == L'Ö' || ic == L'ö') c = 1;
			else if (ic == L'Ä' || ic == L'ä') c = 2;
      else 
#else
#warning change these to \u
#endif
        if (ic == L'?') c = 3;
			else if (ic == L'*') c = 4;
			else {
				c2 = 0;
				if (ic == L'"') c = 26;
				else if (ic == L'@') c = 27;
				else c = 30;
			}
		}
	  }
	c*=char_width;
	*x=c;
	*y=c2;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
int BitmapFont::getWordLength(const wchar_t *p) {
  int n=0;
  while (p && *p && *p != L' ') {
    p++;
    n++;
  }
  return n;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
wchar_t *BitmapFont::makeLine(const wchar_t *t, BitmapFont *font, int line, int width, int style) {

  static wchar_t str[4096];
  wchar_t *p = (wchar_t *)t;
  size_t len = wcslen(t);

  switch (style) {
    case WA_FONT_TEXTOUT_NORMAL:
    case WA_FONT_TEXTOUT_RECT:
    case WA_FONT_TEXTOUT_ELLIPSED:
    case WA_FONT_TEXTOUT_CENTERED:
      return line == 0 ? (wchar_t *)t : NULL;
    case WA_FONT_TEXTOUT_WRAPPEDPATHED: 
    case WA_FONT_TEXTOUT_WRAPPED: {
      size_t maxchar = width / (font->getCharWidth() + font->getVerticalSpacing());
      for (int i = 0; i < line; i++) {
        wchar_t *oldp = p;
        p += maxchar;
        if ((size_t)(p-t) >= len) return NULL;
        while (p >= t) {
          if (p == t || *(p-1) == L' ')
            break;
          p--;
        }
        if (p == oldp) {
          p += maxchar;
          while (p && *p && *p != L' ')
            p++;
        }
      }
      WCSCPYN(str, p, maxchar);
      wchar_t *d = &str[maxchar-1];
      int wr=0;
      if (wcslen(p) > maxchar && *(p+maxchar) != L' ' && wcschr(str, L' '))
        while (d >= str) {
          if (*d == L' ') {
            *d = 0;
            wr=1;
          }
          else {
            if (wr) break;
            d--;
          }
        }
      return str;
      }
  }
  return NULL;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
void BitmapFont::do_textOut(BitmapFont *font, ifc_canvas *c, int x, int y, int x2, int y2, const wchar_t *text, int size, int bold, int opaque, int underline, int italic, int align, ARGB32 color, int style) 
{
  static wchar_t *dotdotdot=L"...";
  if (!text) return;

  BaseCloneCanvas canvas;
  int ret = canvas.clone(c);
  if (!ret) return;

  RECT bounds;

  RECT defbounds={x,y,x2,y2};
  int __w, __h;
  c->getDim(&__w, &__h, NULL);
  if (x2 == -1) defbounds.right = defbounds.left + __w;
  if (y2 == -1) defbounds.bottom = defbounds.top + __h;
#ifdef _WIN32
  RegionI oldclip(&canvas, &defbounds); // get clipping region in oldclip
  RegionI *andclip=NULL;
  oldclip.getBox(&bounds); // get boundaries
#else
  bounds = defbounds;
#warning port me
#endif

/*  if (x2 != -1 && y2 != -1) {
    andclip = new RegionI(x, y, x2, y2); // create region for rect
    andclip->andRegion(oldclip); // and them
    canvas.selectClipRgn(andclip); // select new clipping rect
    andclip->getBox(&bounds); // update boundaries
  }*/

  int lc=-1;
  wchar_t *p = dotdotdot+3; // just a zero to triger next line

  int _x = x+(font->getHorizontalSpacing()/2);
  int _y = y;

  if (style == WA_FONT_TEXTOUT_CENTERED) {
    _y += (y2 - y - font->getCharHeight()) / 2;
  }

  _y -= font->getCharHeight() + font->getVerticalSpacing();

  int xp, yp;

  while (p) {

    if (!*p) {
      lc++;
      p = makeLine(text, font, lc, x2-x, style);
      if (!p || !*p) break;

      _x = x+(font->getHorizontalSpacing()/2);
      _y += font->getCharHeight() + font->getVerticalSpacing();
      if ((align == STDFONT_RIGHT || align == STDFONT_CENTER) && x2 != -1) {
        int l = wcslen(p);
        _x -= l * (font->getCharWidth() + font->getHorizontalSpacing()) - (x2-x);
      }
      if (align == STDFONT_CENTER)
        _x = x + (_x - x) / 2;

    }

    if ((style == WA_FONT_TEXTOUT_ELLIPSED || style == WA_FONT_TEXTOUT_WRAPPEDPATHED)  && x2 != -1) {
      if (_x > x2 - 4 * (font->getCharWidth() + font->getHorizontalSpacing()) && wcslen(p) > 3) {
        p = dotdotdot;
      }
    }

    font->getXYfromChar(*p, &xp, &yp);

    RECT r;
    r.left = xp;
    r.top = yp;
    r.right = xp + font->getCharWidth();
    r.bottom = yp + font->getCharHeight();

    RECT dst;
    dst.left = _x;
    dst.top = _y;
    dst.right = _x + font->getCharWidth();
    dst.bottom = _y + font->getCharHeight();

    if (Wasabi::Std::rectIntersect(dst, bounds))
//    if (IntersectRect(&dummy, &dst, &bounds))       // port me / checks clipping, not passed x,y,x2,y2
      font->getCharTable()->stretchToRectAlpha(&canvas, &r, &dst, 255);

    p++;
    _x += font->getCharWidth();
    _x += font->getHorizontalSpacing();
  }

#ifdef _WIN32
  if (andclip) {
    canvas.selectClipRgn(&oldclip); // restore previously saved clipping region
    delete andclip;
  }
#else
#warning port me
#endif
}

