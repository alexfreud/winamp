#include "main.h"
#include "./storageDwnld.h"
#include "./ifc_omwebstorage.h"
#include "./ifc_wasabihelper.h"
#include "./ifc_omservicehost.h"
#include "..\Components\wac_network\wac_network_http_receiver_api.h"

OmStorageDwnld::OmStorageDwnld(api_downloadManager *downloadManager, BOOL enableCompression)
	: ref(1), userCallback(NULL), userData(NULL), completed(NULL), flags(0),
	  opState(stateInitializing), resultCode(api_downloadManager::TICK_SUCCESS),
	  cookie(NULL), manager(downloadManager), serviceHost(NULL)	
{
	if (NULL != manager)
		manager->AddRef();

	if (FALSE != enableCompression)
		flags |= flagEnableCompression;

	InitializeCriticalSection(&lock);
}

OmStorageDwnld::~OmStorageDwnld()
{
	EnterCriticalSection(&lock);

	if (NULL != manager)
	{
		if (NULL != cookie)
		{
			manager->ReleaseDownload(cookie);
			cookie = NULL;
		}

		manager->Release();
		manager = NULL;
	}

	if (NULL != completed)
	{
		CloseHandle(completed);
		completed = NULL;
	}

	if (NULL != serviceHost)
	{
		serviceHost->Release();
		serviceHost = NULL;
	}
		
	LeaveCriticalSection(&lock);

	DeleteCriticalSection(&lock);
}

HRESULT OmStorageDwnld::CreateInstance(api_downloadManager *downloadManager, BOOL enableCompression, OmStorageDwnld **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = NULL;
	if (NULL == downloadManager)
		return E_INVALIDARG;

	*instance = new OmStorageDwnld(downloadManager, enableCompression);
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

size_t OmStorageDwnld::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmStorageDwnld::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmStorageDwnld::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmStorageAsync))
		*object = static_cast<ifc_omstorageasync*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT OmStorageDwnld::GetState(UINT *state)
{
	if (NULL == state)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*state  = opState;
	LeaveCriticalSection(&lock);
	
	return S_OK;
}

