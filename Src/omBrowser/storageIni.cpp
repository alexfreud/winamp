#include "main.h"
#include "./storageIni.h"
#include "./resource.h"
#include "./ifc_omservice.h"
#include "./loaderIni.h"
#include "./enumIniFile.h"
#include "./enumAsync.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omservicehostext.h"
#include "./ifc_omstoragehandlerenum.h"
#include "./ifc_omstorageext.h"
#include <strsafe.h>

OmStorageIni::OmStorageIni()
	: ref(1)
{
}

OmStorageIni::~OmStorageIni()
{
}

HRESULT OmStorageIni::CreateInstance(OmStorageIni **instance)
{
	if (NULL == instance) return E_POINTER;

	*instance = new OmStorageIni();
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t OmStorageIni::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmStorageIni::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int OmStorageIni::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmStorage))
		*object = static_cast<ifc_omstorage*>(this);
	else if (IsEqualIID(interface_guid, STID_OmFileStorage))
		*object = static_cast<ifc_omfilestorage*>(this);
	else if (IsEqualIID(interface_guid, SUID_OmStorageIni))
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

HRESULT OmStorageIni::GetId(GUID *storageUid)
{
	if (NULL == storageUid) return E_POINTER;
	*storageUid = SUID_OmStorageIni;
	return S_OK;
}

HRESULT OmStorageIni::GetType(GUID *storageType)
{
	if (NULL == storageType) return E_POINTER;
	*storageType = STID_OmFileStorage;
	return S_OK;
}

UINT OmStorageIni::GetCapabilities()
{
	return capLoad | capSave | capDelete | capReload | capPublic;
}

HRESULT OmStorageIni::GetDescription(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	Plugin_LoadString(IDS_STORAGE_INI, pszBuffer, cchBufferMax);
	return S_OK;
}

HRESULT OmStorageIni::Load(LPCWSTR pszAddress, ifc_omservicehost *host, ifc_omserviceenum **ppEnum)
{
	if (NULL == ppEnum) return E_POINTER;
	*ppEnum = NULL;

	if (NULL == pszAddress || L'\0' == *pszAddress)
		return E_INVALIDARG;

	return EnumIniFile::CreateInstance(pszAddress, host, (EnumIniFile**)ppEnum);
}

HRESULT OmStorageIni::Save(ifc_omservice **serviceList, ULONG listCount, UINT saveFlags, ULONG *savedCount)
{
	if (NULL != savedCount)
		*savedCount = 0;

	if(NULL == serviceList && 0 != listCount)
		return E_INVALIDARG;

	HRESULT hr = S_OK; 
	ULONG saved = 0;

	LoaderIni loader;

	for(ULONG i = 0; i < listCount; i++)
	{
		ifc_omservice *service = serviceList[i];
		if (NULL == service) continue;

		if (FAILED(loader.Save(service, saveFlags)))
			hr = E_FAIL;
		else
			saved++;
	}

	if (NULL != savedCount) 
		*savedCount = saved++;

	return hr;
}

HRESULT OmStorageIni::Delete(ifc_omservice **serviceList, ULONG listCount, ULONG *deletedCount)
{	
	if (NULL == serviceList)
	{
		if (NULL != deletedCount) *deletedCount = 0;
		return E_INVALIDARG;
	}

	WCHAR szBuffer[MAX_PATH * 2] = {0}; 
	ULONG deleted = 0;

	for(ULONG i = 0; i < listCount; i++)
	{
		if (SUCCEEDED(serviceList[i]->GetAddress(szBuffer, ARRAYSIZE(szBuffer))) &&
			0 != DeleteFile(szBuffer))
		{
			deleted++;
		}
	}

	if (NULL != deletedCount) 
		*deletedCount = deleted;
	
	return S_OK;
}

