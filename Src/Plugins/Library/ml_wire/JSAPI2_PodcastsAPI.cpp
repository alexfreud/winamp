#include "JSAPI2_PodcastsAPI.h"
#include "../Winamp/JSAPI.h"
#include "api__ml_wire.h"
#include "./rssCOM.h"

JSAPI2::PodcastsAPI::PodcastsAPI( const wchar_t *_key, JSAPI::ifc_info *_info )
{
	info = _info;
	key = _key;
}

enum
{
	DISP_PODCASTS_SUBSCRIBE,
};

#define DISP_TABLE \
	CHECK_ID(Subscribe, DISP_PODCASTS_SUBSCRIBE)\

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L## #str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT JSAPI2::PodcastsAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::PodcastsAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PodcastsAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PodcastsAPI::Subscribe(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);

	if (AGAVE_API_JSAPI2_SECURITY->GetActionAuthorization(L"podcasts", L"subscribe", key, info, JSAPI2::api_security::ACTION_PROMPT) == JSAPI2::api_security::ACTION_ALLOWED)
	{
		RssCOM::SubscribeUrl(JSAPI_PARAM(pdispparams, 1).bstrVal, pvarResult);
	}
	else
	{
		JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
		JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_FALSE);
	}

	return S_OK;
}

#undef CHECK_ID
#define CHECK_ID(str, id) 		case id: return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::PodcastsAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::PodcastsAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::PodcastsAPI::AddRef(void)
{
	return _refCount.fetch_add( 1 );
}


ULONG JSAPI2::PodcastsAPI::Release( void )
{
	LONG lRef = _refCount.fetch_sub( 1 );
	if ( lRef == 0 )
		delete this;

	return lRef;
}
