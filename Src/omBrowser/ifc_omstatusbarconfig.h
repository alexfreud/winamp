#ifndef NULLSOFT_WINAMP_OMBROWSER_STATUSBAR_CONFIG_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_STATUSBAR_CONFIG_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#define CFGID_STATUSBAR_ENABLED		0		// param = (ULONG_PTR)(BOOL)fEnabled

// {98ABEE7F-06F4-4652-886D-58E1769E2592}
static const GUID IFC_OmStatusbarConfig = 
{ 0x98abee7f, 0x6f4, 0x4652, { 0x88, 0x6d, 0x58, 0xe1, 0x76, 0x9e, 0x25, 0x92 } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_omstatusbarconfig : public Dispatchable
{
protected:
	ifc_omstatusbarconfig() {}
	~ifc_omstatusbarconfig() {}

public:
	HRESULT GetEnabled(void);
	HRESULT EnableStatusbar(BOOL fEnable);
	
public:
	DISPATCH_CODES
	{
		API_GETENABLED				= 10,
		API_ENABLESTATUSBAR			= 20,
	};
};

inline HRESULT ifc_omstatusbarconfig::GetEnabled(void)
{
	return _call(API_GETENABLED, (HRESULT)E_NOTIMPL);
}


inline HRESULT ifc_omstatusbarconfig::EnableStatusbar(BOOL fEnable)
{
	return _call(API_ENABLESTATUSBAR, (HRESULT)E_NOTIMPL, fEnable);
}


#endif // NULLSOFT_WINAMP_OMBROWSER_STATUSBAR_CONFIG_INTERFACE_HEADER