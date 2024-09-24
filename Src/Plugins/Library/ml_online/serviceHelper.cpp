#include "main.h"
#include "./serviceHelper.h"
#include "./navigation.h"
#include "./api__ml_online.h"
#include "./resource.h"
#include "./serviceHost.h"

#include <ifc_omservice.h>
#include <ifc_omfilestorage.h>
#include <ifc_omwebstorage.h>
#include <ifc_omserviceeditor.h>
#include <ifc_omserviceeventmngr.h>
#include <ifc_omservicecommand.h>
#include <ifc_omserviceenum.h>
#include <ifc_omxmlserviceenum.h>
#include <ifc_omservicecopier.h>
#include <ifc_mlnavigationhelper.h>
#include <browserUiCommon.h>

#include <vector>

#include <wininet.h>
#include <strsafe.h>

typedef std::vector<ifc_omstorageasync*> AsyncList;

struct SERVICEHELPER
{
	SERVICEHELPER() 
		: discoverAsync(NULL) 
	{}

	ifc_omstorageasync *discoverAsync;
	AsyncList			versionChecks;
	CRITICAL_SECTION	lock;
};

static SERVICEHELPER *serviceHelper = NULL;

static void ServiceHelper_Lock()
{
	if (NULL != serviceHelper)
		EnterCriticalSection(&serviceHelper->lock);
}

static void ServiceHelper_Unlock()
{
	if (NULL != serviceHelper)
		LeaveCriticalSection(&serviceHelper->lock);
}

static void CALLBACK ServiceHelper_Uninitialize()
{
	if (NULL == serviceHelper)
		return;
	
	ServiceHelper_Lock();

	ifc_omstorage *storage = NULL;;
	if (SUCCEEDED(ServiceHelper_QueryWebStorage(&storage)))
	{
		if (NULL != serviceHelper->discoverAsync)
		{
			storage->RequestAbort(serviceHelper->discoverAsync, TRUE);
			storage->EndLoad(serviceHelper->discoverAsync, NULL);
			serviceHelper->discoverAsync->Release();
			serviceHelper->discoverAsync = NULL;
		}

		size_t index = serviceHelper->versionChecks.size();
		while(index--)
		{
			ifc_omstorageasync *async = serviceHelper->versionChecks[index];
			storage->RequestAbort(async, TRUE);
			storage->EndLoad(async, NULL);
		}
		serviceHelper->versionChecks.clear();

		storage->Release();
	}

	ServiceHelper_Unlock();
	DeleteCriticalSection(&serviceHelper->lock);

	//free(serviceHelper);
	delete serviceHelper;
	serviceHelper = NULL;
}

HRESULT ServiceHelper_Initialize()
{
	if (NULL != serviceHelper)
		return S_FALSE;

	//serviceHelper = (SERVICEHELPER*)calloc(1, sizeof(SERVICEHELPER));
	serviceHelper = new SERVICEHELPER();

	if (NULL == serviceHelper) 
		return E_OUTOFMEMORY;
	InitializeCriticalSection(&serviceHelper->lock);

	Plugin_RegisterUnloadCallback(ServiceHelper_Uninitialize);

	return S_OK;
}

HRESULT ServiceHelper_QueryStorage(ifc_omstorage **storage)
{
	if (NULL == storage) return E_POINTER;
	
	if (NULL == OMSERVICEMNGR)
	{
		*storage = NULL;
		return E_UNEXPECTED;
	}

	return OMSERVICEMNGR->QueryStorage(&SUID_OmStorageIni, storage);
}

HRESULT ServiceHelper_QueryWebStorage(ifc_omstorage **storage)
{
	if (NULL == storage) return E_POINTER;
	
	if (NULL == OMSERVICEMNGR)
	{
		*storage = NULL;
		return E_UNEXPECTED;
	}

	return OMSERVICEMNGR->QueryStorage(&SUID_OmStorageUrl, storage);
}

