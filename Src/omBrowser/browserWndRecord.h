#ifndef NULLSOFT_WINAMP_OMBROWSER_WINDOW_RECORD_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_WINDOW_RECORD_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class OmBrowserWndRecord
{
protected:
	OmBrowserWndRecord(HWND hwnd, const GUID *type);
	~OmBrowserWndRecord();
public:
	static HRESULT CreateInstance(HWND hwnd, const GUID *type, OmBrowserWndRecord **instance);

public:
	ULONG AddRef();
	ULONG Release();

	HWND GetHwnd();
	HRESULT GetType(GUID *windowType);
	HRESULT IsEqualType(const GUID *windowType);

protected:
	ULONG ref;
	HWND hwnd;
	GUID type;
};


#endif //NULLSOFT_WINAMP_OMBROWSER_WINDOW_RECORD_HEADER