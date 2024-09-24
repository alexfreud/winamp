#include "./graphics.h"
#include "./shlwapi.h"

BOOL Image_ColorOverEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgb)
{
	LONG pitch;
	UINT a, r, g, b, ma, mr, mg, mb;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;

	if (step < 3) 
		return TRUE;

	if (cy < 0) cy -= cy;

	a = (LOBYTE((rgb)>>24)); r = GetRValue(rgb); g = GetGValue(rgb); b = GetBValue(rgb);
	ma = 255 - a; mr = r * 255; mg = g * 255; mb = b * 255;

	if (0 == a)
		return TRUE;

	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;

	if (0xFF == a) 
	{
		for (; cy-- != 0; line += pitch)
		{
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{
				cursor[0] = (BYTE)b;
				cursor[1] = (BYTE)g;
				cursor[2] = (BYTE)r;
			//	cursor[3] = 0xFF;
			}
		}
		return TRUE;
	}

	if (premult)
	{
		for (; cy-- != 0; line += pitch)
		{
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{
				UINT t = (mb + ma * cursor[0] + 127) / 255;
				cursor[0] = (t > 0xFF) ? 0xFF : t;
				t = (mg + ma * cursor[1] + 127) / 255;
				cursor[1] = (t > 0xFF) ? 0xFF : t;
				t = (mr+ ma * cursor[2] + 127) / 255;
				cursor[2] = (t > 0xFF) ? 0xFF : t;
			}
		}
	}
	else
	{
		WORD k = (((255 - a)*255 + 127)/255);
		for (; cy-- != 0; line += pitch)
		{
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{
				cursor[0] = (b*a + k*cursor[0] + 127)/255;
				cursor[1] = (g*a + k*cursor[1] + 127)/255;
				cursor[2] = (r*a + k*cursor[2] + 127)/255;
	//			cursor[3] = (a*a + k*cursor[3] + 127)/255;
			}
		}
	}
	return TRUE;
}


BOOL Image_ColorizeEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, BOOL premult, COLORREF rgbBottom, COLORREF rgbTop)
{
	LONG pitch;
	UINT rBottom, gBottom, bBottom;
	UINT rTop, gTop, bTop;
	INT step = (bpp>>3);
	LPBYTE startLine, line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;
	
	if (cy < 0) 
		cy = -cy;

	rBottom = GetRValue(rgbBottom); 
	gBottom = GetGValue(rgbBottom); 
	bBottom = GetBValue(rgbBottom);

	rTop = GetRValue(rgbTop);
	gTop = GetGValue(rgbTop);
	bTop = GetBValue(rgbTop);

	UINT a, k;
	
	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	startLine = pPixels + pitch * ofs + x*step;
	
	line = startLine;

	for (y = cy; y-- != 0; line += pitch)
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += step) 
		{
			a = (255- cursor[2]);
			if (a < 0) a = 0;
			if (a > 254) 
			{
				cursor[0] = bTop;
				cursor[1] = gTop;
				cursor[2] = rTop;
			}
			else if (a== 0)
			{
				cursor[0] = bBottom;
				cursor[1] = gBottom;
				cursor[2] = rBottom;
			}
			else
			{
				k = (((255 - a)*255 + 127)/255);
				cursor[0] = (bTop * a + k*bBottom + 127)/255;
				cursor[1] = (gTop * a + k*gBottom + 127)/255;
				cursor[2] = (rTop * a + k*rBottom + 127)/255;
			}
		}
	}
	

	if (32 == bpp && FALSE != premult)
	{
		line = startLine;
		for (y = cy; y-- != 0; line += pitch)
		{	
			for (x = cx, cursor = line; x-- != 0; cursor += step) 
			{
				a = cursor[3];
				k = MulDiv(cursor[0], a, 255);
				cursor[0] = (k < 255) ? k : 255;
				k = MulDiv(cursor[1], a, 255);
				cursor[1] = (k < 255) ? k : 255;
				k = MulDiv(cursor[2], a, 255);
				cursor[2] = (k < 255) ? k : 255;
			}
		}
	}


	return TRUE;
}


BOOL Image_PremultiplyEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp)
{
	if (32 != bpp) 
		return FALSE;

	LONG pitch;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;
	
	if (cy < 0) 
		cy = -cy;
	
	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;
	
	UINT a,k;
	for (; cy-- != 0; line += pitch)
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += step) 
		{
			a = cursor[3];
			if (0 == a)
			{
				cursor[0] = 0;
				cursor[1] = 0;
				cursor[2] = 0;
			}
			else if (255 != a)
			{
				k = MulDiv((UINT)cursor[0], a, 255);
				cursor[0] = (k < 255) ? k : 255;
				k = MulDiv((UINT)cursor[1], a, 255);
				cursor[1] = (k < 255) ? k : 255;
				k = MulDiv((UINT)cursor[2], a, 255);
				cursor[2] = (k < 255) ? k : 255;
			}
		}
	}
	
	return TRUE;
}

