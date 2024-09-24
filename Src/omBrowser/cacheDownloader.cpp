#include "main.h"

#include "./cacheDownloader.h"
#include "./cacheRecord.h"
#include "./ifc_wasabihelper.h"

#include "..\Components\wac_network\wac_network_http_receiver_api.h"

#include <strsafe.h>

CacheDownloader::CacheDownloader(CacheRecord *record, BOOL fEnableCompression)
	: ref(1), manager(NULL), cookie(NULL), owner(record), state(stateReady), enableCompression(fEnableCompression)
{
	InitializeCriticalSection(&lock);
}

CacheDownloader::~CacheDownloader()
{
	EnterCriticalSection( &lock );

	if ( manager != NULL )
	{
		if ( NULL != cookie )
		{
			Abort();
		}

		ifc_wasabihelper *wasabi;
		if ( SUCCEEDED( Plugin_GetWasabiHelper( &wasabi ) ) )
		{
			wasabi->ReleaseWasabiInterface( &DownloadManagerGUID, (void **)&manager );
			wasabi->Release();
		}
		manager = NULL;
	}

	LeaveCriticalSection( &lock );
	DeleteCriticalSection( &lock );
}

HRESULT CacheDownloader::CreateInstance(CacheRecord *record, LPCWSTR pszAddress, BOOL fEnableCompression, CacheDownloader **instance)
{

	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	if (NULL == record || NULL == pszAddress || L'\0' == *pszAddress)
		return E_INVALIDARG;

	HRESULT hr;

	*instance = new CacheDownloader(record, fEnableCompression);
	if (NULL == *instance) 
	{
		hr = E_OUTOFMEMORY;
	}
	else
	{
		hr = (*instance)->Start(pszAddress);
		if (FAILED(hr))
		{
			(*instance)->Release();
			*instance = NULL;
		}
	}

	return hr;

}

size_t CacheDownloader::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t CacheDownloader::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int CacheDownloader::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	*object = NULL;
	return E_NOINTERFACE;
	
}

HRESULT CacheDownloader::Start(LPCWSTR pszAddress)
{	
	HRESULT hr = S_OK;

	LPSTR address = Plugin_WideCharToMultiByte(CP_ACP, 0, pszAddress, -1, NULL, NULL);
	if (NULL == address) return E_OUTOFMEMORY;


	EnterCriticalSection(&lock);
	state = stateInitializing;
	LeaveCriticalSection(&lock);

	if ( manager == NULL )
	{
		ifc_wasabihelper *wasabi;
		hr = Plugin_GetWasabiHelper(&wasabi);
		if (SUCCEEDED(hr))
		{
			hr = wasabi->QueryWasabiInterface(&DownloadManagerGUID, (void**)&manager);
			wasabi->Release();
		}
	}

	if (SUCCEEDED(hr))
	{
		cookie = manager->DownloadEx(address, this, api_downloadManager::DOWNLOADEX_TEMPFILE);
		if (NULL == cookie)
		{
			hr = E_FAIL;
		}
	}

	if (FAILED(hr))
	{
		EnterCriticalSection(&lock);
		state = stateCompleted;
		LeaveCriticalSection(&lock);
	}

	Plugin_FreeAnsiString(address);
	return hr;
}

HRESULT CacheDownloader::Abort()
{
	HRESULT hr;
	EnterCriticalSection(&lock);
	
	if (NULL == manager || NULL == cookie) 
	{
		hr = E_UNEXPECTED;
	}
	else if (stateCompleted == state) 
	{
		hr = S_FALSE;
	}
	else
	{
		state = stateAborting;
		manager->CancelDownload(cookie);
		hr = S_OK;
	}

	LeaveCriticalSection(&lock);
	return hr;
}

UINT CacheDownloader::GetState()
{
	return state;
}

HRESULT CacheDownloader::SetOwner(CacheRecord *record)
{
	owner = record;
	return S_OK;
}

HRESULT CacheDownloader::DownloadCompleted(INT errorCode)
{
	AddRef();
	EnterCriticalSection(&lock);

	state = stateCompleted;
	
	if (NULL != owner)
	{		
		LPCWSTR pszFile = (api_downloadManager::TICK_SUCCESS == errorCode) ? manager->GetLocation(cookie) : NULL;

		owner->DownloadCompleted(pszFile, errorCode);
		owner = NULL;
	}

	manager->ReleaseDownload(cookie);
	cookie = NULL;

	ifc_wasabihelper *wasabi;
	if (SUCCEEDED(Plugin_GetWasabiHelper(&wasabi)))
	{
		wasabi->ReleaseWasabiInterface(&DownloadManagerGUID, (void**)&manager);
		wasabi->Release();
		manager = NULL;
	}
	
	LeaveCriticalSection(&lock);
	
	Release();
	return S_OK;
}

void CacheDownloader::OnInit(DownloadToken token)
{
	EnterCriticalSection(&lock);

	cookie = token;
	if (NULL != cookie)
	{
		manager->RetainDownload(cookie);
	}

	state = stateConnecting;

	if (FALSE != enableCompression)
	{
		api_httpreceiver *receiver = manager->GetReceiver(token);
		if (NULL != receiver)
			receiver->AllowCompression();
	}

	LeaveCriticalSection(&lock);
}

void CacheDownloader::OnFinish(DownloadToken token)
{		
	DownloadCompleted(api_downloadManager::TICK_SUCCESS);
}

void CacheDownloader::OnError(DownloadToken token, int errorCode)
{
	DownloadCompleted(errorCode);
}

void CacheDownloader::OnCancel(DownloadToken token)
{
	DownloadCompleted(api_downloadManager::TICK_NODATA);
}

void CacheDownloader::OnTick(DownloadToken token)
{
	EnterCriticalSection(&lock);

	if (stateConnecting == state)
        state = stateReceiving;
	LeaveCriticalSection(&lock);

}

void CacheDownloader::OnConnect(DownloadToken dlToken)
{
	EnterCriticalSection(&lock);
	state = stateReceiving;
	LeaveCriticalSection(&lock);
}


#define CBCLASS CacheDownloader
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONTICK, OnTick)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONERROR, OnError)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, OnCancel)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, OnConnect)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit)
END_DISPATCH;
#undef CBCLASS