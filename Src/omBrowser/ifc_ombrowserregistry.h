#ifndef NULLSOFT_WINAMP_OMBROWSER_REGISTRY_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_REGISTRY_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// {F6415954-AB16-4283-B530-EE94AB29E968}
static const GUID IFC_OmBrowserRegistry = 
{ 0xf6415954, 0xab16, 0x4283, { 0xb5, 0x30, 0xee, 0x94, 0xab, 0x29, 0xe9, 0x68 } };

#include <bfc/dispatch.h>

class __declspec(novtable) ifc_ombrowserregistry : public Dispatchable
{
protected:
	ifc_ombrowserregistry() {}
	~ifc_ombrowserregistry() {}

public:
	HRESULT GetPath(LPWSTR pszBuffer, INT cchBufferMax);

	HRESULT	Write(void);
	HRESULT Delete(void);
	HRESULT UpdateColors(void);

public:
	DISPATCH_CODES
	{
		API_GETPATH			= 10,
		API_WRITE			= 20,
		API_DELETE			= 30,
		API_UPDATECOLORS	= 40,
	};
};

inline HRESULT ifc_ombrowserregistry::GetPath(LPWSTR pszBuffer, INT cchBufferMax)
{
	return _call(API_GETPATH, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferMax); 
}

inline HRESULT ifc_ombrowserregistry::Write(void)
{
	return _call(API_WRITE, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_ombrowserregistry::Delete(void)
{
	return _call(API_DELETE, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_ombrowserregistry::UpdateColors(void)
{
	return _call(API_UPDATECOLORS, (HRESULT)E_NOTIMPL);
}

#endif // NULLSOFT_WINAMP_OMBROWSER_CONFIG_INTERFACE_HEADER