HRESULT ServiceHelper_Create(UINT serviceId, LPCWSTR pszName, LPCWSTR pszIcon, LPCWSTR pszUrl, UINT flags, UINT generation, BOOL fSave, ifc_omservice **serviceOut)
{
	if (NULL == serviceOut) 
		return E_POINTER;

	*serviceOut = NULL;

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;
	
	ServiceHost *serviceHost;
	if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
		serviceHost = NULL;

	HRESULT hr = OMSERVICEMNGR->CreateService(serviceId, serviceHost, serviceOut);
	if (SUCCEEDED(hr))
	{
		ifc_omserviceeditor *editor;
		if (SUCCEEDED((*serviceOut)->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
		{
			WCHAR szBuffer[4096] = {0};
			editor->BeginUpdate();
			if (NULL != pszName && IS_INTRESOURCE(pszName))
			{
				if (NULL != WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszName, szBuffer, ARRAYSIZE(szBuffer)))
					editor->SetName(szBuffer, FALSE);
			}
			else
				editor->SetName(pszName, FALSE);
			
			if (NULL != pszIcon && IS_INTRESOURCE(pszIcon))
			{
				if (SUCCEEDED(Plugin_MakeResourcePath(szBuffer, ARRAYSIZE(szBuffer), RT_RCDATA, pszIcon, RESPATH_COMPACT)))
					editor->SetIcon(szBuffer, FALSE);
			}
			else
				editor->SetIcon(pszIcon, FALSE);
			
			if (NULL != pszUrl && IS_INTRESOURCE(pszUrl))
			{
				if (SUCCEEDED(Plugin_MakeResourcePath(szBuffer, ARRAYSIZE(szBuffer), RT_HTML, pszUrl, RESPATH_TARGETIE | RESPATH_COMPACT)))
					editor->SetUrl(szBuffer, FALSE);
			}
			else
				editor->SetUrl(pszUrl, FALSE);
			
			editor->SetGeneration(generation);
			
			editor->SetFlags(flags, 0xFFFFFFFF);

			if (FALSE != fSave)
			{
				hr = ServiceHelper_Save(*serviceOut);
				if (FAILED(hr))
				{
					(*serviceOut)->Release();
					*serviceOut = NULL;
				}
			}
			else
			{
				editor->SetModified(0, (UINT)-1);
			}
			editor->EndUpdate();
			editor->Release();
		}
	}

	if (NULL != serviceHost)
		serviceHost->Release();

	return hr;
}

HRESULT ServiceHelper_Save(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;
	
	if (S_OK == ServiceHelper_IsSpecial(service))
		return S_FALSE;

	HRESULT hr;
	ifc_omstorage *storage;
	hr = ServiceHelper_QueryStorage(&storage);
	if (SUCCEEDED(hr))
	{	
		
		UINT serviceFlags;
		if (SUCCEEDED(service->GetFlags(&serviceFlags)) && 0 != (SVCF_AUTOUPGRADE & serviceFlags))
		{
			hr = storage->Save(&service, 1, ifc_omstorage::saveClearModified, NULL);
			if (SUCCEEDED(hr))
			{
				ifc_omserviceeditor *editor;
				if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
				{
					editor->BeginUpdate();
					editor->SetFlags(0, SVCF_AUTOUPGRADE);
					editor->SetModified(0, ifc_omserviceeditor::modifiedFlags);
					editor->EndUpdate();
					editor->Release();
				}
			}
		}
		else
		{
			hr = storage->Save(&service, 1, ifc_omstorage::saveClearModified | ifc_omstorage::saveModifiedOnly, NULL);
		}

		storage->Release();
	}
	return hr;
}

HRESULT ServiceHelper_Delete(ifc_omservice *service, UINT flags)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;

	if (S_OK == ServiceHelper_IsSpecial(service))
		return S_FALSE;
	
	HRESULT hr;

	if (S_OK == ServiceHelper_IsSubscribed(service))
	{
		hr = ServiceHelper_Subscribe(service, FALSE, ((SHF_NOTIFY | SHF_VERBAL) & flags));
		if (S_OK != hr) return hr;
	}

	ifc_omstorage *storage = NULL;
	hr = ServiceHelper_QueryStorage(&storage);
	if (SUCCEEDED(hr) && storage)
	{
		hr = storage->Delete(&service, 1, NULL);
		storage->Release();
	}
	return hr;
}

HRESULT ServiceHelper_SetFlags(ifc_omservice *service, UINT flags, UINT flagsMask)
{
	if (NULL ==service) 
		return E_INVALIDARG;

	UINT serviceFlags;
	HRESULT hr = service->GetFlags(&serviceFlags);
	if (FAILED(hr)) return hr;

	if ((flags & flagsMask) == (serviceFlags & flagsMask))
		return S_FALSE;

	ifc_omserviceeditor *editor;
	hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
	if (SUCCEEDED(hr))
	{
		editor->SetFlags(flags, flagsMask);
		editor->Release();
	}

	return hr;
}
HRESULT ServiceHelper_IsSpecial(ifc_omservice *service)
{	
	if (NULL == service) return E_INVALIDARG;
	
	UINT flags;
	HRESULT hr = service->GetFlags(&flags);
	if (FAILED(hr)) return hr;

	return (0 != (SVCF_SPECIAL & flags)) ? S_OK : S_FALSE;
}

HRESULT ServiceHelper_IsSubscribed(ifc_omservice *service)
{
	if (NULL == service) return E_INVALIDARG;

	//return S_OK;

	UINT flags;
	HRESULT hr = service->GetFlags(&flags);
	if (FAILED(hr)) return hr;

	return (0 != (SVCF_SUBSCRIBED & flags)) ? S_OK : S_FALSE;
}

