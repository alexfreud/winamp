#include "JSAPI2_Application.h"
#include "main.h"
#include "api.h"
#include "language.h"
#include "JSAPI.h"
#include "JSAPI2_Security.h"

JSAPI2::ApplicationAPI::ApplicationAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

#define DISP_TABLE \
	CHECK_ID(LaunchURL)\
	CHECK_ID(version)\
	CHECK_ID(versionstring)\
	CHECK_ID(language)\
	CHECK_ID(languagepack)\
	CHECK_ID(settingspath)\

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{ rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }

HRESULT JSAPI2::ApplicationAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::ApplicationAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::ApplicationAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

void CALLBACK OpenURLAPC(ULONG_PTR param);
HRESULT JSAPI2::ApplicationAPI::LaunchURL(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BOOL, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);

	if (security.GetActionAuthorization(L"application", L"launchurl", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		const wchar_t *url = JSAPI_PARAM(pdispparams, 1).bstrVal;
		wchar_t scheme[16]=L"";
		DWORD size = 16;
		// make sure it's an HTTP url
		if (PathIsURLW(url) && !UrlIsFileUrlW(url) && UrlGetPartW(url, scheme, &size, URL_PART_SCHEME, 0) == S_OK 
			&& (!_wcsicmp(scheme, L"http") || !_wcsicmp(scheme, L"https")))
		{
			JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
			if (JSAPI_PARAM_OPTIONAL(pdispparams, 2, boolVal, VARIANT_FALSE) == VARIANT_TRUE)
				ShellExecuteW(NULL, L"open", url, NULL, L".", 0);
			else
				QueueUserAPC(OpenURLAPC, hMainThread, (ULONG_PTR)_wcsdup(url));
		}
	}
	return S_OK;
}

HRESULT JSAPI2::ApplicationAPI::version(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_I4);
		JSAPI_SET_RESULT(pvarResult, lVal, ((APP_VERSION_NUM&0xf000)>>12) * 100 + ((APP_VERSION_NUM&0xf0)>>4) * 10 + (APP_VERSION_NUM&0xf));
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
HRESULT JSAPI2::ApplicationAPI::versionstring(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(WIDEN(APP_VERSION));
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::ApplicationAPI::language(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(WASABI_API_LNG->GetLanguageIdentifier(LANG_LANG_CODE));
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::ApplicationAPI::languagepack(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(WASABI_API_LNG->GetLanguageIdentifier(LANG_IDENT_STR));
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::ApplicationAPI::settingspath(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(WASABI_API_APP->path_getUserSettingsPath());
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::ApplicationAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::ApplicationAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::ApplicationAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::ApplicationAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
