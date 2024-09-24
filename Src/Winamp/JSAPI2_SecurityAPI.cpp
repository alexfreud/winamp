#include "JSAPI2_SecurityAPI.h"
#include "JSAPI2_Security.h"
#include "main.h"
#include "JSAPI.h"

JSAPI2::SecurityAPI::SecurityAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

enum
{
	DISPID_SECURITYAPI_SETACTIONAUTHORIZATION,
	DISPID_SECURITYAPI_GETACTIONAUTHORIZATION,
};

#define DISP_TABLE \
	CHECK_ID(SetActionAuthorization, DISPID_SECURITYAPI_SETACTIONAUTHORIZATION)\
	CHECK_ID(GetActionAuthorization, DISPID_SECURITYAPI_GETACTIONAUTHORIZATION)\

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT JSAPI2::SecurityAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::SecurityAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::SecurityAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::SecurityAPI::GetActionAuthorization(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_I4);
	JSAPI_SET_RESULT(pvarResult, lVal, security.GetActionAuthorization(JSAPI_PARAM(pdispparams, 1).bstrVal, JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, 0), key, info, JSAPI2::api_security::ACTION_UNDEFINED));
	return S_OK;
}

HRESULT JSAPI2::SecurityAPI::SetActionAuthorization(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 3);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 3, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);

	if (security.GetActionAuthorization(L"security", L"set", key, info, JSAPI2::api_security::ACTION_DISALLOWED) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
		const wchar_t *group = JSAPI_PARAM(pdispparams, 1).bstrVal;
		const wchar_t *action = JSAPI_PARAM(pdispparams, 2).bstrVal;
		int authorization = JSAPI_PARAM(pdispparams, 2).lVal;
		security.SetActionAuthorization(group, action, key, authorization);		
	}
	else
	{
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}
	return S_OK;
}

#undef CHECK_ID
#define CHECK_ID(str, id) 		case id: return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::SecurityAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::SecurityAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::SecurityAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::SecurityAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
