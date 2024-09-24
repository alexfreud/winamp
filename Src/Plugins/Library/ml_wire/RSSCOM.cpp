#include "main.h"
#include "./rssCOM.h"
#include "./util.h"
#include "api__ml_wire.h"
#include "./cloud.h"
#include "./feedUtil.h"
#include "./defaults.h"
#include "./errors.h"

#include "../winamp/jsapi.h"

#include "./rssCOM.h"

#include <strsafe.h>

extern Cloud cloud;


#define DISPTABLE_CLASS	 RssCOM

DISPTABLE_BEGIN()
	DISPENTRY_ADD(DISPATCH_SUBSCRIBE, L"subscribe", OnSubscribe)
DISPTABLE_END

#undef DISPTABLE_CLASS



RssCOM::RssCOM()
{}

RssCOM::~RssCOM()
{}

HRESULT RssCOM::CreateInstance(RssCOM **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new RssCOM();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

STDMETHODIMP_(ULONG) RssCOM::AddRef(void)
{
	return _ref.fetch_add( 1 );
}

STDMETHODIMP_(ULONG) RssCOM::Release(void)
{
	if (0 == _ref.load() )
		return _ref.load();
	
	LONG r = _ref.fetch_sub( 1 );
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP RssCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;

	if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = static_cast<IDispatch*>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = static_cast<IUnknown*>(this);
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


HRESULT RssCOM::OnSubscribe(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

	BSTR url;

	JSAPI_GETSTRING(url, pdispparams, 1, puArgErr);

	SubscribeUrl(url, pvarResult);

	return S_OK;
}

struct SubscribeThreadData
{
	LPWSTR url;
};
static LPCWSTR FormatLangString(LPWSTR pszBuffer, INT cchBufferMax, LPWSTR pszFormat, INT cchFormatMax, INT formatId, ...)
{
	HRESULT hr;
	va_list argList;
	va_start(argList, formatId);
	
	if(NULL != WASABI_API_LNGSTRINGW_BUF(formatId, pszFormat, cchFormatMax))
	{
		hr = StringCchVPrintfExW(pszBuffer, cchBufferMax, NULL, NULL, STRSAFE_IGNORE_NULLS, pszFormat, argList);
	}
	else
	{
		hr = E_FAIL;
	}
			
	va_end(argList);

	return (SUCCEEDED(hr)) ? pszBuffer : NULL;
}

static DWORD WINAPI SubscribeThreadProc(void *param)
{	
	SubscribeThreadData *data = (SubscribeThreadData *)param;
	Channel newFeed;
	newFeed.updateTime = updateTime;
	newFeed.autoUpdate = autoUpdate;
	newFeed.autoDownload = autoDownload;
	newFeed.autoDownloadEpisodes = autoDownloadEpisodes;
	newFeed.SetURL(data->url);
	newFeed.needsRefresh = true;
	
	WCHAR szBuffer1[2048] = {0}, szBuffer2[2048] = {0};
	LPCWSTR pszMessage = NULL, pszTitle = NULL;
		
	switch (DownloadFeedInformation(newFeed))
	{
		case DOWNLOAD_SUCCESS:
			{
				Nullsoft::Utility::AutoLock lock (channels LOCKNAME("AddURL"));
				newFeed.autoDownload = ::autoDownload;
				if (channels.AddChannel(newFeed))
					cloud.Pulse();
			}
			break;
		
		case DOWNLOAD_DUPLICATE:
			pszMessage = FormatLangString(szBuffer1, ARRAYSIZE(szBuffer1), szBuffer2, ARRAYSIZE(szBuffer2), IDS_ALREADY_SUBSCRIBED, newFeed.title, data->url);
			break;
		
		case DOWNLOAD_404:
			pszMessage = FormatLangString(szBuffer1, ARRAYSIZE(szBuffer1), szBuffer2, ARRAYSIZE(szBuffer2), IDS_FILE_NOT_FOUND, data->url);
			break;
		
		case DOWNLOAD_TIMEOUT:
			pszMessage = FormatLangString(szBuffer1, ARRAYSIZE(szBuffer1), szBuffer2, ARRAYSIZE(szBuffer2), IDS_CONNECTION_TIMED_OUT, data->url);
			break;
		
		case DOWNLOAD_ERROR_PARSING_XML:
			pszMessage = FormatLangString(szBuffer1, ARRAYSIZE(szBuffer1), szBuffer2, ARRAYSIZE(szBuffer2), IDS_ERROR_PARSING_XML, data->url);
			break;
		
		case DOWNLOAD_NOTRSS:
			pszMessage = FormatLangString(szBuffer1, ARRAYSIZE(szBuffer1), szBuffer2, ARRAYSIZE(szBuffer2), IDS_INVALID_RSS_FEED, data->url);
			break;

		case DOWNLOAD_CONNECTIONRESET:
			pszMessage = FormatLangString(szBuffer1, ARRAYSIZE(szBuffer1), szBuffer2, ARRAYSIZE(szBuffer2), IDS_CONNECTION_RESET, data->url);
			break;
		
		case DOWNLOAD_NOHTTP:
			pszMessage = WASABI_API_LNGSTRINGW_BUF(IDS_NO_JNETLIB, szBuffer1, ARRAYSIZE(szBuffer1));
			pszTitle = WASABI_API_LNGSTRINGW_BUF(IDS_JNETLIB_MISSING, szBuffer2, ARRAYSIZE(szBuffer2));
			break;

		case DOWNLOAD_NOPARSER:
			pszMessage = WASABI_API_LNGSTRINGW_BUF(IDS_NO_EXPAT, szBuffer1, ARRAYSIZE(szBuffer1));
			pszTitle = WASABI_API_LNGSTRINGW_BUF(IDS_EXPAT_MISSING, szBuffer2, ARRAYSIZE(szBuffer2));
			break;

		
	}

	if(NULL != pszMessage)
	{
		if (NULL == pszTitle)
			pszTitle = WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_SUBSCRIBING_TO_PODCAST, szBuffer2, ARRAYSIZE(szBuffer2));

		MessageBox(plugin.hwndLibraryParent, pszMessage, pszTitle, MB_ICONERROR | MB_OK);
	}

	Plugin_FreeString(data->url);
	free(data);
	return 0;
}

LPCWSTR RssCOM::GetName()
{
	return L"Podcast";
}

HRESULT RssCOM::SubscribeUrl(BSTR url, VARIANT FAR *result)
{
	HRESULT hr;
	SubscribeThreadData *data = (SubscribeThreadData*)malloc(sizeof(SubscribeThreadData));
    if (NULL != data)
	{
		data->url = Plugin_CopyString(url);
		DWORD threadId;
		HANDLE hThread = CreateThread(NULL, NULL, SubscribeThreadProc, (void*)data, NULL, &threadId);
		if (NULL == hThread)
		{
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32(error);
			Plugin_FreeString(data->url);
			free(data);
		}
		else
		{
			CloseHandle(hThread);
			hr = S_OK;
		}
	}
	else
		hr = E_OUTOFMEMORY;

	if (NULL != result)
	{
		VariantInit(result);
		V_VT(result) = VT_BOOL;
		V_BOOL(result) = (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE);
	}

	return hr;
}