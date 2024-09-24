#ifndef NULLSOFT_WINAMP_OMBROWSER_CLASS_HEADER
#define NULLSOFT_WINAMP_OMBROWSER_CLASS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_ombrowserclass.h"

class OmConfigIni;
class OmBrowserRegistry;

class OmBrowserClass : public ifc_ombrowserclass
{
protected:
	OmBrowserClass(LPCWSTR pszName);
	~OmBrowserClass();

public:
	static HRESULT CreateInstance(LPCWSTR pszName, OmBrowserClass **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/*ifc_ombrowserclass */
	HRESULT GetName(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT IsEqual(LPCWSTR pszName);
	HRESULT GetConfig(ifc_omconfig **instance);
	HRESULT GetRegistry(ifc_ombrowserregistry **instance);
	HRESULT UpdateRegColors();

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	CRITICAL_SECTION lock;
	LPWSTR name;
	OmConfigIni *config;
	OmBrowserRegistry *registry;
};

#endif //NULLSOFT_WINAMP_OMBROWSER_CLASS_HEADER