HRESULT OmStorageIni::Reload(ifc_omservice **serviceList, ULONG listCount, ULONG *reloadedCount)
{
	if (NULL != reloadedCount)
		*reloadedCount = 0;

	if(NULL == serviceList && 0 != listCount)
		return E_INVALIDARG;

	HRESULT hr = S_OK; 
	ULONG loaded = 0;
		
	LoaderIni loader;

	for(ULONG i = 0; i < listCount; i++)
	{
		ifc_omservice *service = serviceList[i];
		if (NULL == service) continue;

		loader.RegisterHandlers(NULL);

		ifc_omservicehostext *serviceExt = NULL;
		if (service->QueryInterface(IFC_OmServiceHostExt, (void**)&serviceExt))
		{
			ifc_omservicehost *serviceHost = NULL;
			if (SUCCEEDED(serviceExt->GetHost(&serviceHost)) && serviceHost != NULL)
			{
				ifc_omstorageext *storageExt = NULL;
				if (SUCCEEDED(serviceHost->QueryInterface(IFC_OmStorageExt, (void**)&storageExt)) && storageExt != NULL)
				{	
					ifc_omstoragehandlerenum *handlerEnum = NULL;
					if (SUCCEEDED(storageExt->Enumerate(&SUID_OmStorageIni, &handlerEnum)) && handlerEnum != NULL)
					{
						loader.RegisterHandlers(handlerEnum);
						handlerEnum->Release();
					}
					storageExt->Release();
				}
				serviceHost->Release();
			}
			serviceExt->Release();
		}

		if (FAILED(loader.Reload(service)))
			hr = E_FAIL;
		else
			loaded++;
	}

	if (NULL != reloadedCount) 
		*reloadedCount = loaded++;

	return hr;
}

HRESULT OmStorageIni::BeginLoad(LPCWSTR pszAddress, ifc_omservicehost *serviceHost, ifc_omstorageasync::AsyncCallback callback, void *data, ifc_omstorageasync **async)
{
	if (NULL == async)
		return E_POINTER;

	ifc_omserviceenum *enumerator = NULL;
    HRESULT hr = Load(pszAddress, serviceHost, &enumerator);
	if (SUCCEEDED(hr) && enumerator != NULL)
	{
		EnumAsyncWrapper *asyncWrapper = NULL;
		hr = EnumAsyncWrapper::CreateInstance(enumerator, &asyncWrapper);
		if (SUCCEEDED(hr) && asyncWrapper != NULL)
		{
			asyncWrapper->SetCallback(callback);
			asyncWrapper->SetData(data);
			*async = asyncWrapper;

			hr = asyncWrapper->BeginEnumerate();
			if (FAILED(hr))
			{
				*async = NULL;
				asyncWrapper->Release();			
			}
		}
		enumerator->Release();
	}

	return hr;
}

HRESULT OmStorageIni::EndLoad(ifc_omstorageasync *async, ifc_omserviceenum **ppEnum)
{
	EnumAsyncWrapper *asyncWrapper = (EnumAsyncWrapper*)async;
	if (NULL == asyncWrapper) return E_INVALIDARG;

	if (NULL != ppEnum)
		*ppEnum = NULL;

	UINT state = 0;
	if (FAILED(async->GetState(&state)) || ifc_omstorageasync::stateCompleted != state)
	{
		HANDLE completed = NULL;
		if (FAILED(async->GetWaitHandle(&completed)))
			return E_UNEXPECTED;
		
		while(WAIT_OBJECT_0 != WaitForSingleObjectEx(completed, INFINITE, TRUE));
		CloseHandle(completed);
	}

	HRESULT hr = asyncWrapper->GetResultCode();
	if (SUCCEEDED(hr))
	{
		if (NULL != ppEnum)
		{
			hr = asyncWrapper->GetServiceList(ppEnum);
		}
	}
	return hr;
}

HRESULT OmStorageIni::RequestAbort(ifc_omstorageasync *async, BOOL fDrop)
{
	EnumAsyncWrapper *asyncWrapper = (EnumAsyncWrapper*)async;
	if (NULL == asyncWrapper) return E_INVALIDARG;
	return asyncWrapper->RequestAbort(fDrop);
}

HRESULT OmStorageIni::GetFilter(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	return StringCchCopy(pszBuffer, cchBufferMax, L"*.ini");
}

#define CBCLASS OmStorageIni
START_MULTIPATCH;
 START_PATCH(MPIID_OMSTORAGE)
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, ADDREF, AddRef);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, RELEASE, Release);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETID, GetId);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETTYPE, GetType);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETCAPABILITIES, GetCapabilities);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_GETDESCRIPTION, GetDescription);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_LOAD, Load);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_SAVE, Save);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_DELETE, Delete);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_RELOAD, Reload);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_BEGINLOAD, BeginLoad);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_ENDLOAD, EndLoad);
  M_CB(MPIID_OMSTORAGE, ifc_omstorage, API_REQUESTABORT, RequestAbort);
 NEXT_PATCH(MPIID_OMFILESTORAGE)
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, ADDREF, AddRef);
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, RELEASE, Release);
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMFILESTORAGE, ifc_omfilestorage, API_GETFILTER, GetFilter);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS