#include "main.h"
#include "./cacheRecord.h"
#include "./cacheDownloader.h"
#include "./cacheGroup.h"
#include "./ifc_omcachecallback.h"

#include <shlwapi.h>
#include <strsafe.h>

CacheRecord::CacheRecord(LPCWSTR pszName, LPCWSTR pszAddress, UINT uFlags)
	: ref(1), owner(NULL), name(NULL), path(NULL), flags(uFlags), downloader(NULL), callbackList(NULL)
{
	name = Plugin_CopyString(pszName);
	path = Plugin_CopyString(pszAddress);

	InitializeCriticalSection(&lock);
}

CacheRecord::~CacheRecord()
{
	Plugin_FreeString(name);
    Plugin_FreeString(path);

	EnterCriticalSection(&lock);

	if (NULL != downloader)
	{
		downloader->SetOwner(NULL);
		downloader->Release();
	}

	if (NULL != callbackList)
	{
		size_t index = callbackList->size();
		while(index--)
		{
			ifc_omcachecallback *callback = callbackList->at(index);
			if (NULL != callback) callback->Release();
		}
		delete(callbackList);
	}

	LeaveCriticalSection(&lock);

	DeleteCriticalSection(&lock);
}

HRESULT CacheRecord::CreateInstance(LPCWSTR pszName, LPCWSTR pszAddress, UINT uFlags, CacheRecord **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == pszName || L'\0' == *pszName) return E_INVALIDARG;

	*instance = new CacheRecord(pszName, pszAddress, uFlags);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;

}

INT CacheRecord::Compare(CacheRecord *record1, CacheRecord *record2)
{
	if (NULL == record1 || NULL == record2) 
		return (INT)(INT_PTR)(record1 - record2);

	return record1->CompareTo(record2->name);
}
size_t CacheRecord::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t CacheRecord::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int CacheRecord::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmCacheRecord))
		*object = static_cast<ifc_omcacherecord*>(this);
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

