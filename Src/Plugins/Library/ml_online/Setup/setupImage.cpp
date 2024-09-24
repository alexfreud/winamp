#include "./setupImage.h"
#include "../api__ml_online.h"

#include <shlwapi.h>

static BOOL SetupImage_CopyImage(HDC hdc, HBITMAP bitmapDst, INT x, INT y, INT cx, INT cy, HBITMAP bitmapSrc, INT srcX, INT srcY)
{
	BOOL resultOk = FALSE;
	HDC hdcDst = CreateCompatibleDC(hdc);
	HDC hdcSrc = CreateCompatibleDC(hdc);
	
	if (NULL != hdcDst && NULL != hdcSrc)
	{
		HBITMAP bitmapDstOrig = (HBITMAP)SelectObject(hdcDst, bitmapDst);
		HBITMAP bitmapSrcOrig = (HBITMAP)SelectObject(hdcSrc, bitmapSrc);
		
		resultOk = BitBlt(hdcDst, x, y, cx, cy, hdcSrc, srcX, srcY, SRCCOPY);
		
		SelectObject(hdcDst, bitmapDstOrig);
		SelectObject(hdcSrc, bitmapSrcOrig);
	}

	if (NULL != hdcDst) DeleteDC(hdcDst);
	if (NULL != hdcSrc) DeleteDC(hdcSrc);
	
	return  resultOk;
}

static BOOL SetupImage_ColorizeImage(BYTE *pPixels, LONG x, LONG y, LONG cx, LONG cy, WORD bpp, LONG dstX, LONG dstY, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha)
{
	LONG pitch;
	INT step;
	BYTE rFg, gFg, bFg;
	LPBYTE srcCursor, srcLine;
	LPBYTE dstLine;

	if (bpp < 24) return FALSE;

	step = (bpp>>3);
	pitch = cx*step;
	while (pitch%4) pitch++;
	
	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	
	INT  bK = (bFg - GetBValue(rgbBk));
	INT gK = (gFg - GetGValue(rgbBk));
	INT rK = (rFg - GetRValue(rgbBk));
	
	srcLine = pPixels + pitch * y + x*step;
	dstLine = pPixels + pitch * dstY + dstX*step;

	if (24 == bpp)
	{
		for (; cy-- != 0; srcLine += pitch, dstLine += pitch )
		{
			LONG i;
			LPBYTE dstCursor;
			for (i = cx, srcCursor = srcLine, dstCursor = dstLine ; i-- != 0; srcCursor += 3, dstCursor +=3) 
			{
				dstCursor[0] = bFg - (bK*(255 - srcCursor[0])>>8);
				dstCursor[1] = gFg - (gK*(255 - srcCursor[1])>>8);
				dstCursor[2] = rFg - (rK*(255 - srcCursor[2])>>8);
			}
		}
	}
	else
	{
		// nothing for now
		return FALSE;
	}

	return TRUE;
}


