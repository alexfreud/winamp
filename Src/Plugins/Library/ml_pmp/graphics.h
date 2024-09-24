#ifndef _NULLSOFT_WINAMP_ML_PORTABLES_GRAPHICS_HEADER
#define _NULLSOFT_WINAMP_ML_PORTABLES_GRAPHICS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef SelectBitmap
  #define SelectBitmap(_hdc, _bitmap) ((HBITMAP)SelectObject(_hdc, _bitmap))
#endif

#ifndef SelectFont
  #define SelectFont(_hdc, _font) ((HFONT)SelectObject(_hdc, _font))
#endif

#ifndef SelectBrush
  #define SelectBrush(_hdc, _brush) ((HBRUSH)SelectObject(_hdc, _brush))
#endif

#ifndef GetCurrentBitmap
  #define GetCurrentBitmap(_hdc) ((HBITMAP)GetCurrentObject(_hdc, OBJ_BITMAP))
#endif

#ifndef GetCurrentFont
  #define GetCurrentFont(_hdc) ((HFONT)GetCurrentObject(_hdc, OBJ_FONT))
#endif

#ifndef GetCurrentBrush
  #define GetCurrentBrush(_hdc) ((HBRUSH)GetCurrentObject(_hdc, OBJ_BRUSH))
#endif


#ifndef SET_GRADIENT_VERTEX
  #define SET_GRADIENT_VERTEX(_vertex, _x, _y, _rgb)\
				{(_vertex).x = (_x); (_vertex).y = (_y);\
				 (_vertex).Red = GetRValue(_rgb) << 8; (_vertex).Green = GetGValue(_rgb) << 8;\
				 (_vertex).Blue = GetBValue(_rgb) << 8; (_vertex).Alpha = 0x0000;}
#endif

#ifndef SET_GRADIENT_RECT_MESH
  #define SET_GRADIENT_RECT_MESH(_mesh, _upperLeft, _bottomRight)\
				{(_mesh).UpperLeft = (_upperLeft); (_mesh).LowerRight = (_bottomRight);}
#endif


BYTE Graphics_GetSysFontQuality();
HFONT Graphics_CreateSysFont();
HFONT Graphics_DuplicateFont(HFONT sourceFont, INT heightDeltaPt, BOOL forceBold, BOOL systemQuality);

long Graphics_GetFontHeight(HDC hdc);	 
long Graphics_GetAveStrWidth(HDC hdc, UINT cchLen);
BOOL Graphics_GetWindowBaseUnits(HWND hwnd, LONG *baseUnitX, LONG *baseUnitY);

COLORREF Graphics_GetSkinColor(unsigned int colorIndex);
COLORREF Graphics_BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha);
INT Graphics_GetColorDistance(COLORREF rgb1, COLORREF rgb2);

void Graphics_ClampRect(RECT *rect, const RECT *boxRect);
void Graphics_NormalizeRect(RECT *rect);
void Graphics_GetRectSizeNormalized(const RECT *rect, SIZE *size);
BOOL Graphics_IsRectFit(const RECT *rect, const RECT *boxRect);

BOOL SetSizeEmpty(SIZE *size);
BOOL IsSizeEmpty(SIZE *size);
BOOL SetSize(SIZE *size, long width, long height);

BOOL SetPoint(POINT *pt, long x, long y);
BOOL MakeRectPolygon(POINT vertices[4], long left, long top, long right, long bottom);


#endif //_NULLSOFT_WINAMP_ML_PORTABLES_GRAPHICS_HEADER
