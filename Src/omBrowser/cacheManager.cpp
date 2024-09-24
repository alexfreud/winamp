#include "main.h"
#include "./cacheManager.h"
#include "./cacheGroup.h"

#include "./ifc_wasabihelper.h"

#include <shlwapi.h>
#include <strsafe.h>

CacheManager::CacheManager()
	: ref(1)
{
	
}

CacheManager::~CacheManager()
{
	Clear();
}

HRESULT CacheManager::CreateInstance(CacheManager **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	*instance = new CacheManager();
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;

}

size_t CacheManager::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t CacheManager::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int CacheManager::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmCacheManager))
		*object = static_cast<ifc_omcachemanager*>(this);
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


HRESULT CacheManager::Clear()
{
	size_t index = groupList.size();

	while(index--)
	{
		CacheGroup *group = groupList[index];
		if (NULL != group) group->Release();
	}

	groupList.clear();
	return S_OK;
}

HRESULT CacheManager::Delete(LPCWSTR pszGroup)
{
	HRESULT hr = S_FALSE;
	size_t index = groupList.size();

	while(index--)
	{
		CacheGroup *group = groupList[index];
		if (NULL != group && S_OK == group->IsEqualName(pszGroup))
		{
			groupList.erase(groupList.begin() + index);
			group->Release();
			hr = S_OK;
			break;
		}
	}

	return hr;
}

HRESULT CacheManager::Find(LPCWSTR pszGroup, BOOL fInsertMissing, CacheGroup **groupOut, BOOL *created)
{
	if (NULL != created) *created = FALSE;
	if (NULL == groupOut) return E_POINTER;
	

	HRESULT hr = S_FALSE;
	size_t index = groupList.size();
	CacheGroup *group;

	while(index--)
	{
		group = groupList[index];
		if (NULL != group && S_OK == group->IsEqualName(pszGroup))
		{
			*groupOut = group;
			group->AddRef();
			hr = S_OK;
			break;
		}
	}
	
	if (S_FALSE == hr && FALSE != fInsertMissing)
	{
		hr = CacheGroup::CreateInstance(pszGroup, &group);
		if (SUCCEEDED(hr))
		{
			groupList.push_back(group);
			group->SetOwner(this);
			group->Load();
			group->AddRef();
			*groupOut = group;
			if (NULL != created) *created = TRUE;
		}
	}

	if (S_OK != hr)
		*groupOut = NULL;
		
	return hr;
}
HRESULT CacheManager::Load()
{
	return S_OK;
}


HRESULT CacheManager::GetPath(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 0 == cchBufferMax) 
		return E_INVALIDARG;
	
	HRESULT hr;
	ifc_wasabihelper *wasabi;
	hr = Plugin_GetWasabiHelper(&wasabi);
	if (SUCCEEDED(hr))
	{
		api_application *application;
		hr = wasabi->GetApplicationApi(&application);
		if (SUCCEEDED(hr))
		{
			LPCWSTR pszUser = application->path_getUserSettingsPath();
			if (NULL != pszUser) 
			{
				if (NULL == PathCombine(pszBuffer, pszUser, L"Plugins\\omBrowser\\cache"))
					hr = E_OUTOFMEMORY;
			}
			else
			{
				hr = E_UNEXPECTED;
			}
			application->Release();
		}
		wasabi->Release();

	}
	return hr;
}

#define CBCLASS CacheManager
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_FIND, Find)
CB(API_DELETE, Delete)
CB(API_CLEAR, Clear)

END_DISPATCH;
#undef CBCLASS