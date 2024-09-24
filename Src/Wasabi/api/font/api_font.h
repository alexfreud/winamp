#ifndef __API_FONT_H
#define __API_FONT_H

#include <bfc/dispatch.h>

class ifc_canvas;

class NOVTABLE api_font : public Dispatchable 
{
  public:
    void font_textOut(ifc_canvas *c, int style, int x, int y, int w, int h, const wchar_t *txt);
    int font_getInfo(ifc_canvas *c, const wchar_t *font, int infoid, const wchar_t *txt, int *w, int *h);

  enum {
    API_FONT_FONT_TEXTOUT = 0,
    API_FONT_FONT_GETINFO = 10,
  };
};

inline void api_font::font_textOut(ifc_canvas *c, int style, int x, int y, int w, int h, const wchar_t *txt) 
{
  _voidcall(API_FONT_FONT_TEXTOUT, c, style, x, y, w, h, txt);
}

inline int api_font::font_getInfo(ifc_canvas *c, const wchar_t *font, int infoid, const wchar_t *txt, int *w, int *h) 
{
  return _call(API_FONT_FONT_GETINFO, (int)0, c, font, infoid, txt, w, h);
}

// {1FCA9C7E-5923-4b9c-8906-0F8C331DF21C}
static const GUID fontApiServiceGuid = 
{ 0x1fca9c7e, 0x5923, 0x4b9c, { 0x89, 0x6, 0xf, 0x8c, 0x33, 0x1d, 0xf2, 0x1c } };

extern api_font *fontApi;

#endif
