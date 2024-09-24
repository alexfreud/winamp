#ifndef NULLSOFT_AUTH_LOGINBOX_BROWSER_EVENT_HEADER
#define NULLSOFT_AUTH_LOGINBOX_BROWSER_EVENT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class __declspec(novtable) BrowserEvent : public IUnknown
{
public:
	STDMETHOD_(void, Event_BrowserReady)(HWND hBrowser) = 0;
	STDMETHOD_(void, Event_DocumentReady)(HWND hBrowser) = 0;
	STDMETHOD_(void, Event_BrowserClosing)(HWND hBrowser) = 0;
	STDMETHOD_(void, Event_InvokeApc)(HWND hBrowser, LPARAM param) = 0;
};

#endif // NULLSOFT_AUTH_LOGINBOX_BROWSER_EVENT_HEADER