#include "api__ml_history.h"
#include "JSAPI2_HistoryAPI.h"
#include "../Winamp/JSAPI.h"
#include "JSAPI2_HistoryRecordList.h"
#include "HistoryAPI.h"

extern HistoryAPI historyAPI;

JSAPI2::HistoryAPI::HistoryAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	info = _info;
	key = _key;
	refCount = 1;
}

#define DISP_TABLE \
	CHECK_ID(Query)\


#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE
};

#undef CHECK_ID
#define CHECK_ID(str) 		if (wcscmp(rgszNames[i], L## #str) == 0)	{		rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }
HRESULT JSAPI2::HistoryAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::HistoryAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryAPI::Query(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_INIT_RESULT(pvarResult, VT_DISPATCH);
	
	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"history", L"query", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
			if (pvarResult) // no sense in running the query if they don't care about the return value!
			{
				historyRecordList *query_results = historyAPI.Query(JSAPI_PARAM(pdispparams, 1).bstrVal);
				if (query_results)
					V_DISPATCH(pvarResult) = new JSAPI2::HistoryRecordList(query_results);
				else
					V_DISPATCH(pvarResult) = 0;
			}
			return S_OK;
	}
	else
	{
				V_DISPATCH(pvarResult) = 0;
		return S_OK;
	}

	return E_FAIL;		
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::HistoryAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::HistoryAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::HistoryAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::HistoryAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
