#include "./common.h"
#include "./graphicsObject.h"
#include "./graphics.h"

GraphicsObject::GraphicsObject()
	: ref(1)
{
}

GraphicsObject::~GraphicsObject()
{
}

HRESULT GraphicsObject::CreateInstance(GraphicsObject **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new GraphicsObject();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

size_t GraphicsObject::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t GraphicsObject::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int GraphicsObject::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmGrpahics))
		*object = static_cast<ifc_omgraphics*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT GraphicsObject::GetDistance(COLORREF rgb1, COLORREF rgb2, int *distance)
{
	if (NULL == distance) return E_POINTER;
	*distance = GetColorDistance(rgb1, rgb2);
	return S_OK;
}

HRESULT GraphicsObject::GetDarker(COLORREF rgb1, COLORREF rgb2, COLORREF *result)
{
	if (NULL == result) return E_POINTER;
	*result = GetDarkerColor(rgb1, rgb2);
	return S_OK;
}

HRESULT GraphicsObject::BlendColor(COLORREF rgbTop, COLORREF rgbBottom, int alpha, COLORREF *result)
{
	if (NULL == result) return E_POINTER;
	*result = BlendColors(rgbTop, rgbBottom, alpha);
	return S_OK;
}

HRESULT GraphicsObject::Colorize(BYTE *pixels, long cx, long cy, WORD bpp, COLORREF rgbBk, COLORREF rgbFg, BOOL removeAlpha)
{
	BOOL result = Image_Colorize(pixels, cx, cy, bpp, rgbBk, rgbFg, removeAlpha);
	return (FALSE != result) ? S_OK : S_FALSE;
}

HRESULT GraphicsObject::BlendOnColor(HBITMAP hbmp, RECT *prcPart, BOOL premult, COLORREF rgb)
{
	BOOL result = Image_BlendOnColor(hbmp, prcPart, premult, rgb);
	return (FALSE != result) ? S_OK : S_FALSE;
}

HRESULT GraphicsObject::BlendOnColor2(BYTE *pixels, int bitmapCX, int bitmapCY, long x, long y, long cx, long cy, WORD bpp, BOOL premult, COLORREF rgb)
{
	BOOL result = Image_BlendOnColorEx(pixels, bitmapCX, bitmapCY, x, y, cx, cy, bpp, premult, rgb);
	return (FALSE != result) ? S_OK : S_FALSE;
}

HRESULT GraphicsObject::Premultiply(BYTE *pixels, long cx, long cy)
{
	BOOL result = Image_Premultiply(pixels, cx, cy);
	return (FALSE != result) ? S_OK : S_FALSE;
}

HRESULT GraphicsObject::AlphaBlend(HDC hdcDest, const RECT *rectDest, HDC hdcSrc, const RECT *rectSrc, BLENDFUNCTION blendFunction)
{
	if (NULL == rectDest || NULL == rectSrc)
		return E_INVALIDARG;

	BOOL result = Image_AlphaBlend(hdcDest, rectDest->left, rectDest->top, rectDest->right, rectDest->bottom, hdcSrc, rectSrc->left, rectSrc->top, rectSrc->right, rectSrc->bottom, blendFunction);
	return (FALSE != result) ? S_OK : S_FALSE;
}

HRESULT GraphicsObject::AnimateRotation(HDC hdc, HBITMAP bitmapFrame, int frameCount, COLORREF rgbBk, BOOL fKeepSize, HBITMAP *result)
{
	if (NULL == result) return E_POINTER;
	*result = Image_AnimateRotation(hdc, bitmapFrame, frameCount, rgbBk, fKeepSize);
	if (NULL == *result) return E_FAIL;
	return S_OK;
}

#define CBCLASS GraphicsObject
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETDISTANCE, GetDistance)
CB(API_GETDARKER, GetDarker)
CB(API_BLENDCOLOR, BlendColor)
CB(API_COLORIZE, Colorize)
CB(API_BLENDONCOLOR, BlendOnColor)
CB(API_BLENDONCOLOR2, BlendOnColor2)
CB(API_PREMULTIPLY, Premultiply)
CB(API_ALPHABLEND, AlphaBlend)
CB(API_ANIMATEROTATION, AnimateRotation)
END_DISPATCH;
#undef CBCLASS
