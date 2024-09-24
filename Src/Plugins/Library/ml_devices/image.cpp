#include "main.h"
#include "./image.h"


HBITMAP
Image_Load(const wchar_t *path, unsigned int type, 
		   unsigned int flags, int width, int height)
{
	MLIMAGESOURCE source;
	HWND libraryWindow = Plugin_GetLibraryWindow();
	if (NULL == libraryWindow)
		return NULL;

	source.cbSize = sizeof(source);
	source.lpszName = path;
	source.type = type;
	source.flags = (flags & ~IMAGE_FILTER_MASK);
	source.cxDst = width;
	source.cyDst = height;

	if (0 == (ISF_LOADFROMFILE & source.flags))
	{
		source.hInst = WASABI_API_LNG_HINST;
		if (NULL != source.hInst)
		{
			HBITMAP bitmap = MLImageLoader_LoadDib(libraryWindow, &source);
			if (NULL != bitmap)
				return bitmap;
		}
		
		if (WASABI_API_ORIG_HINST == source.hInst)
			return NULL;

		source.hInst = WASABI_API_ORIG_HINST;
		return (NULL != source.hInst) ? 
					MLImageLoader_LoadDib(libraryWindow, &source) :
					NULL;
	}

	return MLImageLoader_LoadDib(Plugin_GetLibraryWindow(), &source);
}

HBITMAP
Image_LoadEx(HINSTANCE instance, const wchar_t *path, unsigned int type, 
		   unsigned int flags, int width, int height)
{
	MLIMAGESOURCE source = {0};

	source.cbSize = sizeof(source);
	source.hInst = instance;
	source.lpszName = path;
	source.type = type;
	source.flags = (flags & ~IMAGE_FILTER_MASK);
	source.cxDst = width;
	source.cyDst = height;

	return MLImageLoader_LoadDib(Plugin_GetLibraryWindow(), &source);
}

BOOL
Image_FilterEx(void *pixelData, long width, long height, unsigned short bpp, 
			 unsigned int flags, COLORREF backColor, COLORREF frontColor, COLORREF blendColor)
{
	MLIMAGEFILTERAPPLYEX filter;
	HWND libraryWindow;
	BOOL result;

	if (NULL == pixelData)
		return FALSE;

	libraryWindow = Plugin_GetLibraryWindow();
	if (NULL == libraryWindow)
		return FALSE;

	filter.cbSize = sizeof(filter);
	filter.pData = (BYTE*)pixelData;
	filter.cx = width;
	filter.cy = height;
	filter.bpp = bpp;
	filter.imageTag = NULL;

	result = FALSE;

	if (0 != (IMAGE_FILTER_GRAYSCALE & flags))
	{
		filter.filterUID = MLIF_GRAYSCALE_UID;
		result = MLImageFilter_ApplyEx(libraryWindow, &filter);
	}

	filter.rgbBk = backColor;
	filter.rgbFg = frontColor;

	if (32 == bpp)
	{
		filter.filterUID = MLIF_FILTER1_PRESERVE_ALPHA_UID;
		result = MLImageFilter_ApplyEx(libraryWindow, &filter);

		if (0 != (IMAGE_FILTER_BLEND & flags))
		{
			filter.filterUID = MLIF_BLENDONBK_UID;
			filter.rgbBk = blendColor;
			result = MLImageFilter_ApplyEx(libraryWindow, &filter);
		}
	}
	else
	{
		filter.filterUID = MLIF_FILTER1_UID;
		result = MLImageFilter_ApplyEx(libraryWindow, &filter);
	}

	return result;
}

BOOL
Image_Filter(HBITMAP bitmap, unsigned int flags, 
			 COLORREF backColor, COLORREF frontColor, COLORREF blendColor)
{
	DIBSECTION bitmapData;
	BITMAP *bi;

	if (NULL == bitmap)
		return NULL;

	if (sizeof(bitmapData) != GetObjectW(bitmap, sizeof(bitmapData), &bitmapData))
		return FALSE;

	bi = &bitmapData.dsBm;

	return Image_FilterEx(bi->bmBits, bi->bmWidth, bi->bmHeight, bi->bmBitsPixel, 
							flags, backColor, frontColor, blendColor);
}

