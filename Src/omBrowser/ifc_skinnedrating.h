#ifndef NULLSOFT_WINAMP_SKINNED_RATING_INTERFACE_HEADER
#define NULLSOFT_WINAMP_SKINNED_RATING_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {9CF4F23F-1F50-4f12-9B0A-A7F129E21AB8}
static const GUID IFC_SkinnedRating = 
{ 0x9cf4f23f, 0x1f50, 0x4f12, { 0x9b, 0xa, 0xa7, 0xf1, 0x29, 0xe2, 0x1a, 0xb8 } };


#include <bfc/dispatch.h>

class ifc_menucustomizer;

class __declspec(novtable) ifc_skinnedrating : public Dispatchable
{
	
protected:
	ifc_skinnedrating() {}
	~ifc_skinnedrating() {}

public:
		
	HRESULT Draw(HDC hdc, INT maxValue, INT value, INT trackingVal, RECT *prc, UINT fStyle);
	HRESULT HitTest(POINT pt, INT maxValue, RECT *prc, UINT fStyle, LONG *result);
	HRESULT CalcMinRect(INT maxValue, RECT *prc);
	HRESULT CreateMenuCustomizer(HMENU hMenu, ifc_menucustomizer **customizer);

public:
	DISPATCH_CODES
	{
		API_DRAW					= 10,
		API_HITTEST				= 20,
		API_CALCMINRECT			= 30,
		API_CREATEMENUCUSTOMIZER	= 40,
	};
};

inline HRESULT ifc_skinnedrating::Draw(HDC hdc, INT maxValue, INT value, INT trackingVal, RECT *prc, UINT fStyle)
{
	return _call(API_DRAW, (HRESULT)E_NOTIMPL, hdc, maxValue, value, trackingVal, prc, fStyle);
}

inline HRESULT ifc_skinnedrating::HitTest(POINT pt, INT maxValue, RECT *prc, UINT fStyle, LONG *result)
{
	return _call(API_HITTEST, (HRESULT)E_NOTIMPL, pt, maxValue, prc, fStyle, result);
}

inline HRESULT ifc_skinnedrating::CalcMinRect(INT maxValue, RECT *prc)
{
	return _call(API_CALCMINRECT, (HRESULT)E_NOTIMPL, maxValue, prc);
}

inline HRESULT ifc_skinnedrating::CreateMenuCustomizer(HMENU hMenu, ifc_menucustomizer **customizer)
{
	return _call(API_CREATEMENUCUSTOMIZER, (HRESULT)E_NOTIMPL, hMenu, customizer);
}


#endif // NULLSOFT_WINAMP_SKINNED_RATING_INTERFACE_HEADER
