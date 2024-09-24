#include "main.h"
#include "./forceUrl.h"

typedef struct __FORCEURLDATA
{
	UINT	serviceId;
	LPWSTR	url;
} FORCEURLDATA;

#define FORCEURLPROP		L"MLWEBDEV_FORCEURL"

void ForceUrl_Remove()
{
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return;

	FORCEURLDATA *data = (FORCEURLDATA*)GetProp(hLibrary, FORCEURLPROP);
	RemoveProp(hLibrary, FORCEURLPROP);
	if (NULL != data)
	{
		Plugin_FreeString(data->url);
		free(data);
	}
}

HRESULT ForceUrl_Set(UINT serviceId, LPCWSTR pszUrl)
{
	if (NULL == pszUrl) return E_INVALIDARG;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_FAIL;

	FORCEURLDATA *data = (FORCEURLDATA*)GetProp(hLibrary, FORCEURLPROP);

	if (NULL != data)
	{
		Plugin_FreeString(data->url);
		if (data->serviceId != serviceId)
		{
			free(data);
			data = NULL;
		}
	}

	if (NULL == data)
	{
		data = (FORCEURLDATA*)malloc(sizeof(FORCEURLDATA));
		if (NULL == data) return E_OUTOFMEMORY;
		data->serviceId = serviceId;
	}
	
	data->url = Plugin_CopyString(pszUrl);
	if (NULL == data->url || FALSE == SetProp(hLibrary, FORCEURLPROP, data))
	{
		ForceUrl_Remove();
		return E_FAIL;
	}

	return S_OK;
}

HRESULT ForceUrl_Get(UINT serviceId, const wchar_t **ppszUrl)
{
	if (NULL == ppszUrl) return E_POINTER;
	*ppszUrl = NULL;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_FAIL;

	FORCEURLDATA *data = (FORCEURLDATA*)GetProp(hLibrary, FORCEURLPROP);

	if (NULL == data || data->serviceId != serviceId)
		return E_NOINTERFACE;

    *ppszUrl = data->url;
	return S_OK;
}