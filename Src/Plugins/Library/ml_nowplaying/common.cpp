#include "./common.h"
#include "./wasabi.h"

#include <strsafe.h>

LPWSTR Plugin_MallocString(size_t cchLen)
{
	return (LPWSTR)calloc(cchLen, sizeof(WCHAR));
}

void Plugin_FreeString(LPWSTR pszString)
{
	if (NULL != pszString)
	{
		free(pszString);
	}
}

LPWSTR Plugin_ReAllocString(LPWSTR pszString, size_t cchLen)
{
	return (LPWSTR)realloc(pszString, sizeof(WCHAR) * cchLen);
}

LPWSTR Plugin_CopyString(LPCWSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenW(pszSource) + 1;
		
	LPWSTR copy = Plugin_MallocString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(WCHAR) * cchSource);
	}
	return copy;
}

LPSTR Plugin_MallocAnsiString(size_t cchLen)
{
	return (LPSTR)calloc(cchLen, sizeof(CHAR));
}

LPSTR Plugin_CopyAnsiString(LPCSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchSource = lstrlenA(pszSource) + 1;
		
	LPSTR copy = Plugin_MallocAnsiString(cchSource);
	if (NULL != copy)
	{
		CopyMemory(copy, pszSource, sizeof(CHAR) * cchSource);
	}
	return copy;

}
void Plugin_FreeAnsiString(LPSTR pszString)
{
	Plugin_FreeString((LPWSTR)pszString);
}

LPSTR Plugin_WideCharToMultiByte(UINT codePage, DWORD dwFlags, LPCWSTR lpWideCharStr, INT cchWideChar, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
	INT cchBuffer = WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, NULL, 0, lpDefaultChar, lpUsedDefaultChar);
	if (0 == cchBuffer) return NULL;
	
	LPSTR buffer = Plugin_MallocAnsiString(cchBuffer);
	if (NULL == buffer) return NULL; 
		
	if (0 == WideCharToMultiByte(codePage, dwFlags, lpWideCharStr, cchWideChar, buffer, cchBuffer, lpDefaultChar, lpUsedDefaultChar))
	{
		Plugin_FreeAnsiString(buffer);
		return NULL;
	}
	return buffer;
}

LPWSTR Plugin_MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR lpMultiByteStr, INT cbMultiByte)
{
	if (NULL == lpMultiByteStr) return NULL;
	INT cchBuffer = MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, NULL, 0);
	if (NULL == cchBuffer) return NULL;
	
	if (cbMultiByte > 0) cchBuffer++;
	
	LPWSTR buffer = Plugin_MallocString(cchBuffer);
	if (NULL == buffer) return NULL;

	if (0 == MultiByteToWideChar(codePage, dwFlags, lpMultiByteStr, cbMultiByte, buffer, cchBuffer))
	{
		Plugin_FreeString(buffer);
		return NULL;
	}

	if (cbMultiByte > 0)
	{
		buffer[cchBuffer - 1] = L'\0';
	}
	return buffer;
}


LPWSTR Plugin_DuplicateResString(LPCWSTR pszResource)
{
	return (IS_INTRESOURCE(pszResource)) ? 
			(LPWSTR)pszResource : 
			Plugin_CopyString(pszResource);
}

void Plugin_FreeResString(LPWSTR pszResource)
{
	if (!IS_INTRESOURCE(pszResource))
		Plugin_FreeString(pszResource);
}

HRESULT Plugin_CopyResString(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszString)
{
	if (NULL == pszBuffer)
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	if (NULL == pszString)
	{
		pszBuffer[0] = L'\0';
	}
	else if (IS_INTRESOURCE(pszString))
	{
		if (NULL == WASABI_API_LNG)
			hr = E_FAIL;
		else
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszString, pszBuffer, cchBufferMax);
	}
	else
	{
		hr = StringCchCopy(pszBuffer, cchBufferMax, pszString);
	}
	return hr;
}

void Plugin_SafeRelease(IUnknown *pUnk)
{
	if (NULL != pUnk)
		pUnk->Release();
}