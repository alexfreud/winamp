#include "main.h"
#include "./forceUrl.h"

ForceUrl::ForceUrl() : id((UINT)-1), url(NULL)
{
}
ForceUrl::~ForceUrl()
{
	Plugin_FreeString(url);
}

HRESULT ForceUrl::Set(UINT serviceId, LPCWSTR pszUrl)
{
	Plugin_FreeString(url);
	
	id = serviceId;
	url = Plugin_CopyString(pszUrl);

	return S_OK;
}

HRESULT ForceUrl::Peek(UINT serviceId, LPWSTR *pszUrl)
{
	if (NULL == pszUrl) return E_POINTER;
	if (serviceId == id && NULL != url)
	{
		*pszUrl = url;
		url = NULL;
		id = ((UINT)-1);
		return S_OK;
	}

	return S_FALSE;
}

HRESULT ForceUrl::Remove(UINT serviceId)
{
	if (id == serviceId)
	{
		Plugin_FreeString(url);
		url = NULL;
		id = ((UINT)-1);
		return S_OK;
	}
	return S_FALSE;
}	

void ForceUrl::FreeString(LPWSTR pszValue)
{
	Plugin_FreeString(pszValue);
}
