#include "main.h"
#include "./loadimage.h"
#include "./api.h"
#include <api/service/waServiceFactory.h>
#include <api/service/svcs/svc_imgload.h>

#define ABS(x) (((x) > 0) ? (x) : (-x))

static HANDLE ReadFromFile(LPCWSTR pszImage, DWORD *size, HANDLE *hres)
{
	HANDLE hFile = CreateFileW(pszImage, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL), hmem(NULL);
	*size = 0;
	if (INVALID_HANDLE_VALUE != hFile)
	{
		*size = GetFileSize(hFile, NULL);
		if (INVALID_FILE_SIZE != *size)
		{
			hmem = HeapAlloc(GetProcessHeap(), 0, *size);
			if (hmem)
			{
				DWORD readed = 0;
				if (!ReadFile(hFile, hmem, *size, &readed, NULL) || *size != readed)
				{
					HeapFree(GetProcessHeap(), 0, hmem);
					hmem = NULL;
				}
			}
		}
		CloseHandle(hFile);
	} 
	
	*hres = hmem;
	return hmem;
}

static HANDLE ReadFromRes(HMODULE hMod, LPCWSTR pszSection, LPCWSTR pszImage, DWORD *size, HANDLE *hres)
{
	*size = 0;
	HRSRC res = FindResourceW(hMod, pszImage, pszSection);
	if (res)
	{
		*hres = LoadResource(hMod, res);
		*size = SizeofResource(hMod, res);
		return LockResource(*hres);
	}
	return NULL;
}

static HANDLE LoadImg(HANDLE data, DWORD size, INT *cx, INT *cy, BOOL premult)
{
	FOURCC imgload = svc_imageLoader::getServiceType();
	int n = (int) WASABI_API_SVC->service_getNumServices(imgload);
	for (int i=0; i<n; i++)
	{
		waServiceFactory *sf = WASABI_API_SVC->service_enumService(imgload,i);
		if (sf)
		{
			svc_imageLoader * l = (svc_imageLoader*)sf->getInterface();
			if (l)
			{
				if (l->testData(data,size))
				{
					HANDLE ret;
					ret = (premult) ? l->loadImage(data, size, cx, cy) : l->loadImageData(data, size, cx, cy);
					sf->releaseInterface(l);
					return ret;
				}
				sf->releaseInterface(l);
			}
		}
	}
	return NULL;
}

HBITMAP WALoadImage(HMODULE hMod, LPCWSTR pszSection, LPCWSTR pszImage, BOOL bPremult)
{
	BITMAPINFOHEADER header = {0};
	HBITMAP hbmp = NULL;
	HANDLE hres = NULL;
	LPVOID dib = NULL;
	DWORD size = 0;

	header.biBitCount = 32;
	header.biPlanes = 1;
	header.biSize = sizeof(BITMAPINFOHEADER);
	
	LPVOID data = (!hMod) ? ReadFromFile(pszImage, &size, &hres) : ReadFromRes(hMod, pszSection, pszImage, &size, &hres);

	if (data) data = LoadImg(data, size, (int*)&header.biWidth, (int*)&header.biHeight, bPremult);
	
	if (hres)
	{
		(hMod) ? FreeResource(hres) : HeapFree(GetProcessHeap(), 0, hres);
	}
	
	if (data)
	{
		header.biHeight = 0 - header.biHeight;
		hbmp = CreateDIBSection(NULL, (LPBITMAPINFO)&header, DIB_RGB_COLORS, &dib, NULL, 0);
		if (hbmp) CopyMemory(dib, data, header.biWidth * ABS(header.biHeight) * sizeof(DWORD));
		WASABI_API_MEMMGR->sysFree(data);
	}
	else hbmp = NULL;
	return hbmp;
}

HBITMAP WAResizeImage(HBITMAP hbmp, INT cx, INT cy, HBRUSH hbBk)
{
	BITMAP bi = {0};

	if (!hbmp || !GetObject(hbmp, sizeof(BITMAP), &bi)) return hbmp;

	if (bi.bmWidth != cx || bi.bmHeight != cy)
	{
		INT ix = cx, iy = cy;

		HDC hdc = GetDC(NULL),
			hdcSrc = CreateCompatibleDC(hdc),
			hdcDst = CreateCompatibleDC(hdc);
		HBITMAP hbmpOld1 = (HBITMAP)SelectObject(hdcSrc, hbmp);
		hbmp = CreateCompatibleBitmap(hdc, cx, cy);
		HBITMAP hbmpOld2 = (HBITMAP)SelectObject(hdcDst, hbmp);
		if (ix != cx || iy != cy)
		{
			RECT r;
			SetRect(&r, 0, 0, cx, cy);
			FillRect(hdcDst, &r, hbBk);
		}
		SetStretchBltMode(hdcDst, HALFTONE);
		StretchBlt(hdcDst, (cx - ix)/2, (cy - iy)/2, ix, iy, hdcSrc, 0, 0, bi.bmWidth, bi.bmHeight, SRCCOPY);

		SelectObject(hdcDst, hbmpOld2);
		hbmpOld2 = (HBITMAP)SelectObject(hdcSrc, hbmpOld1);
		if (hbmpOld2) DeleteObject(hbmpOld2);

		DeleteDC(hdcSrc);
		DeleteDC(hdcDst);
		ReleaseDC(NULL, hdc);
	}
	return hbmp;
}

