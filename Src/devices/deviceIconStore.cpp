#include "main.h"
#include "./deviceIconStore.h"

#include <strsafe.h>

DeviceIconStore::DeviceIconStore() 
	: ref(1), base(NULL)
{
	InitializeCriticalSection(&lock);
}

DeviceIconStore::~DeviceIconStore()
{
	RemoveAll();
	String_Free(base);
	DeleteCriticalSection(&lock);
}

HRESULT DeviceIconStore::CreateInstance(DeviceIconStore **instance)
{
	if (NULL == instance) 
		return E_POINTER;

	*instance = new DeviceIconStore();

	if (NULL == *instance) 
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t DeviceIconStore::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t DeviceIconStore::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int DeviceIconStore::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_DeviceIconStore))
		*object = static_cast<ifc_deviceiconstore*>(this);
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

void DeviceIconStore::Lock()
{
	EnterCriticalSection(&lock);
}

void DeviceIconStore::Unlock()
{
	LeaveCriticalSection(&lock);
}

HRESULT DeviceIconStore::Add(const wchar_t *path, unsigned int width, unsigned int height, BOOL replaceExisting)
{
	if (FALSE != IS_STRING_EMPTY(path))
		return E_INVALIDARG;

	if(width < 1)
		width = 1;

	if(height < 1)
		height = 1;

	HRESULT hr = S_FALSE;

	Lock();

	size_t index = list.size();
	while(index--)
	{
		Record *record = &list[index];
		if (width == record->width &&
			height == record->height)
		{
			if (FALSE == replaceExisting)
				hr = E_FAIL;
			else
			{
				wchar_t *pathCopy;
				pathCopy = String_Duplicate(path);
				if (NULL == pathCopy)
					hr = E_OUTOFMEMORY;
				else
				{
					record->path = pathCopy;
					hr = S_OK;
				}
			}
			break;
		}
	}
	
	if (S_FALSE == hr)
	{
		Record newRecord;
		newRecord.path = String_Duplicate(path);
		if (NULL == newRecord.path)
			hr = E_OUTOFMEMORY;
		else
		{
			newRecord.width = width;
			newRecord.height = height;
			list.push_back(newRecord);
			hr = S_OK;
		}
	}
		
	Unlock();

	return hr;
}

HRESULT DeviceIconStore::Remove(unsigned int width, unsigned int height)
{
	HRESULT hr;
	size_t index;
	Record *record;

	if(width < 1)
		width = 1;

	if(height < 1)
		height = 1;

	hr = S_FALSE;

	Lock();

	index = list.size();
	while(index--)
	{
		record = &list[index];
		if (record->width == width &&
			record->height == height)
		{
			String_Free(record->path);
			list.erase(list.begin() + index);
			hr = S_OK;
			break;
		}
	}

	Unlock();

	return hr;
}

HRESULT DeviceIconStore::RemovePath(const wchar_t *path)
{
	HRESULT hr;
	size_t index;
	Record *record;

	if (FALSE != IS_STRING_EMPTY(path))
		return E_INVALIDARG;

	hr = S_FALSE;

	Lock();

	index = list.size();
	while(index--)
	{
		record = &list[index];
		if(CSTR_EQUAL == CompareStringW(CSTR_INVARIANT, NORM_IGNORECASE, record->path, -1, path, -1))
		{
			String_Free(record->path);
			list.erase(list.begin() + index);
			hr = S_OK;
		}
	}

	Unlock();

	return hr;
}

HRESULT DeviceIconStore::RemoveAll()
{
	size_t index;
	Record *record;

	Lock();

	index = list.size();
	while(index--)
	{
		record = &list[index];
		String_Free(record->path);
	}
	list.clear();
	
	Unlock();
	
	return S_OK;
}

HRESULT DeviceIconStore::Get(wchar_t *buffer, size_t bufferMax, unsigned int width, unsigned int height)
{
	HRESULT hr;
	Record *record;
	const wchar_t *path;
	size_t index;
	double widthDbl, heightDbl;
	double scaleMin, scaleHorz, scaleVert;

	if (NULL == buffer)
		return E_POINTER;

	if (width < 1)
		width = 1;

	if (height < 1)
		height = 1;

	path = NULL;
	
	widthDbl = width;
	heightDbl = height;

	Lock();

	index = list.size();
	if (index  > 0)
	{
		record = &list[--index];
		scaleHorz = widthDbl/record->width;
		scaleVert = heightDbl/record->height;
		scaleMin = (scaleHorz < scaleVert) ? scaleHorz : scaleVert;
		path = record->path;
		if (1.0 != scaleMin)
		{
			scaleMin = fabs(1.0 - scaleMin);
			while(index--)
			{
				record = &list[index];
				scaleHorz = widthDbl/record->width;
				scaleVert = heightDbl/record->height;
				if (scaleHorz > scaleVert) 
					scaleHorz = scaleVert;
				
				if (1.0 == scaleHorz)
				{
					path = record->path;
					break;
				}
				
				scaleHorz = fabs(1.0 - scaleHorz);
				if (scaleHorz < scaleMin)
				{
					scaleMin = scaleHorz;
					path = record->path;
				}
			}
		}
	}
	
	if (NULL == path)
		hr = E_FAIL;
	else
		hr = GetFullPath(buffer, bufferMax, path);
	
	Unlock();

	return hr;
}

