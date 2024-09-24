#include "main.h"
#include "./serviceManager.h"
#include "./service.h"

#include "./ifc_wasabihelper.h"

#include "./storageIni.h"
#include "./storageXml.h"
#include "./storageUrl.h"
#include "./storageEnum.h"

#include <shlwapi.h>
#include <strsafe.h>

OmServiceManager::OmServiceManager() 
	: ref(1), registerStorage(TRUE)
{
}

OmServiceManager::~OmServiceManager()
{	
	size_t index = storageList.size();
	while(index--)
	{
		storageList[index]->Release();
	}
}

OmServiceManager *OmServiceManager::CreateInstance()
{
	return new OmServiceManager();
}

size_t OmServiceManager::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmServiceManager::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmServiceManager::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmServiceManager))
		*object = static_cast<ifc_omservicemanager*>(this);
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

HRESULT OmServiceManager::RegisterDefaultStorage()
{
	ifc_omstorage *storage;
	if (SUCCEEDED(OmStorageIni::CreateInstance((OmStorageIni**)&storage)))
	{
		RegisterStorage(storage);
		storage->Release();
	}

	if (SUCCEEDED(OmStorageXml::CreateInstance((OmStorageXml**)&storage)))
	{
		RegisterStorage(storage);
		storage->Release();
	}

	if (SUCCEEDED(OmStorageUrl::CreateInstance((OmStorageUrl**)&storage)))
	{
		RegisterStorage(storage);
		storage->Release();
	}

	return S_OK;
}

HRESULT OmServiceManager::RegisterStorage(ifc_omstorage *storage)
{
	if (NULL == storage) return E_INVALIDARG;
	
	HRESULT hr;
	GUID storageId, testId;

	if (FAILED(storage->GetId(&storageId)))
		return E_FAIL;

	size_t index = storageList.size();
	while(index--)
	{
		if(SUCCEEDED(storageList[index]->GetId(&testId)) && IsEqualGUID(storageId, testId))
			break;
	}

	if (((size_t)-1) == index)
	{
		storageList.push_back(storage);
		storage->AddRef();
		hr = S_OK;
	}
	else
	{
		hr = S_FALSE;
	}
	return hr;
}

HRESULT OmServiceManager::UnregisterStorage(const GUID *storageId)
{
	if (NULL == storageId) return E_INVALIDARG;

	HRESULT hr = S_FALSE;
	GUID testId;

	size_t index = storageList.size();
	while(index--)
	{
		ifc_omstorage *storage = storageList[index];
		if(SUCCEEDED(storage->GetId(&testId)) && IsEqualGUID(*storageId, testId))
		{
			storageList.erase(storageList.begin() + index);
			storage->Release();
			hr = S_OK;
			break;
		}
	}
	return hr;
}

HRESULT OmServiceManager::QueryStorage(const GUID *storageId, ifc_omstorage **storageOut)
{
	if (NULL == storageOut) return E_POINTER;
	*storageOut = NULL;

	if (NULL == storageId) 
		return E_INVALIDARG;
	
	if (FALSE != registerStorage && SUCCEEDED(RegisterDefaultStorage()))
		registerStorage = FALSE;

	HRESULT hr = S_FALSE;
	GUID testId;

	size_t index = storageList.size();
	while(index--)
	{
		ifc_omstorage *storage = storageList[index];
		if(SUCCEEDED(storage->GetId(&testId)) && IsEqualGUID(*storageId, testId))
		{
			storage->AddRef();
			*storageOut = storage;
			hr = S_OK;
			break;
		}
	}

	return hr;
}

HRESULT OmServiceManager::EnumStorage(const GUID *filterType, UINT filterCapabilities, ifc_omstorageenumerator **enumOut)
{
	if (FALSE != registerStorage && SUCCEEDED(RegisterDefaultStorage()))
		registerStorage = FALSE;

	return OmStorageEnumerator::CreateInstance(storageList.size() ? &storageList.at(0) : nullptr, storageList.size(),
											   filterType, filterCapabilities,
											   (OmStorageEnumerator**)enumOut);
}

HRESULT OmServiceManager::CreateService(UINT serviceId, ifc_omservicehost *host, ifc_omservice **serviceOut)
{
	if (NULL == serviceOut) return E_POINTER;

	OmService *service = NULL;
	HRESULT hr = OmService::CreateInstance(serviceId, host, &service);
	if (FAILED(hr)) 
	{
		*serviceOut = NULL;
		return hr;
	}

	*serviceOut = service;
	return hr;
}

#define CBCLASS OmServiceManager
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_REGISTERSTORAGE, RegisterStorage)
CB(API_UNREGISTERSTORAGE, UnregisterStorage)
CB(API_QUERYSTORAGE, QueryStorage)
CB(API_ENUMSTORAGE, EnumStorage)
CB(API_CREATESERVICE, CreateService)
END_DISPATCH;
#undef CBCLASS