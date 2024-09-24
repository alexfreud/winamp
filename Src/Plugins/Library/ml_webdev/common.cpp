#include "./common.h"
#include "./wasabi.h"
#include "./main.h"

#include "../winamp/wa_ipc.h"

#include <strsafe.h>

LPWSTR Plugin_MallocString(size_t cchLen)
{
	return (LPWSTR)malloc(sizeof(WCHAR) * cchLen);
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
	return (LPSTR)malloc(sizeof(CHAR) * cchLen);
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

HRESULT Plugin_MakeResourcePath(LPWSTR pszBuffer, INT cchBufferMax, LPCWSTR pszType, LPCWSTR pszName, UINT flags)
{
	HINSTANCE hInstance = WASABI_API_LNG_HINST;
	if (NULL == hInstance || NULL == FindResource(hInstance, pszName, pszType))
		hInstance = Plugin_GetInstance();

	if (NULL == OMUTILITY)
		return E_UNEXPECTED;

	return OMUTILITY->MakeResourcePath(pszBuffer, cchBufferMax, hInstance, pszType, pszName, flags);
}

HWND Plugin_GetDialogOwner(void)
{
	HWND hOwner= Plugin_GetLibrary();
	if (NULL == hOwner || FALSE == IsWindowVisible(hOwner) ||
		FALSE == IsWindowEnabled(hOwner))
	{
		hOwner = Plugin_GetWinamp();
		if (NULL != hOwner)
		{
			HWND hDlgParent = (HWND)SENDWAIPC(hOwner, IPC_GETDIALOGBOXPARENT, 0L);
			if (NULL != hDlgParent) 
				hOwner = hDlgParent;
		}
	}
	return hOwner;
}

HRESULT Plugin_AppendFileFilter(LPTSTR pszBuffer, size_t cchBufferMax, LPCTSTR pName, LPCTSTR pFilter, LPTSTR *ppBufferOut, size_t *pRemaining, BOOL bShowFilter)
{
	HRESULT hr;
	
	LPTSTR pCursor = pszBuffer;

	if (NULL != ppBufferOut)
		*ppBufferOut = pszBuffer;

	if (NULL != pRemaining)
		*pRemaining = cchBufferMax;

	if (NULL == pszBuffer || NULL == pName || NULL == pFilter)
		return E_INVALIDARG;

	pszBuffer[0] = TEXT('\0');

	hr = StringCchCopyEx(pCursor, cchBufferMax, pName, &pCursor, &cchBufferMax, 
			STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
	if (bShowFilter && SUCCEEDED(hr))
	{		
		LPTSTR p = pCursor;
		hr = StringCchPrintfEx(pCursor, cchBufferMax, &pCursor, &cchBufferMax, 
				STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE, TEXT(" (%s)"), pFilter);
		if (SUCCEEDED(hr) && p != pCursor)
			CharLowerBuff(p, (INT)(INT_PTR)(pCursor - p)); 
			
	}
	if (SUCCEEDED(hr))
	{
		pCursor++;
		cchBufferMax--;
		hr = StringCchCopyEx(pCursor, cchBufferMax, pFilter, &pCursor, &cchBufferMax, 
			STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
	}

	if (cchBufferMax < 1)
		hr = STRSAFE_E_INSUFFICIENT_BUFFER;
	
	pCursor++;
	cchBufferMax--;

	if (SUCCEEDED(hr))
	{
		pCursor[0] = TEXT('\0');
		if (NULL != ppBufferOut)
			*ppBufferOut = pCursor;
		if (NULL != pRemaining)
			*pRemaining = cchBufferMax;
	}
	else 
	{
		pszBuffer[0] = TEXT('\0');
		pszBuffer[1] = TEXT('\0');
	}

	return hr;
}
