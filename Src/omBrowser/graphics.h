#ifndef NULLOSFT_ONLINEMEDIA_GRAPHICS_HEADER
#define NULLOSFT_ONLINEMEDIA_GRAPHICS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

INT GetColorDistance(COLORREF rgb1, COLORREF rgb2);
COLORREF GetDarkerColor(COLORREF rgb1, COLORREF rgb2);
COLORREF BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha);

BOOL Image_Colorize(BYTE *pPixels, LONG cx, LONG cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha);
BOOL Image_BlendOnColorEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb);
BOOL Image_BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb);
BOOL Image_Premultiply(BYTE *pPixels, LONG cx, LONG cy);
BOOL Image_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction);

HBITMAP Image_AnimateRotation(HDC hdc, HBITMAP bitmapFrame, INT frameCount, COLORREF rgbBk, BOOL fKeepSize);


#endif //NULLOSFT_ONLINEMEDIA_GRAPHICS_HEADER