HRESULT ServiceHelper_IsModified(ifc_omservice *service)
{
	if (NULL == service) return E_INVALIDARG;

	HRESULT hr;
	ifc_omserviceeditor *editor;
	hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
	if (SUCCEEDED(hr))
	{
		UINT modified;
		hr = editor->GetModified(&modified);
		if (SUCCEEDED(hr))
		{
			hr = (0 == modified) ? S_FALSE : S_OK;
		}
		editor->Release();
	}

	return hr;
}

HRESULT ServiceHelper_MarkModified(ifc_omservice *service, UINT modifiedFlag, UINT modifiedMask)
{
	if (NULL == service)
		return E_INVALIDARG;

	HRESULT hr;
	ifc_omserviceeditor *editor;
	hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
	if (SUCCEEDED(hr))
	{
		hr = editor->SetModified(modifiedFlag, modifiedMask);
		editor->Release();
	}
	return hr;
}
HRESULT ServiceHelper_Load(ifc_omserviceenum **enumerator)
{
	if (NULL == enumerator)
		return E_POINTER;

	HRESULT hr;
	ifc_omstorage *storage;
	hr = ServiceHelper_QueryStorage(&storage);
	if (SUCCEEDED(hr))
	{
		ServiceHost *serviceHost;
		if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
			serviceHost = NULL;

		hr = storage->Load(L"*.ini", serviceHost, enumerator);

		if (NULL != serviceHost)
			serviceHost->Release();

		storage->Release();
	}

	if (FAILED(hr))
		*enumerator = NULL;

	return hr;
}

HRESULT ServiceHelper_UpdateIcon(ifc_omserviceeditor *editor, LPCWSTR pszImage)
{
	WCHAR szBuffer[8192];
	szBuffer[0] = L'\0';

	if (NULL == editor) 
		return E_INVALIDARG;

	ifc_omservice *service;
	if (SUCCEEDED(editor->QueryInterface(IFC_OmService, (void**)&service)))
	{
		if (FAILED(service->GetIcon(szBuffer, ARRAYSIZE(szBuffer))))
			szBuffer[0] = L'\0';
		service->Release();
	}

	HRESULT hr = editor->SetIcon(pszImage, FALSE);
	if (FAILED(hr) || S_FALSE == hr) return hr;

	if (L'\0' != szBuffer)
	{
		ifc_mlnavigationhelper *navHelper;
		if (SUCCEEDED(OMUTILITY->GetMlNavigationHelper(Plugin_GetLibrary(), &navHelper)))
		{		
			navHelper->ReleaseIndex(szBuffer);
			navHelper->Release();
		}
	}

	return S_OK;
}

HRESULT ServiceHelper_Find(UINT serviceId, ifc_omservice **serviceOut)
{
	if (NULL == serviceOut) return E_POINTER;
	*serviceOut = NULL;

	if (0 == serviceId) 
		return E_INVALIDARG;


	Navigation *navigation;
	if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		HNAVITEM hItem = navigation->FindService(serviceId, serviceOut);
		navigation->Release();
		if (NULL != hItem)
			return S_OK;
	}

	ifc_omserviceenum *enumerator;
	HRESULT hr = ServiceHelper_Load(&enumerator);
	if (SUCCEEDED(hr))
	{
		ifc_omservice *service;
		hr = S_FALSE;
		while (S_OK == enumerator->Next(1, &service, NULL))
		{
			if (service->GetId() == serviceId)
			{
				*serviceOut = service;
				hr = S_OK;
				break;
			}
			service->Release();
		}
		enumerator->Release();
	}
	return hr;
}

HRESULT ServiceHelper_SetRating(ifc_omservice *service, UINT rating, UINT flags)
{
	if (NULL == service) 
		return E_POINTER;
	
	ifc_omserviceeditor *editor;
	HRESULT hr;
	hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
	if (SUCCEEDED(hr))
	{
		hr = editor->SetRating(rating);
		if (S_OK == hr)
		{
			if (0 != (SHF_NOTIFY & flags))
			{
				// send notification

				WCHAR szUrl[2048] = {0}, szClient[128] = {0};
				if (NULL == OMBROWSERMNGR || FAILED(OMBROWSERMNGR->GetClientId(szClient, ARRAYSIZE(szClient))))
					szClient[0] = L'\0';

				hr = StringCchPrintf(szUrl, ARRAYSIZE(szUrl), 
							L"http://services.winamp.com/svc/rating?svc_id=%u&unique_id=%s&rating=%d", 
							service->GetId(), szClient, rating*2);

				if (SUCCEEDED(hr))
				{
					hr = ServiceHelper_PostNotificationUrl(szUrl);
				}
				
			}

			if (0 != (SHF_SAVE & flags))
				ServiceHelper_Save(service);
				
		}
		editor->Release();
	}

	return hr;
}

