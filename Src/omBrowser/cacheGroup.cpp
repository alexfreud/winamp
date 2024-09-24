#include "main.h"
#include "./cacheGroup.h"
#include "./cacheRecord.h"
#include "./cacheManager.h"

#include <wininet.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <algorithm>

CacheGroup::CacheGroup(LPCWSTR pszName)
	: ref(1), name(NULL), owner(NULL)
{
	name = Plugin_CopyString(pszName);
}

CacheGroup::~CacheGroup()
{
	Clear();

	Plugin_FreeString(name);
}

HRESULT CacheGroup::CreateInstance(LPCWSTR pszName, CacheGroup **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	*instance = new CacheGroup(pszName);
	if (NULL == *instance) return E_OUTOFMEMORY;

	return S_OK;

}

size_t CacheGroup::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t CacheGroup::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int CacheGroup::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmCacheGroup))
		*object = static_cast<ifc_omcachegroup*>(this);
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

HRESULT CacheGroup::SetOwner(CacheManager *group)
{
	owner = group;
	return S_OK;
}

HRESULT CacheGroup::GetName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 0 == cchBufferMax) 
		return E_INVALIDARG;

	return StringCchCopyEx(pszBuffer, cchBufferMax, name, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT CacheGroup::IsEqualName(LPCWSTR pszGroup)
{
	if (NULL == pszGroup)
		return (NULL == name) ? S_OK : S_FALSE;
	
	if (NULL == name) return S_FALSE;

	INT result = CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszGroup, -1, name, -1);
	if (0 == result)
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}
	return (CSTR_EQUAL == result) ? S_OK : S_FALSE;
}

HRESULT CacheGroup::IsEqual(CacheGroup *group)
{
	return (NULL != group) ? IsEqualName(group->name) : E_INVALIDARG;
}

HRESULT CacheGroup::IsEmpty()
{
	return (0 == recordList.size()) ? S_OK : S_FALSE;
}

HRESULT CacheGroup::Delete(LPCWSTR pszName)
{
	HRESULT hr = S_FALSE;
	size_t index = recordList.size();

	while(index--)
	{
		CacheRecord *record = recordList[index];
		if (NULL != record && S_OK == record->IsEqualName(pszName))
		{
			recordList.erase(recordList.begin() + index);
			record->Release();
			hr = S_OK;
			break;
		}
	}

	return hr;
}

HRESULT CacheGroup::Clear()
{
	size_t index = recordList.size();

	while(index--)
	{
		CacheRecord *record = recordList[index];
		if (NULL != record) record->Release();
	}

	recordList.clear();

	return S_OK;
}

__inline static int __cdecl CacheGroup_RecordSearch(const void *elem1, const void *elem2)
{
	int r = ((CacheRecord*)elem2)->CompareTo((LPCWSTR)elem1); 
	return -r;
}

HRESULT CacheGroup::Find(LPCWSTR pszName, BOOL fInsertMissing, CacheRecord **recordOut, BOOL *created)
{
	if (NULL != created)
		*created = FALSE;

	if (NULL == recordOut) 
		return E_POINTER;
		
	HRESULT hr = S_FALSE;
	size_t index = recordList.size();
	if (0 != index)
	{		
		//CacheRecord *rec = (CacheRecord*)bsearch(pszName, recordList.at(0), recordList.size(), sizeof(CacheRecord*), CacheGroup_RecordSearch);
		auto it = std::find_if(recordList.begin(), recordList.end(),
			[&](CacheRecord* upT) -> bool
			{
				return upT->CompareTo(pszName) == 0;
			}
		);
		if (it != recordList.end())
		{			
			*recordOut = *it;
			(*recordOut)->AddRef();
			hr = S_OK;
		}
	}
	
	if (S_FALSE == hr && FALSE != fInsertMissing)
	{
		hr = CacheRecord::CreateInstance(pszName, NULL, 0, recordOut);
		if (SUCCEEDED(hr))
		{
			recordList.push_back(*recordOut);
			Sort();

			(*recordOut)->AddRef();
			(*recordOut)->SetOwner(this);
			
			if (NULL != created)
				*created = TRUE;
		}
	}

	if (S_OK != hr)
		*recordOut = NULL;
	
	return hr;
}

