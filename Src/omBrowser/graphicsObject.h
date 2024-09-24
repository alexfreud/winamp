#ifndef NULLSOFT_WINAMP_UTILITY_GRAPHICS_OBJECT_HEADER
#define NULLSOFT_WINAMP_UTILITY_GRAPHICS_OBJECT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omgraphics.h"


class GraphicsObject : public ifc_omgraphics
{	
protected:
	GraphicsObject();
	~GraphicsObject();

public:
	static HRESULT CreateInstance(GraphicsObject **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omgraphics */
	HRESULT GetDistance(COLORREF rgb1, COLORREF rgb2, int *distance);
	HRESULT GetDarker(COLORREF rgb1, COLORREF rgb2, COLORREF *result);
	HRESULT BlendColor(COLORREF rgbTop, COLORREF rgbBottom, int alpha, COLORREF *result);
	HRESULT Colorize(BYTE *pixels, long cx, long cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha);
	HRESULT BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb);
	HRESULT BlendOnColor2(BYTE *pixels, int bitmapCX, int bitmapCY, long x, long y, long cx, long cy, WORD bpp, BOOL premult, COLORREF rgb);
	HRESULT Premultiply(BYTE *pixels, long cx, long cy);
	HRESULT AlphaBlend(HDC hdcDest, const RECT *rectDest, HDC hdcSrc, const RECT *rectSrc, BLENDFUNCTION blendFunction);
	HRESULT AnimateRotation(HDC hdc, HBITMAP bitmapFrame, int frameCount, COLORREF rgbBk, BOOL fKeepSize, HBITMAP *result);

protected:
	size_t ref;

protected:
	RECVS_DISPATCH;
};


#endif // NULLSOFT_WINAMP_UTILITY_GRAPHICS_OBJECT_HEADER