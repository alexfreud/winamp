#ifndef NULLSOFT_WINAMP_OMUTILITY_HEADER
#define NULLSOFT_WINAMP_OMUTILITY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_omutility.h"

class CacheManager;
class MlNavigationHelper;
class GraphicsObject;
class StorageHelper;

class OmUtility : public ifc_omutility
{
protected:
	OmUtility();
	~OmUtility();

public:
	static OmUtility *CreateInstance();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_omutility */
	HRESULT EnsurePathExist(LPCWSTR pszDirectory);
	HRESULT MakeResourcePath(LPWSTR pszBuffer, UINT cchBufferMax, HINSTANCE hInstance, LPCWSTR pszType, LPCWSTR pszName, UINT uFlags);
	HRESULT GetCacheManager(ifc_omcachemanager **cacheManager);
	HRESULT GetMlNavigationHelper(HWND hLibrary, ifc_mlnavigationhelper **helper);
	HRESULT QueryImageLoader(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader);
	HRESULT GetGraphics(ifc_omgraphics **graphics);
	HRESULT PostMainThreadCallback(ThreadCallback callback, ULONG_PTR param);
	HRESULT PostMainThreadCallback2(ThreadCallback2 callback, Dispatchable *object, ULONG_PTR param1, ULONG_PTR param2);
	HRESULT GetStorageHelper(ifc_omstoragehelper **helper);

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	CacheManager *cacheManager;
	MlNavigationHelper *navigationHelper;
	GraphicsObject *graphicsObject;
	StorageHelper *storageHelper;
	HWND hListener;
};

#endif //NULLSOFT_WINAMP_OMUTILITY_HEADER