#include "precomp.h"
// ============================================================================================================================================================
// Font abstract class + statics to install TT fonts and Bitmap fonts
// ============================================================================================================================================================
#include "freetypefont.h"
#include <tataki/canvas/ifc_canvas.h>
#include <tataki/bitmap/bitmap.h>
#include <api/config/items/cfgitem.h>
#include <api/memmgr/api_memmgr.h>

#define DO_KERNING // because Keith say so.

// local prototypes
static const wchar_t *find_break(void *f, const wchar_t *str, int width, int antialias);

#define M_PI 3.14159
#define M_2PI (M_PI*2)

#define FAUX_BOLD_RATIO 0.1f
#define FAUX_ITALIC_DEGREES 12

// This was necessary in Freetype 2.1.3 and bellow, but let us rejoice, they've fixed it
//#define NEED_SMALL_KERNING_HACK

static int freetype_width(void *data, const wchar_t *str, int len, int fixed, int antialias);


/**********************************************************************
*
*  FreeType Lib
*
**********************************************************************/

int (*FOLDSTRING)(    

    DWORD dwMapFlags,
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest
)=0;

FreeTypeFont::FreeTypeFont() 
{
  if (!FOLDSTRING)
  {
  HMODULE lib = LoadLibraryA("kernel32.dll");
  if (lib)
  {
//*(void **)&FOLDSTRING = GetProcAddress(lib, "FoldStringW");
  }
  FreeLibrary(lib);
  }
  fontbuffer = NULL;
  font = NULL;
  curboldstrength = 2;
}

int FreeTypeFont::addFontResource2(void *data, int datalen, const wchar_t *name) 
{
  optionsitem = NULL;
  if (FT_Init_FreeType(&flib)) {
    DebugString("FreeType: Cannot load freetype!\n");
    return 0;
  }

  last_encoding = -1;

  facename = name;

  fontbuffer = (char *)data;
  fontbufferlen = datalen;

  if (FT_New_Memory_Face(flib, (FT_Byte *)fontbuffer, fontbufferlen, 0, &font)) {
    DebugString("FreeType: Cannot load font!\n");
    return 0;
  }

  updateCharmap();

  return 1;
}

int FreeTypeFont::addFontResource(OSFILETYPE file, const wchar_t *name) 
{
  optionsitem = NULL;
  if (FT_Init_FreeType(&flib)) 
	{
    DebugString("FreeType: Cannot load freetype!\n");
    return 0;
  }

  last_encoding = -1;

  fontbufferlen = (int)FGETSIZE(file);
  fontbuffer = (char *)WASABI_API_MEMMGR->sysMalloc(fontbufferlen);
  FREAD(fontbuffer, fontbufferlen, 1, file);

  facename = name;

  if (FT_New_Memory_Face(flib, (FT_Byte *)fontbuffer, fontbufferlen, 0, &font)) 
	{
    DebugString("FreeType: Cannot load font!\n");
    return 0;
  }

  updateCharmap();

  return 1;
}

FreeTypeFont::~FreeTypeFont() 
{
  if (fontbuffer) {
    WASABI_API_MEMMGR->sysFree(fontbuffer);
    FT_Done_Face(font);
    FT_Done_FreeType(flib);
  }
}

void FreeTypeFont::updateCharmap() 
{
  if (optionsitem == NULL)
  {
    const GUID options_guid = 
    { 0x280876cf, 0x48c0, 0x40bc, { 0x8e, 0x86, 0x73, 0xce, 0x6b, 0xb4, 0x62, 0xe5 } };
    optionsitem = WASABI_API_CONFIG->config_getCfgItemByGuid(options_guid);
  }
  int i = 0;
  if (optionsitem) i =  optionsitem->getDataAsInt( L"Character mapping", 0); 
  if (i != last_encoding)
  {
    FT_Done_Face(font);
    FT_Done_FreeType(flib);
    FT_Init_FreeType(&flib);
    FT_New_Memory_Face(flib, (FT_Byte *)fontbuffer, fontbufferlen, 0, &font);
    switch (i) {
      case -1:
        break;
      case 0:
        FT_Select_Charmap(font, FT_ENCODING_UNICODE);
        break;
      case 1:
        FT_Select_Charmap(font, FT_ENCODING_APPLE_ROMAN);
        break;
      case 2:
        FT_Select_Charmap(font, FT_ENCODING_ADOBE_LATIN_1);
        break;
      case 3:
        FT_Select_Charmap(font, FT_ENCODING_ADOBE_STANDARD);
        break;
      case 4:
        FT_Select_Charmap(font, FT_ENCODING_ADOBE_CUSTOM);
        break;
      case 5:
        FT_Select_Charmap(font, FT_ENCODING_ADOBE_EXPERT);
        break;
      case 6:
        FT_Select_Charmap(font, FT_ENCODING_SJIS);
        break;
      case 7:
        FT_Select_Charmap(font, FT_ENCODING_BIG5);
        break;
      case 8:
        FT_Select_Charmap(font, FT_ENCODING_WANSUNG);
        break;
      case 9:
        FT_Select_Charmap(font, FT_ENCODING_JOHAB);
        break;
    }
  }
  last_encoding = i;
}

