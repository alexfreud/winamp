#ifndef NULLSOFT_WINAMP_HOOK_INTERFACE_HEADER
#define NULLSOFT_WINAMP_HOOK_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {EB225B60-14C1-424f-B8EA-A8EDE3AD82A9}
static const GUID IFC_WinampHook = 
{ 0xeb225b60, 0x14c1, 0x424f, { 0xb8, 0xea, 0xa8, 0xed, 0xe3, 0xad, 0x82, 0xa9 } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_winamphook: public Dispatchable
{
protected:
	ifc_winamphook() {}
	~ifc_winamphook() {}

public:
	HRESULT IsQuitAllowed(void); // return S_FALSE to block
	HRESULT ResetFont(void);
	HRESULT SkinChanging(void);
	HRESULT SkinChanged(const wchar_t *skinName);
	HRESULT FileMetaChange(const wchar_t *fileName);
	HRESULT SysColorChange(void);
	HRESULT SkinColorChange(const wchar_t *colorTheme);

public:
	DISPATCH_CODES
	{
		API_ISQUITALLOWED	= 10,
		API_RESETFONT		= 20,
		API_SKINCHANGING	= 30,
		API_SKINCHANGED		= 40, 
		API_FILEMETACHANGE	= 50,
		API_SYSCOLORCHANGE	= 60,
		API_SKINCOLORCHANGE = 70
	};
};

inline HRESULT ifc_winamphook::IsQuitAllowed(void)
{
	return _call(API_ISQUITALLOWED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_winamphook::ResetFont(void)
{
	return _call(API_RESETFONT, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_winamphook::SkinChanging(void)
{
	return _call(API_SKINCHANGING, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_winamphook::SkinChanged(const wchar_t *skinName)
{
	return _call(API_SKINCHANGED, (HRESULT)E_NOTIMPL, skinName);
}

inline HRESULT ifc_winamphook::FileMetaChange(const wchar_t *fileName)
{
	return _call(API_FILEMETACHANGE, (HRESULT)E_NOTIMPL, fileName);
}

inline HRESULT ifc_winamphook::SysColorChange(void)
{
	return _call(API_SYSCOLORCHANGE, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_winamphook::SkinColorChange(const wchar_t *colorTheme)
{
	return _call(API_SKINCOLORCHANGE, (HRESULT)E_NOTIMPL, colorTheme);
}

#endif // NULLSOFT_WINAMP_HOOK_INTERFACE_HEADER