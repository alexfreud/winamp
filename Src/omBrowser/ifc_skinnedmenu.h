#ifndef NULLSOFT_WINAMP_SKINNED_MENU_INTERFACE_HEADER
#define NULLSOFT_WINAMP_SKINNED_MENU_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {6623F790-D4D9-43f4-9AFD-980360FB0EED}
static const GUID IFC_SkinnedMenu = 
{ 0x6623f790, 0xd4d9, 0x43f4, { 0x9a, 0xfd, 0x98, 0x3, 0x60, 0xfb, 0xe, 0xed } };

#include <bfc/dispatch.h>


class ifc_menucustomizer;
#define ForceNoSkinnedMenu  ((ifc_menucustomizer*)1)

class __declspec(novtable) ifc_skinnedmenu : public Dispatchable
{
	
protected:
	ifc_skinnedmenu() {}
	~ifc_skinnedmenu() {}

public:
	BOOL TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, ifc_menucustomizer *customizer);
	HRESULT IsEnabled(void);
	HANDLE InitPopupHook(HWND hwnd, ifc_menucustomizer *customizer);
	HRESULT RemovePopupHook(HANDLE popupHook);
	
public:
	DISPATCH_CODES
	{
		API_TRACKPOPUP		= 10,
		API_ISENABLED		= 20,
		API_INITPOPUPHOOK	= 30,
		API_REMOVEPOPUPHOOK	= 40,
	};
};

inline BOOL ifc_skinnedmenu::TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y, HWND hwnd, LPTPMPARAMS lptpm, ifc_menucustomizer *customizer)
{
	return _call(API_TRACKPOPUP, (BOOL)FALSE, hMenu, fuFlags, x, y, hwnd, lptpm, customizer);
}

inline HRESULT ifc_skinnedmenu::IsEnabled(void)
{
	return _call(API_ISENABLED, (BOOL)FALSE);
}

inline HANDLE ifc_skinnedmenu::InitPopupHook(HWND hwnd, ifc_menucustomizer *customizer)
{
	return _call(API_INITPOPUPHOOK, (HANDLE)NULL, hwnd, customizer);
}

inline HRESULT ifc_skinnedmenu::RemovePopupHook(HANDLE popupHook)
{
	return _call(API_REMOVEPOPUPHOOK, (HRESULT)E_NOTIMPL, popupHook);
}

#endif // NULLSOFT_WINAMP_SKINNED_MENU_INTERFACE_HEADER
