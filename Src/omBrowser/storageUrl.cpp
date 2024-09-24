#include "main.h"
#include "./storageUrl.h"
#include "./ifc_omwebstorage.h"
#include "./resource.h"
#include "./storageDwnld.h"
#include "./ifc_omservice.h"
#include "./ifc_omserviceenum.h"
#include "./ifc_omservicehost.h"
#include "./ifc_wasabihelper.h"
#include "./enumXmlBuffer.h"
#include "../Components/wac_downloadManager/wac_downloadManager_api.h"

#include <strsafe.h>

OmStorageUrl::OmStorageUrl()
	: ref(1), manager(NULL)
{
}

OmStorageUrl::~OmStorageUrl()
{
	if (NULL != manager)
	{
		ifc_wasabihelper *wasabi = NULL;
		if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)) && wasabi != NULL)
		{
			wasabi->ReleaseWasabiInterface(&DownloadManagerGUID, (void**)&manager);
			wasabi->Release();
		}
		manager = NULL;
	}
}

HRESULT OmStorageUrl::CreateInstance(OmStorageUrl **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new OmStorageUrl();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

size_t OmStorageUrl::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmStorageUrl::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmStorageUrl::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmStorage))
		*object = static_cast<ifc_omstorage*>(this);
	else if (IsEqualIID(interface_guid, SUID_OmStorageUrl))
		*object = static_cast<ifc_omstorage*>(this);
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

HRESULT OmStorageUrl::GetId(GUID *storageUid)
{
	if (NULL == storageUid) return E_POINTER;
	*storageUid = SUID_OmStorageUrl;
	return S_OK;
}

HRESULT OmStorageUrl::GetType(GUID *storageType)
{
	if (NULL == storageType) return E_POINTER;
	*storageType = STID_OmWebStorage;
	return S_OK;
}

UINT OmStorageUrl::GetCapabilities()
{
	return capLoad | capPublic;
}

HRESULT OmStorageUrl::GetDescription(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	Plugin_LoadString(IDS_STORAGE_URL, pszBuffer, cchBufferMax);
	return S_OK;
}

HRESULT OmStorageUrl::Load(LPCWSTR pszAddress, ifc_omservicehost *host, ifc_omserviceenum **ppEnum)
{
	if (NULL == ppEnum) return E_POINTER;
	*ppEnum = NULL;

	ifc_omstorageasync *async = NULL;
	HRESULT hr = BeginLoad(pszAddress, host, NULL, NULL, &async);
	if (SUCCEEDED(hr) && async != NULL)
	{
		hr = EndLoad(async, ppEnum);
		async->Release();
	}

	return hr;
}

HRESULT OmStorageUrl::Save(ifc_omservice **serviceList, ULONG listCount, UINT saveFlags, ULONG *savedCount)
{
	return E_NOTIMPL;
}

HRESULT OmStorageUrl::Delete(ifc_omservice **serviceList, ULONG listCount, ULONG *deletedCount)
{
	return E_NOTIMPL;
}

HRESULT OmStorageUrl::BeginLoad(LPCWSTR pszAddress, ifc_omservicehost *serviceHost, ifc_omstorageasync::AsyncCallback callback, void *data, ifc_omstorageasync **async)
{
	if (NULL == async) return E_POINTER;
	*async = NULL;

	if (NULL == pszAddress || L'\0' == *pszAddress)
		return E_INVALIDARG;

	LPSTR address = Plugin_WideCharToMultiByte(CP_ACP, 0, pszAddress, -1, NULL, NULL);
	if (NULL == address) return E_OUTOFMEMORY;

	HRESULT hr(S_OK);

	if (NULL == manager)
	{
		ifc_wasabihelper *wasabi = NULL;
		hr = Plugin_GetWasabiHelper(&wasabi);
		if (SUCCEEDED(hr) && wasabi != NULL)
		{
			hr = wasabi->QueryWasabiInterface(&DownloadManagerGUID, (void**)&manager);
			wasabi->Release();
		}
	}

	if (SUCCEEDED(hr))
	{
		OmStorageDwnld *downloader = NULL;
		hr = OmStorageDwnld::CreateInstance(manager, TRUE, &downloader);
		if (SUCCEEDED(hr) && downloader != NULL)
		{
			if (NULL != callback) downloader->SetCallback(callback);
			if (NULL != data) downloader->SetData(data);

			downloader->SetServiceHost(serviceHost);
			*async = downloader;

			if ( manager->DownloadEx( address, downloader, api_downloadManager::DOWNLOADEX_BUFFER ) == 0 )
			{
				hr = E_FAIL;
				downloader->Release();
				*async = NULL;
			}
		}
	}

	Plugin_FreeAnsiString(address);
	return hr;
}

HRESULT OmStorageUrl::EndLoad(ifc_omstorageasync *async, ifc_omserviceenum **ppEnum)
{
	OmStorageDwnld *downloader = (OmStorageDwnld*)async;
	if (NULL == downloader)	return E_INVALIDARG;

	if (NULL != ppEnum)
		*ppEnum = NULL;

	UINT state = 0;
	if (FAILED(async->GetState(&state)) || ifc_omstorageasync::stateCompleted != state)
	{
		HANDLE completed = NULL;
		if (FAILED(async->GetWaitHandle(&completed)) || completed == NULL)
			return E_UNEXPECTED;

		while(WAIT_OBJECT_0 != WaitForSingleObjectEx(completed, INFINITE, TRUE));
		CloseHandle(completed);
	}

	HRESULT hr = downloader->GetResultCode();
	if (SUCCEEDED(hr))
	{
		void *buffer = NULL;
		size_t bufferSize;
		hr = downloader->GetBuffer(&buffer, &bufferSize);
		if (SUCCEEDED(hr))
		{	
			if (NULL != ppEnum)
			{
				EnumXmlBuffer *enumerator = NULL;
				ifc_omservicehost *serviceHost = NULL;
				if (FAILED(downloader->GetServiceHost(&serviceHost)))
					serviceHost = NULL;

				hr = EnumXmlBuffer::CreateInstance(buffer, bufferSize, async, serviceHost, &enumerator);
				if (SUCCEEDED(hr))
				{
					*ppEnum = enumerator;
				}

				if (NULL != serviceHost)
					serviceHost->Release();
			}
		}
	}
	return hr;
}

HRESULT OmStorageUrl::RequestAbort(ifc_omstorageasync *async, BOOL fDrop)
{
	OmStorageDwnld *downloader = (OmStorageDwnld*)async;
	if (NULL == downloader)	return E_INVALIDARG;

	return downloader->RequestAbort(fDrop);
}

#define CBCLASS OmStorageUrl
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETID, GetId);
CB(API_GETTYPE, GetType);
CB(API_GETCAPABILITIES, GetCapabilities);
CB(API_GETDESCRIPTION, GetDescription);
CB(API_LOAD, Load);
CB(API_SAVE, Save);
CB(API_DELETE, Delete);
CB(API_BEGINLOAD, BeginLoad);
CB(API_ENDLOAD, EndLoad);
CB(API_REQUESTABORT, RequestAbort);
END_DISPATCH;
#undef CBCLASS