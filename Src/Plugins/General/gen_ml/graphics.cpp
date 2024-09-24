#include ".\graphics.h"
#include "ml.h"
#include <math.h>

HBITMAP CreateBitmapMask(HBITMAP originalBmp, int cx, int cy)
{
	return CreateBitmapMask(originalBmp, NULL, cx, cy);
}

HBITMAP CreateBitmapMask(HBITMAP originalBmp, COLORREF transColor)
{
	return CreateBitmapMask(originalBmp, transColor, 0, 0);
}

HBITMAP CreateBitmapMask(HBITMAP originalBmp, COLORREF transColor, int cx, int cy)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask;
	BITMAP bm;

	GetObjectW(originalBmp, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);

	HBITMAP obmp = (HBITMAP)SelectObject(hdcMem, originalBmp);
	HBITMAP omsk = (HBITMAP)SelectObject(hdcMem2, hbmMask);

	if (transColor == NULL) transColor = GetPixel(hdcMem, cx, cy);

	COLORREF oldColorBK = SetBkColor(hdcMem, transColor);

	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

	SetBkColor(hdcMem, oldColorBK);

	SelectObject(hdcMem, obmp);
	SelectObject(hdcMem2, omsk);

	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

    return hbmMask;
}

HBITMAP ConvertTo24bpp(HBITMAP bmp, int bpp)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbm24;
	BITMAP bm;

	GetObjectW(bmp, sizeof(BITMAP), &bm);

	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);

	void *bits;
	BITMAPINFOHEADER bi;

	ZeroMemory (&bi, sizeof (bi));
	bi.biSize = sizeof (bi);
	bi.biWidth= bm.bmWidth;
	bi.biHeight = -bm.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount= (WORD)(0xFF & bpp);

	hbm24 = CreateDIBSection(hdcMem2, (BITMAPINFO *)&bi, DIB_RGB_COLORS, &bits, NULL, NULL);

	HBITMAP oBmp = (HBITMAP)SelectObject(hdcMem, bmp);
	HBITMAP oBmp24 = (HBITMAP)SelectObject(hdcMem2, hbm24);

	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, oBmp);
	SelectObject(hdcMem2, oBmp24);

	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

    return hbm24;
}


// works only with DIB
HBITMAP PatchBitmapColors24(HBITMAP bitmap, COLORREF color1, COLORREF color2, BMPFILTERPROC filterProc)
{
	BITMAP bm;

	COLOR24 clrBG, clrFG;
	INT x, y;

	if (!filterProc) return NULL;

	if (!GetObjectW(bitmap, sizeof(BITMAP), &bm) || !bm.bmBits || 24 != bm.bmBitsPixel) return NULL;

	clrBG.rgbRed	= (BYTE)(0xFF & color1);
	clrFG.rgbRed	= (BYTE)(0xFF & color2);
	clrBG.rgbGreen	= (BYTE)(0xFF & (color1>>8));
	clrFG.rgbGreen	= (BYTE)(0xFF & (color2>>8));
	clrBG.rgbBlue	= (BYTE)(0xFF & (color1>>16));
	clrFG.rgbBlue	= (BYTE)(0xFF & (color2>>16));

	for (y = 0; y < bm.bmHeight; y++)
	{
		LONG width = (bm.bmWidthBytes%4)?bm.bmWidth*4:bm.bmWidthBytes;
		// bm.bmWidthBytes can lie so is safer to go with bm.bmWidth if the dword alignment cbeck fails
		// http://blogs.msdn.com/oldnewthing/archive/2004/10/26/247918.aspx#248529
		COLOR24 *cursor = (COLOR24*)(((BYTE*)bm.bmBits) + (width*y));
		for (x = 0; x < bm.bmWidth; x++, cursor++) filterProc(&clrBG, &clrFG, cursor);
	}
	return bitmap;
}

void Filter1(const COLOR24 *color1, const COLOR24 *color2, COLOR24 *pixel)
{
	pixel->rgbBlue	= (BYTE)(color1->rgbBlue - (int)((1.f -  (pixel->rgbBlue /255.f))* (color1->rgbBlue - color2->rgbBlue)));
	pixel->rgbGreen = (BYTE)(color1->rgbGreen - (int)((1.f -  (pixel->rgbGreen /255.f))* (color1->rgbGreen - color2->rgbGreen)));
	pixel->rgbRed	= (BYTE)(color1->rgbRed - (int)((1.f -  (pixel->rgbRed /255.f))* (color1->rgbRed - color2->rgbRed)));
}

void Filter2(const COLOR24 *color1, const COLOR24 *color2, COLOR24 *pixel)
{
	float chrom = (float)pixel->rgbBlue / 255.f;
	pixel->rgbBlue = (BYTE)(color1->rgbBlue * (1.f - chrom) + color2->rgbBlue * chrom);
	pixel->rgbGreen = (BYTE)(color1->rgbGreen * (1.f - chrom) + color2->rgbGreen * chrom);
	pixel->rgbRed = (BYTE)(color1->rgbRed * (1.f - chrom) + color2->rgbRed * chrom);
}