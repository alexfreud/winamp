#include "./main.h"
#include "./api__ml_online.h"
#include "./jsapi2_omcom.h"
#include "./navigation.h"

#include <ifc_omservice.h>
#include <browserView.h>

#include "../Winamp/JSAPI.h"


JSAPI2::OnlineServicesAPI::OnlineServicesAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

#define DISP_TABLE \
	CHECK_ID(Login)\


#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE
};

#undef CHECK_ID
#define CHECK_ID(str) 		if (wcscmp(rgszNames[i], L## #str) == 0)	{		rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }
HRESULT JSAPI2::OnlineServicesAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		DISP_TABLE

			rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::OnlineServicesAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::OnlineServicesAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::OnlineServicesAPI::Login(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	const wchar_t *url = JSAPI_PARAM(pdispparams, 1).bstrVal;

//	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"onlineservices", L"login", key, info, JSAPI2::api_security::ACTION_DISALLOWED) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		HWND hBrowser = info->GetHWND();
		if (NULL != hBrowser && IsWindow(hBrowser))
		{
			ifc_omservice *service;
			if (FALSE != BrowserControl_GetService(hBrowser, &service))
			{
				wchar_t szBuffer[4096] = {0};
				LPCWSTR navigateUrl;

				if (NULL != AGAVE_API_AUTH && 
					0 == AGAVE_API_AUTH->ClientToWeb(GUID_NULL, url, szBuffer, ARRAYSIZE(szBuffer)))
				{
					navigateUrl = szBuffer;
				}
				else
					navigateUrl = url;

				BrowserControl_Navigate(hBrowser, navigateUrl, TRUE);

				JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
				service->Release();
			}
		}
	}
	return S_OK;		
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::OnlineServicesAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::OnlineServicesAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG JSAPI2::OnlineServicesAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::OnlineServicesAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
