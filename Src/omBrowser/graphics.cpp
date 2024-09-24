#include "./main.h"
#include "./graphics.h"

#include <math.h>
#include <strsafe.h>


INT GetColorDistance(COLORREF rgb1, COLORREF rgb2)
{
	return (1000 * ((GetRValue(rgb1) - GetRValue(rgb2)) + 
			(GetGValue(rgb1) - GetGValue(rgb2)) +
			(GetBValue(rgb1) - GetBValue(rgb2))))/ (3 * 255);
}

COLORREF GetDarkerColor(COLORREF rgb1, COLORREF rgb2)
{
	INT g1 = (GetRValue(rgb1)*299 + GetGValue(rgb1)*587 + GetBValue(rgb1)*114);
	INT g2 = (GetRValue(rgb2)*299 + GetGValue(rgb2)*587 + GetBValue(rgb2)*114);
	return (g1 < g2) ? rgb1 : rgb2;
}

COLORREF BlendColors(COLORREF rgbTop, COLORREF rgbBottom, INT alpha)
{
	if (alpha > 254) return rgbTop;
	if (alpha < 0) return rgbBottom;

	WORD k = (((255 - alpha)*255 + 127)/255);
	
	return RGB( (GetRValue(rgbTop)*alpha + k*GetRValue(rgbBottom) + 127)/255, 
				(GetGValue(rgbTop)*alpha + k*GetGValue(rgbBottom) + 127)/255, 
				(GetBValue(rgbTop)*alpha + k*GetBValue(rgbBottom) + 127)/255);
}

BOOL Image_Colorize(BYTE *pPixels, LONG cx, LONG cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha)
{
	LONG pitch, x;
	INT step;
	BYTE rFg, gFg, bFg;
	LPBYTE cursor, line;

	if (bpp < 24) return FALSE;

	step = (bpp>>3);
	pitch = cx*step;
	while (pitch%4) pitch++;
	
	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	
	INT  bK = (bFg - GetBValue(rgbBk));
	INT gK = (gFg - GetGValue(rgbBk));
	INT rK = (rFg - GetRValue(rgbBk));

	if (24 == bpp)
	{
		for (line = pPixels; cy-- != 0; line += pitch )
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += 3) 
			{
				cursor[0] = bFg - (bK*(255 - cursor[0])>>8);
				cursor[1] = gFg - (gK*(255 - cursor[1])>>8);
				cursor[2] = rFg - (rK*(255 - cursor[2])>>8);
			}
		}
	}
	else if (32 == bpp)
	{
		if (removeAlpha)
		{
			BYTE rBk, gBk, bBk;
			rBk = GetRValue(rgbBk); gBk = GetGValue(rgbBk); bBk = GetBValue(rgbBk);
			for (line = pPixels; cy-- != 0; line += pitch )
			{	
				for (x = cx, cursor = line; x-- != 0; cursor += 4) 
				{
					if (0x00 == cursor[3]) 
					{
						cursor[0] = bBk;
						cursor[1] = gBk;
						cursor[2] = rBk;
						cursor[3] = 0xFF;
					}
					else if (0xFF == cursor[3])
					{
						cursor[0] = bFg - (bK*(255 - cursor[0])>>8);
						cursor[1] = gFg - (gK*(255 - cursor[1])>>8);
						cursor[2] = rFg - (rK*(255 - cursor[2])>>8);
					}
					else
					{
						cursor[0] = ((bFg - (bK*(255 - cursor[0])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*bBk + 127)/255;
						cursor[1] = ((gFg - (gK*(255 - cursor[1])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*gBk + 127)/255;
						cursor[2] = ((rFg - (rK*(255 - cursor[2])>>8))*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*rBk + 127)/255;
						cursor[3] = 0xFF;
					}
				}
			}
		}
		else
		{
			for (line = pPixels; cy-- != 0; line += pitch )
			{	
				for (x = cx, cursor = line; x-- != 0; cursor += 4) 
				{
					cursor[0] = bFg - (bK*(255 - cursor[0])>>8);
					cursor[1] = gFg - (gK*(255 - cursor[1])>>8);
					cursor[2] = rFg - (rK*(255 - cursor[2])>>8);
				}
			}
		}
	}
	return TRUE;
}

BOOL Image_BlendOnColorEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb)
{
	LONG pitch;
	WORD r, g, b;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;

	if (bpp != 32) 
		return TRUE;

	if (cy < 0) cy -= cy;

	r = GetRValue(rgb); g = GetGValue(rgb); b = GetBValue(rgb);

	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;

	if (premult)
	{
		for (; cy-- != 0; line += pitch)
		{
			for (x = cx, cursor = line; x-- != 0; cursor += 4) 
			{
				if (0x00 == cursor[3]) 
				{
					cursor[0] = (BYTE)b;
					cursor[1] = (BYTE)g;
					cursor[2] = (BYTE)r;
					cursor[3] = 0xFF;
				}
				else if (cursor[3] != 0xFF)
				{
					WORD a = 255 - cursor[3];
					WORD destB = (cursor[0] * 255 + a * b + 127) / 255;
					WORD destG = (cursor[1] * 255 + a * g + 127) / 255;
					WORD destR = (cursor[2] * 255 + a * r + 127) / 255;

					cursor[0] = (destB > 0xFF) ? 0xFF : destB;
					cursor[1] = (destG > 0xFF) ? 0xFF : destG;
					cursor[2] = (destR > 0xFF) ? 0xFF : destR;
					cursor[3] = 0xFF;
				}
			}
		}
	}
	else
	{
		for (; cy-- != 0; line += pitch)
		{
			for (x = cx, cursor = line; x-- != 0; cursor += 4) 
			{
				if (0x00 == cursor[3]) 
				{
					cursor[0] = (BYTE)b;
					cursor[1] = (BYTE)g;
					cursor[2] = (BYTE)r;
					cursor[3] = 0xFF;
				}
				else if (cursor[3] != 0xFF)
				{
					WORD a = (((255 - cursor[3])*255 + 127)/255);
					cursor[0] = (cursor[0]*cursor[3] + a*b + 127)/255;
					cursor[1] = (cursor[1]*cursor[3] + a*g + 127)/255;
					cursor[2] = (cursor[2]*cursor[3] + a*r + 127)/255;
					cursor[3] = 0xFF;
				}
			}
		}
	}
	return TRUE;
}

BOOL Image_BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return Image_BlendOnColorEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel, premult, rgb);
}

