#include "main.h"
#include "./serviceHelper.h"
#include "./wasabi.h"
#include "./serviceHost.h"

#include <ifc_omservice.h>
#include <storageIni.h>
#include <ifc_omserviceeditor.h>
#include <ifc_omserviceeditor.h>
#include <ifc_mlnavigationhelper.h>

#include <strsafe.h>

HRESULT ServiceHelper_QueryStorage(ifc_omstorage **storage)
{
	if (NULL == storage) return E_POINTER;
	
	if (NULL == OMSERVICEMNGR)
	{
		*storage = NULL;
		return E_UNEXPECTED;
	}

	return OMSERVICEMNGR->QueryStorage(&SUID_OmStorageIni, storage);
}

HRESULT ServiceHelper_Create(UINT serviceId, LPCWSTR pszName, LPCWSTR pszIcon, LPCWSTR pszUrl, UINT flags, BOOL fSave, ifc_omservice **serviceOut)
{
	if (NULL == serviceOut) 
		return E_POINTER;

	*serviceOut = NULL;

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;
	
	WebDevServiceHost *serviceHost;
	if (FAILED(WebDevServiceHost::GetCachedInstance(&serviceHost)))
		serviceHost = NULL;

	HRESULT hr = OMSERVICEMNGR->CreateService(serviceId, serviceHost, serviceOut);
	if (SUCCEEDED(hr))
	{
		ifc_omserviceeditor *editor;
		if (SUCCEEDED((*serviceOut)->QueryInterface(IFC_OmServiceEditor, (void**)&editor)))
		{
			WCHAR szBuffer[4096] = {0};
			editor->BeginUpdate();
			if (NULL != pszName && IS_INTRESOURCE(pszName))
			{
				if (NULL != WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszName, szBuffer, ARRAYSIZE(szBuffer)))
					editor->SetName(szBuffer, FALSE);
			}
			else
				editor->SetName(pszName, FALSE);
			
			if (NULL != pszIcon && IS_INTRESOURCE(pszIcon))
			{
				if (SUCCEEDED(Plugin_MakeResourcePath(szBuffer, ARRAYSIZE(szBuffer), RT_RCDATA, pszIcon, RESPATH_COMPACT)))
					editor->SetIcon(szBuffer, FALSE);
			}
			else
				editor->SetIcon(pszIcon, FALSE);
			
			if (NULL != pszUrl && IS_INTRESOURCE(pszUrl))
			{
				if (SUCCEEDED(Plugin_MakeResourcePath(szBuffer, ARRAYSIZE(szBuffer), RT_HTML, pszUrl, RESPATH_TARGETIE | RESPATH_COMPACT)))
					editor->SetUrl(szBuffer, FALSE);
			}
			else
				editor->SetUrl(pszUrl, FALSE);
			
			editor->SetFlags(flags, 0xFFFFFFFF);

			if (FALSE != fSave)
			{
				hr = ServiceHelper_Save(*serviceOut);
				if (FAILED(hr))
				{
					(*serviceOut)->Release();
					*serviceOut = NULL;
				}
			}
			else
			{
				editor->SetModified(0, (UINT)-1);
			}
			editor->EndUpdate();
			editor->Release();
		}
	}

	if (NULL != serviceHost)
		serviceHost->Release();

	return hr;
}

HRESULT ServiceHelper_Save(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;
	
	HRESULT hr;
	ifc_omstorage *storage;
	hr = ServiceHelper_QueryStorage(&storage);
	if (SUCCEEDED(hr))
	{		
		hr = storage->Save(&service, 1, ifc_omstorage::saveModifiedOnly | ifc_omstorage::saveClearModified, NULL);
		storage->Release();
	}
	return hr;
}

HRESULT ServiceHelper_Delete(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;

	ifc_omstorage *storage;
	HRESULT hr = ServiceHelper_QueryStorage(&storage);
	if (SUCCEEDED(hr))
	{
		hr = storage->Delete(&service, 1, NULL);
		storage->Release();
	}
	return hr;
}

HRESULT ServiceHelper_Reload(ifc_omservice *service)
{
	if (NULL == service) 
		return E_INVALIDARG;

	if (NULL == OMSERVICEMNGR)
		return E_UNEXPECTED;

	ifc_omstorage *storage;
	HRESULT hr = ServiceHelper_QueryStorage(&storage);
	if (SUCCEEDED(hr))
	{
		hr = storage->Reload(&service, 1, NULL);
		storage->Release();
	}

	return hr;
}

HRESULT ServiceHelper_UpdateIcon(ifc_omserviceeditor *editor, LPCWSTR pszImage)
{
	WCHAR szBuffer[8192];
	szBuffer[0] = L'\0';

	if (NULL == editor) 
		return E_INVALIDARG;

	ifc_omservice *service;
	if (SUCCEEDED(editor->QueryInterface(IFC_OmService, (void**)&service)))
	{
		if (FAILED(service->GetIcon(szBuffer, ARRAYSIZE(szBuffer))))
			szBuffer[0] = L'\0';
		service->Release();
	}

	HRESULT hr = editor->SetIcon(pszImage, FALSE);
	if (FAILED(hr) || S_FALSE == hr) return hr;

	if (L'\0' != szBuffer)
	{
		ifc_mlnavigationhelper *navHelper;
		if (SUCCEEDED(OMUTILITY->GetMlNavigationHelper(Plugin_GetLibrary(), &navHelper)))
		{		
			navHelper->ReleaseIndex(szBuffer);
			navHelper->Release();
		}
	}

	return S_OK;
}

HRESULT ServiceHelper_IsSpecial(ifc_omservice *service)
{
	if (NULL == service)
		return E_INVALIDARG;

	UINT serviceFlags;
	HRESULT hr = service->GetFlags(&serviceFlags);
	if (SUCCEEDED(hr))
	{
		hr = (0 != ((WDSVCF_ROOT | WDSVCF_SPECIAL) & serviceFlags)) ? S_OK : S_FALSE;
	}
	
	return hr;
}

HRESULT ServiceHelper_IsPreAuthorized(ifc_omservice *service)
{
	if (NULL == service)
		return E_INVALIDARG;

	UINT serviceFlags;
	HRESULT hr = service->GetFlags(&serviceFlags);
	if (SUCCEEDED(hr))
	{
		hr = (0 != (WDSVCF_PREAUTHORIZED & serviceFlags)) ? S_OK : S_FALSE;
	}

	return hr;
}

HRESULT ServiceHelper_RegisterPreAuthorized(ifc_omservice *service)
{
	if (NULL == AGAVE_API_JSAPI2_SECURITY)
		return E_UNEXPECTED;
					
	HRESULT hr;
	WCHAR szBuffer[64];
	hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), L"%u", service->GetId());
	if (SUCCEEDED(hr))
	{
		UINT flags;
		bool bypassEnabled = (SUCCEEDED(service->GetFlags(&flags)) && 0 != (WDSVCF_PREAUTHORIZED & flags));
		AGAVE_API_JSAPI2_SECURITY->SetBypass(szBuffer, bypassEnabled);
	}
	return hr;
}