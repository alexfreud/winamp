#ifndef NULLSOFT_WINAMP_SKIN_HELPER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_SKIN_HELPER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {DC98C56E-F649-469e-82DC-234FAA8083C1}
static const GUID IFC_SkinHelper = 
{ 0xdc98c56e, 0xf649, 0x469e, { 0x82, 0xdc, 0x23, 0x4f, 0xaa, 0x80, 0x83, 0xc1 } };

#include <bfc/dispatch.h>

typedef struct embedWindowState embedWindowState;
typedef int (CALLBACK *FFCALLBACK)(embedWindowState* /*windowState*/, INT /*eventId*/, LPARAM /*param*/);

class __declspec(novtable) ifc_skinhelper : public Dispatchable
{
	
protected:
	ifc_skinhelper() {}
	~ifc_skinhelper() {}

public:
	HRESULT GetColor(UINT colorIndex, COLORREF *pColor);
    HRESULT GetColorEx(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor);
	HRESULT GetBrush(UINT colorIndex, HBRUSH *pBrush);
	
	HFONT GetFont(void);

	HRESULT SkinWindow(HWND hwnd, const GUID *windowGuid, UINT flagsEx, FFCALLBACK callbackFF);
	HRESULT SkinControl(HWND hwnd, UINT type, UINT style);
	HRESULT UnskinWindow(HWND hwnd);

	HRESULT GetHostCss(OLECHAR **ppchHostCss);

public:
	DISPATCH_CODES
	{
		API_GETCOLOR			= 10,
		API_GETCOLOREX			= 20,
		API_GETBRUSH			= 30,
		API_GETFONT				= 40,
		API_SKINWINDOW			= 50,
		API_SKINCONTROL			= 60,
		API_UNSKINWINDOW			= 70,
	};
};

inline HRESULT ifc_skinhelper::GetColor(UINT colorIndex, COLORREF *pColor)
{
	return _call(API_GETCOLOR, (HRESULT)E_NOTIMPL, colorIndex, pColor);
}

inline HRESULT ifc_skinhelper::GetColorEx(UINT uObject, UINT uPart, UINT uState, COLORREF *pColor)
{
	return _call(API_GETCOLOREX, (HRESULT)E_NOTIMPL, uObject, uPart, uState, pColor);
}

inline HRESULT ifc_skinhelper::GetBrush(UINT colorIndex, HBRUSH *pBrush)
{
	return _call(API_GETBRUSH, (HRESULT)E_NOTIMPL, colorIndex, pBrush);
}

inline HFONT ifc_skinhelper::GetFont()
{
	return _call(API_GETFONT, (HFONT)NULL);
}

inline HRESULT ifc_skinhelper::SkinWindow(HWND hwnd, const GUID *windowGuid, UINT flagsEx, FFCALLBACK callbackFF)
{
	return _call(API_SKINWINDOW, (HRESULT)E_NOTIMPL, hwnd, windowGuid, flagsEx, callbackFF);
}

inline HRESULT ifc_skinhelper::SkinControl(HWND hwnd, UINT type, UINT style)
{
	return _call(API_SKINCONTROL, (HRESULT)E_NOTIMPL, hwnd, type, style);
}

inline HRESULT ifc_skinhelper::UnskinWindow(HWND hwnd)
{
	return _call(API_UNSKINWINDOW, (HRESULT)E_NOTIMPL);
}

#endif // NULLSOFT_WINAMP_SKIN_HELPER_INTERFACE_HEADER