int FreeTypeFont::tweakSize(const wchar_t *face, int size) 
{
  if (WCSCASEEQLSAFE(face, L"nonstep")) return size-1;
  if (WCSCASEEQLSAFE(face, L"04B_08__")) return size+3;
  if (WCSCASEEQLSAFE(face, L"Blocky")) return size+6;
  if (WCSCASEEQLSAFE(face, L"04b_03b_")) return size+3;
  if (WCSCASEEQLSAFE(face, L"04b_09__")) return size+3;
  if (WCSCASEEQLSAFE(face, L"04b_21")) return size+3;
  if (WCSCASEEQLSAFE(face, L"Radiosta")) return size+2;
  if (WCSCASEEQLSAFE(face, L"ETHNOCENTRIC")) return size+3;
  if (WCSCASEEQLSAFE(face, L"ETHNOCEN")) return size+3;
  if (WCSCASEEQLSAFE(face, L"pixel")) return size+3;
  return size;
}

#define VERTICAL_TPADDING       2
#define VERTICAL_BPADDING       0
#define HORIZONTAL_LPADDING    -1
#define HORIZONTAL_RPADDING    1

void FreeTypeFont::prepareCanvas(api_canvas *c, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, const wchar_t *txt, int width, int height) 
{
  // Our "size" variable is fun to calculate!
  //int vRez = GetDeviceCaps(c->getHDC(), LOGPIXELSY); // this needs to be a Canvas method or something.
  int fsize = tweakSize(facename, size);
  int nHeight = MulDiv(fsize << 6, 72, 96);
  FT_Set_Char_Size(font, 0, nHeight, 0, 0);  

  updateCharmap();

  font->style_flags = 0;
  if (bold) 
	{
    font->style_flags |= FT_STYLE_FLAG_BOLD;
    curboldstrength = bold;//(bold && c->getTextAntialias()) ? 2 : bold;
  }
  if (italic)
    font->style_flags |= FT_STYLE_FLAG_ITALIC;
  if (underline)
    font->underline_thickness = 1;
  else
    font->underline_thickness = 0;

  if (height == -1 || width == -1) {
    getTextExtent(c, txt, &width, &height, size, bold, underline, italic, 0);
  }

  blt = new SkinBitmap(width+1, height, color & 0xffffff);
  blt->setHasAlpha(1);
}

void FreeTypeFont::restoreCanvas(api_canvas *c, int x, int y) 
{
  unsigned int *bits = (unsigned int *)blt->getBits();
  int n = blt->getWidth() * blt->getHeight();

#ifndef NO_MMX
  if (Blenders::MMX_AVAILABLE()) 
    for (int i = 0; i < n; i++)
      *bits++ = Blenders::BLEND_MUL_MMX(*bits | 0xff000000, *bits >> 24);
  else
#endif
    for (int i = 0; i < n; i++)
      *bits++ = Blenders::BLEND_MUL(*bits | 0xff000000, *bits >> 24);

#ifndef NO_MMX
  Blenders::BLEND_MMX_END();
#endif

  blt->blit(c, x + HORIZONTAL_LPADDING, y + VERTICAL_TPADDING);
  delete blt; 
  blt = NULL;
}

int FreeTypeFont::getAscent()
{
  FT_Glyph glyph;
  FT_BBox box;

  FT_Load_Glyph(font, FT_Get_Char_Index(font, 'M'), FT_LOAD_DEFAULT);
  FT_Get_Glyph(font->glyph, &glyph);
  FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &box);
  FT_Done_Glyph(glyph);

  return box.yMax >> 6;
}

