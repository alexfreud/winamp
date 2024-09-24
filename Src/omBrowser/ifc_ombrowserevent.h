#ifndef NULLSOFT_WINAMP_OMBROWSER_EVENTHANDLER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_EVENTHANDLER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {900AD92F-E2B9-4615-B1DB-ACC37FCD676C}
static const GUID IFC_OmBrowserEvent = 
{ 0x900ad92f, 0xe2b9, 0x4615, { 0xb1, 0xdb, 0xac, 0xc3, 0x7f, 0xcd, 0x67, 0x6c } };


class __declspec(novtable) ifc_ombrowserevent : public Dispatchable
{

protected:
	ifc_ombrowserevent() {}
	~ifc_ombrowserevent() {}

public:
	void WindowCreate(HWND hwnd, const GUID *windowType);
	void WindowClose(HWND hwnd, const GUID *windowType);

public:
	DISPATCH_CODES
	{		
		API_WINDOWCREATE = 10,
		API_WINDOWCLOSE = 20,
	};
};

inline void ifc_ombrowserevent::WindowCreate(HWND hwnd, const GUID *windowType)
{
	_voidcall(API_WINDOWCREATE, hwnd, windowType);
}

inline void ifc_ombrowserevent::WindowClose(HWND hwnd, const GUID *windowType)
{
	_voidcall(API_WINDOWCLOSE, hwnd, windowType);
}

#endif //NULLSOFT_WINAMP_OMBROWSER_EVENTHANDLER_INTERFACE_HEADER