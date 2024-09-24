#ifndef NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_CONFIG_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_CONFIG_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {40D85420-F7E6-4995-A4B9-6A00D4E64452}
static const GUID IFC_OmToolbarConfig = 
{ 0x40d85420, 0xf7e6, 0x4995, { 0xa4, 0xb9, 0x6a, 0x0, 0xd4, 0xe6, 0x44, 0x52 } };

#define CFGID_TOOLBAR_BOTTOMDOCK		0		// param = (ULONG_PTR)(BOOL)fBottomDock
#define CFGID_TOOLBAR_AUTOHIDE			1		// param = (ULONG_PTR)(BOOL)fAutoHide
#define CFGID_TOOLBAR_TABSTOP			2		// param = (ULONG_PTR)(BOOL)fTabStop
#define CFGID_TOOLBAR_FORCEADDRESS	3		// param = (ULONG_PTR)(BOOL)fForce
#define CFGID_TOOLBAR_FANCYADDRESS	4		// param = (ULONG_PTR)(BOOL)fFancy

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omtoolbarconfig : public Dispatchable
{
protected:
	ifc_omtoolbarconfig() {}
	~ifc_omtoolbarconfig() {}

public:
	HRESULT GetBottomDockEnabled(void);
	HRESULT GetAutoHideEnabled(void);
	HRESULT GetTabStopEnabled(void);
	HRESULT GetForceAddressbarEnabled(void);
	HRESULT GetFancyAddressbarEnabled(void);

	HRESULT EnableBottomDock(BOOL fEnable);
	HRESULT EnableAutoHide(BOOL fEnable);
	HRESULT EnableTabStop(BOOL fEnable);
	HRESULT EnableForceAddressbar(BOOL fEnable);
	HRESULT EnableFancyAddressbar(BOOL fEnable);

public:
	DISPATCH_CODES
	{
		API_GETBOTTOMDOCKENABLED		= 10,
		API_ENABLEBOTTOMDOCK			= 20,
		API_GETAUTOHIDEENABLED			= 30,
		API_ENABLEAUTOHIDE				= 40,
		API_GETTABSTOPENABLED			= 50,
		API_ENABLETABSTOP				= 60,
		API_GETFORCEADDRESSBARENABLED	= 70,
		API_ENABLEFORCEADDRESSBAR		= 80,
		API_GETFANCYADDRESSBARENABLED	= 90,
		API_ENABLEFANCYADDRESSBAR		= 100,
	};
};

inline HRESULT ifc_omtoolbarconfig::GetBottomDockEnabled(void)
{
	return _call(API_GETBOTTOMDOCKENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omtoolbarconfig::EnableBottomDock(BOOL fEnable)
{
	return _call(API_ENABLEBOTTOMDOCK, (HRESULT)E_NOTIMPL, fEnable);
}

inline HRESULT ifc_omtoolbarconfig::GetAutoHideEnabled(void)
{
	return _call(API_GETAUTOHIDEENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omtoolbarconfig::EnableAutoHide(BOOL fEnable)
{
	return _call(API_ENABLEAUTOHIDE, (HRESULT)E_NOTIMPL, fEnable);
}

inline HRESULT ifc_omtoolbarconfig::GetTabStopEnabled(void)
{
	return _call(API_GETTABSTOPENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omtoolbarconfig::EnableTabStop(BOOL fEnable)
{
	return _call(API_ENABLETABSTOP, (HRESULT)E_NOTIMPL, fEnable);
}

inline HRESULT ifc_omtoolbarconfig::GetForceAddressbarEnabled(void)
{
	return _call(API_GETFORCEADDRESSBARENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omtoolbarconfig::EnableForceAddressbar(BOOL fEnable)
{
	return _call(API_ENABLEFORCEADDRESSBAR, (HRESULT)E_NOTIMPL, fEnable);
}

inline HRESULT ifc_omtoolbarconfig::GetFancyAddressbarEnabled(void)
{
	return _call(API_GETFANCYADDRESSBARENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_omtoolbarconfig::EnableFancyAddressbar(BOOL fEnable)
{
	return _call(API_ENABLEFANCYADDRESSBAR, (HRESULT)E_NOTIMPL, fEnable);
}

#endif // NULLSOFT_WINAMP_OMBROWSER_TOOLBAR_CONFIG_INTERFACE_HEADER