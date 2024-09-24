#ifndef NULLSOFT_WINAMP_WASABI_HELPER_INTERFACE_HEADER
#define NULLSOFT_WINAMP_WASABI_HELPER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

class api_service;

#include <bfc/dispatch.h>

#include <api/application/api_application.h>
#include "../Agave/Config/api_config.h"
#include "../Agave/Language/api_language.h"
#include <api/memmgr/api_memmgr.h>
#include <api/service/svcs/svc_imgload.h>
#include "../Winamp/JSAPI2_api_security.h"
#include "../Components/wac_downloadManager/wac_downloadManager_api.h"
#include "../nu/threadpool/api_threadpool.h"

// {CA8954C8-C15F-4a62-A592-86A9464DE9A5}
static const GUID IFC_WasabiHelper = 
{ 0xca8954c8, 0xc15f, 0x4a62, { 0xa5, 0x92, 0x86, 0xa9, 0x46, 0x4d, 0xe9, 0xa5 } };

class __declspec(novtable) ifc_wasabihelper : public Dispatchable
{
protected:
	ifc_wasabihelper() {}
	~ifc_wasabihelper() {}

public:
	HRESULT QueryWasabiInterface(const GUID *iid, void **ppi);
	HRESULT ReleaseWasabiInterface(const GUID *iid, void *pi);

	HRESULT GetServiceManager(api_service **serviceManager);
	HRESULT GetApplicationApi(api_application **application);
	HRESULT GetConfigApi(api_config ** config);
	HRESULT GetMemoryManager(api_memmgr **memoryManager);
	HRESULT GetPngLoader(svc_imageLoader **pngLoader);
	HRESULT GetLanguageManager(api_language **languageManager);
	HRESULT GetLanguageModule(HINSTANCE *module);
	HRESULT GetSecurityApi(JSAPI2::api_security **security);
	HRESULT GetDownloadManager(api_downloadManager **downloadManager);
	HRESULT GetThreadpoolApi(api_threadpool **threadpool);

public:
	DISPATCH_CODES
	{
		API_QUERYWASABIINTERFACE		= 10,
		API_RELEASEWASABIINTERFACE	= 20,
		API_GETSERVICEMANAGER		= 30,
		API_GETAPPLICATION			= 40,
		API_GETCONFIG				= 50, 
		API_GETMEMORYMANAGER			= 60,
		API_GETPNGLOADER			= 70,
		API_GETLANGUAGEMANAGER		= 80,
		API_GETLANGUAGEMODULE		= 90,
		API_GETSECURITY				= 100,
		API_GETDOWNLOADMANAGER		= 110,
		API_GETTHREADPOOL			= 120,
	};
};

inline HRESULT ifc_wasabihelper::QueryWasabiInterface(const GUID *iid, void **ppi)
{
	return _call(API_QUERYWASABIINTERFACE, (HRESULT)E_NOTIMPL, iid, ppi);
}

inline HRESULT ifc_wasabihelper::ReleaseWasabiInterface(const GUID *iid, void *pi)
{
	return _call(API_RELEASEWASABIINTERFACE, (HRESULT)E_NOTIMPL, iid, pi);
}

inline HRESULT ifc_wasabihelper::GetServiceManager(api_service **serviceManager)
{
	return _call(API_GETSERVICEMANAGER, (HRESULT)E_NOTIMPL, serviceManager);
}

inline HRESULT ifc_wasabihelper::GetApplicationApi(api_application **application)
{
	return _call(API_GETAPPLICATION, (HRESULT)E_NOTIMPL, application);
}

inline HRESULT ifc_wasabihelper::GetConfigApi(api_config ** config)
{
	return _call(API_GETCONFIG, (HRESULT)E_NOTIMPL, config);
}

inline HRESULT ifc_wasabihelper::GetMemoryManager(api_memmgr **memoryManager)
{
	return _call(API_GETMEMORYMANAGER, (HRESULT)E_NOTIMPL, memoryManager);
}

inline HRESULT ifc_wasabihelper::GetPngLoader(svc_imageLoader **pngLoader)
{
	return _call(API_GETPNGLOADER, (HRESULT)E_NOTIMPL, pngLoader);
}

inline HRESULT ifc_wasabihelper::GetLanguageManager(api_language **languageManager)
{
	return _call(API_GETLANGUAGEMANAGER, (HRESULT)E_NOTIMPL, languageManager);
}

inline HRESULT ifc_wasabihelper::GetLanguageModule(HINSTANCE *module)
{
	return _call(API_GETLANGUAGEMODULE, (HRESULT)E_NOTIMPL, module);
}

inline HRESULT ifc_wasabihelper::GetSecurityApi(JSAPI2::api_security **security)
{
	return _call(API_GETSECURITY, (HRESULT)E_NOTIMPL, security);
}

inline HRESULT ifc_wasabihelper::GetDownloadManager(api_downloadManager **downloadManager)
{
	return _call(API_GETDOWNLOADMANAGER, (HRESULT)E_NOTIMPL, downloadManager);
}

inline HRESULT ifc_wasabihelper::GetThreadpoolApi(api_threadpool **threadpool)
{
	return _call(API_GETTHREADPOOL, (HRESULT)E_NOTIMPL, threadpool);
}

#endif // NULLSOFT_WINAMP_WASABI_HELPER_INTERFACE_HEADER