#include "main.h"
#include "./utility.h"
#include "./cacheManager.h"
#include "./mlNavigationHelper.h"
#include "./graphicsObject.h"
#include "./storageHelper.h"
#include "./wasabiHelper.h"

#define LISTENER_CLASS			L"Nullsoft_OmBrowserListener"
#define LWM_FIRST				(WM_USER + 11)
#define LWM_INVOKECALLBACK		(LWM_FIRST + 0)

#define ICBP_TYPE_DISPPARAM2	0x0001

struct INVOKECALLBACKPARAM
{
	INVOKECALLBACKPARAM() : type(0), callback(NULL) {}

	UINT type;
	void *callback;
};

struct ICBP_DISPPARAM2
{
	ICBP_DISPPARAM2() : object(NULL), param1(0), param2(0) {}

	INVOKECALLBACKPARAM header;
	Dispatchable *object;
	ULONG_PTR param1;
	ULONG_PTR param2;
};

static void CALLBACK InvokeCallback_MarshallingApc(ULONG_PTR data)
{
	INVOKECALLBACKPARAM *icbp = (INVOKECALLBACKPARAM*)data;
	if (NULL == icbp) return;

	switch(icbp->type)
	{
		case ICBP_TYPE_DISPPARAM2:
			if (NULL != icbp->callback)
			{
				ICBP_DISPPARAM2 *p = (ICBP_DISPPARAM2*)icbp;
				((ifc_omutility::ThreadCallback2)icbp->callback)(p->object, p->param1, p->param2);
				if (NULL != p->object) p->object->Release();
			}
			break;
	}

	free(icbp);
}

static LRESULT WINAPI Listener_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case LWM_INVOKECALLBACK:
			if (NULL != lParam)
				((ifc_omutility::ThreadCallback)lParam)((ULONG_PTR)wParam);
			return TRUE;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

OmUtility::OmUtility() 
	: ref(1), cacheManager(NULL), navigationHelper(NULL), graphicsObject(NULL), 
	  storageHelper(NULL), hListener(NULL)
{
	WNDCLASS listenerClass = {0};
	if (0 == GetClassInfo(Plugin_GetInstance(), LISTENER_CLASS, &listenerClass))
	{		
		ZeroMemory(&listenerClass, sizeof(listenerClass));
		listenerClass.hInstance = Plugin_GetInstance();
		listenerClass.lpfnWndProc = Listener_WindowProc;
		listenerClass.lpszClassName =LISTENER_CLASS;
		listenerClass.style = 0;
		RegisterClassW(&listenerClass);
	}

	hListener = CreateWindow(LISTENER_CLASS, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, Plugin_GetInstance(), NULL); 
}

OmUtility::~OmUtility()
{		
	if (NULL != navigationHelper) 
		navigationHelper->Release();

	if (NULL != cacheManager) 
		cacheManager->Release();

	if (NULL != graphicsObject)
		graphicsObject->Release();

	if (NULL != hListener)
		DestroyWindow(hListener);
	
	if (NULL != storageHelper)
		storageHelper->Release();
}

OmUtility *OmUtility::CreateInstance()
{
	return new OmUtility();
}

size_t OmUtility::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmUtility::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int OmUtility::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmUtility))
		*object = static_cast<ifc_omutility*>(this);
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

HRESULT OmUtility::EnsurePathExist(LPCWSTR pszDirectory)
{
	return Plugin_EnsurePathExist(pszDirectory);
}

HRESULT OmUtility::MakeResourcePath(LPWSTR pszBuffer, UINT cchBufferMax, HINSTANCE hInstance, LPCWSTR pszType, LPCWSTR pszName, UINT uFlags)
{
	return Plugin_MakeResourcePath(pszBuffer, cchBufferMax, hInstance, pszType, pszName, uFlags); 
}

HRESULT OmUtility::GetCacheManager(ifc_omcachemanager **managerOut)
{
	if (NULL == managerOut)
		return E_POINTER;

	if (NULL == cacheManager) 
	{
		HRESULT hr = CacheManager::CreateInstance(&cacheManager);
		if (FAILED(hr)) 
		{
			*managerOut = NULL;
			return hr;
		}
	}

	cacheManager->AddRef();
	*managerOut = cacheManager;
	return S_OK;
}