HRESULT ServiceHelper_PostNotificationUrl(LPCWSTR pszUrl)
{
	HRESULT hr;
	ifc_omstorage *storage;
	hr = ServiceHelper_QueryWebStorage(&storage);
	if (SUCCEEDED(hr))
	{
		ifc_omstorageasync *async;
		hr = storage->BeginLoad(pszUrl, NULL, NULL, NULL, &async);
        if(SUCCEEDED(hr))
		{
			async->Release();
		}
		storage->Release();
	}

	return hr;
}
HRESULT ServiceHelper_ResetPermissions(ifc_omservice *service, UINT flags)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (0 != (SHF_VERBAL & flags))
	{
		WCHAR szText[1024] = {0}, szFormat[512] = {0}, szName[128] = {0};

		service->GetName(szName, ARRAYSIZE(szName));
		WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGEBOX_POLICYRESET, szFormat, ARRAYSIZE(szFormat));
		StringCchPrintf(szText, ARRAYSIZE(szText), szFormat, szName);
		
		if (IDNO == Plugin_MessageBox(szText, MAKEINTRESOURCE(IDS_MESSAGEBOX_POLICYRESET_CAPTION), 
							MB_ICONQUESTION | MB_YESNO))
		{
			return S_FALSE;
		}
	}

	if (NULL == AGAVE_API_JSAPI2_SECURITY)
		return E_UNEXPECTED;
	
	WCHAR szBuffer[64] = {0};
	if (FAILED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%u", service->GetId())))
		return E_FAIL;

	AGAVE_API_JSAPI2_SECURITY->ResetAuthorization(szBuffer);
	return S_OK;
}

HRESULT ServiceHelper_IsPreAuthorized(ifc_omservice *service)
{
	if (NULL == service)
		return E_INVALIDARG;

	UINT serviceFlags;
	HRESULT hr = service->GetFlags(&serviceFlags);
	if (SUCCEEDED(hr))
	{
		hr = (0 != (SVCF_PREAUTHORIZED & serviceFlags)) ? S_OK : S_FALSE;
	}

	return hr;
}

HRESULT ServiceHelper_Subscribe(ifc_omservice *service, BOOL subscribe, UINT flags)
{
	if (NULL == service)
		return E_POINTER;
	
	if ((TRUE == subscribe) == (S_OK == ServiceHelper_IsSubscribed(service)))
		return S_FALSE;
	
	HRESULT hr;

	if (FALSE == subscribe && 0 != (SHF_VERBAL & flags))
	{
		WCHAR szText[1024] = {0}, szFormat[512] = {0}, szName[128] = {0};
		if (FAILED(service->GetName(szName, ARRAYSIZE(szName))))
			szName[0] = L'\0';

		WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGEBOX_UNSUBSCRIBE, szFormat, ARRAYSIZE(szFormat));
		StringCchPrintf(szText,ARRAYSIZE(szText), szFormat, szName);
		
		if (IDNO == Plugin_MessageBox(szText, MAKEINTRESOURCE(IDS_MESSAGEBOX_UNSUBSCRIBE_CAPTION), 
						MB_ICONQUESTION | MB_YESNO))
		{
			return S_FALSE;
		}
	}
	
	ifc_omserviceeditor *editor = NULL;
	hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
	if (SUCCEEDED(hr) && editor)
	{
		hr = editor->SetFlags((FALSE == subscribe) ? 0 : SVCF_SUBSCRIBED, SVCF_SUBSCRIBED);
		if (S_OK == hr)
		{
			if (0 != (SHF_SAVE & flags))
			 ServiceHelper_Save(service);
		}
		editor->Release();

		if (FALSE == subscribe)
			ServiceHelper_Delete(service, 0);
	}

	if (S_OK == hr)
	{
		if (NULL == AGAVE_API_JSAPI2_SECURITY)
		{
			WCHAR szBuffer[64] = {0};
			if (SUCCEEDED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%u", service->GetId())))
			{
				AGAVE_API_JSAPI2_SECURITY->ResetAuthorization(szBuffer);
			}
		}

		if (0 != (SHF_NOTIFY & flags))
		{
			LPWSTR pszUrl;
			UINT serviceId = service->GetId();
			LPCWSTR action = (FALSE != subscribe) ? L"add" : L"remove";;
			if (NULL != action && SUCCEEDED(Plugin_BuildActionUrl(&pszUrl, action, &serviceId, 1)))
			{
				hr = ServiceHelper_PostNotificationUrl(pszUrl);
				Plugin_FreeString(pszUrl);
			}
		}
	}

	return hr;
}

