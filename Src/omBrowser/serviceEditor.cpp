#include "main.h"
#include "./service.h"
#include <strsafe.h>

static HRESULT Editor_SetStr(LPWSTR *ppTarget, LPCWSTR pValue, BOOL fUtf8, BOOL fCompare, LCID locale, UINT compareFlags)
{
	if (NULL == pValue)
	{
		if (NULL != *ppTarget)
		{
			Plugin_FreeString(*ppTarget);
			*ppTarget = NULL;
			return S_OK;
		}
		return S_FALSE;
	}

	LPWSTR v = NULL;

	if (FALSE == fUtf8)
	{
		if (NULL != *ppTarget)
		{
			if (FALSE != fCompare && CSTR_EQUAL == CompareString(locale, compareFlags, *ppTarget, -1, pValue, -1))
				return S_FALSE;
		}

		v = Plugin_CopyString(pValue);
		if (NULL == v) return E_OUTOFMEMORY;
	}
	else
	{
		v = Plugin_MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pValue, -1);
		if (NULL == v) return E_OUTOFMEMORY;

		if (NULL != *ppTarget)
		{
			if (FALSE != fCompare && CSTR_EQUAL == CompareString(locale, compareFlags, *ppTarget, -1, v, -1))
			{
				Plugin_FreeString(v);
				return S_FALSE;
			}
		}
	}

	Plugin_FreeString(*ppTarget);
	*ppTarget = v;
	
	return S_OK;
}


HRESULT OmService::SetName(LPCWSTR pszName, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&name, pszName, utf8, TRUE, LOCALE_USER_DEFAULT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedName);
	return hr;
}

HRESULT OmService::SetUrl(LPCWSTR pszUrl, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&url, pszUrl, utf8, TRUE, CSTR_INVARIANT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedUrl);
	return hr;
}

HRESULT OmService::SetIcon(LPCWSTR pszPath, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&icon, pszPath, utf8, TRUE, CSTR_INVARIANT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedIcon);
	return hr;
}

HRESULT OmService::SetRating(UINT nRating)
{
	HRESULT hr;

	EnterCriticalSection(&lock);
	if (rating == nRating) 
	{
		hr = S_FALSE;
	}
	else
	{
		rating = nRating;
		hr = S_OK;
	}
	LeaveCriticalSection(&lock);

	if (S_OK == hr)
		modified.Mark(modifiedRating);
	return hr;
}

HRESULT OmService::SetVersion(UINT nVersion)
{
	HRESULT hr;
	
	EnterCriticalSection(&lock);
	if (version == nVersion) 
		hr = S_FALSE;
	else
	{	
		version = nVersion;
		hr = S_OK;
	}
	LeaveCriticalSection(&lock);

	if (S_OK == hr)
		modified.Mark(modifiedVersion);
	
	return hr;
}

HRESULT OmService::SetGeneration(UINT nGeneration)
{
	HRESULT hr;
	
	EnterCriticalSection(&lock);
	if (generation == nGeneration) 
		hr = S_FALSE;
	else
	{	
		generation = nGeneration;
		hr = S_OK;
	}
	LeaveCriticalSection(&lock);

	if (S_OK == hr)
		modified.Mark(modifiedGeneration);
	
	return hr;
}

HRESULT OmService::SetFlags(UINT nFlags, UINT nMask)
{
	HRESULT hr;
	
	EnterCriticalSection(&lock);
	UINT newFlags = (flags & ~nMask) | (nFlags & nMask);
	if (flags == newFlags) 
		hr = S_FALSE;
	else
	{	
		flags = newFlags;
		hr = S_OK;
	}
	LeaveCriticalSection(&lock);

	if (S_OK == hr)
		modified.Mark(modifiedFlags);
	
	return hr;
}

HRESULT OmService::SetDescription(LPCWSTR pszDescription, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&description, pszDescription, utf8, TRUE, LOCALE_USER_DEFAULT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedDescription);
	return hr;
}

HRESULT OmService::SetAuthorFirst(LPCWSTR pszName, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&authorFirst, pszName, utf8, TRUE, LOCALE_USER_DEFAULT, 0);
	LeaveCriticalSection(&lock);
	if (S_OK == hr) modified.Mark(modifiedAuthorFirst);
	return hr;
}

HRESULT OmService::SetAuthorLast(LPCWSTR pszName, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&authorLast, pszName, utf8, TRUE, LOCALE_USER_DEFAULT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedAuthorLast);
	return hr;
}

HRESULT OmService::SetUpdated(LPCWSTR pszDate, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&updated, pszDate, utf8, TRUE, CSTR_INVARIANT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedUpdated);
	return hr;
}

HRESULT OmService::SetPublished(LPCWSTR pszDate, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&published, pszDate, utf8, TRUE, CSTR_INVARIANT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedPublished);
	return hr;
}

HRESULT OmService::SetThumbnail(LPCWSTR pszPath, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&thumbnail, pszPath, utf8, TRUE, CSTR_INVARIANT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedThumbnail);
	return hr;
}

HRESULT OmService::SetScreenshot(LPCWSTR pszPath, BOOL utf8)
{
	EnterCriticalSection(&lock);
	HRESULT hr = Editor_SetStr(&screenshot, pszPath, utf8, TRUE, CSTR_INVARIANT, 0);
	LeaveCriticalSection(&lock);

	if (S_OK == hr) modified.Mark(modifiedScreenshot);
	return hr;
}

HRESULT OmService::SetModified(UINT nFlags, UINT nMask)
{
	modified.Set(nFlags, nMask);
	return S_OK;
}

HRESULT OmService::GetModified(UINT *pFlags)
{
	if (NULL == pFlags) return E_POINTER;
	*pFlags = modified.Get();
	return S_OK;
}

HRESULT OmService::BeginUpdate()
{
	modified.BeginUpdate();
	return S_OK;
}

HRESULT OmService::EndUpdate()
{
	modified.EndUpdate();
	return S_OK;
}