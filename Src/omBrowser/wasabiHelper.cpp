#include "main.h"
#include "./wasabiHelper.h"
#include <api/service/waservicefactory.h>

EXTERN_C const GUID pngLoaderGUID = 
{ 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };

WasabiHelper::WasabiHelper(api_service *serviceManager) 
	: ref(1), serviceManager(NULL), applicationApi(NULL), configApi(NULL), memoryApi(NULL), 
	pngLoader(NULL), languageManager(NULL), securityApi(NULL), threadpoolApi(NULL), languageModule(NULL)
{
	this->serviceManager = serviceManager;
}

WasabiHelper::~WasabiHelper()
{
	ReleaseWasabiInterface(&applicationApiServiceGuid, applicationApi);
	ReleaseWasabiInterface(&AgaveConfigGUID, configApi);
	ReleaseWasabiInterface(&languageApiGUID, languageManager);
	ReleaseWasabiInterface(&pngLoaderGUID, pngLoader);
	ReleaseWasabiInterface(&memMgrApiServiceGuid, memoryApi);
	ReleaseWasabiInterface(&JSAPI2::api_securityGUID, securityApi);
	ReleaseWasabiInterface(&ThreadPoolGUID, threadpoolApi);
}

HRESULT WasabiHelper::CreateInstance(api_service *serviceManager, WasabiHelper **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == serviceManager) return E_INVALIDARG;

	*instance = new WasabiHelper(serviceManager);
	if (*instance == NULL) return E_OUTOFMEMORY;

	return S_OK;
}

size_t WasabiHelper::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t WasabiHelper::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int WasabiHelper::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_WasabiHelper))
		*object = static_cast<ifc_wasabihelper*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT WasabiHelper::QueryWasabiInterface(const GUID *iid, void **ppi)
{
	if (NULL == ppi) return E_POINTER;
	*ppi = NULL;

	if (NULL == iid) return E_INVALIDARG;

	if (NULL == serviceManager) return E_UNEXPECTED;
	waServiceFactory *serviceFactory = serviceManager->service_getServiceByGuid(*iid);
	if (NULL == serviceFactory)	return E_NOINTERFACE;

	*ppi = serviceFactory->getInterface();
	if (NULL == *ppi) return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT WasabiHelper::ReleaseWasabiInterface(const GUID *iid, void *pi)
{
	if (NULL == iid || NULL == pi) return E_INVALIDARG;

	if (NULL == serviceManager) return E_UNEXPECTED;
	waServiceFactory *serviceFactory = serviceManager->service_getServiceByGuid(*iid);
	if (NULL == serviceFactory)	return E_NOINTERFACE;

	serviceFactory->releaseInterface(pi);
	return S_OK;
}

HRESULT WasabiHelper::GetServiceManager(api_service **manager)
{
	if (NULL == manager) return E_POINTER;
	*manager = serviceManager;

	if (NULL == *manager) return E_UNEXPECTED;
	(*manager)->AddRef();

	return S_OK;
}

HRESULT WasabiHelper::GetApplicationApi(api_application **application)
{
	if (NULL == application) return E_POINTER;
	HRESULT hr = (NULL == applicationApi)? QueryWasabiInterface(&applicationApiServiceGuid, (void**)&applicationApi) : S_OK;

	*application = applicationApi;
	if (NULL != applicationApi)
		applicationApi->AddRef();

	return hr;
}

HRESULT WasabiHelper::GetConfigApi(api_config ** config)
{
	if (NULL == config) return E_POINTER;
	HRESULT hr = (NULL == configApi)? QueryWasabiInterface(&AgaveConfigGUID, (void**)&configApi) : S_OK;

	*config = configApi;
	if (NULL != configApi)
		configApi->AddRef();

	return hr;
}

HRESULT WasabiHelper::GetMemoryManager(api_memmgr **memoryManager)
{
	if (NULL == memoryManager) return E_POINTER;
	HRESULT hr = (NULL == memoryApi)? QueryWasabiInterface(&memMgrApiServiceGuid, (void**)&memoryApi) : S_OK;

	*memoryManager = memoryApi;
	if (NULL != memoryApi)
		memoryApi->AddRef();

	return hr;
}

HRESULT WasabiHelper::GetPngLoader(svc_imageLoader **loader)
{
	if (NULL == loader) return E_POINTER;
	HRESULT hr = (NULL == pngLoader)? QueryWasabiInterface(&pngLoaderGUID, (void**)&pngLoader) : S_OK;

	*loader = pngLoader;
	if (NULL != pngLoader)
		pngLoader->AddRef();

	return hr;
}

HRESULT WasabiHelper::InitLanguageSupport()
{	
	if (NULL != languageManager && NULL != languageModule)
		return S_FALSE;

	if(NULL == languageManager)
	{
		languageModule = NULL;
		HRESULT hr = QueryWasabiInterface(&languageApiGUID, (void**)&languageManager);
		if (FAILED(hr))
			return hr;
	}

	if (NULL == languageModule)
	{
		languageModule = languageManager->StartLanguageSupport(Plugin_GetInstance(), omBrowserLangGUID);
		if (NULL == languageModule)
		{
			languageManager->Release();
			languageManager = NULL;
			return E_FAIL;
		}
	}
	
	return S_OK;
}

HRESULT WasabiHelper::GetLanguageManager(api_language **language)
{
	if (NULL == language) return E_POINTER;

	HRESULT hr = InitLanguageSupport();
	if (FAILED(hr)) return hr;

	*language = (SUCCEEDED(hr)) ? languageManager : NULL;
	if (NULL != *language)
		(*language)->AddRef();

	return hr;
}

HRESULT WasabiHelper::GetLanguageModule(HINSTANCE *module)
{
	if (NULL == module) return E_POINTER;

	HRESULT hr = InitLanguageSupport();
	if (FAILED(hr)) return hr;

	*module = languageModule;
	return S_OK;
}

HRESULT WasabiHelper::GetSecurityApi(JSAPI2::api_security **security)
{
	if (NULL == security) return E_POINTER;
	HRESULT hr = (NULL == securityApi)? QueryWasabiInterface(&JSAPI2::api_securityGUID, (void**)&securityApi) : S_OK;

	*security = securityApi;
	if (NULL != securityApi)
		securityApi->AddRef();
	return hr;
}

HRESULT WasabiHelper::GetThreadpoolApi(api_threadpool **threadpool)
{
	if (NULL == threadpool) return E_POINTER;
	HRESULT hr = (NULL == threadpoolApi)? QueryWasabiInterface(&ThreadPoolGUID, (void**)&threadpoolApi) : S_OK;

	*threadpool = threadpoolApi;
	if (NULL != *threadpool)
		threadpoolApi->AddRef();
	return hr;
}

#define CBCLASS WasabiHelper
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_QUERYWASABIINTERFACE, QueryWasabiInterface)
CB(API_RELEASEWASABIINTERFACE, ReleaseWasabiInterface)
CB(API_GETSERVICEMANAGER, GetServiceManager)
CB(API_GETAPPLICATION, GetApplicationApi)
CB(API_GETCONFIG, GetConfigApi)
CB(API_GETMEMORYMANAGER, GetMemoryManager)
CB(API_GETPNGLOADER, GetPngLoader)
CB(API_GETLANGUAGEMANAGER, GetLanguageManager)
CB(API_GETLANGUAGEMODULE, GetLanguageModule)
CB(API_GETSECURITY, GetSecurityApi)
CB(API_GETTHREADPOOL, GetThreadpoolApi)
END_DISPATCH;
#undef CBCLASS