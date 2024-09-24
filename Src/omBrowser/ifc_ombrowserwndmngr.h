#ifndef NULLSOFT_WINAMP_OMBROWSER_WINDOW_MANAGER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_WINDOW_MANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {81E8333B-388F-444e-8233-138E15ACC761}
static const GUID IFC_OmBrowserWindowManager =
{ 0x81e8333b, 0x388f, 0x444e, { 0x82, 0x33, 0x13, 0x8e, 0x15, 0xac, 0xc7, 0x61 } };

#include <bfc/dispatch.h>

class ifc_ombrowserwndenum;

class __declspec(novtable) ifc_ombrowserwndmngr : public Dispatchable
{
protected:
	ifc_ombrowserwndmngr() {}
	~ifc_ombrowserwndmngr() {}

public:
	HRESULT RegisterWindow(HWND hwnd, const GUID *windowType);
	HRESULT UnregisterWindow(HWND hwnd);
	HRESULT Enumerate(const GUID *windowType, unsigned int *serviceIdFilter, ifc_ombrowserwndenum **enumerator); // serviceIdFilter can be NULL if you want to get all windows

public:
	DISPATCH_CODES
	{
		API_REGISTERWINDOW		= 10,
		API_UNREGISTERWINDOW		= 20,
		API_ENUMERATE			= 30,
	};
};

inline HRESULT ifc_ombrowserwndmngr::RegisterWindow(HWND hwnd, const GUID *windowType)
{
	return _call(API_REGISTERWINDOW, (HRESULT)E_NOTIMPL, hwnd, windowType); 
}

inline HRESULT ifc_ombrowserwndmngr::UnregisterWindow(HWND hwnd)
{
	return _call(API_UNREGISTERWINDOW, (HRESULT)E_NOTIMPL, hwnd);
}

inline HRESULT ifc_ombrowserwndmngr::Enumerate(const GUID *windowType, unsigned int *serviceIdFilter, ifc_ombrowserwndenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, windowType, serviceIdFilter, enumerator);
}

#endif // NULLSOFT_WINAMP_OMBROWSER_WINDOW_MANAGER_INTERFACE_HEADER