HRESULT DeviceIconStore::GetExact(wchar_t *buffer, size_t bufferMax, unsigned int width, unsigned int height)
{
	HRESULT hr;
	size_t index;
	Record *record;

	if (NULL == buffer)
		return E_POINTER;

	if(width < 1)
		width = 1;

	if(height < 1)
		height = 1;

	hr = E_FAIL;

	Lock();

	index = list.size();
	while(index--)
	{
		record = &list[index];
		if (record->width == width &&
			record->height == height)
		{
			hr = GetFullPath(buffer, bufferMax, record->path);
			break;
		}
	}

	Unlock();

	return hr;
}

HRESULT DeviceIconStore::SetBasePath(const wchar_t *path)
{
	HRESULT hr;

	Lock();
	
	String_Free(base);
	base = String_Duplicate(path);
	
	if (NULL == base && NULL != path)
		hr = E_OUTOFMEMORY;
	else
		hr = S_OK;
	
	Unlock();

	return hr;
}

HRESULT DeviceIconStore::GetBasePath(wchar_t *buffer, size_t bufferMax)
{
	HRESULT hr;
	
	if (NULL == buffer)
		return E_POINTER;

	Lock();

	if (0 == String_CopyTo(buffer, base, bufferMax) && 
		FALSE == IS_STRING_EMPTY(base))
	{
		hr = E_FAIL;
	}
	else
		hr = S_OK;

	Unlock();

	return hr;
}

HRESULT DeviceIconStore::Clone(ifc_deviceiconstore **instance)
{
	HRESULT hr;
	DeviceIconStore *clone;
		
	if (NULL == instance)
		return E_POINTER;

	hr = DeviceIconStore::CreateInstance(&clone);
	if (FAILED(hr))
		return hr;

	Lock();
	
	clone->base = String_Duplicate(base);
	if (NULL == clone->base && NULL != base)
		hr = E_OUTOFMEMORY;
	else
	{
		size_t index, count;
		Record target;
		const Record *source;

		count = list.size();

		for(index = 0; index < count; index++)
		{
			source = &list[index];
			target.path = String_Duplicate(source->path);
			if (NULL == target.path)
			{
				hr = E_OUTOFMEMORY;
				break;
			}
			else
			{
				target.width = source->width;
				target.height = source->height;
				clone->list.push_back(target);
			}
		}
	}
	
	Unlock();

	if (FAILED(hr))
		clone->Release();
	else
		*instance = clone;

	return hr;
}

HRESULT DeviceIconStore::Enumerate(EnumeratorCallback callback, void *user)
{
	size_t index, count;
	wchar_t buffer[MAX_PATH*2] = {0};
	Record *record;

	if (NULL == callback)
		return E_POINTER;

	Lock();

	count = list.size();
	for(index = 0; index < count; index++)
	{
		record = &list[index];
		if (SUCCEEDED(GetFullPath(buffer, ARRAYSIZE(buffer), record->path)))
		{
			if (FALSE == callback(buffer, record->width, record->height, user))
			{
				break;
			}
		}
	}

	Unlock();

	return S_OK;
}

HRESULT DeviceIconStore::GetFullPath(wchar_t *buffer, size_t bufferMax, const wchar_t *path)
{
	HRESULT hr;

	if (NULL == buffer)
		return E_POINTER;

	if (FALSE != IS_STRING_EMPTY(path))
		return E_INVALIDARG;

	Lock();

	if (FALSE == PathIsRelative(path) || 
		FALSE != IS_STRING_EMPTY(base))
	{
		if (0 == String_CopyTo(buffer, path, bufferMax))
			hr = E_OUTOFMEMORY;
		else
			hr = S_OK;
	}
	else
	{
		if (NULL == PathCombine(buffer, base, path))
			hr = E_FAIL;
		else
			hr = S_OK;
	}
	
	Unlock();

	return hr;
}

#define CBCLASS DeviceIconStore
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_ADD, Add)
CB(API_REMOVE, Remove)
CB(API_REMOVEPATH, RemovePath)
CB(API_REMOVEALL, RemoveAll)
CB(API_GET, Get)
CB(API_GETEXACT, GetExact)
CB(API_SETBASEPATH, SetBasePath)
CB(API_GETBASEPATH, GetBasePath)
CB(API_CLONE, Clone)
CB(API_ENUMERATE, Enumerate)
END_DISPATCH;
#undef CBCLASS