HRESULT OmStorageDwnld::GetWaitHandle(HANDLE *handle)
{
	if (NULL == handle)
		return E_POINTER;

	HRESULT hr = S_OK;

	EnterCriticalSection(&lock);

	if (NULL == completed)
	{
		completed = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == completed)
		{
			*handle = NULL;
			DWORD error = GetLastError();
			hr = HRESULT_FROM_WIN32(error);
		}
	}

	if (SUCCEEDED(hr) && FALSE == DuplicateHandle(GetCurrentProcess(), completed, 
					GetCurrentProcess(), handle, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		*handle = NULL;
		DWORD error = GetLastError();
		hr = HRESULT_FROM_WIN32(error);
	}
	
	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmStorageDwnld::GetData(void **data)
{
	if (NULL == data)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*data = userData;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmStorageDwnld::GetCallback(AsyncCallback *callback)
{
	if (NULL == callback)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*callback = userCallback;
	LeaveCriticalSection(&lock);

	return S_OK;
}

void OmStorageDwnld::OnInit(DownloadToken token)
{
	EnterCriticalSection(&lock);

	AddRef();
	cookie = token;
	opState = stateConnecting;

	if (NULL != manager)
	{
		manager->RetainDownload(cookie);
		
		if (0 != (flagEnableCompression & flags))
		{
			api_httpreceiver *receiver = manager->GetReceiver(token);
			if (NULL != receiver)
				receiver->AllowCompression();
		}
	}

	LeaveCriticalSection(&lock);
}

void OmStorageDwnld::OnConnect(DownloadToken token)
{
	EnterCriticalSection(&lock);
	opState = stateReceiving;
	LeaveCriticalSection(&lock);
}

void OmStorageDwnld::OnTick(DownloadToken token)
{
	EnterCriticalSection(&lock);

	if (stateConnecting == opState)
        opState = stateReceiving;	

	LeaveCriticalSection(&lock);
}

void OmStorageDwnld::OnFinish(DownloadToken token)
{
	DownloadCompleted(api_downloadManager::TICK_SUCCESS);
}

void OmStorageDwnld::OnError(DownloadToken token, int errorCode)
{
	DownloadCompleted(errorCode);
}

void OmStorageDwnld::OnCancel(DownloadToken token)
{
	DownloadCompleted(api_downloadManager::TICK_NODATA);
}

void OmStorageDwnld::DownloadCompleted( INT errorCode )
{
	EnterCriticalSection( &lock );

	resultCode = errorCode;
	opState = stateCompleted;
	HANDLE event = completed;

	LeaveCriticalSection( &lock );

	if ( NULL != event )
	{
		SetEvent( event );
	}

	EnterCriticalSection( &lock );
	AsyncCallback cb = userCallback;
	LeaveCriticalSection( &lock );

	if ( NULL != cb )
	{
		cb( this );
	}

	Release();
}

HRESULT OmStorageDwnld::SetData(void *data)
{
	EnterCriticalSection(&lock);
	userData = data;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmStorageDwnld::SetCallback(AsyncCallback callback)
{
	EnterCriticalSection(&lock);
	userCallback = callback;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmStorageDwnld::GetResultCode()
{
	EnterCriticalSection(&lock);

	HRESULT hr;
	if (NULL == cookie || NULL == manager)
	{
		hr = E_UNEXPECTED;
	}
	else
	{
		switch(resultCode)
		{
			case api_downloadManager::TICK_SUCCESS:			hr = E_DWNLD_OK; break;
			case api_downloadManager::TICK_FAILURE:			hr = (0 != (flagUserAbort & flags)) ? E_DWNLD_ABORT : E_DWNLD_FAIL; break;
			case api_downloadManager::TICK_TIMEOUT:			hr = E_DWNLD_TIMEOUT; break;
			case api_downloadManager::TICK_CANT_CONNECT:	hr = E_DWNLD_CANT_CONNECT; break;
			case api_downloadManager::TICK_WRITE_ERROR:		hr = E_DWNLD_WRITE_ERROR; break;
			default: hr = E_DWNLD_BUSY; break;
		}
	}

	LeaveCriticalSection(&lock);

	return hr;
}

HRESULT OmStorageDwnld::GetBuffer(void **buffer, size_t *bufferSize)
{
	if (NULL == buffer) return E_POINTER;

	EnterCriticalSection(&lock);

	HRESULT hr = GetResultCode();
	if (SUCCEEDED(hr) && 0 != manager->GetBuffer(cookie, buffer, bufferSize))
		hr = E_DWNLD_FAIL;

	LeaveCriticalSection(&lock);

	if (FAILED(hr))
	{
		*buffer = NULL;
		*bufferSize = 0;
	}

	return hr;
}

HRESULT OmStorageDwnld::RequestAbort(BOOL fDrop)
{
	EnterCriticalSection(&lock);
	if (FALSE != fDrop)
	{
		userData = NULL;
		userCallback = NULL;
	}

	if (NULL != cookie && NULL != manager)
	{
		opState = stateAborting;
		flags |= flagUserAbort;
		manager->CancelDownload(cookie);
	}

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmStorageDwnld::SetServiceHost(ifc_omservicehost *host)
{
	EnterCriticalSection(&lock);
	
	if (NULL != serviceHost)
		serviceHost->Release();

	serviceHost = host;
	if (NULL != serviceHost)
		serviceHost->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmStorageDwnld::GetServiceHost(ifc_omservicehost **host)
{
	if (NULL == host)
		return E_POINTER;

	EnterCriticalSection(&lock);

	*host = serviceHost;
	if (NULL != serviceHost)
		serviceHost->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

#define CBCLASS OmStorageDwnld
START_MULTIPATCH;
 START_PATCH(MPIID_OMSTORAGEASYNC)
  M_CB(MPIID_OMSTORAGEASYNC, ifc_omstorageasync, ADDREF, AddRef);
  M_CB(MPIID_OMSTORAGEASYNC, ifc_omstorageasync, RELEASE, Release);
  M_CB(MPIID_OMSTORAGEASYNC, ifc_omstorageasync, QUERYINTERFACE, QueryInterface);
  M_CB(MPIID_OMSTORAGEASYNC, ifc_omstorageasync, API_GETSTATE, GetState);
  M_CB(MPIID_OMSTORAGEASYNC, ifc_omstorageasync, API_GETWAITHANDLE, GetWaitHandle);
  M_CB(MPIID_OMSTORAGEASYNC, ifc_omstorageasync, API_GETDATA, GetData);
 NEXT_PATCH(MPIID_DOWNLOADCALLBACK)
  M_CB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, ADDREF, AddRef);
  M_CB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, RELEASE, Release);
  M_CB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, QUERYINTERFACE, QueryInterface);
  M_VCB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, OnFinish);
  M_VCB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, IFC_DOWNLOADMANAGERCALLBACK_ONTICK, OnTick);
  M_VCB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, IFC_DOWNLOADMANAGERCALLBACK_ONERROR, OnError);
  M_VCB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, OnCancel);
  M_VCB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, OnConnect);
  M_VCB(MPIID_DOWNLOADCALLBACK, ifc_downloadManagerCallback, IFC_DOWNLOADMANAGERCALLBACK_ONINIT, OnInit);
 END_PATCH
END_MULTIPATCH;
#undef CBCLASS