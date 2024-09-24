#include "main.h"
#include "./resource.h"
#include "./external.h"
#include "./navigation.h"
#include "./commands.h"

#include "../winamp/jsapi.h"
#include "../winamp/jsapi_CallbackParameters.h"

#include "./serviceHelper.h"

#include <browserView.h>
#include <ifc_omservice.h>
#include <ifc_omserviceeditor.h>

#define DISPTABLE_CLASS	 ExternalDispatch

DISPTABLE_BEGIN()
	DISPENTRY_ADD(DISPATCH_SERVICE_OPEN, L"serviceOpen", OnServiceOpen)
	DISPENTRY_ADD(DISPATCH_SERVICE_CREATE, L"serviceCreate", OnServiceCreate)
	DISPENTRY_ADD(DISPATCH_SERVICE_GETINFO, L"serviceGetInfo", OnServiceGetInfo)
	DISPENTRY_ADD(DISPATCH_SERVICE_SETINFO, L"serviceSetInfo", OnServiceSetInfo)
DISPTABLE_END

#undef DISPTABLE_CLASS


static BOOL DispParam_GetStringOpt(LPCWSTR *str, DISPPARAMS *paramInfo, UINT paramNumber, UINT *argErr)
{
	if (paramInfo->cArgs < paramNumber || 
		VT_NULL == paramInfo->rgvarg[paramInfo->cArgs - paramNumber].vt)
	{
		*str = NULL;
		return FALSE;
	}
	
	JSAPI_GETSTRING((*str), paramInfo, paramNumber, argErr);
	return TRUE;
}

ExternalDispatch::ExternalDispatch() 
	: ref(1)
{
}

ExternalDispatch::~ExternalDispatch()
{
}

HRESULT ExternalDispatch::CreateInstance(ExternalDispatch **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new ExternalDispatch();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

LPCWSTR ExternalDispatch::GetName()
{
	return L"WebDev";
}

ULONG ExternalDispatch::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}


ULONG ExternalDispatch::Release(void)
{ 
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP ExternalDispatch::QueryInterface(REFIID riid, void **ppvObject)
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

HRESULT ExternalDispatch::OnServiceOpen(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 2);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
		
	UINT serviceId;
	JSAPI_GETUNSIGNED_AS_NUMBER(serviceId, pdispparams, 1, puArgErr);
	 
	LPCWSTR forceUrl = NULL;
	if (pdispparams->cArgs > 1)
	{
        switch(pdispparams->rgvarg[0].vt)
		{
			case VT_BSTR:	forceUrl = pdispparams->rgvarg[0].bstrVal; break;
			case VT_I4:		forceUrl = MAKEINTRESOURCE(pdispparams->rgvarg[0].lVal); break;
		}
	}

	if (FALSE == DispParam_GetStringOpt(&forceUrl, pdispparams,2, puArgErr))
		forceUrl = NULL;
	
	HRESULT hr;
	
	Navigation *navigation;
	hr = Plugin_GetNavigation(&navigation);
	if (SUCCEEDED(hr))
	{
		ifc_omservice *service;
		if (NULL != navigation->FindService(serviceId, &service))
		{
			hr = Command_NavigateService(service, forceUrl, FALSE);
			service->Release();
		}
		else
		{
			hr = E_FAIL;
		}
		navigation->Release();
	}
		
	JSAPI_SET_RESULT(pvarResult, boolVal, (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE));

	return S_OK;
}

HRESULT ExternalDispatch::OnServiceCreate(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	
	HRESULT hr = Command_CreateService();
	
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE));

	return S_OK;
}

HRESULT ExternalDispatch::OnServiceGetInfo(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	
	JSAPI_INIT_RESULT(pvarResult, VT_DISPATCH);
	
	UINT serviceId;
	JSAPI_GETUNSIGNED_AS_NUMBER(serviceId, pdispparams, 1, puArgErr);

	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
    if (SUCCEEDED(hr))
	{
		ifc_omservice *service;
		if (NULL != navigation->FindService(serviceId, &service))
		{
			WCHAR szBuffer[2048];
			JSAPI::CallbackParameters *params = new JSAPI::CallbackParameters;
			params->AddLong(L"id", service->GetId());
			
			if (FAILED(service->GetName(szBuffer, ARRAYSIZE(szBuffer)))) szBuffer[0] = L'\0';
			params->AddString(L"name", szBuffer);

			if (FAILED(service->GetUrl(szBuffer, ARRAYSIZE(szBuffer)))) szBuffer[0] = L'\0';
			params->AddString(L"url", szBuffer);

			if (FAILED(service->GetIcon(szBuffer, ARRAYSIZE(szBuffer)))) szBuffer[0] = L'\0';
			params->AddString(L"icon", szBuffer);

			params->AddBoolean(L"preauthorized", (S_OK == ServiceHelper_IsPreAuthorized(service)));
	        
			service->Release();
			V_DISPATCH(pvarResult) = params;
		}
		else
		{
			hr = E_FAIL;
		}
		navigation->Release();
	}

	if (FAILED(hr))
	{
		V_DISPATCH(pvarResult) = 0;
	}

	return S_OK;
}

HRESULT ExternalDispatch::OnServiceSetInfo(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 2, 5);
	
	UINT serviceId;
	JSAPI_GETUNSIGNED_AS_NUMBER(serviceId, pdispparams, 1, puArgErr);
	 
	Navigation *navigation;
	HRESULT hr = Plugin_GetNavigation(&navigation);
    if (SUCCEEDED(hr))
	{
		ifc_omservice *service;
		HNAVITEM hItem = navigation->FindService(serviceId, &service);
		if (NULL != hItem)
		{
			ifc_omserviceeditor *editor;
			hr = service->QueryInterface(IFC_OmServiceEditor, (void**)&editor);
			if (SUCCEEDED(hr))
			{
				LPCWSTR value = NULL;
			
				editor->BeginUpdate();

				if (FALSE != DispParam_GetStringOpt(&value, pdispparams, 2, puArgErr) && FAILED(editor->SetName(value, FALSE)))
					hr = E_FAIL;

				if (FALSE != DispParam_GetStringOpt(&value, pdispparams, 3, puArgErr) && FAILED(ServiceHelper_UpdateIcon(editor, value)))
					hr = E_FAIL;

				if (FALSE != DispParam_GetStringOpt(&value, pdispparams, 4, puArgErr) && FAILED(editor->SetUrl(value, FALSE)))
					hr = E_FAIL;

				VARIANT_BOOL  authorized = JSAPI_PARAM_OPTIONAL(pdispparams, 5, boolVal, VARIANT_FALSE);
				if (S_OK == editor->SetFlags((VARIANT_TRUE == authorized) ? WDSVCF_PREAUTHORIZED : 0, WDSVCF_PREAUTHORIZED))
				{
					if (VARIANT_TRUE == authorized)
						Command_ResetPermissions(service);
				}

				editor->EndUpdate();
				editor->Release();
			}

			if(SUCCEEDED(hr))
				hr = ServiceHelper_Save(service);
			
			service->Release();
		}
		else
		{
			hr = E_FAIL;
		}

		navigation->Release();
	}
	
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, (SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE));

	return S_OK;
}