void FreeTypeFont::drawText(int x, int y, const wchar_t *wtxt, int len, COLORREF color, int antialias)
{
  POINT pen = { x << 6, y};

  // we start left to right
  direction = 1;

  WCHAR *neutral = NULL;
  WCHAR *rtlstr = NULL;
	 
  wchar_t *freeme = 0;

  /* TODO: change this to be 
    AutoChar ucs4(txt, 12000, WC_COMPOSITECHECK); // convert from UTF-16 to 'raw' Unicode
    However, we have to re-write the LTR part of the code below
  */
  if (FOLDSTRING)
  {
    wchar_t *txt = WMALLOC((len+1));
	  FOLDSTRING(MAP_PRECOMPOSED, wtxt, -1, txt, len+1);
    freeme=txt;
    wtxt=txt;
  }
  
  lastchar = 0;
  for (int i=0 ; *wtxt && i<len;) 
  {
    WORD cur_dir;
    GetStringTypeExW(LOCALE_SYSTEM_DEFAULT, CT_CTYPE2, wtxt, 1, &cur_dir);
    if (cur_dir == C2_RIGHTTOLEFT) 
    {
      // we're now about to write some right-to-left text
      // we need to scan the string to determine the length of this right-to-left block
      if (!neutral) { neutral = WMALLOC(len+1); }
      if (!rtlstr) { rtlstr = WMALLOC(len+1); }
      rtlstr[0] = 0;
      neutral[0] = 0;

      const wchar_t *p = wtxt;
      while (1) 
      {
        WORD char_dir;
        if (*p) 
        {
          GetStringTypeExW(LOCALE_SYSTEM_DEFAULT, CT_CTYPE2, p, 1, &char_dir);
          if (char_dir != C2_RIGHTTOLEFT && char_dir != C2_LEFTTORIGHT) 
          {
            size_t l = wcslen(neutral);
            neutral[l] = *p;
            neutral[l+1] = 0;
          }
        }
        if (!*p || char_dir == C2_LEFTTORIGHT)
        {
          // we now need to write rtlstr as right-to-left
          int w = freetype_width(font, rtlstr, (int)wcslen(rtlstr), 1, antialias); // this is in fixed point units

          // save current pen position
          int oldpenx = pen.x; 

          // jump to the end of the block
          pen.x += w;

          p = rtlstr;

          direction = -1;

          while (p && *p) 
          {
            // move to the left by the width of the char
            pen.x -= freetype_width(font, p, 1, 1, antialias);
            // render the rtl character
            drawChar(pen.x, pen.y, *p, color, antialias);
            p++;
          }

          // now jump our to the end of the rtl block
          pen.x = oldpenx + w;

          // skip what we just printed from the source string
          wtxt += wcslen(rtlstr);

          // and continue like nothing happened
          direction = 1;

          break;
        } 
        else if (char_dir == C2_RIGHTTOLEFT) 
        {
          wcscat(rtlstr, neutral);
          *neutral = 0;
          size_t l = wcslen(rtlstr);
          rtlstr[l] = *p;
          rtlstr[l+1] = 0;
        }
        p++;
      }
    } 
    else 
    {
      pen.x += drawChar(pen.x, pen.y, *wtxt, color, antialias);
      wtxt++;
    }
  }
  if (rtlstr) FREE(rtlstr);
  if (neutral) FREE(neutral);
  if (freeme) FREE(freeme);
}


