#include "./main.h"
#include "./service.h"
#include "./ifc_omservicehost.h"
#include "./ifc_omserviceeventmngr.h"

#include "../winamp/wa_ipc.h"

#include <strsafe.h>

#define  DEFAULT_GENERATION		2

#define IS_INVALIDISPATCH(__disp) (((IDispatch *)1) == (__disp) || NULL == (__disp))

OmService::OmService(UINT serviceId, ifc_omservicehost *serviceHost) 
	: ref(1), id(serviceId), name(NULL), url(NULL), icon(NULL), rating(0), version(0),
	  flags(0), generation(2), description(NULL), authorFirst(NULL), authorLast(NULL),
	  published(NULL), updated(NULL), thumbnail(NULL), screenshot(NULL), address(NULL),
	  host(serviceHost)
{
	if (NULL != host)
		host->AddRef();

	modified.SetEventHandler(ModifiedEvent, (ULONG_PTR)this);

	InitializeCriticalSection(&lock);
	InitializeCriticalSection(&eventLock);
}

OmService::~OmService()
{	
	Plugin_FreeString(name);
	Plugin_FreeString(url);
	Plugin_FreeString(icon);
	Plugin_FreeString(address);

	Plugin_FreeString(description);
	Plugin_FreeString(authorFirst);
	Plugin_FreeString(authorLast);
	Plugin_FreeString(published);
	Plugin_FreeString(updated);
	Plugin_FreeString(thumbnail);
	Plugin_FreeString(screenshot);

	UnregisterAllEventHandlers();

	if (NULL != host)
		host->Release();

	DeleteCriticalSection(&lock);
	DeleteCriticalSection(&eventLock);
}

HRESULT OmService::CreateInstance(UINT serviceId, ifc_omservicehost *host, OmService **serviceOut)
{
	if (NULL == serviceOut) return E_POINTER;
	*serviceOut = new OmService(serviceId, host);
	if (NULL == *serviceOut) return E_OUTOFMEMORY;

	return S_OK;
}

size_t OmService::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmService::Release()
{
	if (0 == ref)
		return ref;

	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

int OmService::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmService))
		*object = static_cast<ifc_omservice*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceDetails))
		*object = static_cast<ifc_omservicedetails*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceEditor))
		*object = static_cast<ifc_omserviceeditor*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceCopier))
		*object = static_cast<ifc_omservicecopier*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceCommand))
		*object = static_cast<ifc_omservicecommand*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceEventMngr))
		*object = static_cast<ifc_omserviceeventmngr*>(this);
	else if (IsEqualIID(interface_guid, IFC_OmServiceHostExt))
		*object = static_cast<ifc_omservicehostext*>(this);
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

