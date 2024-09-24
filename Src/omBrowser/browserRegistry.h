#ifndef NULLSOFT_WINAMP_BROWSER_REGISTRY_HEADER
#define NULLSOFT_WINAMP_BROWSER_REGISTRY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_ombrowserregistry.h"

class obj_ombrowser;

class OmBrowserRegistry : public ifc_ombrowserregistry
{
protected:
	OmBrowserRegistry(LPCWSTR pszName);
	~OmBrowserRegistry();

public:
	static HRESULT CreateInstance(LPCWSTR pszName, OmBrowserRegistry **instance);

public:

	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_ombrowserregistry */
	HRESULT GetPath(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT	Write(void);
	HRESULT Delete(void);
	HRESULT UpdateColors(void);

protected:
	HRESULT CreatePath(LPWSTR pszBuffer, INT cchBufferMax);
	HRESULT CreateRoot(HKEY *hKey, DWORD *pDisposition);
	HRESULT WriteColors(HKEY hKey);

protected:	
	RECVS_DISPATCH;

protected:
	ULONG ref;
	LPWSTR name;
	LPWSTR path;

};

#endif //NULLSOFT_WINAMP_BROWSER_REGISTRY_HEADER