BOOL
Image_BlendEx(void *pixelData, long width, long height, unsigned short bpp, COLORREF blendColor)
{
	MLIMAGEFILTERAPPLYEX filter;
	HWND libraryWindow;

	if (NULL == pixelData || 32 != bpp)
		return FALSE;

	libraryWindow = Plugin_GetLibraryWindow();
	if (NULL == libraryWindow)
		return FALSE;

	filter.cbSize = sizeof(filter);
	filter.pData = (BYTE*)pixelData;
	filter.cx = width;
	filter.cy = height;
	filter.bpp = bpp;
	filter.imageTag = NULL;

	
	filter.filterUID = MLIF_BLENDONBK_UID;
	filter.rgbBk = blendColor;
	return MLImageFilter_ApplyEx(libraryWindow, &filter);
}

BOOL
Image_Blend(HBITMAP bitmap, COLORREF blendColor)
{
	DIBSECTION bitmapData;
	BITMAP *bi;

	if (NULL == bitmap)
		return NULL;

	if (sizeof(bitmapData) != GetObjectW(bitmap, sizeof(bitmapData), &bitmapData))
		return FALSE;

	bi = &bitmapData.dsBm;

	return Image_BlendEx(bi->bmBits, bi->bmWidth, bi->bmHeight, bi->bmBitsPixel, blendColor);
}

HBITMAP 
Image_LoadSkinnedEx(HINSTANCE instance, const wchar_t *path, unsigned int type,
								  unsigned int flags, int width, int height,
								  COLORREF backColor, COLORREF frontColor, COLORREF blendColor)
{
	HBITMAP bitmap;

	bitmap = Image_LoadEx(instance, path, type, flags, width, height);
	if (NULL == bitmap)
		return NULL;

	Image_Filter(bitmap, flags, backColor, frontColor, blendColor);
		
	return bitmap;
}

HBITMAP 
Image_LoadSkinned(const wchar_t *path, unsigned int type,
				  unsigned int flags, int width, int height,
				  COLORREF backColor, COLORREF frontColor, COLORREF blendColor)
{
	HBITMAP bitmap;

	bitmap = Image_Load(path, type, flags, width, height);
	if (NULL == bitmap)
		return NULL;

	Image_Filter(bitmap, flags, backColor, frontColor, blendColor);
		
	return bitmap;
}

HBITMAP
Image_DuplicateDib(HBITMAP source)
{
	HBITMAP bitmap;
	DIBSECTION sourceDib;
	HDC windowDC;
	void *pixelData;

	if (NULL == source)
		return NULL;
			
	if (sizeof(sourceDib) != GetObjectW(source, sizeof(sourceDib), &sourceDib))
		return FALSE;

	
    sourceDib.dsBmih.biSize = sizeof(BITMAPINFOHEADER);
	if (sourceDib.dsBmih.biHeight > 0)
		sourceDib.dsBmih.biHeight = -sourceDib.dsBmih.biHeight;
    
	windowDC = GetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE);
 
	bitmap = CreateDIBSection(windowDC, (BITMAPINFO*)&sourceDib.dsBmih, DIB_RGB_COLORS, &pixelData, NULL, 0);
	
	if (NULL != windowDC)
		ReleaseDC(NULL, windowDC);

	if (NULL == bitmap)
		return NULL;

	CopyMemory(pixelData, sourceDib.dsBm.bmBits, sourceDib.dsBm.bmWidthBytes * sourceDib.dsBm.bmHeight);

	return bitmap;

}
BOOL 
Image_ColorOver(HBITMAP bitmap, const RECT *prcPart, BOOL premult, COLORREF rgb)
{
	DIBSECTION bitmapData;
	BITMAP *bi;

	if (NULL == bitmap)
		return NULL;

	if (sizeof(bitmapData) != GetObjectW(bitmap, sizeof(bitmapData), &bitmapData))
		return FALSE;

	if (BI_RGB != bitmapData.dsBmih.biCompression ||
		1 != bitmapData.dsBmih.biPlanes || 
		32 != bitmapData.dsBm.bmBitsPixel)
	{
		return FALSE;
	}

	bi = &bitmapData.dsBm;

	return (NULL == prcPart) ?
			Image_ColorOverEx((BYTE*)bi->bmBits, bi->bmWidth, bi->bmHeight,
								0, 0, bi->bmWidth, bi->bmHeight,
								bi->bmBitsPixel, premult, rgb) :
			Image_ColorOverEx((BYTE*)bi->bmBits, bi->bmWidth, bi->bmHeight,
								prcPart->left, prcPart->top,
								prcPart->right - prcPart->left, prcPart->bottom - prcPart->top,
								bi->bmBitsPixel, premult, rgb);

}

