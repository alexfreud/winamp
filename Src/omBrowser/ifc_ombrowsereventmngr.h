#ifndef NULLSOFT_WINAMP_OMBROWSER_EVENTMANAGER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_EVENTMANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {EF73F304-7730-4e80-B938-A80B5C971CC5}
static const GUID IFC_OmBrowserEventManager = 
{ 0xef73f304, 0x7730, 0x4e80, { 0xb9, 0x38, 0xa8, 0xb, 0x5c, 0x97, 0x1c, 0xc5 } };


class obj_ombrowser;
class ifc_ombrowserevent;

class __declspec(novtable) ifc_ombrowsereventmngr : public Dispatchable
{

protected:
	ifc_ombrowsereventmngr() {}
	~ifc_ombrowsereventmngr() {}

public:
	HRESULT RegisterHandler(ifc_ombrowserevent *handler);
	HRESULT UnregisterHandler(ifc_ombrowserevent *handler);
	HRESULT Signal_WindowCreate(HWND hwnd, const GUID *windowType);
	HRESULT Signal_WindowClose(HWND hwnd, const GUID *windowType);

public:
	DISPATCH_CODES
	{
		API_REGISTERHANDLER			= 10,
		API_UNREGISTERHANDLER		= 20,
		API_SIGNAL_WINDOWCREATE		= 30,
		API_SIGNAL_WINDOWCLOSE		= 40,
	};
};

inline HRESULT ifc_ombrowsereventmngr::RegisterHandler(ifc_ombrowserevent *handler)
{
	return _call(API_REGISTERHANDLER, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT ifc_ombrowsereventmngr::UnregisterHandler(ifc_ombrowserevent *handler)
{
	return _call(API_UNREGISTERHANDLER, (HRESULT)E_NOTIMPL, handler);
}

inline HRESULT ifc_ombrowsereventmngr::Signal_WindowCreate(HWND hwnd, const GUID *windowType)
{
	return _call(API_SIGNAL_WINDOWCREATE, (HRESULT)E_NOTIMPL, hwnd, windowType);
}

inline HRESULT ifc_ombrowsereventmngr::Signal_WindowClose(HWND hwnd, const GUID *windowType)
{
	return _call(API_SIGNAL_WINDOWCLOSE, (HRESULT)E_NOTIMPL, hwnd, windowType);
}

#endif //NULLSOFT_WINAMP_OMBROWSER_EVENTMANAGER_INTERFACE_HEADER