SetupImage::SetupImage(HDC hdc, HBITMAP bitmapSource, INT maxColors) 
	: ref(1), bitmap(NULL), pixels(NULL), 
	 table(NULL), tableSize(0), tableCount(0), insertCursor(0), readCursor(0)
{
	ZeroMemory(&header, sizeof(BITMAPINFOHEADER));

	BITMAP bm;
	if (sizeof(BITMAP) != GetObject(bitmapSource, sizeof(BITMAP), &bm))
		return;

	if (bm.bmHeight < 0) 
		bm.bmHeight = -bm.bmHeight;
		
	header.biSize = sizeof(BITMAPINFOHEADER);
	header.biCompression = BI_RGB;
	header.biBitCount = 24;
	header.biPlanes = 1;
	header.biWidth = bm.bmWidth;
	header.biHeight = -(bm.bmHeight * (maxColors + 1));
	
	bitmap = CreateDIBSection(hdc, (LPBITMAPINFO)&header, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
	if (NULL == bitmap)
		return;

	if (FALSE == SetupImage_CopyImage(hdc, bitmap, 0, 0, bm.bmWidth, bm.bmHeight, bitmapSource, 0, 0))
	{
		DeleteObject(bitmap);
		bitmap = NULL;
		return;
	}
	
	tableSize = maxColors;
	table = (IMAGEINDEX*)calloc(tableSize, sizeof(IMAGEINDEX));
	if (NULL == table)
	{
		DeleteObject(bitmap);
		bitmap = NULL;
		return;
	}
}

SetupImage::~SetupImage()
{
	if (NULL != bitmap)
	{
		DeleteObject(bitmap);
		bitmap = NULL;
	}
	
	if (NULL != table)
	{
		free(table);
		table = NULL;
	}
}

SetupImage *SetupImage::CreateInstance(HDC hdc, HBITMAP bitmapSource, INT maxColors)
{
	if (NULL == bitmapSource || maxColors < 1 || maxColors > 120)
		return NULL;

	SetupImage *instance = new SetupImage(hdc, bitmapSource, maxColors);
	if (NULL == instance) return NULL;
	if (NULL == instance->bitmap || NULL == instance->table)
	{
		instance->Release();
		instance = NULL;
	}
	return instance;
}

SetupImage *SetupImage::CreateFromPluginBitmap(HDC hdc, LPCWSTR pszModuleName, LPCWSTR resourceName, INT maxColors)
{
	SetupImage *instance = NULL;
	WCHAR szPath[MAX_PATH] = {0};
		
	if (0 != GetModuleFileName(WASABI_API_ORIG_HINST, szPath, ARRAYSIZE(szPath)))
	{
		PathRemoveFileSpec(szPath);
		PathAppend(szPath, pszModuleName);
		HMODULE hModule = LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE | 0x00000020/*LOAD_LIBRARY_AS_IMAGE_RESOURCE*/);
		if (NULL != hModule)
		{
			HBITMAP bitmapSource = (HBITMAP)LoadImage(hModule, resourceName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
			if (NULL != bitmapSource)
				instance = CreateInstance(hdc, bitmapSource, maxColors);
			FreeLibrary(hModule);
		}
	}
	return instance;
}

ULONG SetupImage::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG SetupImage::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

BOOL SetupImage::GetSize(SIZE *pSize)
{
	if (NULL == pSize) return FALSE;

	INT cy = header.biHeight;
	if (cy < 0) cy = -cy;
		
	pSize->cx = header.biWidth;
	pSize->cy = cy / (tableSize + 1);
	return TRUE;
}

BOOL SetupImage::DrawImage(HDC hdc, INT x, INT y, INT cx, INT cy, INT srcX, INT srcY, COLORREF rgbBk, COLORREF rgbFg)
{
	BYTE bitmapIndex = 0xFF;

	SIZE imageSize;
	if (!GetSize(&imageSize))
		return FALSE;

	for(BYTE i = readCursor; i < tableCount; i++)
	{
		if (table[i].rgbBk == rgbBk && table[i].rgbFg == rgbFg)
		{
			bitmapIndex = i;
			break;
		}
	}

	if (0xFF == bitmapIndex)
	{
		if (readCursor > tableCount)
			readCursor = tableCount;
		for(BYTE i = 0; i < readCursor; i++)
		{
			if (table[i].rgbBk == rgbBk && table[i].rgbFg == rgbFg)
			{
				bitmapIndex = i;
				break;
			}
		}

		if (0xFF == bitmapIndex)
		{			
			if (tableCount < tableSize)
			{	
				insertCursor = tableCount;
				tableCount++;
			}
			else if (++insertCursor == tableCount)
				insertCursor = 0;
			
			INT targetY = (insertCursor + 1) * imageSize.cy;
			
			if (!SetupImage_ColorizeImage(pixels, 0, 0, imageSize.cx, imageSize.cy, 
							header.biBitCount, 0, targetY, rgbBk, rgbFg, TRUE))
			{
				return FALSE;
			}
			table[insertCursor].rgbBk = rgbBk;
			table[insertCursor].rgbFg = rgbFg;
			bitmapIndex = insertCursor; 

		}
	}

	readCursor = bitmapIndex;
	srcY += ((bitmapIndex + 1) * imageSize.cy);
		
	INT dstY = y;
	INT dstCY = cy;

	INT imageHeight = header.biHeight;
	if (imageHeight < 0) 
	{
		header.biHeight = -imageHeight;
		dstY += (cy - 1);
		dstCY = -cy;
	}

	BOOL resultOk = StretchDIBits(hdc, x, dstY, cx, dstCY, srcX, srcY, cx, cy, 
				pixels, (BITMAPINFO*)&header, DIB_RGB_COLORS, SRCCOPY);
	
	if (imageHeight < 0) 
		header.biHeight = imageHeight;
	
	return resultOk;
}

BOOL SetupImage::ResetCache()
{
	if (NULL != table)
		ZeroMemory(&table, sizeof(IMAGEINDEX) * tableSize);
	tableCount = 0;
	insertCursor = 0;
	readCursor = 0;
	return TRUE;
}