BOOL 
Image_ColorOverEx(unsigned char *pPixels, int bitmapCX, int bitmapCY, 
				  long x, long y, long cx, long cy, unsigned short bpp, 
				  BOOL premult, COLORREF rgb)
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
				int t = (mb + ma * cursor[0] + 127) / 255;
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

BOOL 
Image_Premultiply(HBITMAP hbmp, const RECT *prcPart)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return (NULL == prcPart) ?
			Image_PremultiplyEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
								0, 0, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight, 
								dibsec.dsBm.bmBitsPixel) :
			Image_PremultiplyEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
								prcPart->left, prcPart->top, RECTWIDTH(*prcPart), RECTHEIGHT(*prcPart),
								dibsec.dsBm.bmBitsPixel);
}

BOOL 
Image_PremultiplyEx(unsigned char *pPixels, int bitmapCX, int bitmapCY, 
					long x, long y, long cx, long cy, unsigned short bpp)
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
	
	UINT a;
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
				cursor[0] = (BYTE)MulDiv(cursor[0], a, 255);
				cursor[1] = (BYTE)MulDiv(cursor[1], a, 255);
				cursor[2] = (BYTE)MulDiv(cursor[2], a, 255);
			}
		}
	}
	
	return TRUE;
}

BOOL 
Image_Demultiply(HBITMAP hbmp, const RECT *prcPart)
{
	DIBSECTION dibsec;
	if (!hbmp || sizeof(DIBSECTION) != GetObject(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || dibsec.dsBm.bmBitsPixel != 32) 
		return FALSE;

	return (NULL == prcPart) ?
			Image_DemultiplyEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
								0, 0, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight, 
								dibsec.dsBm.bmBitsPixel) :
			Image_DemultiplyEx((BYTE*)dibsec.dsBm.bmBits, dibsec.dsBm.bmWidth, dibsec.dsBm.bmHeight,
								prcPart->left, prcPart->top, RECTWIDTH(*prcPart), RECTHEIGHT(*prcPart),
								dibsec.dsBm.bmBitsPixel);
}

BOOL 
Image_DemultiplyEx(unsigned char *pPixels, int bitmapCX, int bitmapCY, 
					long x, long y, long cx, long cy, unsigned short bpp)
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
	
	UINT a;
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
				cursor[0] = (BYTE)MulDiv(cursor[0], 255, a);
				cursor[1] = (BYTE)MulDiv(cursor[1], 255, a);
				cursor[2] = (BYTE)MulDiv(cursor[2], 255, a);
			}
		}
	}
	
	return TRUE;
}
BOOL 
Image_Saturate(HBITMAP hbmp, const RECT *prcPart, int n, BOOL fScale)
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

BOOL 
Image_SaturateEx(unsigned char *pPixels, int bitmapCX, int bitmapCY, 
				 long x, long y, long cx, long cy, unsigned short bpp, 
				 int n, BOOL fScale)
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

BOOL 
Image_AdjustAlpha(HBITMAP hbmp, const RECT *prcPart, int n, BOOL fScale)
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

BOOL 
Image_AdjustAlphaEx(unsigned char *pPixels, int bitmapCX, int bitmapCY, 
					long x, long y, long cx, long cy, unsigned short bpp, 
					int n, BOOL fScale)
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

BOOL 
Image_AdjustSaturationAlpha(HBITMAP hbmp, const RECT *prcPart, int nSaturation, int nAlpha)
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

