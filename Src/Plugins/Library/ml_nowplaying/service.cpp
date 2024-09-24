#include "main.h"
#include "./service.h"
#include "./wasabi.h"
#include "./resource.h"
#include "../replicant/nu/Autowide.h"
#include "../winamp/wa_ipc.h"
#include <strsafe.h>

#define IS_INVALIDISPATCH(__disp) (((IDispatch *)1) == (__disp) || NULL == (__disp))

OmService::OmService(UINT nId)
	: ref(1), id(nId), name(NULL), url(NULL), icon(NULL)
{
}

OmService::~OmService()
{
	Plugin_FreeResString(name);
	Plugin_FreeString(url);
	Plugin_FreeResString(icon);
}

HRESULT OmService::CreateInstance(OmService **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = NULL;
	
	OmService *service = new OmService(SERVICE_ID);
	if (NULL == service) return E_OUTOFMEMORY;

	wchar_t nowplayingurl[1024] = {0};
	lstrcpynW(nowplayingurl, AutoWide(g_config->ReadString("nowplayingurl","")), ARRAYSIZE(nowplayingurl));

	service->SetName(MAKEINTRESOURCE(IDS_SERVICE_NAME));
	service->SetUrl((nowplayingurl[0] ? nowplayingurl : SERVICE_HOMEURL));
	service->SetIcon(MAKEINTRESOURCE(IDR_SERVICE_ICON));

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
	return Plugin_CopyResString(pszBuffer, cchBufferMax, name);
}

HRESULT OmService::GetUrl(wchar_t *pszBuffer, int cchBufferMax)
{
	return StringCchCopyEx(pszBuffer, cchBufferMax, url, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT OmService::GetIcon(wchar_t *pszBuffer, int cchBufferMax)
{
	if (NULL != icon && IS_INTRESOURCE(icon))
	{
		WCHAR szPath[2*MAX_PATH] = {0};
		if (0 == GetModuleFileName(Plugin_GetInstance(), szPath, ARRAYSIZE(szPath)))
			return E_FAIL;

		return StringCchPrintf(pszBuffer, cchBufferMax, L"res://%s/#%d/#%d", szPath, RT_RCDATA, icon);
	}
	
	return StringCchCopyEx(pszBuffer, cchBufferMax, icon, NULL, NULL, STRSAFE_IGNORE_NULLS);
}

HRESULT OmService::GetExternal(IDispatch **ppDispatch)
{
	if (NULL == ppDispatch) 
		return E_POINTER;
	
	*ppDispatch = NULL;

	HWND hWinamp = Plugin_GetWinamp();
	if (NULL == hWinamp)
		return E_UNEXPECTED;
	
	// So far we do not use JSAPI2 in nowplaying
	// // try JSAPI2 first
	// WCHAR szBuffer[64] = {0};
	// if (SUCCEEDED(StringCchPrintfW(szBuffer, ARRAYSIZE(szBuffer), L"%u", id)))
	// *ppDispatch = (IDispatch*)SENDWAIPC(hWinamp, IPC_JSAPI2_GET_DISPATCH_OBJECT, (WPARAM)szBuffer);
		
	if (IS_INVALIDISPATCH(*ppDispatch))
	{ 	// try JSAPI1
		*ppDispatch = (IDispatch*)SENDWAIPC(hWinamp, IPC_GET_DISPATCH_OBJECT, 0);
		if (IS_INVALIDISPATCH(*ppDispatch))
		{ // Fail
			*ppDispatch = NULL;
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT OmService::SetName(LPCWSTR pszName)
{
	Plugin_FreeResString(name);
	name = Plugin_DuplicateResString(pszName);
	return S_OK;
}

HRESULT OmService::SetUrl(LPCWSTR pszUrl)
{
	Plugin_FreeString(url);
	url = Plugin_CopyString(pszUrl);
	return S_OK;
}

HRESULT OmService::SetIcon(LPCWSTR pszIcon)
{
	Plugin_FreeResString(icon);
	icon = Plugin_DuplicateResString(pszIcon);
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

	