int FreeTypeFont::drawChar(int x0, int y0, unsigned long c, COLORREF color, int antialias) 
{
  unsigned int *bits = (unsigned int *)blt->getBits();
  int width = blt->getWidth();
  int height = blt->getHeight();

  x0;

  FT_BitmapGlyph ftg;

  int glyph_index = FT_Get_Char_Index(font, c);

  FT_Vector delta = { 0, 0 };
#ifdef DO_KERNING
  if (lastchar && FT_HAS_KERNING(font)) {
    FT_Get_Kerning(font, lastchar, glyph_index, FT_KERNING_DEFAULT, &delta);
    x0 += delta.x;
  }
#endif

  int rc = FT_Load_Glyph(font, glyph_index, FT_LOAD_DEFAULT|FT_LOAD_NO_BITMAP);
  if (rc)
    return 0;

  rc = FT_Get_Glyph(font->glyph, (FT_Glyph*)&ftg);
  if (rc)
    return 0;

  if (font->style_flags & FT_STYLE_FLAG_ITALIC) 
  {

    FT_Matrix mat;

    // sets up the matrix, 0 degrees
    double sintheta = sin(0.0);    
    double costheta = cos(0.0);
    mat.xx = (FT_Fixed)(costheta * (1<<16));
    mat.xy = (FT_Fixed)(sintheta * (1<<16));
    mat.yx = -mat.xy;
    mat.yy = mat.xx;

    // shear the vectors for italic, 10 to 12 deg suggested
    FT_Fixed f = (FT_Fixed)(tan(M_2PI/(360/FAUX_ITALIC_DEGREES)) * (1<<16)); 
    mat.xy += FT_MulFix(f, mat.xx);
    mat.yy += FT_MulFix(f, mat.yx);

    // do the transform
    FT_Vector v = {0,0};
    FT_Vector_Transform(&v, &mat);
    FT_Glyph_Transform((FT_Glyph)ftg, &mat, &v);

  }

  // get the glyph
  rc = FT_Glyph_To_Bitmap((FT_Glyph*)&ftg, antialias?ft_render_mode_normal:ft_render_mode_mono, NULL, 1);
  if (rc) {
    FT_Done_Glyph((FT_Glyph)ftg);
    return 0;
  }

  FT_Bitmap *bmp = &ftg->bitmap;

  int r = 1;
  if (font->style_flags & FT_STYLE_FLAG_BOLD) 
    r = MAX((int)((float)(font->glyph->advance.x >> 6) * FAUX_BOLD_RATIO), 2);

  int ys = MAX(0, y0 - ftg->top);
  int ye = MIN(height, (int)(y0 + bmp->rows - ftg->top));
  int xs = MAX(0, (x0 >> 6) + ftg->left);
  int xe = MIN(width, (int)((x0 >> 6) + ftg->left + bmp->width));

  unsigned char *_bmpbits = bmp->buffer + (ys + ftg->top - y0) * bmp->pitch + (xs - ftg->left - (x0 >> 6));
  unsigned int *_linebits = (unsigned int *)(bits + ys * width + xs);

  if (ftg->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
    for (int z = 0; z < r; z++) {
      unsigned char *bmpbits = _bmpbits;
      unsigned int *linebits = _linebits + z;
      for (int y = ys; y < ye; y++) {
        if (z > 0) {
          for (int x = xs; x < xe; x++) {
            if (x != width-z) {
              *linebits |= overlay((int)(*linebits >> 24), (int)*bmpbits, curboldstrength, r, z) << 24;
              linebits++; bmpbits++;
            } else {
              linebits++; bmpbits++;
            }
          }
        } else {
          for (int x = xs; x < xe; x++)
            *linebits++ |= *bmpbits++ << 24;
        }
        bmpbits += bmp->pitch - (xe - xs);
        linebits += width - (xe - xs);
      }
    }
  } else if (ftg->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
    for (int z = 0; z < r; z++) {
      unsigned char *bmpbits = _bmpbits;
      unsigned int *linebits = _linebits + z;
      for (int y = ys; y < ye; y++) {
        for (int x = xs; x < xe; x++) {
          /*          if (y == ys || y == ye-1 || x == xs || x == xe-1)
          *linebits++ |= 0xFF << 24;
          else
          linebits++;*/
          int byte = (x-xs)>>3;
          *linebits++ |= (*(bmpbits + byte) & (1 << (7-((x-xs)-(byte<<3)))) ? 0xFF : 0) << 24;
        }
        bmpbits += bmp->pitch;
        linebits += width - (xe - xs);
      }
    }
  } else {
    // simply draw rectangles
    for (int z = 0; z < r; z++) {
      unsigned char *bmpbits = _bmpbits;
      unsigned int *linebits = _linebits + z;
      for (int y = ys; y < ye; y++) {
        for (int x = xs; x < xe; x++) {
          if (y == ys || y == ye-1 || x == xs || x == xe-1)
            *linebits++ |= 0xFF << 24;
          else
            linebits++;
        }
        bmpbits += bmp->pitch;
        linebits += width - (xe - xs);
      }
    }
  }


  FT_Done_Glyph((FT_Glyph)ftg);

  lastchar = glyph_index;

  return font->glyph->advance.x + delta.x + ((r - 1) << 6);
}

