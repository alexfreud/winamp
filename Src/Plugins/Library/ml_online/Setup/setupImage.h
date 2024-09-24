#ifndef NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPIMAGE_HEADER
#define NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPIMAGE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <windows.h>

class SetupImage
{
private:
	typedef struct __IMAGEINDEX
	{		
		COLORREF rgbBk;
		COLORREF rgbFg;
	} IMAGEINDEX;

protected:
	SetupImage(HDC hdc, HBITMAP bitmapSource, INT maxColors);
	~SetupImage();

public:
	static SetupImage *CreateInstance(HDC hdc, HBITMAP bitmapSource, INT maxColors);
	static SetupImage *CreateFromPluginBitmap(HDC hdc, LPCWSTR pszModuleName, LPCWSTR resourceName, INT maxColors);

public:
	ULONG AddRef();
	ULONG Release();

	BOOL GetSize(SIZE *pSize);
	BOOL DrawImage(HDC hdc, INT x, INT y, INT cx, INT cy, INT srcX, INT srcY, COLORREF rgbBk, COLORREF rgbFg);

	BOOL ResetCache();

private:
	ULONG		ref;
	HBITMAP		bitmap;
	BYTE			*pixels;
	BITMAPINFOHEADER header;


	IMAGEINDEX	*table;
	BYTE			tableSize;
	BYTE			tableCount;
	BYTE			insertCursor;
	BYTE			readCursor;
};

#endif // NULLOSFT_ONLINEMEDIA_PLUGIN_SETUPIMAGE_HEADER