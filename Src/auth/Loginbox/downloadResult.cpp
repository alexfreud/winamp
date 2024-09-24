#include "./downloadResult.h"
#include "./loginStatus.h"
#include "../api.h"
#include "../resource.h"
#include "../jnetlib/api_httpget.h"

#include <strsafe.h>

LoginDownloadResult::LoginDownloadResult(api_downloadManager *pManager, UINT uType, Callback fnCallback, void *pData, LoginStatus *pStatus) 
	: ref(1), manager(pManager), flags(0), callback(fnCallback), data(pData),
	address(NULL), result(api_downloadManager::TICK_NODATA), 
	cookie(0), completed(NULL), status(pStatus), statusCookie((UINT)-1)
{
	InitializeCriticalSection(&lock);
	
	SetType(uType);
	SetState(stateInitializing);

	if (NULL != status)
		status->AddRef();

	if (NULL != manager)
		manager->AddRef();
}

LoginDownloadResult::~LoginDownloadResult()
{
	EnterCriticalSection(&lock);
	
	if(NULL != manager)
	{
		if (0 != cookie)
		{
			manager->ReleaseDownload(cookie);
			cookie = 0;
		}

		manager->Release();
	}

	if (NULL != completed)
		CloseHandle(completed);
	
	if (NULL != address)
		free(address);

	if (NULL != status)
	{
		if (((UINT)-1) != statusCookie)
			status->Remove(statusCookie);

		status->Release();
	}

	LeaveCriticalSection(&lock);
	DeleteCriticalSection(&lock);
}

HRESULT LoginDownloadResult::CreateInstance(api_downloadManager *pManager, UINT uType, Callback fnCallback, void *pData, LoginStatus *pStatus, LoginDownloadResult **instance)
{
	if (NULL == instance) return E_POINTER;
	
	if (NULL == pManager) 
	{
		*instance = NULL;
		return E_INVALIDARG;
	}
	*instance = new LoginDownloadResult(pManager, uType, fnCallback, pData, pStatus);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;

}

size_t LoginDownloadResult::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t LoginDownloadResult::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int LoginDownloadResult::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	*object = NULL;
	return E_NOINTERFACE;
}

void LoginDownloadResult::SetState(UINT uState)
{
	flags = (flags & ~stateMask) | (uState & stateMask);
}

void LoginDownloadResult::SetType(UINT uType)
{
	flags = (flags & ~typeMask) | (uType & typeMask);
}

void LoginDownloadResult::SetFlags(UINT uFlags, UINT uMask)
{
	uMask &= flagsMask;
	flags = (flags & ~uMask) | (uFlags & uMask);
}


HRESULT LoginDownloadResult::GetState(UINT *state)
{
	if (NULL == state)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*state  = (flags & stateMask);
	LeaveCriticalSection(&lock);
	
	return S_OK;
}

HRESULT LoginDownloadResult::GetType(UINT *type)
{
	if (NULL == type)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*type  = (flags & typeMask);
	LeaveCriticalSection(&lock);
	
	return S_OK;
}

HRESULT LoginDownloadResult::GetFile(LPCWSTR *ppszPath)
{
	EnterCriticalSection(&lock);

	HRESULT hr;
	if (NULL == cookie || NULL == manager)
	{
		hr = E_UNEXPECTED;
	}
	else if (stateCompleted != (stateMask & flags))
	{
		hr = E_DWNLD_BUSY;
	}
	else
	{
		switch(result)
		{
			case api_downloadManager::TICK_SUCCESS:			hr = E_DWNLD_OK; break;
			case api_downloadManager::TICK_FAILURE:			hr = (0 != (flagUserAbort & flags)) ? E_DWNLD_ABORT : E_DWNLD_FAIL; break;
			case api_downloadManager::TICK_TIMEOUT:			hr = E_DWNLD_TIMEOUT; break;
			case api_downloadManager::TICK_CANT_CONNECT:		hr = E_DWNLD_CANT_CONNECT; break;
			case api_downloadManager::TICK_WRITE_ERROR:		hr = E_DWNLD_WRITE_ERROR; break;
			default: hr = E_DWNLD_BUSY; break;
		}
	}
	
	if (NULL != ppszPath)
	{
		if (SUCCEEDED(hr))
			*ppszPath = manager->GetLocation(cookie);
		else
			*ppszPath = NULL;
	}

	LeaveCriticalSection(&lock);
	
	return hr;
}
	
HRESULT LoginDownloadResult::GetWaitHandle(HANDLE *handle)
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

