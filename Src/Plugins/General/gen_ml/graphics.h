#ifndef NULLSOFT_MEIDALIBRARY_GRAPHICS_HEADER
#define NULLSOFT_MEIDALIBRARY_GRAPHICS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

typedef struct _COLOR24 COLOR24;

typedef void (*BMPFILTERPROC)(const COLOR24*, const COLOR24*, COLOR24*);

HBITMAP CreateBitmapMask(HBITMAP originalBmp, COLORREF transColor); // Creates Mask Bitmap using specified color (helper)
HBITMAP CreateBitmapMask(HBITMAP originalBmp, int cx, int cy); // Creates Mask Bitmap using color from specified location (helper)
HBITMAP CreateBitmapMask(HBITMAP originalBmp, COLORREF transColor, int cx, int cy); // Creates Mask Bitmap if transColor is NULL uses color from specified position 

HBITMAP ConvertTo24bpp(HBITMAP bmp, int bpp=24); // creates a new bitmap with 24bpp. You responsible for destroying both

HBITMAP PatchBitmapColors24(HBITMAP bitmap, COLORREF color1, COLORREF color2, BMPFILTERPROC filterProc);

void Filter1(const COLOR24 *color1, const COLOR24 *color2, COLOR24 *pixel); // default filter 1
void Filter2(const COLOR24 *color1, const COLOR24 *color2, COLOR24 *pixel); // default filter 2

#endif //NULLSOFT_MEIDALIBRARY_GRAPHICS_HEADER

