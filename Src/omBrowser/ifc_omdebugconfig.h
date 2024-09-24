#ifndef NULLSOFT_WINAMP_OMBROWSER_DEBUG_CONFIG_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_DEBUG_CONFIG_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {07057926-4DCD-4686-AE81-F74AE36B931B}
static const GUID IFC_OmDebugConfig = 
{ 0x7057926, 0x4dcd, 0x4686, { 0xae, 0x81, 0xf7, 0x4a, 0xe3, 0x6b, 0x93, 0x1b } };

#define CFGID_DEBUG_FILTERMENU		0		// param = (ULONG_PTR)(BOOL)fFilter
#define CFGID_DEBUG_SCRIPTERROR		1		// param = (ULONG_PTR)(BOOL)fShow
#define CFGID_DEBUG_SCRIPTDEBUGGER	2		// param = (ULONG_PTR)(BOOL)fShow
#define CFGID_DEBUG_BROWSERPATH		3		// param = (ULONG_PTR)(LPCWSTR)pszPath

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omdebugconfig : public Dispatchable
{
protected:
	ifc_omdebugconfig() {}
	~ifc_omdebugconfig() {}

public:
	
	HRESULT GetMenuFilterEnabled(void);
	HRESULT GetScriptErrorEnabled(void);
	HRESULT GetScriptDebuggerEnabled(void);
	HRESULT GetBrowserPath(LPWSTR pszBuffer, INT cchBufferMax);

	HRESULT EnableMenuFilter(BOOL fEnable);
	HRESULT EnableScriptError(BOOL fEnable);
	HRESULT EnableScriptDebugger(BOOL fEnable);
	HRESULT SetBrowserPath(LPCWSTR pszPath);

public:
	DISPATCH_CODES
	{
		API_GETMENUFILTERENABLED			= 10,
		API_GETSCRIPTERRORENABLED		= 20,
		API_GETSCRIPTDEBUGGERENABLED	= 30,
		API_GETBROWSERPATH				= 40,

		API_ENABLEMENUFILTER				= 110,
		API_ENABLESCRIPTERROR			= 120,
		API_ENABLESCRIPTDEBUGGER		= 130,
		API_SETBROWSERPATH				= 140,
	};
};

inline HRESULT ifc_omdebugconfig::GetMenuFilterEnabled(void)
{
	return _call(API_GETMENUFILTERENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omdebugconfig::GetScriptErrorEnabled(void)
{
	return _call(API_GETSCRIPTERRORENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omdebugconfig::GetScriptDebuggerEnabled(void)
{
	return _call(API_GETSCRIPTDEBUGGERENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omdebugconfig::GetBrowserPath(LPWSTR pszBuffer, INT cchBufferMax)
{
	return _call(API_GETBROWSERPATH, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax); 
}

inline HRESULT ifc_omdebugconfig::EnableMenuFilter(BOOL fEnable)
{
	return _call(API_ENABLEMENUFILTER, (HRESULT)E_NOTIMPL, fEnable);
}

inline HRESULT ifc_omdebugconfig::EnableScriptError(BOOL fEnable)
{
	return _call(API_ENABLESCRIPTERROR, (HRESULT)E_NOTIMPL, fEnable);
}

inline HRESULT ifc_omdebugconfig::EnableScriptDebugger(BOOL fEnable)
{
	return _call(API_ENABLESCRIPTDEBUGGER, (HRESULT)E_NOTIMPL, fEnable);
}

inline HRESULT ifc_omdebugconfig::SetBrowserPath(LPCWSTR pszPath)
{
	return _call(API_SETBROWSERPATH, (HRESULT)E_NOTIMPL, pszPath);
}

#endif // NULLSOFT_WINAMP_OMBROWSER_DEBUG_CONFIG_INTERFACE_HEADER