BOOL Image_SaturateEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, INT n, BOOL fScale)
{
	if (32 != bpp) 
		return FALSE;

	LONG pitch;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;
	
	if (FALSE == fScale)
	{
		if (n < 0) n = 0;
		else if (n > 1000) n = 1000;
	}
	else
	{
		if (n < -1000) n = -1000;
		else if (n > 1000) n = 1000;
	}

	if (cy < 0) 
		cy = -cy;

	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;
	
	COLORREF rgb;
	INT k;
	WORD h, l, s;

	for (; cy-- != 0; line += pitch)
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += step) 
		{
			rgb = RGB(cursor[2], cursor[1], cursor[0]);
			ColorRGBToHLS(rgb, &h, &l, &s);
			if(FALSE == fScale)
				s = ((WORD)((240 * n)/1000));
			else
			{
				k = s;
				s = (WORD)(k + (k * n) /1000);
			}
			
			rgb = ColorHLSToRGB(h, l, s);
							
			cursor[0] = GetBValue(rgb);
			cursor[1] = GetGValue(rgb);
			cursor[2] = GetRValue(rgb);
		}
	}
	
	return TRUE;
}


BOOL Image_AdjustAlphaEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, INT n, BOOL fScale)
{
	if (32 != bpp) 
		return FALSE;

	LONG pitch;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;
	
	if (FALSE == fScale)
	{
		if (n < 0) n = 0;
		else if (n > 1000) n = 1000;
	}
	else
	{
		if (n < -1000) n = -1000;
		else if (n > 1000) n = 1000;
	}

	if (cy < 0) 
		cy = -cy;

	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;

	INT k;

	for (; cy-- != 0; line += pitch)
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += step) 
		{
			if(FALSE == fScale)
				cursor[3] = ((BYTE)((255 * n)/1000));
			else
			{
				k = cursor[3];
				k = k + MulDiv(k, n, 1000);
				if (k > 255) k = 255;
				cursor[3] = (BYTE)k;
			}
		}
	}
	
	return TRUE;
}

BOOL Image_AdjustSaturationAlphaEx(BYTE *pPixels, INT bitmapCX, INT bitmapCY, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, INT nSaturation, INT nAlpha)
{
	if (32 != bpp) 
		return FALSE;

	LONG pitch;
	INT step = (bpp>>3);
	LPBYTE line, cursor;
	pitch = bitmapCX * step;
	while (pitch%4) pitch++;
	
	if (nSaturation < -1000) nSaturation = -1000;
	else if (nSaturation > 1000) nSaturation = 1000;

	if (nAlpha < -1000) nAlpha = -1000;
	else if (nAlpha > 1000) nAlpha = 1000;

	if (cy < 0) 
		cy = -cy;

	INT ofs = (bitmapCY > 0) ? (bitmapCY - (y + cy)) : y;
	line = pPixels + pitch * ofs + x*step;

	INT k;
	COLORREF rgb;
	WORD h, l, s;

	for (; cy-- != 0; line += pitch)
	{	
		for (x = cx, cursor = line; x-- != 0; cursor += step) 
		{
			k = cursor[3];
			k = k + MulDiv(k, nAlpha, 1000);
			if (k > 255) k = 255;
			cursor[3] = (BYTE)k;

			rgb = RGB(cursor[2], cursor[1], cursor[0]);
			ColorRGBToHLS(rgb, &h, &l, &s);
			
			k = s;
			k = k + MulDiv(k, nSaturation, 1000);
			if (k > 240) k = 240;
			s = (WORD)k;
			
			rgb = ColorHLSToRGB(h, l, s);
			cursor[0] = GetBValue(rgb);
			cursor[1] = GetGValue(rgb);
			cursor[2] = GetRValue(rgb);
		}
	}
	
	return TRUE;
}
BOOL Image_ColorOver(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return Image_ColorOverEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel, premult, rgb);
}

BOOL Image_Premultiply(HBITMAP hbmp, RECT *prcPart)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return Image_PremultiplyEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel);
}

BOOL Image_Saturate(HBITMAP hbmp, RECT *prcPart, INT n, BOOL fScale)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return Image_SaturateEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel, n, fScale);
}

BOOL Image_AdjustAlpha(HBITMAP hbmp, RECT *prcPart, INT n, BOOL fScale)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return Image_AdjustAlphaEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel, n, fScale);
}

BOOL Image_AdjustSaturationAlpha(HBITMAP hbmp, RECT *prcPart, INT nSaturation, INT nAlpha)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return Image_AdjustSaturationAlphaEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
					prcPart->left, prcPart->top,
					prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
					dibsec.dsBm.bmBitsPixel, nSaturation, nAlpha);
}

COLORREF Color_Blend(COLORREF rgbTop, COLORREF rgbBottom, INT alpha)
{
	if (alpha > 254) return rgbTop;
	if (alpha < 0) return rgbBottom;

	WORD k = (((255 - alpha)*255 + 127)/255);
	
	return RGB( (GetRValue(rgbTop)*alpha + k*GetRValue(rgbBottom) + 127)/255, 
				(GetGValue(rgbTop)*alpha + k*GetGValue(rgbBottom) + 127)/255, 
				(GetBValue(rgbTop)*alpha + k*GetBValue(rgbBottom) + 127)/255);
}