HRESULT OmService::StringCchCopyExMT(LPTSTR pszDest, size_t cchDest, LPCTSTR pszSrc, LPTSTR *ppszDestEnd, size_t *pcchRemaining, DWORD dwFlags)
{
	if (NULL == pszDest)
		return E_POINTER;

	EnterCriticalSection(&lock);
	HRESULT hr = StringCchCopyEx(pszDest, cchDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
	LeaveCriticalSection(&lock);
	return hr;
}

UINT OmService::GetId()
{
	return id;
}

HRESULT OmService::SetId(UINT serviceId)
{
	id = serviceId;
	return S_OK;
}

HRESULT OmService::GetName(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, name, NULL, NULL,STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
}

HRESULT OmService::GetUrl(LPWSTR pszBuffer, UINT cchBufferMax)
{	
	EnterCriticalSection(&lock);

	HRESULT hr = GetUrlDirect(pszBuffer, cchBufferMax);
	if(SUCCEEDED(hr))
	{
		if (NULL != host)
		{
			HRESULT hostResult = host->GetUrl(this, pszBuffer, cchBufferMax);
			if (FAILED(hostResult) && E_NOTIMPL != hostResult)
				hr = hostResult;
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmService::GetUrlDirect(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, url, NULL, NULL, STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
}

HRESULT OmService::GetIcon(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, icon, NULL, NULL, STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
}

HRESULT OmService::GetExternal(IDispatch **ppDispatch)
{
	if (NULL == ppDispatch)
		return E_POINTER;

	*ppDispatch = NULL;

	HWND hWinamp = NULL;
	IDispatch *winampDisp = NULL;

	if (SUCCEEDED(Plugin_GetWinampWnd(&hWinamp)) && NULL != hWinamp)
	{
		EnterCriticalSection(&lock);
		UINT generation_cached = generation;
		LeaveCriticalSection(&lock);

		if (2 == generation_cached)
		{
			WCHAR szBuffer[64] = {0};
			if (SUCCEEDED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%u", id)))
				winampDisp = (IDispatch*)SENDWAIPC(hWinamp, IPC_JSAPI2_GET_DISPATCH_OBJECT, (WPARAM)szBuffer);
		}
		else if (1 == generation_cached)
		{
			winampDisp = (IDispatch*)SENDWAIPC(hWinamp, IPC_GET_DISPATCH_OBJECT, 0);
		}

		if (IS_INVALIDISPATCH(winampDisp))
			winampDisp = NULL;
	}

	*ppDispatch = winampDisp;

	EnterCriticalSection(&lock);
	ifc_omservicehost *host_cached = host;
	if (NULL != host_cached)
		host_cached->AddRef();

	LeaveCriticalSection(&lock);

	if (NULL != host_cached)
	{
		host_cached->GetExternal(this, ppDispatch);
		host_cached->Release();
	}

	return S_OK;
}

HRESULT OmService::GetRating(UINT *ratingOut)
{
	if (NULL == ratingOut) 
		return E_POINTER;

	*ratingOut = rating;
	return S_OK;
}

HRESULT OmService::GetVersion(UINT *versionOut)
{
	if (NULL == versionOut) 
		return E_POINTER;

	*versionOut = version;
	return S_OK;
}

HRESULT OmService::GetGeneration(UINT *generationOut)
{
	if (NULL == generationOut) 
		return E_POINTER;

	EnterCriticalSection(&lock);
	*generationOut = (0 == generation) ? DEFAULT_GENERATION : generation;
	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmService::GetFlags(UINT *flagsOut)
{
	if (NULL == flagsOut) 
		return E_POINTER;

	*flagsOut = flags;
	return S_OK;
}

HRESULT OmService::UpdateFlags(UINT flagsIn)
{
	flags = flagsIn;
	return S_OK;
}

HRESULT OmService::SetAddress(LPCWSTR pszAddress)
{
	EnterCriticalSection(&lock);

	HRESULT hr;
	Plugin_FreeString(address);
	if (NULL == pszAddress)
	{
		address = NULL;
		hr = S_OK;
	}
	else
	{
		address = Plugin_CopyString(pszAddress);
		hr = (NULL == address) ? E_OUTOFMEMORY : S_OK;
	}	

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmService::GetAddress(LPWSTR pszBuffer, UINT cchBufferMax)
{
	return StringCchCopyExMT(pszBuffer, cchBufferMax, address, NULL, NULL,STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE);
}

HRESULT OmService::GetHost(ifc_omservicehost **ppHost)
{
	if (NULL == ppHost) return E_POINTER;

	EnterCriticalSection(&lock);

	*ppHost = host;
	if (NULL != host) 
		host->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

HRESULT OmService::SetHost(ifc_omservicehost *serviceHost)
{
	EnterCriticalSection(&lock);

	if (NULL != host)
		host->Release();

	host =  serviceHost;

	if (NULL != host) 
		host->AddRef();

	LeaveCriticalSection(&lock);

	return S_OK;
}

void CALLBACK OmService::ModifiedEvent(UINT nMark, FlagTracker *instance, ULONG_PTR user)
{
	OmService *service = (OmService*)user;
	if (NULL == service) return;

	service->Signal_ServiceChange(nMark);
}

HRESULT OmService::CopyTo(ifc_omservice *service, UINT *modifiedFlags)
{
	if (NULL == service || id != service->GetId())
		return E_INVALIDARG;

	HRESULT hr;
	ifc_omserviceeditor *editor = NULL;
	hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
	if (SUCCEEDED(hr) && editor != NULL)
	{
		editor->BeginUpdate();

		if (FAILED(editor->SetName(name, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetUrl(url, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetIcon(icon, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetRating(rating))) hr = E_FAIL;
		if (FAILED(editor->SetVersion(version))) hr = E_FAIL;
		if (FAILED(editor->SetFlags(flags, 0xFFFFFFFF))) hr = E_FAIL;
		if (FAILED(editor->SetDescription(description, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetAuthorFirst(authorFirst, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetAuthorLast(authorLast, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetUpdated(updated, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetPublished(published, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetThumbnail(thumbnail, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetScreenshot(screenshot, FALSE))) hr = E_FAIL;
		if (FAILED(editor->SetGeneration(generation))) hr = E_FAIL;

		editor->EndUpdate();

		if (NULL != modifiedFlags)
			editor->GetModified(modifiedFlags);

		editor->Release();
	}

	return hr;
}

HRESULT OmService::QueryState(HWND hBrowser, const GUID *commandGroup, UINT commandId)
{
	HRESULT hr;

	EnterCriticalSection(&lock);

	if (NULL != host)
		hr = host->QueryCommandState(this, hBrowser, commandGroup, commandId);
	else
		hr = E_NOTIMPL;

	LeaveCriticalSection(&lock);
	return hr;
}

HRESULT OmService::Exec(HWND hBrowser, const GUID *commandGroup, UINT commandId, ULONG_PTR commandArg)
{
	HRESULT hr;

	EnterCriticalSection(&lock);

	if (NULL != host)
		hr = host->ExecuteCommand(this, hBrowser, commandGroup, commandId, commandArg);
	else
		hr = E_NOTIMPL;

	LeaveCriticalSection(&lock);
	return hr;
}