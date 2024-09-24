#include "main.h"
#include "./import.h"
#include "./api__ml_online.h"
#include "./resource.h"
#include "./serviceHost.h"
#include "./serviceHelper.h"
#include "./navigation.h"

#include <ifc_omstorage.h>
#include <ifc_omfilestorage.h>
#include <ifc_omstorageenum.h>
#include <ifc_omservice.h>
#include <ifc_omserviceenum.h>

#include <strsafe.h>

static HRESULT ImportFile_GetEnumerator(ifc_omstorageenumerator **enumerator)
{
	if (NULL == OMSERVICEMNGR) return E_UNEXPECTED;
	return OMSERVICEMNGR->EnumStorage(&STID_OmFileStorage, ifc_omstorage::capPublic | ifc_omstorage::capLoad, enumerator);
}

HRESULT ImportService_GetFileSupported()
{
	if (NULL == OMSERVICEMNGR) 
		return E_UNEXPECTED;

	ifc_omstorageenumerator *enumerator;
	HRESULT hr = ImportFile_GetEnumerator(&enumerator);
	
	if (SUCCEEDED(hr))
	{		
		ifc_omstorage *storage;
		if(S_OK == enumerator->Next(1, &storage, NULL))
		{
			storage->Release();
			hr = S_OK;
		}
		else
		{
			hr = S_FALSE;
		}

		enumerator->Release();
	}
	return hr;
}

static HRESULT ImportFile_GetFilter(LPWSTR pszBuffer, UINT cchBufferMax, DWORD *defaultIndex)
{
	if (NULL != defaultIndex) 
		*defaultIndex = 0;

	if (NULL == pszBuffer) 
		return E_POINTER;

	HRESULT hr;
	WCHAR szName[128] = {0}, szList[512] = {0};

	LPWSTR cursor = pszBuffer;
	size_t remaining = cchBufferMax;

	LPWSTR listC = szList;
	size_t listR = ARRAYSIZE(szList);
	
	DWORD counter = 0;

	szList[0] = L'\0';
	pszBuffer[0] = L'\0';

	WASABI_API_LNGSTRINGW_BUF(IDS_FILEFILTER_ALL, szName, ARRAYSIZE(szName));
	hr = Plugin_AppendFileFilter(cursor, remaining, szName, L"*.*", &cursor, &remaining, TRUE);
	if (FAILED(hr)) return hr;
	counter++;

	ifc_omstorageenumerator *enumerator;
	hr = ImportFile_GetEnumerator(&enumerator);
	if (SUCCEEDED(hr))
	{		
		ifc_omstorage *storage;
		ifc_omfilestorage *fileStorage;
		while(S_OK == enumerator->Next(1, &storage, NULL))
		{
			if (SUCCEEDED(storage->QueryInterface(IFC_OmFileStorage, (void**)&fileStorage)))
			{
				WCHAR szFilter[64] = {0};
				if (SUCCEEDED(fileStorage->GetFilter(szFilter, ARRAYSIZE(szFilter))) && 
					L'\0' != szFilter[0] &&
					SUCCEEDED(storage->GetDescription(szName, ARRAYSIZE(szName))))
				{
					hr = Plugin_AppendFileFilter(cursor, remaining, szName, szFilter, &cursor, &remaining, TRUE);
					if (FAILED(hr)) break;

					counter++;
			
					if (listC == szList || SUCCEEDED(StringCchCopyEx(listC, listR, L";", &listC, &listR, 0)))
						StringCchCopyEx(listC, listR, szFilter, &listC, &listR, 0);
				}
				fileStorage->Release();
			}
			storage->Release();
		}
		enumerator->Release();
	}

	if (SUCCEEDED(hr) && L'\0' != szList[0])
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_FILEFILTER_ALLKNOWN, szName, ARRAYSIZE(szName));
		hr = Plugin_AppendFileFilter(cursor, remaining, szName, szList, &cursor, &remaining, TRUE);
		if (FAILED(hr)) return hr;
		
		counter++;

		if (NULL != defaultIndex) 
			*defaultIndex = counter;
	}

	return hr;
}

