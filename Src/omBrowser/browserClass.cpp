#include "main.h"
#include "./browserClass.h"
#include "./browserRegistry.h"
#include "./configIni.h"
#include <strsafe.h>

OmBrowserClass::OmBrowserClass(LPCWSTR pszName) 
	: ref(1), name(NULL), config(NULL), registry(NULL)
{
	InitializeCriticalSection(&lock);
	name = Plugin_CopyString(pszName);
}

OmBrowserClass::~OmBrowserClass() 
{
	if (NULL != config) 
		config->Release();

	if (NULL != registry) 
	{
		registry->Delete();
		registry->Release();
	}

	Plugin_FreeString(name);

	DeleteCriticalSection(&lock);
}

HRESULT OmBrowserClass::CreateInstance(LPCWSTR pszName, OmBrowserClass **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	*instance = new OmBrowserClass(pszName);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t OmBrowserClass::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmBrowserClass::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
	{
		if (NULL != name && L'\0' != *name)
			Plugin_UnregisterBrowserClass(name);
		delete(this);
	}

	return r;
}

int OmBrowserClass::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmBrowserClass))
		*object = static_cast<ifc_ombrowserclass*>(this);
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

HRESULT OmBrowserClass::GetName(LPWSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;
	return StringCchCopyEx(pszBuffer, cchBufferMax, name, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT OmBrowserClass::IsEqual(LPCWSTR pszName)
{
	if (NULL == pszName || L'\0' == *pszName) 
		return E_INVALIDARG;

	if (NULL == name) 
		return E_UNEXPECTED;

	INT result = CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszName, -1, name, -1);
	if (0 == result) 
	{
		DWORD errorCode = GetLastError();
		return HRESULT_FROM_WIN32(errorCode);
	}

	return (CSTR_EQUAL == result) ? S_OK : S_FALSE;
}

HRESULT OmBrowserClass::GetConfig(ifc_omconfig **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	if (NULL == config)
	{
		hr = OmConfigIni::CreateInstance(name, &config);
		if (FAILED(hr))
		{
			config = NULL;
		}
	}

	if (SUCCEEDED(hr))
	{
		*instance = config;
		config->AddRef();
	}

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmBrowserClass::GetRegistry(ifc_ombrowserregistry **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	if (NULL == registry)
	{
		hr = OmBrowserRegistry::CreateInstance(name, &registry);
		if (FAILED(hr))
		{
			registry = NULL;
		}
		else
		{
			registry->Write();
		}
	}

	if (SUCCEEDED(hr))
	{
		*instance = registry;
		registry->AddRef();
	}

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmBrowserClass::UpdateRegColors()
{
	HRESULT hr = S_FALSE;
	EnterCriticalSection(&lock);

	if (NULL != registry)
		hr = registry->UpdateColors();
	
	LeaveCriticalSection(&lock);

	return hr;
}

#define CBCLASS OmBrowserClass
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_ISEQUAL, IsEqual)
CB(API_GETCONFIG, GetConfig)
CB(API_GETREGISTRY, GetRegistry)
CB(API_UPDATEREGCOLORS, UpdateRegColors)
END_DISPATCH;
#undef CBCLASS