HBITMAP Image_AnimateRotation(HDC hdc, HBITMAP bitmapFrame, INT frameCount, COLORREF rgbBk, BOOL fKeepSize)
{
	if (NULL == bitmapFrame)
		return NULL;

	BITMAP bm;
	if (sizeof(BITMAP) != GetObject(bitmapFrame, sizeof(BITMAP), &bm))
		return NULL;
		
	if (bm.bmHeight < 0) bm.bmHeight = -bm.bmHeight;

	HBITMAP hbmp = NULL;

	HDC hdcDst = CreateCompatibleDC(hdc);
	HDC hdcSrc = CreateCompatibleDC(hdc);
	if (NULL != hdcDst && NULL != hdcSrc)
	{
		INT side;
		if (FALSE == fKeepSize)
			side = (INT)ceil(_hypot(bm.bmWidth, bm.bmHeight));
		else
		{
			side = bm.bmWidth;
			if (bm.bmHeight > side)
				side = bm.bmHeight;
		}
		hbmp = CreateCompatibleBitmap(hdc, side, side * frameCount);
		if (NULL != hbmp)
		{
			LONG  centerX, centerY;
			centerX = side/2;
			centerY = side/2;

			XFORM xForm;
			ZeroMemory(&xForm, sizeof(XFORM));

			HBITMAP hbmpFrameOld = (HBITMAP)SelectObject(hdcSrc, bitmapFrame);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcDst, hbmp);

			RECT rcFill;
			SetRect(&rcFill, 0, 0, side, side * frameCount);
			SetBkColor(hdcDst, rgbBk);
			ExtTextOut(hdcDst, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
			INT graphicsMode = SetGraphicsMode(hdcDst, GM_ADVANCED);
			INT top = (side - bm.bmHeight)/2;
			INT left = (side - bm.bmWidth)/2;

			for (INT i = 0; i < frameCount; i++)
			{
				double fangle = (double)(360/frameCount * i)/180. * 3.1415926;
				xForm.eM11 = (float)cos(fangle);
				xForm.eM12 = (float)sin(fangle);
				xForm.eM21 = (float)-sin(fangle);
				xForm.eM22 = (float)cos(fangle);
				xForm.eDx = (float)(centerX - cos(fangle)*centerX + sin(fangle)*centerY);
				xForm.eDy = (float)(centerY - cos(fangle)*centerY - sin(fangle)*centerX);
				SetWorldTransform(hdcDst, &xForm);
				BitBlt(hdcDst, left, top, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
				centerY += side;
				top += side;

			}

			SetGraphicsMode(hdcDst, graphicsMode);
			ModifyWorldTransform(hdcDst, NULL, MWT_IDENTITY);

			SelectObject(hdcSrc, hbmpFrameOld);
			SelectObject(hdcDst, hbmpOld);
			
		}	
	}
	if (NULL != hdcDst) DeleteDC(hdcDst);
	if (NULL != hdcSrc) DeleteDC(hdcSrc);

	return hbmp;
}

BOOL Image_Premultiply(BYTE *pPixels, LONG cx, LONG cy)
{
	LONG pitch, x;
	pitch = cx* 4;
	LPBYTE cursor, line;
		
	for (line = pPixels; cy-- != 0; line += pitch )
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += 4) 
		{
			if (0x00 == cursor[3]) 
			{
				cursor[0] = 0x00;
				cursor[1] = 0x00;
				cursor[2] = 0x00;
			}
			else if (0xFF != cursor[3])
			{
				cursor[0] = (cursor[0] * cursor[3]) >> 8;
				cursor[1] = (cursor[1] * cursor[3]) >> 8;
				cursor[2] = (cursor[2] * cursor[3]) >> 8;
			}
		}
	}

	return TRUE;
}

BOOL Image_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction)
{
	return AlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, blendFunction);
}