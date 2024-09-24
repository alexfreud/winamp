#include "SecurityCOM.h"
#include "JSAPI2_Security.h"
#include "JSAPI.h"
enum
{
	DISP_SECURITY_SETACTIONAUTHORIZATION = 777,
};

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT SecurityCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("SetActionAuthorization", DISP_SECURITY_SETACTIONAUTHORIZATION )
		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT SecurityCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT SecurityCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT SecurityCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISP_SECURITY_SETACTIONAUTHORIZATION:
		{
			JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 2, 4);
			JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
			const wchar_t *key = JSAPI_PARAM(pdispparams, 1).bstrVal;
			switch(JSAPI_NUM_PARAMS(pdispparams))
			{
			case 2: // key and authorization
				JSAPI2::security.SetActionAuthorization(key, 0, 0, JSAPI_PARAM(pdispparams, 2).lVal);
				break;
			case 3: // key, group and authorization
				JSAPI2::security.SetActionAuthorization(key, JSAPI_PARAM(pdispparams, 2).bstrVal, 0, JSAPI_PARAM(pdispparams, 3).lVal);
				break;
			case 4: // key, group, action and authorization
				JSAPI2::security.SetActionAuthorization(key, JSAPI_PARAM(pdispparams, 2).bstrVal, JSAPI_PARAM(pdispparams, 3).bstrVal, JSAPI_PARAM(pdispparams, 4).lVal);
				break;
			}
			
			return S_OK;
		}
		break;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP SecurityCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG SecurityCOM::AddRef(void)
{
	return 0;
}

ULONG SecurityCOM::Release(void)
{
	return 0;
}


void SecurityCOM::SetActionAuthorization(const wchar_t *key, const wchar_t *group, const wchar_t *action, int authorization)
{

}