HRESULT CacheRecord::SetOwner(CacheGroup *group)
{
	EnterCriticalSection(&lock);
	owner = group;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT CacheRecord::GetName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 0 == cchBufferMax) 
		return E_INVALIDARG;

	return StringCchCopyEx(pszBuffer, cchBufferMax, name, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT CacheRecord::GetPath(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 0 == cchBufferMax) 
		return E_INVALIDARG;
	
	HRESULT hr;
	EnterCriticalSection(&lock);

	if (NULL == path)
	{
		*pszBuffer = L'\0';

		if (NULL != downloader)
		{
			hr = E_PENDING;
		}
		else if (0 != (flagDownloadFailed & flags))
		{
			hr = E_FAIL;
		}
		else if (NULL != name && L'\0' != *name && PathIsURL(name) &&
				CSTR_EQUAL != CompareString(CSTR_INVARIANT, NORM_IGNORECASE, name, 6, L"res://", 6))
		{
			hr = Download();
			if (SUCCEEDED(hr)) hr = E_PENDING;
		}
		else
		{
			hr = E_FAIL;
		}
	}
	else if (FALSE == PathIsRelative(path))
	{
		hr = StringCchCopy(pszBuffer, cchBufferMax, path);
	}
	else
	{
		if (NULL == owner) 
		{
			hr = E_FAIL;
		}
		else if (FAILED(owner->GetPath(pszBuffer, cchBufferMax)) || 
				FALSE == PathAppend(pszBuffer, path))
		{
			hr = E_OUTOFMEMORY;
		}
		else
		{
			hr = S_OK;
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT CacheRecord::SetPath(LPCWSTR pszPath)
{
	EnterCriticalSection(&lock);

	if (NULL != downloader)
	{
		downloader->Abort();
		downloader->Release();
		downloader = NULL;
	}

	Plugin_FreeString(path);
	if (NULL == pszPath)
	{
		path = NULL;
	}
	else
	{
		INT cchCommon = 0;
		if (0 != owner)
		{
			WCHAR szBase[MAX_PATH*2] = {0};
			if (SUCCEEDED(owner->GetPath(szBase, ARRAYSIZE(szBase))))
				cchCommon = PathCommonPrefix(szBase, pszPath, NULL);
		}
		
		if (0 != cchCommon)
		{
			LPCWSTR p = pszPath + cchCommon;
			INT cchSource = lstrlenW(p) + 1/* \0 */;
			path = Plugin_MallocString(cchSource + 1 /*to add '.'*/);
			if (NULL != path)
			{
				*path = L'.';
                CopyMemory(path + 1, p, sizeof(WCHAR) * cchSource);
			}
		}
		else
		{
    		path = Plugin_CopyString(pszPath);
		}

		if (NULL == path) return E_OUTOFMEMORY;
	}

	if (0 == (flagNoStore & flags) && NULL != owner)
		owner->Store(name, path);

	if (NULL != callbackList)
	{
		size_t index = callbackList->size();
		while(index--)
		{
			ifc_omcachecallback *cb = callbackList->at(index);
			if (NULL != cb) cb->PathChanged(this);
		}
	}
	LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT CacheRecord::Download()
{
	HRESULT hr;
	EnterCriticalSection(&lock);

	if (NULL != downloader)
		hr = E_PENDING;
	else
	{
		flags &= ~flagDownloadFailed;
		hr = CacheDownloader::CreateInstance(this, name, FALSE, &downloader);
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT CacheRecord::DownloadCompleted(LPCWSTR pszFile, INT errorCode)
{
	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	CacheDownloader *temp = downloader;
	downloader = NULL;
	
	if (api_downloadManager::TICK_SUCCESS == errorCode)
	{
		WCHAR szTarget[MAX_PATH] = {0};
		hr = GetBasePath(szTarget, ARRAYSIZE(szTarget));
		if(SUCCEEDED(hr))
		{
			Plugin_EnsurePathExist(szTarget);
			LPCWSTR fileName = PathFindFileName(name);
			if (NULL == fileName)
			{
				WCHAR szTemp[32] = {0};
				StringCchPrintf(szTemp, ARRAYSIZE(szTemp), L"%08u.cache", GetTickCount());
				PathAppend(szTarget, szTemp);
			}
			else
			{
				PathAppend(szTarget, fileName);
			}

			WCHAR szExt[64] = {0};
			UINT attempt = 0;
			UINT pExtMax = 0;
			LPWSTR pExt = PathFindExtension(szTarget);
			if (pExt == szTarget || L'.' != *pExt || FAILED(StringCchCopy(szExt, ARRAYSIZE(szExt), pExt)))
			{
				szExt[0] = L'\0';
			}
			else
			{
				pExtMax = ARRAYSIZE(szTarget) - (UINT)(pExt - szTarget);
			}

			while(FALSE == CopyFile(pszFile, szTarget, TRUE))
			{
				DWORD error = GetLastError();
				if (ERROR_FILE_EXISTS != error)
				{
					hr = HRESULT_FROM_WIN32(error);
					break;
				}
				
				hr = StringCchPrintf(pExt, pExtMax, L"(%u)%s", ++attempt, szExt);
				if (FAILED(hr)) break;
			}
		}

		if (SUCCEEDED(hr))
		{
			SetPath(szTarget);
		}
	}
	else
	{
		flags |= flagDownloadFailed;
	}
	
	if (NULL != temp)
		temp->Release();

	LeaveCriticalSection(&lock);

	return S_OK;
}

INT CacheRecord::CompareTo(LPCWSTR pszName)
{
	if (NULL == pszName || NULL == name) 
		return (INT)(INT_PTR)(name - pszName);
	
	return CompareString(CSTR_INVARIANT, NORM_IGNORECASE, name, -1, pszName, -1) - 2;
}

HRESULT CacheRecord::IsEqualName(LPCWSTR pszName)
{
	HRESULT hr;
	EnterCriticalSection(&lock);
	if (NULL == pszName)
	{
		hr = (NULL == name) ? S_OK : S_FALSE;
	}
	else if (NULL == name)
	{
		hr = S_FALSE;
	}
	else
	{
		INT result = CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszName, -1, name, -1);
		if (0 == result)
		{
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32(error);
		}
		else
		{
			hr = (CSTR_EQUAL == result) ? S_OK : S_FALSE;
		}
	}
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT CacheRecord::IsEqual(CacheRecord *record)
{
	if (NULL == record) return E_INVALIDARG;
	
	HRESULT hr;

	EnterCriticalSection(&record->lock);
	hr = IsEqualName(record->name);
	LeaveCriticalSection(&record->lock);

	return hr;
}

HRESULT CacheRecord::RegisterCallback(ifc_omcachecallback *callback)
{
	if (NULL == callback) 
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	if (NULL == callbackList)
	{
		callbackList = new CallbackList();
		if (NULL == callbackList) hr = E_OUTOFMEMORY;
	}
	else
	{
		size_t index = callbackList->size();
		while(index--)
		{
			if (callbackList->at(index) == callback)
				hr = E_FAIL;
		}
	}
	
	if (SUCCEEDED(hr) && NULL != callbackList)
	{
		callbackList->push_back(callback);
		callback->AddRef();
	}
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT CacheRecord::UnregisterCallback(ifc_omcachecallback *callback)
{
	if (NULL == callback) 
		return E_INVALIDARG;

	HRESULT hr = S_FALSE;

	EnterCriticalSection(&lock);

	if (NULL != callbackList)
	{
		size_t index = callbackList->size();
		while(index--)
		{
			ifc_omcachecallback *test = callbackList->at(index);
			if (test == callback)
			{
				callbackList->erase(callbackList->begin() + index);
				test->Release();
				hr = S_OK;

				if (0 == callbackList->size())
				{
					delete(callbackList);
					callbackList = NULL;
				}
			}
		}
	}
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT CacheRecord::GetFlags(UINT *puFlags)
{
	if (NULL == puFlags) return E_POINTER;
	*puFlags = flags;
	return S_OK;
}

HRESULT CacheRecord::SetFlags(UINT uFlags, UINT uMask)
{
	EnterCriticalSection(&lock);
	flags = (flags & ~uMask) | (uFlags & uMask);
	LeaveCriticalSection(&lock);
	return S_OK;
}

HRESULT CacheRecord::GetBasePath(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 0 == cchBufferMax) 
		return E_INVALIDARG;
		
	HRESULT hr;
	EnterCriticalSection(&lock);

	if (NULL != owner)
	{
		hr = owner->GetPath(pszBuffer, cchBufferMax);
	}
	else
	{
		*pszBuffer = L'\0';
		hr = E_UNEXPECTED;
	}
	LeaveCriticalSection(&lock);

	return hr;
}

#define CBCLASS CacheRecord
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_GETPATH, GetPath)
CB(API_SETPATH, SetPath)
CB(API_GETFLAGS, GetFlags)
CB(API_SETFLAGS, SetFlags)
CB(API_DOWNLOAD, Download)
CB(API_REGISTERCALLBACK, RegisterCallback)
CB(API_UNREGISTERCALLBACK, UnregisterCallback)

END_DISPATCH;
#undef CBCLASS