BOOL 
Image_AdjustSaturationAlphaEx(unsigned char *pPixels, int bitmapCX, int bitmapCY, 
							  long x, long y, long cx, long cy, 
							  unsigned short bpp, int nSaturation, int nAlpha)
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

BOOL
Image_FillBorder(HDC targetDC, const RECT *targetRect, 
				 HDC sourceDC, const RECT *sourceRect, 
				 BOOL fillCenter, BYTE alphaConstant)
{
	INT prevStretchMode;
	long centerWidth, centerHeight;
	long sliceWidth, sliceHeight;

	const BLENDFUNCTION blendFunction = { AC_SRC_OVER, 0, alphaConstant, AC_SRC_ALPHA };
	const long targetWidth = RECTWIDTH(*targetRect);
	const long targetHeight = RECTHEIGHT(*targetRect);
	const long sourceWidth = RECTWIDTH(*sourceRect);
	const long sourceHeight = RECTHEIGHT(*sourceRect);
	
	if (NULL == targetDC || NULL == sourceDC)
		return FALSE;
	
	sliceWidth = sourceWidth/2;
	sliceHeight = sourceHeight/2;

	if (sliceWidth*2 > targetWidth)
		sliceWidth = targetWidth/2;

	if (sliceHeight*2 > targetHeight)
		sliceHeight = targetHeight/2;

	if (0 == sliceWidth || 0 == sliceHeight)
		return FALSE;
	
	prevStretchMode = SetStretchBltMode(targetDC, COLORONCOLOR);
	SetViewportOrgEx(sourceDC, 0, 0, NULL);
	
	GdiAlphaBlend(targetDC, targetRect->left, targetRect->top, sliceWidth, sliceHeight, 
				  sourceDC, sourceRect->left, sourceRect->top, sliceWidth, sliceHeight, blendFunction);
	GdiAlphaBlend(targetDC, targetRect->right - sliceWidth, targetRect->top, sliceWidth, sliceHeight, 
		   sourceDC, sourceRect->right - sliceWidth, sourceRect->top, sliceWidth, sliceHeight, blendFunction);
	GdiAlphaBlend(targetDC, targetRect->left, targetRect->bottom - sliceHeight, sliceWidth, sliceHeight, 
		   sourceDC, sourceRect->left, sourceRect->bottom - sliceHeight, sliceWidth, sliceHeight, blendFunction);
	GdiAlphaBlend(targetDC, targetRect->right - sliceWidth, targetRect->bottom - sliceHeight, sliceWidth, sliceHeight,
		   sourceDC, sourceRect->right - sliceWidth, sourceRect->bottom - sliceHeight, sliceWidth, sliceHeight, blendFunction);

	if (targetWidth > 2*sliceWidth)
	{
		centerWidth = sourceWidth - 2*sliceWidth;
		if(centerWidth < 1)
			centerWidth = 1;

		GdiAlphaBlend(targetDC, targetRect->left + sliceWidth, targetRect->top, targetWidth - (sliceWidth * 2), sliceHeight, 
				   sourceDC, sourceRect->left + sliceWidth, sourceRect->top, centerWidth, sliceHeight, blendFunction); 

		GdiAlphaBlend(targetDC, targetRect->left + sliceWidth, targetRect->bottom - sliceHeight, targetWidth - (sliceWidth * 2), sliceHeight,
				   sourceDC, sourceRect->left + sliceWidth, sourceRect->bottom - sliceHeight, centerWidth, sliceHeight, blendFunction); 
	}
	else
		centerWidth = 0;

	if (targetHeight > 2*sliceHeight)
	{
		centerHeight = sourceHeight - 2*sliceHeight;
		if(centerHeight < 1)
			centerHeight = 1;

		GdiAlphaBlend(targetDC, targetRect->left, targetRect->top + sliceHeight, sliceWidth, targetHeight - (sliceHeight* 2), 
				   sourceDC, sourceRect->left, sourceRect->top + sliceHeight, sliceWidth, centerHeight, blendFunction); 

		GdiAlphaBlend(targetDC, targetRect->right - sliceWidth, targetRect->top + sliceHeight, sliceWidth, targetHeight - (sliceHeight* 2), 
				   sourceDC, sourceRect->right - sliceWidth, sourceRect->top + sliceHeight, sliceWidth, centerHeight, blendFunction); 
	}
	else 
		centerHeight = 0;

	if (FALSE != fillCenter && 
		0 != centerWidth && 0 != centerHeight)
	{
		GdiAlphaBlend(targetDC, targetRect->left + sliceWidth, targetRect->top + sliceHeight, targetWidth - (sliceWidth * 2), targetHeight - (sliceHeight* 2), 
				   sourceDC, sourceRect->left + sliceWidth, sourceRect->top + sliceHeight, centerWidth, centerHeight, blendFunction); 

	}

	SetStretchBltMode(targetDC, prevStretchMode);
	return TRUE;
}