HRESULT ServiceHelper_ResetSubscription(UINT flags)
{
	if (S_OK == ServiceHelper_IsDiscovering())
	{
		if (0 != (SHF_VERBAL & flags))
		{
			Plugin_MessageBox(MAKEINTRESOURCE(IDS_MESSAGEBOX_DISCOVERBUSY), 
						MAKEINTRESOURCE(IDS_MESSAGEBOX_DISCOVERBUSY_CAPTION), 
						MB_ICONASTERISK | MB_OK);
		}

		return E_PENDING;
	}

	if (0 != (SHF_VERBAL & flags))
	{
		if (IDNO == Plugin_MessageBox(MAKEINTRESOURCE(IDS_MESSAGEBOX_RESETTODEFAULT), 
						MAKEINTRESOURCE(IDS_MESSAGEBOX_RESETTODEFAULT_CAPTION), 
						MB_ICONQUESTION | MB_YESNO))
		{
			return S_FALSE;
		}
	}
	
	HRESULT hr;

	Navigation *navigation;
	if (SUCCEEDED(Plugin_GetNavigation(&navigation)))
	{
		navigation->DeleteAll();
		navigation->Release();
	}

	ifc_omstorage *storage;
	hr = ServiceHelper_QueryStorage(&storage);
	if (SUCCEEDED(hr))
	{
		ifc_omserviceenum *enumerator;
		hr = ServiceHelper_Load(&enumerator);
		if (SUCCEEDED(hr))
		{
			ifc_omservice *service;
			std::vector<UINT> removeList;
			while(S_OK == enumerator->Next(1, &service, NULL))
			{
				if (S_OK == ServiceHelper_IsSubscribed(service))
					removeList.push_back(service->GetId());

				ServiceHelper_ResetPermissions(service, 0);
				storage->Delete(&service, 1, NULL);
				service->Release();
			}
			enumerator->Release();

			if (0 != removeList.size())
			{
				LPWSTR pszUrl;
				if (SUCCEEDED(Plugin_BuildActionUrl(&pszUrl, L"remove", removeList.data(), removeList.size())))
				{
					ServiceHelper_PostNotificationUrl(pszUrl);
					Plugin_FreeString(pszUrl);
				}
			}
		}
		storage->Release();
	}
	
	if (SUCCEEDED(hr))
	{
		// discover new services
		hr = ServiceHelper_BeginDiscover(L"http://services.winamp.com/svc/default.php");
	}

	return hr;
}

HRESULT ServiceHelper_IsDiscovering()
{
	HRESULT hr;
	ServiceHelper_Lock();

	if (NULL == serviceHelper)
	{
		hr = E_UNEXPECTED;
	}
	else
	{
		hr = (NULL != serviceHelper->discoverAsync) ? S_OK : S_FALSE;
	}

	ServiceHelper_Unlock();

	return hr;
}

