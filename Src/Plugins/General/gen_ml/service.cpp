#include "main.h"
#include "service.h"
#include "api__gen_ml.h"
#include "../winamp/wa_ipc.h"
#include <strsafe.h>

#define IS_INVALIDISPATCH(__disp) (((IDispatch *)1) == (__disp) || NULL == (__disp))

OmService::OmService(UINT nId)
	: ref(1), id(nId), name(NULL), url(NULL)
{
}

OmService::~OmService()
{
	free(name);
	free(url);
}

HRESULT OmService::CreateInstance(UINT nId, LPCWSTR pszName, OmService **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;

	OmService *service = new OmService(nId);
	if (NULL == service) return E_OUTOFMEMORY;

	service->SetName(pszName);
	wchar_t url[256] = {0};
	if (nId == SERVICE_LABS)
	{
		lstrcpynW(url, L"http://www.winamp.com/labs/pc", 256);
	}

	service->SetUrl(url);

	*instance = service;
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

unsigned int OmService::GetId()
{
	return id;
}

HRESULT OmService::GetName(wchar_t *pszBuffer, int cchBufferMax)
{
	return StringCchCopyW(pszBuffer, cchBufferMax, name);
}

HRESULT OmService::GetUrl(wchar_t *pszBuffer, int cchBufferMax)
{
	return StringCchCopyExW(pszBuffer, cchBufferMax, url, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT OmService::GetIcon(wchar_t *pszBuffer, int cchBufferMax)
{
	return E_FAIL;
}

HRESULT OmService::GetExternal(IDispatch **ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;

	*ppDispatch = NULL;

	// try JSAPI2 first
	WCHAR szBuffer[64] = {0};
	if (SUCCEEDED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%u", id)))
	{
		*ppDispatch = (IDispatch*)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)szBuffer, IPC_JSAPI2_GET_DISPATCH_OBJECT);
		AGAVE_API_JSAPI2_SECURITY->SetBypass(szBuffer, true);
	}
	else
		return E_FAIL;

	return S_OK;
}

HRESULT OmService::SetName(LPCWSTR pszName)
{
	free(name);
	name = wcsdup(pszName);
	return S_OK;
}

HRESULT OmService::SetUrl(LPCWSTR pszUrl)
{
	free(url);
	url = wcsdup(pszUrl);
	return S_OK;
}

HRESULT OmService::SetIcon(LPCWSTR pszIcon)
{
	return S_OK;
}

#define CBCLASS OmService
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_GETID, GetId)
CB(API_GETNAME, GetName)
CB(API_GETURL, GetUrl)
CB(API_GETICON, GetIcon)
CB(API_GETEXTERNAL, GetExternal)
END_DISPATCH;
#undef CBCLASS