const ImageInfo *
Image_GetBestFit(const ImageInfo *images, size_t count, unsigned int width, unsigned int height)
{
	const ImageInfo *image, *bestFit;
	double widthDbl, heightDbl;
	double scaleMin, scaleHorz, scaleVert;

	if (NULL == images || count < 1)
		return NULL;

	if (width < 1)
		width = 1;

	if (height < 1)
		height = 1;

	widthDbl = width;
	heightDbl = height;

	image = &images[--count];
	scaleHorz = widthDbl/image->width;
	scaleVert = heightDbl/image->height;
	scaleMin = (scaleHorz < scaleVert) ? scaleHorz : scaleVert;
	bestFit = image;

	if (1.0 != scaleMin)
	{
		scaleMin = fabs(1.0 - scaleMin);
		while(count--)
		{
			image = &images[count];
			scaleHorz = widthDbl/image->width;
			scaleVert = heightDbl/image->height;
			if (scaleHorz > scaleVert) 
				scaleHorz = scaleVert;
			
			if (1.0 == scaleHorz)
			{
				bestFit = image;
				break;
			}
			
			scaleHorz = fabs(1.0 - scaleHorz);
			if (scaleHorz < scaleMin)
			{
				scaleMin = scaleHorz;
				bestFit = image;
			}
		}
	}
	
	return bestFit;
}