HRESULT LoginDownloadResult::GetData(void **data)
{
	if (NULL == data)
		return E_POINTER;

	EnterCriticalSection(&lock);
	*data = this->data;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT LoginDownloadResult::RequestAbort(BOOL fDrop)
{
	EnterCriticalSection(&lock);
	if (FALSE != fDrop)
	{
		data = NULL;
		callback = NULL;
	}

	if (0 != cookie && NULL != manager && 0 == (flagUserAbort & flags))
	{
		manager->CancelDownload(cookie);
		SetState(stateAborting);
		SetFlags(flagUserAbort, flagUserAbort);
	}
	
	LeaveCriticalSection(&lock);
	return S_OK;

}

HRESULT LoginDownloadResult::GetUrl(LPSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	HRESULT hr;
	EnterCriticalSection(&lock);

	if(NULL == manager || 0 == cookie)
		hr = E_UNEXPECTED;
	else
	{
		api_httpreceiver *receiver = manager->GetReceiver(cookie);
		if (NULL == receiver) hr = E_FAIL;
		else
		{
			LPCSTR url = receiver->get_url();
			hr = StringCchCopyExA(pszBuffer, cchBufferMax, url, NULL, NULL, STRSAFE_IGNORE_NULLS);
		}
	}
	
	LeaveCriticalSection(&lock);
	return hr;
}

void LoginDownloadResult::SetStatus()
{
	EnterCriticalSection(&lock);

	if (NULL != status && ((UINT)-1 == statusCookie))
	{
		LPCWSTR pszStatus;
		switch(typeMask & flags)
		{
			case typeProviderList:	pszStatus = MAKEINTRESOURCE(IDS_STATUS_UPDATEBEGIN); break;
			default:				pszStatus = NULL; break;
		}
		
		if (NULL != pszStatus)
		{
			BSTR bstrText;
			if (FALSE == IS_INTRESOURCE(pszStatus))
				bstrText = SysAllocString(pszStatus);
			else
			{
				WCHAR szBuffer[256] = {0};
				WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszStatus, szBuffer, ARRAYSIZE(szBuffer));
				bstrText = SysAllocString(szBuffer);
			}

			statusCookie = status->Add(bstrText);
			if (((UINT)-1) == statusCookie)
				SysFreeString(bstrText);
		}
	}
	LeaveCriticalSection(&lock);
}


void LoginDownloadResult::RemoveStatus()
{
	EnterCriticalSection(&lock);
	if (NULL != status)
	{
		if (((UINT)-1) != statusCookie)
		{
			status->Remove(statusCookie);
			statusCookie = (UINT)-1;
		}
	}
	LeaveCriticalSection(&lock);
}

HRESULT LoginDownloadResult::CreateDownloadFileName(LPSTR pszBuffer, UINT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	HRESULT hr;
	EnterCriticalSection(&lock);

	if(NULL == manager || 0 == cookie)
		hr = E_UNEXPECTED;
	else
	{
		api_httpreceiver *receiver = manager->GetReceiver(cookie);
		if (NULL == receiver) hr = E_FAIL;
		else
		{
			LPCSTR url = receiver->get_url();
			LPCSTR contentDisposition = receiver->getheader("content-disposition");
			LPCSTR contentType = receiver->getheader("content-type");
			hr = S_OK;
		}
	}
	
	LeaveCriticalSection(&lock);
	return hr;
}

void LoginDownloadResult::DownloadCompleted(int errorCode)
{
	EnterCriticalSection(&lock);
	
	result = errorCode;
	SetState(stateCompleted);
	HANDLE hEvent = completed;
	Callback cb = callback;
	
	LeaveCriticalSection(&lock);

	if (NULL != hEvent)
		SetEvent(hEvent);
	
	if (NULL != cb)
		cb(this, data);

	RemoveStatus();
	Release();
}

void LoginDownloadResult::Event_DownloadInit(DownloadToken token)
{
	EnterCriticalSection(&lock);

	AddRef();

	cookie = token;
	SetState(stateConnecting);

	if (NULL != manager)
	{
		manager->RetainDownload(cookie);
		
		if (typeProviderList == (typeMask & flags))
		{
			api_httpreceiver *receiver = manager->GetReceiver(token);
			if (NULL != receiver)
				receiver->AllowCompression();
		}
	}
	
	LeaveCriticalSection(&lock);

	SetStatus();
	
}

void LoginDownloadResult::Event_DownloadConnect(DownloadToken token)
{
	EnterCriticalSection(&lock);
	SetState(stateReceiving);
	LeaveCriticalSection(&lock);
}

void LoginDownloadResult::Event_DownloadTick(DownloadToken token)
{
	EnterCriticalSection(&lock);
	if (stateConnecting == (stateMask & flags))
		SetState(stateReceiving);
	LeaveCriticalSection(&lock);
}

void LoginDownloadResult::Event_DownloadFinish(DownloadToken token)
{
	DownloadCompleted(api_downloadManager::TICK_SUCCESS);
}

void LoginDownloadResult::Event_DownloadError(DownloadToken token, int errorCode)
{
	DownloadCompleted(errorCode);
}

void LoginDownloadResult::Event_DownloadCancel(DownloadToken token)
{
	DownloadCompleted(api_downloadManager::TICK_NODATA);
}



#define CBCLASS LoginDownloadResult
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONINIT, Event_DownloadInit)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCONNECT, Event_DownloadConnect)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONFINISH, Event_DownloadFinish)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONERROR, Event_DownloadError)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONCANCEL, Event_DownloadCancel)
VCB(IFC_DOWNLOADMANAGERCALLBACK_ONTICK, Event_DownloadTick)
END_DISPATCH;
#undef CBCLASS