#include "main.h"
#include "./stringBuilder.h"
#include <strsafe.h>

StringBuilder::StringBuilder() 
	: buffer(NULL), cursor(NULL), allocated(0), remaining(0)
{
}

StringBuilder::~StringBuilder()
{
	String_Free(buffer);
}

HRESULT StringBuilder::Allocate(size_t newSize)
{	
	if (newSize <= allocated) 
		return S_FALSE;

	LPWSTR t = String_ReAlloc(buffer, newSize);
	if (NULL == t) return E_OUTOFMEMORY;

	cursor = t + (cursor - buffer);
	buffer = t;

	remaining += newSize - allocated;
	allocated = newSize;
	
	return S_OK;
}

void StringBuilder::Clear(void)
{
	if (NULL != buffer)
	{
		buffer[0] = L'\0';
	}
	cursor = buffer;
	remaining = allocated;
}

LPCWSTR StringBuilder::Get(void)
{
	return buffer;
}

HRESULT StringBuilder::Set(size_t index, WCHAR value)
{
	if (NULL == buffer)
		return E_POINTER;
	
	if (index >= allocated) 
		return E_INVALIDARG;

	buffer[index] = value;
    return S_OK;
}

HRESULT StringBuilder::Append(LPCWSTR pszString)
{	
	HRESULT hr;
	if (NULL == buffer)
	{
		hr = Allocate(1024);
		if (FAILED(hr)) return hr;
	}

	size_t cchCursor = remaining;
	hr = StringCchCopyEx(cursor, cchCursor, pszString, &cursor, &remaining, STRSAFE_IGNORE_NULLS);
	if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
	{
		size_t offset = cchCursor - remaining;
		size_t requested = lstrlen(pszString) + (allocated - remaining) + 1;
		size_t newsize = allocated * 2;
		while (newsize < requested) newsize = newsize * 2;
		
		hr = Allocate(newsize);
		if (FAILED(hr)) return hr;

		hr = StringCchCopyEx(cursor, remaining, pszString + offset, &cursor, &remaining, STRSAFE_IGNORE_NULLS);
	}

	return hr;
}
	