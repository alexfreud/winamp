#include <precomp.h>
#include "fontapi.h"
#include <api/font/font.h>

api_font *fontApi = NULL;

FontApi::FontApi() 
{
  Font::init();
}

FontApi::~FontApi()
{
  Font::uninstallAll();
}

void FontApi::font_textOut(ifc_canvas *c, int style, int x, int y, int w, int h, const wchar_t *txt)
{
  Font::dispatchTextOut(c, style, x, y, w, h, txt);
}

int FontApi::font_getInfo(ifc_canvas *c, const wchar_t *font, int infoid, const wchar_t *txt, int *w, int *h) 
{
  return Font::dispatchGetInfo(c, font, infoid, txt, w, h);
}

#define CBCLASS FontApi
START_DISPATCH;
  VCB(API_FONT_FONT_TEXTOUT, font_textOut);
  CB(API_FONT_FONT_GETINFO, font_getInfo);
END_DISPATCH;
#undef CBCLASS