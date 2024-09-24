#include "main.h"
#include "./util.h"
#include "api__ml_downloads.h"

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

HRESULT Plugin_FileExtensionFromUrl(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszUrl, LPCWSTR defaultExtension)
{
	LPCWSTR cursor = pszUrl;
	while (L'\0' != *cursor && L'?' != *cursor)
		cursor = CharNext(cursor);
	
	LPCWSTR end = cursor;
	
	while (cursor != pszUrl && L'/' != *cursor && L'\\' != *cursor && L'.' != *cursor)
		cursor = CharPrev(pszUrl, cursor);
	
	if (L'.' == *cursor && cursor != pszUrl)
	{
		if (CharNext(cursor) < end) 
		{
			INT cchExtension = (INT)(INT_PTR)(end - cursor);
			return StringCchCopyN(pszBuffer, cchBufferMax, cursor, cchExtension);
		}
	}

	return StringCchCopy(pszBuffer, cchBufferMax, defaultExtension);
}

void Plugin_ReplaceBadPathChars(LPWSTR pszPath) 
{
	if (NULL == pszPath) 
		return;

	while (L'\0' != *pszPath) 
	{
		switch(*pszPath)
		{
			case L'?':
			case L'/':
			case L'\\':
			case L':':
			case L'*':
			case L'\"':
			case L'<':
			case L'>':
			case L'|':
				*pszPath = L'_';
				break;
			default:
				if (*pszPath < 32) *pszPath = L'_';
				break;
		}
		pszPath = CharNextW(pszPath);
	}
}

INT Plugin_CleanDirectory(LPWSTR pszPath) 
{
	if (NULL == pszPath) 
		return 0;

	INT cchPath = lstrlen(pszPath);
	LPWSTR cursor = pszPath + cchPath;
	while (cursor-- != pszPath && (L' ' == *cursor || L'.' == *cursor)) 
		*cursor = L'\0';
	
	return (cchPath - (INT)(INT_PTR)(cursor - pszPath) - 1);
}

HRESULT Plugin_EnsurePathExist(LPCWSTR pszDirectory)
{
	DWORD ec = ERROR_SUCCESS;

	UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

	if (0 == CreateDirectory(pszDirectory, NULL))
	{
		ec = GetLastError();
		if (ERROR_PATH_NOT_FOUND == ec)
		{
			LPCWSTR pszBlock = pszDirectory;
			WCHAR szBuffer[MAX_PATH] = {0};
			
			LPCTSTR pszCursor = PathFindNextComponent(pszBlock);
			ec = (pszCursor == pszBlock || S_OK != StringCchCopyN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock))) ?
					ERROR_INVALID_NAME : ERROR_SUCCESS;
			
			pszBlock = pszCursor;
			
			while (ERROR_SUCCESS == ec && NULL != (pszCursor = PathFindNextComponent(pszBlock)))
			{
				if (pszCursor == pszBlock || S_OK != StringCchCatN(szBuffer, ARRAYSIZE(szBuffer), pszBlock, (pszCursor - pszBlock)))
					ec = ERROR_INVALID_NAME;

				if (ERROR_SUCCESS == ec && !CreateDirectory(szBuffer, NULL))
				{
					ec = GetLastError();
					if (ERROR_ALREADY_EXISTS == ec) ec = ERROR_SUCCESS;
				}
				pszBlock = pszCursor;
			}
		}

		if (ERROR_ALREADY_EXISTS == ec) 
			ec = ERROR_SUCCESS;
	}

	SetErrorMode(errorMode);
	SetLastError(ec);
	return HRESULT_FROM_WIN32(ec);
}