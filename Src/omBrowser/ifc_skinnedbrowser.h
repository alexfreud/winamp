#ifndef NULLSOFT_WINAMP_OMBROWSER_SKINNED_BROWSER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_SKINNED_BROWSER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {9E98C972-7C54-46e1-B265-21B343B2F226}
static const GUID IFC_SkinnedBrowser = 
{ 0x9e98c972, 0x7c54, 0x46e1, { 0xb2, 0x65, 0x21, 0xb3, 0x43, 0xb2, 0xf2, 0x26 } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_skinnedbrowser : public Dispatchable
{
	
protected:
	ifc_skinnedbrowser() {}
	~ifc_skinnedbrowser() {}

public:
	HRESULT GetHostCss(wchar_t **ppchHostCss);
	COLORREF GetBackColor(void);
	COLORREF GetTextColor(void);
	COLORREF GetLinkColor(void);
	COLORREF GetActiveLinkColor(void);
	COLORREF GetVisitedLinkColor(void);
	COLORREF GetHoveredLinkColor(void);

	
public:
	DISPATCH_CODES
	{
		API_GETHOSTCSS			= 10,
		API_GETBACKCOLOR		= 20,
		API_GETTEXTCOLOR		= 30,
		API_GETLINKCOLOR		= 40,
		API_GETACTIVELINKCOLOR	= 50,
		API_GETVISITEDLINKCOLOR	= 60,
		API_GETHOVEREDLINKCOLOR	= 70,
	};
};

inline HRESULT ifc_skinnedbrowser::GetHostCss(wchar_t **ppchHostCss)
{
	return _call(API_GETHOSTCSS, (HRESULT)E_NOTIMPL, ppchHostCss);
}

inline COLORREF ifc_skinnedbrowser::GetBackColor(void)
{
	return _call(API_GETBACKCOLOR, (COLORREF)0x00FFFFFF);
}

inline COLORREF ifc_skinnedbrowser::GetTextColor(void)
{
	return _call(API_GETTEXTCOLOR, (COLORREF)0x00000000);
}

inline COLORREF ifc_skinnedbrowser::GetLinkColor(void)
{
	return _call(API_GETLINKCOLOR, (COLORREF)0x000000E0);
}

inline COLORREF ifc_skinnedbrowser::GetActiveLinkColor(void)
{
	return _call(API_GETACTIVELINKCOLOR, (COLORREF)0x000000FF);
}

inline COLORREF ifc_skinnedbrowser::GetVisitedLinkColor(void)
{
	return _call(API_GETVISITEDLINKCOLOR, (COLORREF)0x00FF00FF);
}

inline COLORREF ifc_skinnedbrowser::GetHoveredLinkColor(void)
{
	return _call(API_GETHOVEREDLINKCOLOR, (COLORREF)0x000000F0);
}
#endif // NULLSOFT_WINAMP_OMBROWSER_SKINNED_BROWSER_INTERFACE_HEADER