HRESULT OmUtility::GetMlNavigationHelper(HWND hLibrary, ifc_mlnavigationhelper **helper)
{
	if (NULL == helper)
		return E_POINTER;

	if (NULL == hLibrary)
	{
		*helper = NULL;
		return E_INVALIDARG;
	}

	if (NULL == navigationHelper) 
	{
		HRESULT hr;
		ifc_omcachemanager *cache = NULL;
		
		if (FAILED(GetCacheManager(&cache)))
		{
			hr = E_FAIL;
			cache = NULL;
		}
		else
		{
			hr = MlNavigationHelper::CreateInstance(hLibrary, cache, &navigationHelper);
		}
		
		if (NULL != cache) 
			cache->Release();

		if (FAILED(hr)) 
		{
			*helper = NULL;
			return hr;
		}
	}
	else
	{
		if (hLibrary != navigationHelper->GetLibrary())
			return E_UNEXPECTED;
	}
	
	navigationHelper->AddRef();
	*helper = navigationHelper;
	return S_OK;
}

HRESULT OmUtility::QueryImageLoader(HINSTANCE hInstance, LPCWSTR pszName, BOOL fPremultiply, ifc_omimageloader **imageLoader)
{
	return Plugin_QueryImageLoader(hInstance, pszName, fPremultiply, imageLoader);
}

HRESULT OmUtility::GetGraphics(ifc_omgraphics **graphics)
{
	if (NULL == graphics)
		return E_POINTER;

	if (NULL == graphicsObject)
	{
		HRESULT hr = GraphicsObject::CreateInstance(&graphicsObject);
		if (FAILED(hr)) return hr;
	}

	graphicsObject->AddRef();
	*graphics = graphicsObject;
	return S_OK;
}

HRESULT OmUtility::PostMainThreadCallback(ThreadCallback callback, ULONG_PTR data)
{
	if (NULL == callback) 
		return E_INVALIDARG;

	if (NULL != hListener && 
		FALSE != PostMessage(hListener, LWM_INVOKECALLBACK, (WPARAM)data, (LPARAM)callback))
	{
		return S_OK;
	}

	ifc_wasabihelper *wasabi = NULL;
	HRESULT hr = Plugin_GetWasabiHelper(&wasabi);
	if (SUCCEEDED(hr) && wasabi != NULL)
	{
		api_application *application = NULL;
		hr = wasabi->GetApplicationApi(&application);
		if (SUCCEEDED(hr) && application != NULL)
		{
			HANDLE hThread = application->main_getMainThreadHandle();
			if (NULL == hThread)
				hr = E_FAIL;
			else
			{
				DWORD r = QueueUserAPC((PAPCFUNC)callback, hThread, data);
				if (0 == r)
				{
					r = GetLastError();
					hr = HRESULT_FROM_WIN32(r);
				}
			}
			application->Release();
		}
		wasabi->Release();
	}
	return hr;
}

HRESULT OmUtility::PostMainThreadCallback2(ThreadCallback2 callback, Dispatchable *object, ULONG_PTR param1, ULONG_PTR param2)
{
	if (NULL == callback || NULL == object) 
		return E_INVALIDARG;

	ICBP_DISPPARAM2 *data = (ICBP_DISPPARAM2*)calloc(1, sizeof(ICBP_DISPPARAM2));
	if (NULL == data) return E_OUTOFMEMORY;

	data->header.type = ICBP_TYPE_DISPPARAM2;
	data->header.callback = callback;
	data->object = object;
	if (NULL != data->object)
		data->object->AddRef();

	data->param1 = param1;
	data->param2 = param2;

	HRESULT hr = PostMainThreadCallback(InvokeCallback_MarshallingApc, (ULONG_PTR)data);
	if (FAILED(hr))
	{
		if (NULL != data->object) 
			data->object->Release();

		free(data);
	}

	return hr;
}

HRESULT OmUtility::GetStorageHelper(ifc_omstoragehelper **helper)
{
	if (NULL == helper) 
		return E_POINTER;

	if (NULL == storageHelper)
	{
		HRESULT hr = StorageHelper::CreateInstance(&storageHelper);
		if (FAILED(hr))
		{
			*helper = NULL;
			return hr;
		}
	}

	*helper = storageHelper;
	storageHelper->AddRef();

	return S_OK;
}

#define CBCLASS OmUtility
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_ENSUREPATHEXIST, EnsurePathExist)
CB(API_MAKERESPATH, MakeResourcePath)
CB(API_GETCACHEMANAGER, GetCacheManager)
CB(API_GETMLNAVIGATIONHELPER, GetMlNavigationHelper)
CB(API_QUERYIMAGELOADER, QueryImageLoader)
CB(API_GETGRAPHICS, GetGraphics)
CB(API_POSTMAINTHREADCALLBACK, PostMainThreadCallback)
CB(API_POSTMAINTHREADCALLBACK2, PostMainThreadCallback2)
CB(API_GETSTORAGEHELPER, GetStorageHelper)
END_DISPATCH;
#undef CBCLASS