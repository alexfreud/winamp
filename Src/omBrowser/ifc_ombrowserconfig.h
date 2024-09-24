#ifndef NULLSOFT_WINAMP_OMBROWSER_BROWSER_CONFIG_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_BROWSER_CONFIG_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {41660B13-0547-4b8f-934B-A688306F0D4A}
static const GUID IFC_OmBrowserConfig =
{ 0x41660b13, 0x547, 0x4b8f, { 0x93, 0x4b, 0xa6, 0x88, 0x30, 0x6f, 0xd, 0x4a } };

#define CFGID_BROWSER_CLIENTID		0			//param = (LPCWSTR)pszClientId

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_ombrowserconfig : public Dispatchable
{
protected:
	ifc_ombrowserconfig() {}
	~ifc_ombrowserconfig() {}

public:
	HRESULT GetClientId(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT SetClientId(LPWSTR pszClientId);
	UINT GetX();
	HRESULT SetX(UINT x);
	UINT GetY();
	HRESULT SetY(UINT y);

public:
	DISPATCH_CODES
	{
		API_GETCLIENTID		= 10,
		API_SETCLIENTID		= 20,
		API_GETX			= 30,
		API_SETX			= 40,
		API_GETY			= 50,
		API_SETY			= 60,
	};
};

inline HRESULT ifc_ombrowserconfig::GetClientId(LPWSTR pszBuffer, INT cchBufferMax)
{
	return _call(API_GETCLIENTID, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax); 
}

inline HRESULT ifc_ombrowserconfig::SetClientId(LPWSTR pszClientId)
{
	return _call(API_SETCLIENTID, (HRESULT)E_NOTIMPL, pszClientId);
}

inline UINT ifc_ombrowserconfig::GetX(void)
{
	return _call(API_GETX, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_ombrowserconfig::SetX(UINT x)
{
	return _call(API_SETX, (HRESULT)E_NOTIMPL, x);
}

inline UINT ifc_ombrowserconfig::GetY(void)
{
	return _call(API_GETY, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_ombrowserconfig::SetY(UINT y)
{
	return _call(API_SETY, (HRESULT)E_NOTIMPL, y);
}

#endif // NULLSOFT_WINAMP_OMBROWSER_BROWSER_CONFIG_INTERFACE_HEADER