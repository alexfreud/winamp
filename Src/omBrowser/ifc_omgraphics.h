#ifndef NULLSOFT_WINAMP_OMGRAPHICS_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMGRAPHICS_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif



#include <bfc/dispatch.h>


// {0CE26A63-B611-468b-A679-810AAD0FB124}
static const GUID IFC_OmGrpahics = 
{ 0xce26a63, 0xb611, 0x468b, { 0xa6, 0x79, 0x81, 0xa, 0xad, 0xf, 0xb1, 0x24 } };

class __declspec(novtable) ifc_omgraphics: public Dispatchable
{

protected:
	ifc_omgraphics() {}
	~ifc_omgraphics() {}

public:
	HRESULT GetDistance(COLORREF rgb1, COLORREF rgb2, int *distance);
	HRESULT GetDarker(COLORREF rgb1, COLORREF rgb2, COLORREF *result);
	HRESULT BlendColor(COLORREF rgbTop, COLORREF rgbBottom, int alpha, COLORREF *result);

	HRESULT Colorize(BYTE *pixels, long cx, long cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha);
	HRESULT BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb);
	HRESULT BlendOnColor2(BYTE *pixels, int bitmapCX, int bitmapCY, long x, long y, long cx, long cy, WORD bpp, BOOL premult, COLORREF rgb);
	HRESULT Premultiply(BYTE *pixels, long cx, long cy);
	HRESULT AlphaBlend(HDC hdcDest, const RECT *rectDest, HDC hdcSrc, const RECT *rectSrc, BLENDFUNCTION blendFunction);
	HRESULT AnimateRotation(HDC hdc, HBITMAP bitmapFrame, int frameCount, COLORREF rgbBk, BOOL fKeepSize, HBITMAP *result);

public:
	DISPATCH_CODES
	{
		API_GETDISTANCE		= 10,
		API_GETDARKER		= 20,
		API_BLENDCOLOR		= 30,
		API_COLORIZE		= 40,
		API_BLENDONCOLOR	= 50,
		API_BLENDONCOLOR2	= 60,
		API_PREMULTIPLY		= 70,
		API_ALPHABLEND		= 80,
		API_ANIMATEROTATION	= 90,
	};
};

inline HRESULT ifc_omgraphics::GetDistance(COLORREF rgb1, COLORREF rgb2, int *distance)
{
	return _call(API_GETDISTANCE, (HRESULT)E_NOTIMPL, rgb1, rgb2, distance);
}

inline HRESULT ifc_omgraphics::GetDarker(COLORREF rgb1, COLORREF rgb2, COLORREF *result)
{
	return _call(API_GETDARKER, (HRESULT)E_NOTIMPL, rgb1, rgb2, result);
}

inline HRESULT ifc_omgraphics::BlendColor(COLORREF rgbTop, COLORREF rgbBottom, int alpha, COLORREF *result)
{
	return _call(API_BLENDCOLOR, (HRESULT)E_NOTIMPL, rgbTop, rgbBottom, alpha, result);
}

inline HRESULT ifc_omgraphics::Colorize(BYTE *pixels, long cx, long cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha)
{
	return _call(API_COLORIZE, (HRESULT)E_NOTIMPL, pixels, cx, cy, bpp, rgbBk, rgbFg, removeAlpha);
}

inline HRESULT ifc_omgraphics::BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb)
{
	return _call(API_BLENDONCOLOR, (HRESULT)E_NOTIMPL, hbmp, prcPart, premult, rgb);
}

inline HRESULT ifc_omgraphics::BlendOnColor2(BYTE *pixels, int bitmapCX, int bitmapCY, long x, long y, long cx, long cy, WORD bpp, BOOL premult, COLORREF rgb)
{
	return _call(API_BLENDONCOLOR2, (HRESULT)E_NOTIMPL, pixels, bitmapCX, bitmapCY, x, y, cx, cy, bpp, premult, rgb);
}

inline HRESULT ifc_omgraphics::Premultiply(BYTE *pixels, long cx, long cy)
{
	return _call(API_PREMULTIPLY, (HRESULT)E_NOTIMPL, pixels, cx, cy);
}

inline HRESULT ifc_omgraphics::AlphaBlend(HDC hdcDest, const RECT *rectDest, HDC hdcSrc, const RECT *rectSrc, BLENDFUNCTION blendFunction)
{
	return _call(API_ALPHABLEND, (HRESULT)E_NOTIMPL, hdcDest, rectDest, hdcSrc, rectSrc, blendFunction);
}

inline HRESULT ifc_omgraphics::AnimateRotation(HDC hdc, HBITMAP bitmapFrame, int frameCount, COLORREF rgBk, BOOL fKeepSize, HBITMAP *result)
{
	return _call(API_ANIMATEROTATION, (HRESULT)E_NOTIMPL, bitmapFrame, frameCount, rgBk, fKeepSize, result);
}


#endif //NULLSOFT_WINAMP_OMGRAPHICS_INTERFACE_HEADER