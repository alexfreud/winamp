#ifndef NULLSOFT_WINAMP_API_WINAMP_HEADER
#define NULLSOFT_WINAMP_API_WINAMP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {5968E566-805E-442f-B6A0-916F9CD4E8A8}
static const GUID winampApiGuid = 
{ 0x5968e566, 0x805e, 0x442f, { 0xb6, 0xa0, 0x91, 0x6f, 0x9c, 0xd4, 0xe8, 0xa8 } };


class __declspec(novtable) api_winamp : public Dispatchable
{
protected:
	api_winamp() {}
	~api_winamp() {}

public:
	HWND GetMainWindow(void);
	HWND GetDlgParent(void);
	HRESULT OpenUrl(HWND hwnd, const wchar_t *url);
	int GetRegVer();

public:
	DISPATCH_CODES
	{
		API_GETMAINWINDOW = 10,
		API_GETDLGPARENT = 20,
		API_OPENURL = 30,
		API_GETREGVER = 40,
	};
};

inline HWND api_winamp::GetMainWindow(void)
{
	return _call(API_GETMAINWINDOW, (HWND)NULL);
}

inline HWND api_winamp::GetDlgParent(void)
{
	return _call(API_GETDLGPARENT, (HWND)NULL);
}

inline HRESULT api_winamp::OpenUrl(HWND hwnd, const wchar_t *url)
{
	return _call(API_OPENURL, (HRESULT)E_NOTIMPL, hwnd, url);
}

inline int api_winamp::GetRegVer()
{
	return _call(API_GETREGVER, (int)0);
}

#endif // NULLSOFT_WINAMP_API_WINAMP_HEADER