BOOL
Image_AlphaBlend(HDC targetDC, const RECT *targetRect, 
						   HDC sourceDC, const RECT *sourceRect, BYTE sourceAlpha, 
						   HBITMAP sourceBitmap, const RECT *paintRect, AlphaBlendFlags flags, 
						   RECT *rectOut)
{
	BOOL result, clipSource;
	RECT fillRect;
	int sourceX, sourceY, sourceWidth, sourceHeight;
	const BLENDFUNCTION blendFunction = 
	{ 
		AC_SRC_OVER, 
		0, 
		sourceAlpha, 
		AC_SRC_ALPHA 
	};
		
	if (NULL != paintRect)
	{
		if (FALSE == IntersectRect(&fillRect, targetRect, paintRect))
			return TRUE;

		clipSource = TRUE;
	}
	else
	{
		CopyRect(&fillRect, targetRect);
		clipSource = FALSE;
	}

	if (NULL != sourceRect)
	{
		sourceX = sourceRect->left;
		sourceY = sourceRect->top;
		sourceWidth = RECTWIDTH(*sourceRect);
		sourceHeight = RECTHEIGHT(*sourceRect);
	}
	else
	{
		BITMAP bitmapInfo;
		if (sizeof(bitmapInfo) != GetObject(sourceBitmap, sizeof(bitmapInfo), &bitmapInfo))
			return FALSE;

		sourceX = 0;
		sourceY = 0;
		sourceWidth = bitmapInfo.bmWidth;
		sourceHeight = bitmapInfo.bmHeight;
		if (sourceHeight < 0)
			sourceHeight = -sourceHeight;
	}
	
	if (0 != (AlphaBlend_ScaleSource & flags))
	{
		RECT rect;
		double scaleHorz, scaleVert;

		scaleHorz = (double)RECTWIDTH(*targetRect) / sourceWidth;
		scaleVert = (double)RECTHEIGHT(*targetRect) / sourceHeight;
		if (scaleHorz > scaleVert)
			scaleHorz = scaleVert;

		SetRect(&rect, 0, 0, (int)(sourceWidth * scaleHorz), (int)(sourceHeight * scaleHorz));
		
		if (0 != (AlphaBlend_AlignLeft & flags))
			rect.left = targetRect->left;
		else if (0 != (AlphaBlend_AlignRight & flags))
			rect.left = targetRect->right - rect.right;
		else 
			rect.left = targetRect->left + (RECTWIDTH(*targetRect) - rect.right)/2;

		if (0 != (AlphaBlend_AlignTop & flags))
			rect.top = targetRect->top;
		else if (0 != (AlphaBlend_AlignBottom & flags))
			rect.top = targetRect->bottom - rect.bottom;
		else 
			rect.top = targetRect->top + (RECTHEIGHT(*targetRect) - rect.bottom)/2;

		rect.right += rect.left;
		rect.bottom += rect.top;

		if (NULL != rectOut)
			CopyRect(rectOut, &rect);

		if (NULL != paintRect)
		{
			if (FALSE == IntersectRect(&fillRect, &rect, paintRect))
				return TRUE;
		}
		else
			CopyRect(&fillRect, &rect);

		sourceX += (int)((fillRect.left - rect.left)/scaleHorz);
		sourceY += (int)((fillRect.top - rect.top)/ scaleHorz);
		sourceWidth -= (int)((RECTWIDTH(rect) - RECTWIDTH(fillRect))/scaleHorz);
		sourceHeight -= (int)((RECTHEIGHT(rect) - RECTHEIGHT(fillRect))/scaleHorz);
		clipSource = FALSE;
	}
	else
	{
		if (sourceWidth < RECTWIDTH(*targetRect) ||
			sourceHeight < RECTHEIGHT(*targetRect))
		{
			RECT rect;

			if (0 != (AlphaBlend_AlignLeft & flags))
				rect.left = targetRect->left;
			else if (0 != (AlphaBlend_AlignRight & flags))
				rect.left = targetRect->right - sourceWidth;
			else 
				rect.left = targetRect->left + (RECTWIDTH(*targetRect) - sourceWidth)/2;

			if (0 != (AlphaBlend_AlignTop & flags))
				rect.top = targetRect->top;
			else if (0 != (AlphaBlend_AlignBottom & flags))
				rect.top = targetRect->bottom - sourceHeight;
			else 
				rect.top = targetRect->top + (RECTHEIGHT(*targetRect) - sourceHeight)/2;

			rect.right = rect.left + sourceWidth;
			rect.bottom = rect.top + sourceHeight;

			if (NULL != paintRect)
			{
				if (FALSE == IntersectRect(&fillRect, &rect, paintRect))
					return TRUE;

				sourceX += (fillRect.left - rect.left);
				sourceY += (fillRect.top - rect.top);
				sourceWidth -= (RECTWIDTH(rect) - RECTWIDTH(fillRect));
				sourceHeight -= (RECTHEIGHT(rect) - RECTHEIGHT(fillRect));
			}
			else
				CopyRect(&fillRect, &rect);

			if (NULL != rectOut)
				CopyRect(rectOut, &rect);

			clipSource = FALSE;
		}
		else if (NULL != rectOut)
			CopyRect(rectOut, targetRect);
	}

	
	
	if (FALSE != clipSource)
	{
		sourceX += (fillRect.left - targetRect->left);
		sourceY += (fillRect.top - targetRect->top);
		sourceWidth -= (RECTWIDTH(*targetRect) - RECTWIDTH(fillRect));
		sourceHeight -= (RECTHEIGHT(*targetRect) - RECTHEIGHT(fillRect));
	}

	if (NULL != sourceBitmap)
		SelectBitmap(sourceDC, sourceBitmap);

	result = GdiAlphaBlend(targetDC, fillRect.left, fillRect.top, RECTWIDTH(fillRect), RECTHEIGHT(fillRect),
						   sourceDC, sourceX, sourceY, sourceWidth, sourceHeight, blendFunction);

	return result;

}