void FreeTypeFont::textOut(api_canvas *c, int x, int y, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias) 
{
  color = RGBTOBGR(color);
  bkcolor = RGBTOBGR(bkcolor);
  y++; x++;
  prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor, txt);
  int maxheight = getAscent();

  drawText(0, maxheight, txt, (int)wcslen(txt), color, antialias);

  /*  POINT pen = { 0, maxheight };

  lastchar = 0;
  for (; *txt; txt++) {
  pen.x += drawChar(pen.x, pen.y, *txt, color);
  }*/

  restoreCanvas(c, x + xoffset , y + yoffset);
}

void FreeTypeFont::textOut2(api_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias) 
{
  color = RGBTOBGR(color);
  bkcolor = RGBTOBGR(bkcolor);
  y++; x++;
  prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor, txt, w, h);
  int maxheight = getAscent();
  int width = getTextWidth(c,txt,size,bold,underline,italic,antialias);

  int xstart = 0;
  if (align == DT_RIGHT)
  {
    xstart = w - width;
  } 
  else if (align == DT_CENTER)
  {
    xstart = (w - width) / 2;
  }


  drawText(xstart, maxheight, txt, (int)wcslen(txt), color, antialias);

  /*  POINT pen = { xstart << 6, maxheight };

  lastchar = 0;
  for (; *txt; txt++) {
  pen.x += drawChar(pen.x, pen.y, *txt, color);
  }*/

  restoreCanvas(c, x + xoffset , y + yoffset);
}

void FreeTypeFont::textOutEllipsed(api_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias) 
{
  color = RGBTOBGR(color);
  bkcolor = RGBTOBGR(bkcolor);
  y++; x++;
  if (txt == NULL) 
    return;

  RECT r;
  r.left = x+xoffset;
  r.top = y+yoffset;
  r.right = r.left + w;
  r.bottom = r.top + h;
  prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor, txt, w, h);

  int len = ((int)wcslen(txt) + 3);
  wchar_t *tmp = (wchar_t *)MALLOC(sizeof(wchar_t) * len);
  wcsncpy(tmp, txt, len);
  int width, height;
  width = getTextWidth(c, txt, size, bold, underline, italic, (int)antialias);
  height = getAscent();
  int dddw = getTextWidth(c, L"...", size, bold, underline, italic, antialias);

  if (width > r.right - r.left)
  {
    wchar_t *p = tmp + wcslen(tmp);
    width = r.right - r.left - dddw;
    while(p > tmp &&
      getTextWidth(c,tmp,size,bold,underline,italic,antialias) > width) 
    {
      *p-- = '\0';
    }
    wcscpy(p, L"...");
  }

  drawText(0, height, tmp, (int)wcslen(tmp), color, antialias);

  /*  POINT pen = { 0, height };

  lastchar = 0;
  for (char *p = tmp; *p; p++) {
  pen.x += drawChar(pen.x, pen.y, *p, color);
  }*/

  FREE(tmp);

  restoreCanvas(c, r.left, r.top);
}