static HRESULT ImportFile_ProcessFile(HWND hOwner, ifc_omstorageenumerator *enumerator,
									  ifc_omservicehost *serviceHost, ifc_omstorage *serviceStorage,
									  LPCWSTR pszFile, ULONG *converted)
{
	ifc_omstorage *storage;
	enumerator->Reset();

	Navigation	*navigation;
	if (FAILED(Plugin_GetNavigation(&navigation)))
		return E_FAIL;

	ULONG loaded(0), saved(0);
	while(S_OK == enumerator->Next(1, &storage, NULL))
	{
		ifc_omserviceenum *serviceEnum;
		HRESULT hr = storage->Load(pszFile, serviceHost, &serviceEnum);
		if(SUCCEEDED(hr))
		{
			ifc_omservice *service;
			while(S_OK == serviceEnum->Next(1, &service, NULL))
			{
				loaded++;
				if (SUCCEEDED(service->SetAddress(NULL)))
				{
					service->UpdateFlags(SVCF_SUBSCRIBED | SVCF_PREAUTHORIZED);

					ULONG savedOk;
					if (SUCCEEDED(serviceStorage->Save(&service, 1, ifc_omstorage::saveClearModified, &savedOk)))
					{
						navigation->CreateItem(service, 1);
						saved += savedOk;
					}
				}
				service->Release();
			}
			serviceEnum->Release();
			break;
		}
		else if (OMSTORAGE_E_UNKNOWN_FORMAT != hr)
		{
			break;
		}

		storage->Release();
	}

	if (NULL != converted) 
		*converted = saved;

	navigation->Release();

	return S_OK;
}

static HRESULT ImportFile_ProcessList(HWND hOwner, LPCWSTR pszList)
{
	if (NULL == pszList) 
		return E_INVALIDARG;
	
	LPCWSTR base, block, c;
	base = pszList;
	c = base;
	block = NULL;
	ULONG converted;

	ifc_omstorageenumerator *enumerator;
	HRESULT hr = ImportFile_GetEnumerator(&enumerator);
	if (FAILED(hr)) return hr;

	ServiceHost *serviceHost;
	hr = ServiceHost::GetCachedInstance(&serviceHost);
	if (SUCCEEDED(hr))
	{
		ifc_omstorage *serviceStorage;
		hr = ServiceHelper_QueryStorage(&serviceStorage);
		if (SUCCEEDED(hr))
		{
			ULONG scanned(0);
			while(L'\0' != *c)
			{
				block = c;
				while (L'\0' != *c) c++;
				if (c != block && block != base)
				{
					WCHAR szBuffer[MAX_PATH * 2] = {0};
					scanned++;
					if (SUCCEEDED(StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"%s\\%s", base, block)) && 
						SUCCEEDED(ImportFile_ProcessFile(hOwner, enumerator, serviceHost, serviceStorage, szBuffer, &converted)))
					{
					}
				}
				c++;
			}

			if (pszList == block && c != pszList && 
				SUCCEEDED(ImportFile_ProcessFile(hOwner, enumerator, serviceHost, serviceStorage, pszList, &converted)))
			{
			}
			serviceStorage->Release();
		}
		serviceHost->Release();
	}

	enumerator->Release();
	return hr;
}

HRESULT ImportService_FromFile(HWND hOwner)
{
	if (NULL == OMSERVICEMNGR) 
		return E_UNEXPECTED;

	OPENFILENAME of = {0};
	of.lStructSize = sizeof(of);

	WCHAR szFilter[1024] = {0};
	HRESULT hr = ImportFile_GetFilter(szFilter, ARRAYSIZE(szFilter), &of.nFilterIndex);
	if (FAILED(hr)) return hr;

	UINT cchResultMax = 16384;
	LPWSTR pszResult = Plugin_MallocString(cchResultMax);
	if (NULL == pszResult) return E_OUTOFMEMORY;
	*pszResult = L'\0';
		
	of.hwndOwner = hOwner;
	of.lpstrFilter = szFilter;
	of.lpstrFile = pszResult;
	of.nMaxFile = cchResultMax;
	of.lpstrInitialDir = WASABI_API_APP->path_getUserSettingsPath();
	of.lpstrTitle = WASABI_API_LNGSTRINGW(IDS_IMPORT_FILES);
	of.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT;
	
	if (0 == GetOpenFileName(&of))
	{
		INT err = CommDlgExtendedError();
		hr = (0 == err) ? S_FALSE : E_FAIL;
	}
	else
	{
		hr = ImportFile_ProcessList(hOwner, pszResult);
	}

	Plugin_FreeString(pszResult);
	return hr;
}