#include "./main.h"
#include "./service.h"

#include <strsafe.h>

HRESULT OmService::GetDescription(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, description, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT OmService::GetAuthorFirst(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, authorFirst, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT OmService::GetAuthorLast(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, authorLast, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT OmService::GetPublished(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;

	EnterCriticalSection(&lock);

	LPCWSTR source = (NULL != published && L'\0' != *published) ? published : updated;
	HRESULT hr = StringCchCopyEx(pszBuffer, cchBufferMax, source, NULL, NULL, STRSAFE_IGNORE_NULLS);

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmService::GetUpdated(LPWSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer) return E_POINTER;

	EnterCriticalSection(&lock);

	LPCWSTR source = (NULL != updated && L'\0' != *updated) ? updated : published;
	HRESULT hr = StringCchCopyEx(pszBuffer, cchBufferMax, source, NULL, NULL, STRSAFE_IGNORE_NULLS);

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmService::GetThumbnail(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, thumbnail, NULL, NULL, STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
}

HRESULT OmService::GetScreenshot(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, screenshot, NULL, NULL, STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
}