static int freetype_width(void *data, const wchar_t *str, int len, int fixed, int antialias)
{
  FT_Face font = (FT_Face)data;

  int w = 0;

  int prev, index;
  const wchar_t *p;
  int count = 0;
  for (p = str; *p && count < len;) 
  {
    FT_Vector delta = { 0, 0 };

    FT_BitmapGlyph ftg;

    index = FT_Get_Char_Index(font, *p);

#ifdef DO_KERNING
    if (w > 0 && FT_HAS_KERNING(font)) {
      FT_Get_Kerning(font, prev, index, ft_kerning_default, &delta);
    }
#endif

    int rc = FT_Load_Glyph(font, index, FT_LOAD_DEFAULT|FT_LOAD_NO_BITMAP);
    if (rc) 
    {
      p++;
      count++;
      continue;
    }

    w += (delta.x + font->glyph->advance.x); 
    prev = index;


    rc = FT_Get_Glyph(font->glyph, (FT_Glyph*)&ftg);
    if (rc) 
    {
      p++;
      count++;
      continue;
    }

    if (font->style_flags & FT_STYLE_FLAG_ITALIC)
    {

      FT_Matrix mat;

      // sets up the matrix, 0 degrees
      double sintheta = sin(0.0);    
      double costheta = cos(0.0);
      mat.xx = (FT_Fixed)(costheta * (1<<16));
      mat.xy = (FT_Fixed)(sintheta * (1<<16));
      mat.yx = -mat.xy;
      mat.yy = mat.xx;

      // shear the vectors for italic, 10 to 12 deg suggested
      FT_Fixed f = (FT_Fixed)(tan(M_2PI/(360/FAUX_ITALIC_DEGREES)) * (1<<16)); 
      mat.xy += FT_MulFix(f, mat.xx);
      mat.yy += FT_MulFix(f, mat.yx);

      // do the transform
      FT_Vector v = {0,0};
      FT_Vector_Transform(&v, &mat);
      FT_Glyph_Transform((FT_Glyph)ftg, &mat, &v);

    }

    // get the glyph
		rc = FT_Glyph_To_Bitmap((FT_Glyph*)&ftg, antialias?ft_render_mode_normal:ft_render_mode_mono, NULL, 1);
    if (rc) 
    {
      FT_Done_Glyph((FT_Glyph)ftg);
      p++;
      count++;
      continue;
    }
    FT_Bitmap *bmp = &ftg->bitmap;

    int ys = MAX(0, ftg->top);
    int ye = bmp->rows - ftg->top;
    int xs = MAX(0, ftg->left);
    int xe = ftg->left + bmp->width;

    FT_Done_Glyph((FT_Glyph)ftg);

    int r = 1;
    if (font->style_flags & FT_STYLE_FLAG_BOLD)
    {
      r = MAX((int)((float)(font->glyph->advance.x >> 6) * FAUX_BOLD_RATIO), 2);
    }
    w += ((r - 1) << 6);
    p++;
    count++;
  }

  if (fixed) return w;
  return (w >> 6);
}


void FreeTypeFont::textOutWrapped(api_canvas *c, int x, int y, int w, int h, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias) 
{
  color = RGBTOBGR(color);
  bkcolor = RGBTOBGR(bkcolor);
  y++; x++;
  prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor, txt, w, h);

  int ascent = getAscent();
  int descent = getTextHeight2(c, size, bold, underline, italic, antialias);
  descent -= ascent;

  int yoff = ascent;
  const wchar_t *cur = txt, *next;
  int length = (int)wcslen(txt);

  //NO.
  //for(int yoff = ascent; 
  for(yoff = ascent; 
    yoff < h; 
    yoff += ascent + descent) {

      next = find_break(font, cur, w, antialias);

      /*    POINT pen = { 0, yoff };

      lastchar = 0;
      for (; cur < next; cur++) {
      pen.x += drawChar(pen.x, pen.y, *cur, color);
      }*/
      drawText(0, yoff, cur, (int)(next-cur), color, antialias);

      cur = next;
      while (cur && *cur && *cur == ' ')
        cur++;

      if (*cur == '\r' || *cur == '\n') cur++;

      if (cur >= txt + length)
        break;
    }

    restoreCanvas(c, x + xoffset , y + yoffset);
}

void FreeTypeFont::textOutWrappedPathed(api_canvas *c, int x, int y, int w, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias) 
{
  color = RGBTOBGR(color);
  bkcolor = RGBTOBGR(bkcolor);
  DebugString("writeme -- FreeTypeFont::textOutWrappedPathed...\n");
}

void FreeTypeFont::textOutCentered(api_canvas *c, RECT *r, const wchar_t *txt, int size, int bold, int opaque, int underline, int italic, int align, COLORREF color, COLORREF bkcolor, int xoffset, int yoffset, int antialias) 
{
  color = RGBTOBGR(color);
  bkcolor = RGBTOBGR(bkcolor);
  r->top++;
  r->left++;

  RECT rr = *r;
  rr.left += xoffset;
  rr.right += xoffset;
  rr.top += yoffset;
  rr.bottom += yoffset;

  prepareCanvas(c, size, bold, opaque, underline, italic, color, bkcolor, txt, rr.right - rr.left, rr.bottom - rr.top);

  int width, height;
  height = getAscent();
  width = getTextWidth(c, txt, size, bold, underline, italic, (int)antialias);

  drawText(((rr.right - rr.left - width) / 2), height, txt, (int)wcslen(txt), color, (int)antialias);
  /*  POINT pen = { ((rr.right - rr.left - width) / 2) << 6, height };

  lastchar = 0;
  for (; *txt; txt++) {
  pen.x += drawChar(pen.x, pen.y, *txt, color);
  }*/

  restoreCanvas(c, rr.left, rr.top);
}

