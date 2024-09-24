#ifndef NULLSOFT_WINAMP_OMBROWSER_CLASS_INTERFACE_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_CLASS_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/dispatch.h>

// {1B06FE2B-A1E7-43b5-B470-573CDF5D54F9}
static const GUID IFC_OmBrowserClass = 
{ 0x1b06fe2b, 0xa1e7, 0x43b5, { 0xb4, 0x70, 0x57, 0x3c, 0xdf, 0x5d, 0x54, 0xf9 } };

class ifc_omconfig;
class ifc_ombrowserregistry;

class __declspec(novtable) ifc_ombrowserclass : public Dispatchable
{
protected:
	ifc_ombrowserclass() {}
	~ifc_ombrowserclass() {}

public:
	HRESULT GetName(wchar_t *pszBuffer, int cchBufferLen);
	HRESULT IsEqual(const wchar_t *pszName);
	HRESULT GetConfig(ifc_omconfig **instance);
	HRESULT GetRegistry(ifc_ombrowserregistry **instance);
	HRESULT UpdateRegColors(void);


public:
	DISPATCH_CODES
	{
		API_GETNAME			= 10,
		API_ISEQUAL			= 20,
		API_GETCONFIG		= 30,
		API_GETREGISTRY		= 40,
		API_UPDATEREGCOLORS = 50,
	};
};

inline HRESULT ifc_ombrowserclass::GetName(wchar_t *pszBuffer, int cchBufferLen)
{
	return _call(API_GETNAME, (HRESULT)E_NOTIMPL, pszBuffer, cchBufferLen);
}

inline HRESULT ifc_ombrowserclass::IsEqual(const wchar_t *pszName)
{
	return _call(API_ISEQUAL, (HRESULT)E_NOTIMPL, pszName);
}

inline HRESULT ifc_ombrowserclass::GetConfig(ifc_omconfig **instance)
{
	return _call(API_GETCONFIG, (HRESULT)E_NOTIMPL, instance);
}

inline HRESULT ifc_ombrowserclass::GetRegistry(ifc_ombrowserregistry **instance)
{
	return _call(API_GETREGISTRY, (HRESULT)E_NOTIMPL, instance);
}

inline HRESULT ifc_ombrowserclass::UpdateRegColors(void)
{
	return _call(API_UPDATEREGCOLORS, (HRESULT)E_NOTIMPL);
}
#endif //NULLSOFT_WINAMP_OMBROWSER_CLASS_INTERFACE_HEADER