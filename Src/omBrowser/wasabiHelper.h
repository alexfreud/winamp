#ifndef NULLSOFT_WINAMP_WASABI_HELPER_HEADER
#define NULLSOFT_WINAMP_WASABI_HELPER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_wasabihelper.h"

class WasabiHelper : public ifc_wasabihelper
{
protected:
	WasabiHelper(api_service *serviceManager);
	~WasabiHelper();

public:
	static HRESULT CreateInstance(api_service *serviceManager, WasabiHelper **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_wasabihelper */
	HRESULT QueryWasabiInterface(const GUID *iid, void **ppi);
	HRESULT ReleaseWasabiInterface(const GUID *iid, void *pi);
	HRESULT GetServiceManager(api_service **serviceManager);
	HRESULT GetApplicationApi(api_application **application);
	HRESULT GetConfigApi(api_config ** config);
	HRESULT GetMemoryManager(api_memmgr **memoryManager);
	HRESULT GetPngLoader(svc_imageLoader **loader);
	HRESULT GetLanguageManager(api_language **languageManager);
	HRESULT GetLanguageModule(HINSTANCE *module);
	HRESULT GetSecurityApi(JSAPI2::api_security **security);
	HRESULT GetThreadpoolApi(api_threadpool **threadpool);

protected:
	HRESULT InitLanguageSupport();

protected:
	RECVS_DISPATCH;

protected:
	ULONG ref;
	api_service *serviceManager;
	api_application *applicationApi;
	api_config *configApi;
	api_memmgr *memoryApi;
	svc_imageLoader *pngLoader;
	api_language *languageManager;
	JSAPI2::api_security *securityApi;
	api_threadpool *threadpoolApi;
	HINSTANCE languageModule;
};

#endif //NULLSOFT_WINAMP_WASABI_HELPER_HEADER