HRESULT CacheGroup::GetPath(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer || 0 == cchBufferMax) 
		return E_INVALIDARG;
	
	HRESULT hr;

	if (NULL != owner)
	{
		hr = owner->GetPath(pszBuffer, cchBufferMax);
		if (SUCCEEDED(hr) && NULL != name)
		{
			if (FALSE == PathAppend(pszBuffer, name))
				hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		*pszBuffer = L'\0';
		hr = E_UNEXPECTED;
	}

	return hr;
}

static HRESULT CacheGroup_FormatStoreData(LPWSTR pszBuffer, size_t cchBufferMax, LPCWSTR pszPath, UINT *cchLength)
{
	if (NULL == pszBuffer) 
		return E_INVALIDARG;
	
	size_t remaining = cchBufferMax;
	HRESULT hr;
	hr = StringCchCopyEx(pszBuffer, remaining, L"path=", &pszBuffer, &remaining, 0);
	if (FAILED(hr)) return hr;

	hr = StringCchCopyEx(pszBuffer, remaining, pszPath, &pszBuffer, &remaining, 0);
	if (FAILED(hr)) return hr;

	if (0 == remaining) 
		return E_OUTOFMEMORY;

	remaining--;
	pszBuffer++;
	*pszBuffer = L'\0';
	
	if (NULL != cchLength)
		*cchLength = (UINT)(cchBufferMax - remaining) + 1;

	return S_OK;
}

HRESULT CacheGroup::Store(LPCWSTR pszName, LPCWSTR pszPath)
{
	HRESULT hr;
	WCHAR szBuffer[2048] = {0};
	
	hr = GetPath(szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr)) return hr;

	Plugin_EnsurePathExist(szBuffer);
	
	if (FALSE == PathAppend(szBuffer, L"cache.ini"))
		return E_OUTOFMEMORY;
	
	LPSTR pathAnsi = Plugin_WideCharToMultiByte(CP_UTF8, 0, szBuffer, -1,  NULL, NULL);
	if (NULL == pathAnsi) return E_OUTOFMEMORY;
	
	LPSTR nameAnsi = Plugin_WideCharToMultiByte(CP_UTF8, 0, pszName, -1,  NULL, NULL);
	if (NULL == nameAnsi) return E_OUTOFMEMORY;

	LPSTR dataAnsi = NULL;
	if (NULL != pszPath)
	{
		UINT cchData;
		hr = CacheGroup_FormatStoreData(szBuffer, ARRAYSIZE(szBuffer), pszPath, &cchData);
		if (SUCCEEDED(hr))
		{
			dataAnsi = Plugin_WideCharToMultiByte(CP_UTF8, 0, szBuffer, cchData, NULL, NULL);
			if (NULL == dataAnsi)
				hr = E_OUTOFMEMORY;
		}
	}

	if (SUCCEEDED(hr) && 0 == WritePrivateProfileSectionA(nameAnsi, dataAnsi, pathAnsi))
	{
		DWORD error = GetLastError();
		hr = HRESULT_FROM_WIN32(error);
	}
		
	Plugin_FreeAnsiString(dataAnsi);
	Plugin_FreeAnsiString(nameAnsi);
	Plugin_FreeAnsiString(pathAnsi);

	return hr;
}

