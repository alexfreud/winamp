#include "./setupGroupFilter.h"
#include "../api__ml_online.h"
#include "../serviceHost.h"
#include "../config.h"

#include <ifc_omservice.h>
#include <ifc_omstorage.h>
#include <ifc_omserviceenum.h>
#include <ifc_omfilestorage.h>


SetupGroupFilter::SetupGroupFilter(const GUID *filterId)
	: ref(1)
{
	id = (NULL != filterId) ? *filterId : GUID_NULL;
}

SetupGroupFilter::~SetupGroupFilter()
{
}

HRESULT SetupGroupFilter::CreateInstance(const GUID *filterId, SetupGroupFilter **instance)
{
	if (NULL == filterId)
	{
		*instance = NULL;
		return E_INVALIDARG;
	}

	if (FALSE != IsEqualGUID(*filterId, FUID_SetupFeaturedGroupFilter))
		return SetupFeaturedGroupFilter::CreateInstance((SetupFeaturedGroupFilter**)instance);
	if (FALSE != IsEqualGUID(*filterId, FUID_SetupKnownGroupFilter))
		return SetupKnownGroupFilter::CreateInstance((SetupKnownGroupFilter**)instance);
		
	*instance = NULL;
	return E_NOINTERFACE;
}

ULONG SetupGroupFilter::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG SetupGroupFilter::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	return r;
}

HRESULT SetupGroupFilter::GetId(GUID *filterId)
{
	if (NULL == filterId)
		return E_POINTER;
	*filterId = id;
	return S_OK;
}

BOOL CALLBACK SetupGroupFilter::AppendServiceIdCallback(UINT serviceId, void *data)
{
	ServiceIdList *list = (ServiceIdList*)data;
	if (NULL == list) return FALSE;
	list->push_back(serviceId);
	return TRUE;
}

SetupFeaturedGroupFilter::SetupFeaturedGroupFilter() 
	: SetupGroupFilter(&FUID_SetupFeaturedGroupFilter)
{
}

SetupFeaturedGroupFilter::~SetupFeaturedGroupFilter()
{
}

HRESULT SetupFeaturedGroupFilter::CreateInstance(SetupFeaturedGroupFilter **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = new SetupFeaturedGroupFilter();
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT SetupFeaturedGroupFilter::Initialize()
{
	HRESULT hr;
	ifc_omstorage *storage;
	
	filterList.clear();

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;

	hr = OMSERVICEMNGR->QueryStorage(&SUID_OmStorageIni, &storage);
	if(FAILED(hr))
		return hr;
	
	ServiceHost *serviceHost;
	if (FAILED(ServiceHost::GetCachedInstance(&serviceHost)))
		serviceHost = NULL;

	ifc_omserviceenum *serviceEnum;
	hr = storage->Load(L"*.ini", serviceHost, &serviceEnum);
	if(SUCCEEDED(hr))
	{
		ifc_omservice *service;
		while(S_OK == serviceEnum->Next(1, &service, NULL) && NULL != service)
		{
			filterList.push_back(service->GetId());
		}
		serviceEnum->Release();
	}
	storage->Release();

	ServiceIdList promoList;
	Config_ReadServiceIdList("Setup", "featuredExtra", ',', AppendServiceIdCallback, &promoList);

	ServiceIdList historyList;
	Config_ReadServiceIdList("Setup", "featuredHistory", ',', AppendServiceIdCallback, &historyList);

	size_t index = historyList.size();
	size_t filterIndex, filterSize = filterList.size();

	while(index--)
	{
		size_t promoSize = promoList.size();
		for(filterIndex = 0; filterIndex < promoSize; filterIndex++)
		{
			if (promoList[filterIndex] == historyList[index])
			{
				promoList.erase(promoList.begin() + filterIndex);
				break;
			}
		}
		if (filterIndex == promoSize)
		{
			for(filterIndex = 0; filterIndex < filterSize; filterIndex++)
			{
				if (filterList[filterIndex] == historyList[index])
					break;
			}
		
			if (filterIndex == filterSize)
				filterList.push_back(historyList[index]);
		}
	}

	return hr;
}

HRESULT SetupFeaturedGroupFilter::ProcessService(ifc_omservice *service, UINT *filterResult)
{
	if (NULL == service)
		return E_INVALIDARG;

	if (NULL == filterResult)
		return E_POINTER;

	size_t index = filterList.size();
	while(index--) 
	{
		if (filterList[index] == service->GetId())
		{
			filterList.erase(filterList.begin() + index);
			*filterResult = serviceIgnore;
			break;						
		}
	}
	return S_OK;
}

SetupKnownGroupFilter::SetupKnownGroupFilter()
	: SetupGroupFilter(&FUID_SetupKnownGroupFilter)
{
}

SetupKnownGroupFilter::~SetupKnownGroupFilter()
{
}

HRESULT SetupKnownGroupFilter::CreateInstance(SetupKnownGroupFilter **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = new SetupKnownGroupFilter();
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

HRESULT SetupKnownGroupFilter::Initialize()
{
	Config_ReadServiceIdList("Setup", "featuredExtra", ',', AppendServiceIdCallback, &filterList);
	return S_OK;
}

HRESULT SetupKnownGroupFilter::ProcessService(ifc_omservice *service, UINT *filterResult)
{
	if (NULL == service)
		return E_INVALIDARG;

	if (NULL == filterResult)
		return E_POINTER;

	size_t index = filterList.size();
	while(index--) 
	{
		if (filterList[index] == service->GetId())
		{
			filterList.erase(filterList.begin() + index);
			*filterResult &= ~serviceForceUnsubscribe;
			*filterResult |= serviceForceSubscribe;
			break;						
		}
	}

	return S_OK;
}
