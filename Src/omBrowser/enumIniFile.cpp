#include "main.h"
#include "./enumIniFile.h"
#include "./service.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omstorage.h"
#include "./ifc_omstoragehandlerenum.h"
#include "./ifc_omstorageext.h"
#include "./ifc_omfilestorage.h"

#include <shlwapi.h>
#include <strsafe.h>

#define OMS_GROUP			"OnlineService"

#define OMS_ID				"id"
#define OMS_NAME			"name"
#define OMS_URL				"url"
#define OMS_ICON			"icon"
#define OMS_FLAGS			"flags"
#define OMS_RATING			"rating"
#define OMS_VERSION			"version"
#define OMS_DESCRIPTION		"description"
#define OMS_AUTHORFIRST		"authorFirst"
#define OMS_AUTHORLAST		"authorLast"
#define OMS_PUBLISHED		"publishedDate"
#define OMS_UPDATED			"updatedDate"
#define OMS_THUMBNAIL		"thumbnail"
#define OMS_SCREENSHOT		"screenshot"
#define OMS_GENERATION		"generation"

EnumIniFile::EnumIniFile(LPCWSTR pszAddress, ifc_omservicehost *serviceHost)
	: ref(1), address(NULL), host(serviceHost), hFind(NULL)
{
	address = Plugin_CopyString(pszAddress);

	if (NULL != host) 
	{
		host->AddRef();

		ifc_omstorageext *storageExt = NULL;
		if (SUCCEEDED(host->QueryInterface(IFC_OmStorageExt, (void**)&storageExt)) && storageExt != NULL)
		{	
			ifc_omstoragehandlerenum *handlerEnum = NULL;
			if (SUCCEEDED(storageExt->Enumerate(&SUID_OmStorageIni, &handlerEnum)) && handlerEnum != NULL)
			{
				reader.RegisterHandlers(handlerEnum);
				handlerEnum->Release();
			}
			storageExt->Release();
		}
	}
}

EnumIniFile::~EnumIniFile()
{
	Plugin_FreeString(address);
	
	if (NULL != host) 
		host->Release();

	if (NULL != hFind)
		FindClose(hFind);
}

HRESULT EnumIniFile::CreateInstance(LPCWSTR pszAddress, ifc_omservicehost *host, EnumIniFile **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	WCHAR szBuffer[MAX_PATH * 2] = {0};
	HRESULT hr = Plugin_ResolveRelativePath(pszAddress, host, szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr)) return hr;

	// test if we can load this one
	if (FALSE == PathIsDirectory(szBuffer) && PathFileExists(szBuffer))
	{
		UINT id = GetPrivateProfileInt(TEXT(OMS_GROUP), WTEXT(OMS_ID), 0, pszAddress);
		if (0 == id) return OMSTORAGE_E_UNKNOWN_FORMAT;
	}

	*instance = new EnumIniFile(szBuffer, host);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;
}

size_t EnumIniFile::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t EnumIniFile::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int EnumIniFile::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmServiceEnum))
		*object = static_cast<ifc_omserviceenum*>(this);
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

HRESULT EnumIniFile::Next(ULONG listSize, ifc_omservice **elementList, ULONG *elementCount)
{
	if(NULL != elementCount)
		*elementCount = 0;
	
	if (0 == listSize || NULL == elementList) 
		return E_INVALIDARG;
	
	ULONG counter = 0;
	BOOL bFoundNext = FALSE;
	HRESULT hr = S_OK;

	if (NULL == hFind)
	{
		hFind = FindFirstFile(address, &fData);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			DWORD error = GetLastError();
			return HRESULT_FROM_WIN32(error);
		}
		bFoundNext = TRUE;
	}
	else
	{
		bFoundNext = FindNextFile(hFind, &fData);
	}

	if (bFoundNext)
	{		
		do 
		{	
			LPCWSTR p = address;
			while(p && L'.' == *p && L'\0' != *p) p++;
			if (p && L'\0' != *p)
			{
				WCHAR base[MAX_PATH] = {0};
				StringCchCopy(base, MAX_PATH, address);
				PathRemoveFileSpec(base);

				int baseLen = lstrlen(base);
				base[baseLen] = L'\0';

				if (!PathAppend(base, fData.cFileName))
				{
					base[baseLen] = L'\0';
					PathAppend(base, fData.cAlternateFileName);
				}

				hr = reader.Load(base, host, &elementList[counter]);
				if (S_OK == hr)
				{
					listSize--;
					counter++;
					if (0 == listSize) break;
				}
			}
			bFoundNext = FindNextFile(hFind, &fData);

		} while(bFoundNext);
	}

	if(NULL != elementCount)
		*elementCount = counter;

	return (counter > 0) ? S_OK : S_FALSE;
}

HRESULT EnumIniFile::Reset(void)
{
	if (NULL != hFind)
	{
		FindClose(hFind);
		hFind = NULL;
	}

	return S_OK;
}

HRESULT EnumIniFile::Skip(ULONG elementCount)
{
	return E_NOTIMPL;
}

#define CBCLASS EnumIniFile
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
END_DISPATCH;
#undef CBCLASS