static void CALLBACK ServiceHelper_DiscoverComplete(ifc_omstorageasync *result)
{
	if (NULL == result)
		return;

	SERVICEHELPER *helper;
	if (FAILED(result->GetData((void**)&helper)) || NULL == helper)
		return;

	ServiceHelper_Lock();

	ifc_omstorage *webStorage;
	HRESULT hr = ServiceHelper_QueryWebStorage(&webStorage);
	if (SUCCEEDED(hr))
	{
		ifc_omserviceenum *serviceEnum;
		if (SUCCEEDED(webStorage->EndLoad(result, &serviceEnum)))
		{
			Navigation *navigation;
			if (FAILED(Plugin_GetNavigation(&navigation)))
				navigation = NULL;

			ifc_omstorage *localStorage;
			if (SUCCEEDED(ServiceHelper_QueryStorage(&localStorage)))
			{
				std::vector<UINT> registerList;

				ifc_omservice *service;
				while(S_OK == serviceEnum->Next(1, &service, NULL))
				{
					ifc_omserviceeditor *editor;
					if (SUCCEEDED(service->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
					{
						if (SUCCEEDED(editor->SetFlags(SVCF_SUBSCRIBED, SVCF_SUBSCRIBED)))
							registerList.push_back(service->GetId());

						editor->Release();
					}

					if (SUCCEEDED(localStorage->Save(&service, 1, ifc_omstorage::saveClearModified, NULL)))
					{
						navigation->CreateItemAsync(service);
					}

					service->Release();
				}

				if (0 != registerList.size())
				{
					LPWSTR pszUrl;
					if (SUCCEEDED(Plugin_BuildActionUrl(&pszUrl, L"add", registerList.data(), registerList.size())))
					{
						ServiceHelper_PostNotificationUrl(pszUrl);
						Plugin_FreeString(pszUrl);
					}
				}
			}

			if (NULL != navigation)
				navigation->Release();

			serviceEnum->Release();
		}
		webStorage->Release();
	}


	if (serviceHelper->discoverAsync == result)
	{
		serviceHelper->discoverAsync->Release();
		serviceHelper->discoverAsync = NULL;
	}
	
	ServiceHelper_Unlock();
}

HRESULT ServiceHelper_BeginDiscover(LPCWSTR address)
{
	ifc_omstorage *storage;
	HRESULT hr = ServiceHelper_QueryWebStorage(&storage);
	if (FAILED(hr)) return hr;
	
	ServiceHost *serviceHost;
	if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
		serviceHost = NULL;
	
	ServiceHelper_Lock();	
	
	if (NULL == serviceHelper->discoverAsync)
		hr = storage->BeginLoad(address, serviceHost, ServiceHelper_DiscoverComplete, serviceHelper, &serviceHelper->discoverAsync);
	else
		hr = E_PENDING;

	ServiceHelper_Unlock();

	storage->Release();

	if (NULL != serviceHost)
		serviceHost->Release();

	return hr;
}
HRESULT ServiceHelper_GetDetailsUrl(LPWSTR pszBuffer, UINT cchBufferMax, ifc_omservice *service, BOOL fLite)
{
	if (NULL == pszBuffer) 
		return E_POINTER;

	if (0 == service)
		return E_INVALIDARG;

	HRESULT hr;
	size_t remaining = cchBufferMax;
	LPWSTR cursor = pszBuffer;

	hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
					L"http://services.winamp.com/svc/details?svc_id=%u", service->GetId());
	
	WCHAR szClient[128] = {0};
	if (SUCCEEDED(hr) && 
		NULL != OMBROWSERMNGR && 
		SUCCEEDED(OMBROWSERMNGR->GetClientId(szClient, ARRAYSIZE(szClient))) &&
		L'\0' != szClient[0])
	{
		hr = StringCchPrintfEx(cursor, remaining, &cursor, &remaining, STRSAFE_NULL_ON_FAILURE, 
			L"&unique_id=%s", szClient);
	}

	if (FALSE != fLite && SUCCEEDED(hr))
	{
		hr = StringCchCopyEx(cursor, remaining, L"&detail=lite", &cursor, &remaining, STRSAFE_NULL_ON_FAILURE);
	}
	return hr;
}
static void CALLBACK ServiceHelper_VersionCheckComplete(ifc_omstorageasync *result)
{
	if (NULL == result)
		return;

	ifc_omservice *target;
	if (FAILED(result->GetData((void**)&target)) || NULL == target)
		return;
	
	ServiceHelper_Lock();

	size_t index = serviceHelper->versionChecks.size();
	while(index--)
	{
		if (result == serviceHelper->versionChecks[index])
		{
			auto it = serviceHelper->versionChecks.begin() + index;
			if (it < serviceHelper->versionChecks.end())
			{
				it = serviceHelper->versionChecks.erase(it);
			}
			break;
		}
	}

	ServiceHelper_Unlock();

	UINT originalFlags;
	if (FAILED(target->GetFlags(&originalFlags)))
		originalFlags = 0;

	ifc_omserviceeditor *editor;
	if (SUCCEEDED(target->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
		editor->BeginUpdate();
	else
		editor = NULL;
	

	ifc_omstorage *storage;
	HRESULT hr = ServiceHelper_QueryWebStorage(&storage);
	if (SUCCEEDED(hr))
	{
		ifc_omserviceenum *serviceEnum;
		if (SUCCEEDED(storage->EndLoad(result, &serviceEnum)))
		{
			ifc_omservice *source = NULL;
			while(S_OK == serviceEnum->Next(1, &source, NULL))
			{
				if (source->GetId() == target->GetId())
					break;
				
				source->Release();
				source = NULL;
			}

			ifc_omxmlserviceenum *xmlServiceEnum;
			if (SUCCEEDED(serviceEnum->QueryInterface(IFC_OmXmlServiceEnum, (void**)&xmlServiceEnum)))
			{
				UINT statusCode;
				if (SUCCEEDED(xmlServiceEnum->GetStatusCode(&statusCode)))
				{
					switch(statusCode)
					{
						case 410:	// force remove
							if (NULL != target)
							{
								WCHAR szText[1024] = {0}, szFormat[512] = {0}, szName[128] = {0};
								target->GetName(szName, ARRAYSIZE(szName));
								WASABI_API_LNGSTRINGW_BUF(IDS_MESSAGEBOX_FORCEREMOVE, szFormat, ARRAYSIZE(szFormat));
								StringCchPrintf(szText,ARRAYSIZE(szText), szFormat, szName);
								Plugin_MessageBox(szText, MAKEINTRESOURCE(IDS_MESSAGEBOX_FORCEREMOVE_CAPTION), MB_ICONINFORMATION | MB_OK);
								ServiceHelper_Delete(target, 0);
								target->Release();
								target = NULL;
							}
							break;

						case 200:
							if (NULL != source)
							{
								ifc_omservicecopier *copier;
								if (SUCCEEDED(source->QueryInterface(IFC_OmServiceCopier, (void**)&copier)))
								{
									UINT targetFlags, sourceFlags;
									if (FAILED(target->GetFlags(&targetFlags)))	targetFlags = 0;
									if (SUCCEEDED(source->GetFlags(&sourceFlags)))
										targetFlags |= sourceFlags;

									copier->CopyTo(target, NULL);
									copier->Release();
									
									editor->SetFlags(targetFlags, targetFlags);
								}

							}
							break;
					}
				}
				xmlServiceEnum->Release();
			}

			if (NULL != source)
				source->Release();

			serviceEnum->Release();
		}
		storage->Release();
	}
	
	if (NULL != target && NULL != AGAVE_API_JSAPI2_SECURITY)
	{									
		WCHAR szBuffer[64] = {0};
		if (SUCCEEDED(StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"%u", target->GetId())))
		{
			UINT flags = 0;
			if (SUCCEEDED(target->GetFlags(&flags)))
			{
				bool bypassEnabled = (0 != (SVCF_PREAUTHORIZED & flags));
				AGAVE_API_JSAPI2_SECURITY->SetBypass(szBuffer, bypassEnabled);
			}
		}
	}

	if (NULL != editor)
	{			
		editor->SetFlags(SVCF_VALIDATED, SVCF_VALIDATED | SVCF_VERSIONCHECK);

		if (NULL != target)
		{
			UINT targetFlags;
			if (FAILED(target->GetFlags(&targetFlags)))	
				targetFlags = 0;

			if ((targetFlags & ~ifc_omservice::RuntimeFlagsMask) == (originalFlags & ~ifc_omservice::RuntimeFlagsMask))
				editor->SetModified(0, ifc_omserviceeditor::modifiedFlags);
		}
		editor->EndUpdate();
		editor->Release();
	}

	if (NULL != target)
	{
		ifc_omserviceeventmngr *eventManager;
		if (SUCCEEDED(target->QueryInterface(IFC_OmServiceEventMngr, (void**)&eventManager)))
		{
			eventManager->Signal_CommandStateChange(&CMDGROUP_SERVICE, SVCCOMMAND_BLOCKNAV);
			eventManager->Release();
		}
		
		ServiceHelper_Save(target);
		target->Release();
	}

}

HRESULT ServiceHelper_BeginVersionCheck(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (NULL == serviceHelper)
		return E_UNEXPECTED;
	
	HRESULT hr = S_OK;
	ServiceHelper_Lock();
	size_t index = serviceHelper->versionChecks.size();
	while(index--)
	{
		ifc_omstorageasync *async = serviceHelper->versionChecks[index];
		UINT serviceId;
		if (SUCCEEDED(async->GetData((void**)&serviceId)))
		{
			if (serviceId == service->GetId())
			{
				hr = S_FALSE;
			}
		}
	}

	if (S_OK == hr)
	{
		ifc_omstorage *storage;
		HRESULT hr = ServiceHelper_QueryWebStorage(&storage);
		if (SUCCEEDED(hr))
		{
			WCHAR szAddress[2048] = {0};
			hr = ServiceHelper_GetDetailsUrl(szAddress, ARRAYSIZE(szAddress), service, TRUE);
			if (SUCCEEDED(hr))
			{
				ServiceHost *serviceHost;
				if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
				serviceHost = NULL;

				ifc_omserviceeditor *editor;
				if (FAILED(service->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
					editor = NULL;
				else
				{
					editor->BeginUpdate();
					editor->SetFlags(SVCF_VERSIONCHECK, SVCF_VERSIONCHECK);
				}
				service->AddRef();
				serviceHelper->versionChecks.push_back(NULL);
				//ifc_omstorageasync **pAsync = serviceHelper->versionChecks.end() - 1;
				ifc_omstorageasync** pAsync = (serviceHelper->versionChecks.size() ? 
												&serviceHelper->versionChecks.at(serviceHelper->versionChecks.size() - 1) : 0);
				hr = storage->BeginLoad(szAddress, serviceHost, ServiceHelper_VersionCheckComplete, service, pAsync);
				if (FAILED(hr))
				{
					service->Release();
					serviceHelper->versionChecks.pop_back();
					if (NULL != editor) 
						editor->SetFlags(0, SVCF_VERSIONCHECK);
				}

				if (NULL != serviceHost) 
					serviceHost->Release();

				if (NULL != editor)
				{
					editor->EndUpdate();
					editor->Release();
				}
			}
			storage->Release();
		}
		
	}

	ServiceHelper_Unlock();

	return hr;
}
static void CALLBACK ServiceHelper_UpdateOperationInfoApc(ULONG_PTR param)
{
	HWND hBrowser = (HWND)param;
	if (NULL != hBrowser && IsWindow(hBrowser))
	{
		ServiceHelper_UpdateOperationInfo(hBrowser);
	}
}

HRESULT ServiceHelper_UpdateOperationInfo(HWND hBrowser)
{
	if (NULL == hBrowser) 
		return E_INVALIDARG;

	DWORD currentTID = GetCurrentThreadId();
	DWORD winampTID = GetWindowThreadProcessId(Plugin_GetWinamp(), NULL);
	if (NULL != winampTID && winampTID != currentTID)
	{
		HRESULT hr;
		if (NULL != OMUTILITY)
			hr = OMUTILITY->PostMainThreadCallback(ServiceHelper_UpdateOperationInfoApc, (ULONG_PTR)hBrowser);
		else 
			hr = E_FAIL;
		return hr;
	}

	ifc_omservice *service;
	if (FALSE == BrowserControl_GetService(hBrowser, &service))
		return E_FAIL;

	UINT flags;
	if (SUCCEEDED(service->GetFlags(&flags)))
	{
		WCHAR szText[128] = {0}, szTitle[128] = {0};
		OPERATIONINFO operation = {0};
		operation.cbSize = sizeof(operation);
		operation.mask = NBCOM_FLAGS;

		if (0 == (SVCF_VERSIONCHECK & flags))
		{
			operation.flags = NBCOF_HIDEWIDGET;
		}
		else
		{
			operation.mask |= (NBCOM_TITLE | NBCOM_TEXT);
			operation.flags = NBCOF_SHOWWIDGET;

			WASABI_API_LNGSTRINGW_BUF(IDS_SERVICE_CHECKINGVERSION, szTitle, ARRAYSIZE(szTitle));
			operation.title = szTitle;

			WASABI_API_LNGSTRINGW_BUF(IDS_PLEASE_WAIT, szText, ARRAYSIZE(szText));
			operation.text = szText;
		}
		BrowserControl_ShowOperation(hBrowser, &operation);
	}
	service->Release();

	return S_OK;
}
static void CALLBACK ServiceHelper_ShowWindowTimer(UINT_PTR eventId, DWORD elapsedMs, ULONG_PTR data)
{
	Plugin_KillTimer(eventId);
	HWND hTarget = (HWND)data;

	if (hTarget == Plugin_GetLibrary())
		PostMessage(hTarget, WM_ML_IPC, 0, ML_IPC_ENSURE_VISIBLE);
	else
		ShowWindow(hTarget, SW_SHOWNORMAL);
}

HRESULT ServiceHelper_ShowService(UINT serviceId, UINT showMode)
{
	HRESULT hr;
	
	BOOL serviceFound = FALSE;
	ifc_omservice *service;
	if (0 != serviceId &&  ROOTSERVICE_ID != serviceId &&
		S_OK == ServiceHelper_Find(serviceId, &service) &&
		NULL != service)
	{
		if (S_OK == ServiceHelper_IsSubscribed(service))
			serviceFound = TRUE;

		service->Release();
	}

	WCHAR szBuffer[INTERNET_MAX_URL_LENGTH] = {0};
	LPCWSTR pszUrl = NULL;

	if (FALSE == serviceFound)
	{
		if (0 != serviceId && ROOTSERVICE_ID != serviceId &&
			SUCCEEDED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), 
						L"http://client.winamp.com/service/detail/gallery/%u", serviceId)))
		{
			pszUrl = szBuffer;
		}
		serviceId = ROOTSERVICE_ID;
	}
	
	Navigation *navigation;
	hr = Plugin_GetNavigation(&navigation);
	if (FAILED(hr)) return hr;
	
	if (SHOWMODE_POPUP == showMode)
	{
		showMode = SHOWMODE_ENSUREVISIBLE;
		if (FALSE != serviceFound)
		{
			HNAVITEM hItem = navigation->FindService(serviceId, NULL);
			if (NULL != hItem)
			{
				HWND hPopup;
				hr = navigation->CreatePopup(hItem, &hPopup);
				if (SUCCEEDED(hr))
				{	
					Plugin_SetTimer(100, ServiceHelper_ShowWindowTimer, (ULONG_PTR)hPopup);
					showMode = SHOWMODE_POPUP;
				}
			}
		}
	}

	if (SHOWMODE_POPUP != showMode)
	{
		hr = navigation->ShowService(serviceId, pszUrl);
		if (SUCCEEDED(hr))
		{
			if (SHOWMODE_ENSUREVISIBLE == showMode)
				Plugin_SetTimer(100, ServiceHelper_ShowWindowTimer, (ULONG_PTR)Plugin_GetLibrary());
		}
	}

	navigation->Release();
	return hr;
}