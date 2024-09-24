#include "JSAPI2_Playlist.h"
#include "../Winamp/JSAPI.h"
#include "api__playlist.h"
#include "PlaylistManager.h"

JSAPI2::PlaylistObject::PlaylistObject(const wchar_t *_key)
{
	key = _key;
}

#define DISP_TABLE \
	CHECK_ID(Clear)\
	CHECK_ID(AppendURL)\
	CHECK_ID(GetItemFilename)\
	CHECK_ID(GetItemTitle)\
	CHECK_ID(GetItemLength)\
	CHECK_ID(GetItemExtendedInfo)\
	CHECK_ID(Reverse)\
	CHECK_ID(SwapItems)\
	CHECK_ID(Randomize)\
	CHECK_ID(RemoveItem)\
	CHECK_ID(SortByTitle)\
	CHECK_ID(SortByFilename)\
	CHECK_ID(SetItemFilename)\
	CHECK_ID(SetItemTitle)\
	CHECK_ID(SetItemLength)\
	CHECK_ID(InsertURL)\
	CHECK_ID(numitems)\
	

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
};

#undef CHECK_ID
#define CHECK_ID(str) 		if (wcscmp(rgszNames[i], L## #str) == 0)	{		rgdispid[i] = JSAPI_DISP_ENUMIFY(str); continue; }
HRESULT JSAPI2::PlaylistObject::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::PlaylistObject::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PlaylistObject::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::PlaylistObject::Clear(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.Clear();
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::AppendURL(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 1, 3);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 3, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	const wchar_t *filename = JSAPI_PARAM(pdispparams, 1).bstrVal;
	const wchar_t *title = JSAPI_PARAM_OPTIONAL(pdispparams, 2, bstrVal, 0);
	int length = JSAPI_PARAM_OPTIONAL(pdispparams, 3, lVal, -1);
	playlist.AppendWithInfo(filename, title, length);
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::InsertURL(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT_OPTIONAL(pdispparams, 2, 4);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 3, VT_BSTR, puArgErr);
	JSAPI_VERIFY_PARAMTYPE_OPTIONAL(pdispparams, 4, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	const wchar_t *filename = JSAPI_PARAM(pdispparams, 2).bstrVal;
	const wchar_t *title = JSAPI_PARAM_OPTIONAL(pdispparams, 3, bstrVal, 0);
	int length = JSAPI_PARAM_OPTIONAL(pdispparams, 4, lVal, -1);
	playlist.Insert(JSAPI_PARAM(pdispparams, 1).lVal, filename, title, length);
	return S_OK;
}


HRESULT JSAPI2::PlaylistObject::GetItemFilename(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
	JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(playlist.ItemName(JSAPI_PARAM(pdispparams, 1).lVal)));
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::GetItemTitle(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
	JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(playlist.ItemTitle(JSAPI_PARAM(pdispparams, 1).lVal)));
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::GetItemLength(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_I4);
	JSAPI_SET_RESULT(pvarResult, lVal, playlist.GetItemLengthMilliseconds(JSAPI_PARAM(pdispparams, 1).lVal));
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::GetItemExtendedInfo(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
	wchar_t metadata[1024]=L"";
	playlist.GetItemExtendedInfo(JSAPI_PARAM(pdispparams, 1).lVal, JSAPI_PARAM(pdispparams, 2).bstrVal, metadata, sizeof(metadata)/sizeof(*metadata));
	JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(metadata));
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::Reverse(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.Reverse();
	return S_OK;
}


HRESULT JSAPI2::PlaylistObject::SwapItems(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.Swap(JSAPI_PARAM(pdispparams, 1).lVal, JSAPI_PARAM(pdispparams, 2).lVal);
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::Randomize(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlistManager.Randomize(&playlist);
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::RemoveItem(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	
	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.Remove(JSAPI_PARAM(pdispparams, 1).lVal);
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::SortByTitle(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.SortByTitle();
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::SortByFilename(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.SortByFilename();
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::SetItemFilename(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.SetItemFilename(JSAPI_PARAM(pdispparams, 1).lVal, JSAPI_PARAM(pdispparams, 2).bstrVal);
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::SetItemTitle(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.SetItemTitle(JSAPI_PARAM(pdispparams, 1).lVal, JSAPI_PARAM(pdispparams, 2).bstrVal);
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::SetItemLength(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 2);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 2, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BOOL);
	JSAPI_SET_RESULT(pvarResult, boolVal, VARIANT_TRUE);
	playlist.SetItemLengthMilliseconds(JSAPI_PARAM(pdispparams, 1).lVal, JSAPI_PARAM(pdispparams, 2).lVal);
	return S_OK;
}

HRESULT JSAPI2::PlaylistObject::numitems(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
//	JSAPI_VERIFY_METHOD(wFlags);
//	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_I4);
		JSAPI_SET_RESULT(pvarResult, lVal, (LONG)playlist.GetNumItems());
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}


#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::PlaylistObject::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::PlaylistObject::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_PlaylistObject))
		*ppvObject = (PlaylistObject *)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG JSAPI2::PlaylistObject::AddRef(void)
{
	return this->_refCount.fetch_add( 1 );
}


ULONG JSAPI2::PlaylistObject::Release( void )
{
	std::size_t l_Ref = this->_refCount.fetch_sub( 1 );
	if ( l_Ref == 0 )
		delete this;

	return l_Ref;
}