int FreeTypeFont::getTextWidth(api_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased) 
{
  int w;
  updateCharmap();
  getTextExtent(c, text, &w, NULL, size, bold, underline, italic, antialiased);
  return w;
}

int FreeTypeFont::getTextHeight(api_canvas *c, const wchar_t *text, int size, int bold, int underline, int italic, int antialiased) {
  int h;
  updateCharmap();
  getTextExtent(c, text, NULL, &h, size, bold, underline, italic, antialiased);
  {
    // calcul for multiline text
    const wchar_t *p=text;
    int n=0;
    while(p && *p!=0) if(*p++=='\n') n++;
    if(n) h*=(n+1);
  }
  return h;
}

int FreeTypeFont::getTextHeight2(api_canvas *c, int size, int bold, int underline, int italic, int antialiased) 
{
  return getTextHeight(c, L"Mg", size, bold, underline, italic, antialiased);
}

void FreeTypeFont::getTextExtent(api_canvas *c, const wchar_t *txt, int *w, int *h, int size, int bold, int underline, int italic, int antialias) 
{  
  updateCharmap();
// Our "size" variable is fun to calculate!
  //int vRez = GetDeviceCaps(c->getHDC(), LOGPIXELSY); // this needs to be a Canvas method or something.
  int fsize = tweakSize(facename, size);
  int nHeight = MulDiv(fsize << 6, 72, 96);
  FT_Set_Char_Size(font, 0, nHeight, 0, 0);  

  font->style_flags = 0;
  if (bold)
    font->style_flags |= FT_STYLE_FLAG_BOLD;
  if (italic)
    font->style_flags |= FT_STYLE_FLAG_ITALIC;
  if (underline)
    font->underline_thickness = 1;
  else
    font->underline_thickness = 0;

  SIZE rsize={0,0};
  ASSERT(txt != NULL);
  if (*txt == 0)
  {
    if (w != NULL) *w = 0;
    if (h != NULL) *h = 0;
    return;
  }

  FT_BBox box;
  FT_Glyph glyph;

  int minh = 0, maxh = 0;

  if (w) 
		*w = freetype_width(font, txt, (int)wcslen(txt), 0, antialias) + HORIZONTAL_RPADDING + HORIZONTAL_LPADDING;

  if (h) 
  {
    FT_Load_Glyph(font, FT_Get_Char_Index(font, 'M'), FT_LOAD_DEFAULT);
    FT_Get_Glyph(font->glyph, &glyph);
    FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &box);
    maxh = box.yMax;
    FT_Done_Glyph(glyph);

    FT_Load_Glyph(font, FT_Get_Char_Index(font, 'g'), FT_LOAD_DEFAULT);
    FT_Get_Glyph(font->glyph, &glyph);
    FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_unscaled, &box);
    minh = box.yMin;
    FT_Done_Glyph(glyph);

    *h = ((maxh - minh) >> 6) + VERTICAL_TPADDING + VERTICAL_BPADDING;
  }
}


int FreeTypeFont::isBitmap()
{
  return 0;
}

static const wchar_t *find_break(void *f, const wchar_t *str, int width, int antialias) 
{
  const wchar_t *softret, *lastsoft, *hardret;

  if (freetype_width(f, str, (int)wcslen(str), 0, antialias) <= width)
    return str + wcslen(str);

  for(hardret = str; *hardret; hardret ++)
    if (*hardret == '\r' || *hardret == '\n')
      break;

  if (hardret && freetype_width(f, str, (int)(hardret - str), 0, antialias) <= width) {
    return hardret;
  }
  for(softret = str; *softret && !isspace(*softret); softret++)
    ;

  if (freetype_width(f, str, (int)(softret - str), 0, antialias) <= width)
  {
    do 
    {
      lastsoft = softret;

      for(softret = lastsoft+1; *softret && !isspace(*softret); softret++)
        ;

    } while (lastsoft && *lastsoft && freetype_width(f, str, (int)(softret - str), 0, antialias) <= width);

    softret = lastsoft;
  }
  else 
  {
    for(softret = str; *softret; softret++)
      if (freetype_width(f, str, (int)(softret - str), 0, antialias) > width)
        break;

    softret--;
  }

  return softret;
}