HRESULT CacheGroup::Load()
{
	HRESULT hr;
	WCHAR szBuffer[4096] = {0};

	hr = GetPath(szBuffer, ARRAYSIZE(szBuffer));
	if (FAILED(hr)) return hr;

	if (FALSE == PathAppend(szBuffer, L"cache.ini"))
		return E_OUTOFMEMORY;

	HANDLE hFile = CreateFile(szBuffer, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD error = GetLastError();
		return HRESULT_FROM_WIN32(error);
	}

	DWORD read = 0;
	CHAR szLine[INTERNET_MAX_URL_LENGTH + 3] = {0};
	WCHAR szUnicode[ARRAYSIZE(szLine)] = {0};
	UINT lineIndex = 0;
	CacheRecord *record = NULL;
	UINT recordSet = 0;
	size_t inserted = 0;

	RecordList cleanupList;

	while (0 != ReadFile(hFile, szBuffer, sizeof(szBuffer), &read, NULL))
	{
		LPCSTR cursor = (LPCSTR)szBuffer;
		LPCSTR end = cursor + read;
		
		for(;;)
		{
			if (cursor >= end)
				break;

			if (cursor == end || ('\r' == *cursor && '\n' == *(cursor+1)))
			{
				szLine[lineIndex] = '\0';
				
				if (lineIndex > 0)
				{
					if ('[' == szLine[0] && ']' == szLine[lineIndex-1])
					{
						if (NULL != record)
						{
							if (0 != (0x00000001 & recordSet))
							{
								recordList.push_back(record);
								inserted++;
							}
							else
							{
								cleanupList.push_back(record);
							}
							
							record = NULL;
						}
						
						szLine[lineIndex-1] = '\0';
                        if (0 != MultiByteToWideChar(CP_UTF8, 0, szLine + 1, -1, szUnicode, ARRAYSIZE(szUnicode)) && 
							SUCCEEDED(CacheRecord::CreateInstance(szUnicode, NULL, 0, &record)))
						{
							record->SetOwner(this);
							recordSet = 0;
						}
					}
					else
					{
						const char kw_path[] = "path=";
						UINT cchKey = ARRAYSIZE(kw_path) - 1; 
						if (NULL != record && 
							lineIndex > cchKey && 
							CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, kw_path, cchKey, szLine, cchKey) &&
							0 != MultiByteToWideChar(CP_UTF8, 0, szLine + cchKey, -1, szUnicode, ARRAYSIZE(szUnicode)))
						{
							if(SUCCEEDED(record->SetPath(szUnicode)) && 
								SUCCEEDED(record->GetPath(szUnicode, ARRAYSIZE(szUnicode))) &&
								FALSE != PathFileExists(szUnicode))
							{
								recordSet |= 0x00000001;
							}
						}
					}

				}
    
				lineIndex = 0;
				cursor += 2;
				continue;
			}
			
			if (lineIndex < ARRAYSIZE(szLine) - 2)
			{
				szLine[lineIndex++] = *cursor;
			}

			cursor++;
		}

		if (0 == read)
		{
			if (NULL != record)
			{
				if (0 != (0x00000001 & recordSet))
				{
					recordList.push_back(record);
					inserted++;
				}
				else
					record->Release();
			}
			break;
		}
	}
	CloseHandle(hFile);

	if (0 != inserted)
	{
		Sort();
	}

	size_t i = cleanupList.size();
	if (0 != i)
	{
		while (i--)
		{
			aTRACE_LINE("ADD CACHE CLEANUP!!!!");
			cleanupList[i]->Release();
		}
	}
	return S_OK;
}

__inline static int __cdecl CacheGroup_RecordComparer(const void *elem1, const void *elem2)
{
	return CacheRecord::Compare((CacheRecord*)elem1, (CacheRecord*)elem2);
}

__inline static bool __cdecl CacheGroup_RecordComparer_V2(const void* elem1, const void* elem2)
{
	return CacheGroup_RecordComparer(elem1, elem2) < 0;
}

HRESULT CacheGroup::Sort()
{
	HRESULT hr;
	if (0 == recordList.size())
	{
		hr = S_FALSE;
	}
	else
	{
		//qsort(recordList.first(), recordList.size(), sizeof(CacheRecord*), CacheGroup_RecordComparer);
		std::sort(recordList.begin(), recordList.end(), CacheGroup_RecordComparer_V2);
		hr = S_OK;
	}

	return hr;

}

#define CBCLASS CacheGroup
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETNAME, GetName)
CB(API_FIND, Find)
CB(API_DELETE, Delete)
CB(API_CLEAR, Clear)

END_DISPATCH;
#undef CBCLASS