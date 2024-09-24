#ifndef NULLSOFT_AUTH_LOGINBOX_GRAPHICS_HEADER
#define NULLSOFT_AUTH_LOGINBOX_GRAPHICS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL Image_ColorOver(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb);
BOOL Image_ColorOverEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb);

BOOL Image_ColorizeEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgbBottom, COLORREF rgbTop);

BOOL Image_Premultiply(HBITMAP hbmp, RECT *prcPart);
BOOL Image_PremultiplyEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp);

BOOL Image_Saturate(HBITMAP hbmp, RECT *prcPart, INT n, BOOL fScale);
BOOL Image_SaturateEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, INT n, BOOL fScale);

BOOL Image_AdjustAlpha(HBITMAP hbmp, RECT *prcPart, INT n, BOOL fScale);
BOOL Image_AdjustAlphaEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, INT n, BOOL fScale);

BOOL Image_AdjustSaturationAlpha(HBITMAP hbmp, RECT *prcPart, INT nSaturation, INT nAlpha);
BOOL Image_AdjustSaturationAlphaEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, INT nSaturation, INT nAlpha);

COLORREF Color_Blend(COLORREF rgbTop, COLORREF rgbBottom, INT alpha);

#endif //NULLSOFT_AUTH_LOGINBOX_GRAPHICS_HEADER