HBITMAP WABlendOnColor(HBITMAP hbmp, COLORREF rgbBk)
{
	DIBSECTION dibsec = {0};
	LONG pitch, x;
	INT cx, cy;
	LPBYTE cursor = NULL, line = NULL, pData = NULL;

	if (!hbmp || sizeof(DIBSECTION) != GetObjectW(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || 32 != dibsec.dsBm.bmBitsPixel) return hbmp;

	cx = dibsec.dsBm.bmWidth;
	cy = dibsec.dsBm.bmHeight;
	pitch = dibsec.dsBm.bmWidth*4;
	pData = (LPBYTE)dibsec.dsBm.bmBits;

	for (line = pData; cy-- != 0; line += pitch )
	{
		for (x = cx, cursor = line; x-- != 0; cursor += 4) 
		{
			if (0x00 == cursor[3]) 
			{
				cursor[0] = GetBValue(rgbBk);
				cursor[1] = GetGValue(rgbBk);
				cursor[2] = GetRValue(rgbBk);
				cursor[3] = 0xFF;
			}
			else if (cursor[3] != 0xFF)
			{
				cursor[0] = (cursor[0]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*GetBValue(rgbBk) + 127)/255;
				cursor[1] = (cursor[1]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*GetGValue(rgbBk) + 127)/255;
				cursor[2] = (cursor[2]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*GetRValue(rgbBk) + 127)/255;
				cursor[3] = 0xFF;
			}
		}
	}
	return hbmp;
}

HBITMAP WACreatePatternBitmap(INT cx, INT cy, INT bpp, HBRUSH hbPattern, LPBYTE *ppData)
{
	BITMAPINFOHEADER bi = {0};
	RECT r;
	HDC hdcTmp = CreateCompatibleDC(NULL);

	bi.biSize = sizeof (bi);
	bi.biWidth= cx;
	bi.biHeight = -cy;
	bi.biPlanes = 1;
	bi.biBitCount= (WORD)bpp;

	HBITMAP hbmpPattern = CreateDIBSection(hdcTmp, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (LPVOID*)ppData, NULL, NULL);
	if (!hbmpPattern) return NULL;

	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcTmp, hbmpPattern);
	SetRect(&r, 0, 0, cx, cy);
	FillRect(hdcTmp, &r, hbPattern);
	SelectObject(hdcTmp, hbmpOld);
	DeleteDC(hdcTmp);
	return hbmpPattern;
}

HBITMAP WABlendOnARGB32(HBITMAP hbmp, LPBYTE pRGB)
{
	DIBSECTION dibsec = {0};
	LONG pitch, x;
	INT cx, cy;
	LPBYTE cursor = NULL, cursor2 = NULL, line = NULL, line2 = NULL, pData = NULL;

	if (!hbmp || sizeof(DIBSECTION) != GetObjectW(hbmp, sizeof(DIBSECTION), &dibsec) ||
		BI_RGB != dibsec.dsBmih.biCompression || 1 != dibsec.dsBmih.biPlanes || 32 != dibsec.dsBm.bmBitsPixel) return hbmp;

	cx = dibsec.dsBm.bmWidth;
	cy = dibsec.dsBm.bmHeight;
	pitch = dibsec.dsBm.bmWidth*4;
	pData = (LPBYTE)dibsec.dsBm.bmBits;

	for (line = pData, line2 = pRGB; cy-- != 0; line += pitch, line2 += pitch )
	{
		for (x = cx, cursor = line, cursor2 = line2; x-- != 0; cursor += 4, cursor2 += 4) 
		{
			if (0x00 == cursor[3])  *((DWORD*)cursor) = *((DWORD*)cursor2);
			else if (cursor[3] != 0xFF)
			{
				cursor[0] = (cursor[0]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*cursor2[0] + 127)/255;
				cursor[1] = (cursor[1]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*cursor2[1] + 127)/255;
				cursor[2] = (cursor[2]*cursor[3] + (((255 - cursor[3])*255 + 127)/255)*cursor2[2] + 127)/255;
			}
			cursor[3] = 0xFF;
		}
	}

	return hbmp;
}