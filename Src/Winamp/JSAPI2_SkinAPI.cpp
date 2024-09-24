#include "JSAPI2_SkinAPI.h"
#include "JSAPI2_Security.h"
#include "main.h"
#include "JSAPI.h"
#include "wa_dlg.h"
#include "api.h"
#include "../nu/AutoWide.h"
#include <tataki/color/skinclr.h>
#include <api/service/waservicefactory.h>

JSAPI2::SkinAPI::SkinAPI(const wchar_t *_key, JSAPI::ifc_info *_info)
{
	Tataki::Init(serviceManager); // we might need it, go ahead and init here
	info = _info;
	key = _key;
	refCount = 1;
}

JSAPI2::SkinAPI::~SkinAPI()
{
	Tataki::Quit();
}

enum
{
	DISPID_SKINAPI_GETCLASSICCOLOR,
	DISPID_SKINAPI_GETPLAYLISTCOLOR,
	DISPID_SKINAPI_GETSKINCOLOR,
	DISPID_SKINAPI_NAME,
	DISPID_SKINAPI_FONT,
	DISPID_SKINAPI_FONTSIZE,

};

#define DISP_TABLE \
	CHECK_ID(GetClassicColor, DISPID_SKINAPI_GETCLASSICCOLOR)\
	CHECK_ID(GetPlaylistColor, DISPID_SKINAPI_GETPLAYLISTCOLOR)\
	CHECK_ID(GetSkinColor, DISPID_SKINAPI_GETSKINCOLOR)\
	CHECK_ID(name, DISPID_SKINAPI_NAME)\
	CHECK_ID(font, DISPID_SKINAPI_FONT)\
	CHECK_ID(fontsize, DISPID_SKINAPI_FONTSIZE)\

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L## #str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT JSAPI2::SkinAPI::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::SkinAPI::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::SkinAPI::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::SkinAPI::GetClassicColor(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	COLORREF color = WADlg_getColor(JSAPI_PARAM(pdispparams, 1).lVal);
	color = ((color >> 16) & 0xff | (color & 0xff00) | ((color << 16) & 0xff0000));
	wchar_t colorString[8] = {0};
	StringCchPrintfW(colorString, 8, L"#%06X", color);
	JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(colorString));

	return S_OK;
}

HRESULT JSAPI2::SkinAPI::GetPlaylistColor(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_I4, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	COLORREF color = Skin_PLColors[JSAPI_PARAM(pdispparams, 1).lVal];
	color = ((color >> 16) & 0xff | (color & 0xff00) | ((color << 16) & 0xff0000));
	wchar_t colorString[8] = {0};
	StringCchPrintfW(colorString, 8, L"#%06X", color);
	JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(colorString));

	return S_OK;
}

HRESULT JSAPI2::SkinAPI::GetSkinColor(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_METHOD(wFlags);
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 1);
	JSAPI_VERIFY_PARAMTYPE(pdispparams, 1, VT_BSTR, puArgErr);

	JSAPI_INIT_RESULT(pvarResult, VT_BSTR);

	ARGB32 color;
	if (SkinColor::TryGetColor(&color, JSAPI_PARAM(pdispparams, 1).bstrVal))
	{
		color = ((color >> 16) & 0xff | (color & 0xff00) | ((color << 16) & 0xff0000));
		wchar_t colorString[8] = {0};
		StringCchPrintfW(colorString, 8, L"#%06X", color);
		JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(colorString));
	}
	else
	{
		JSAPI_EMPTY_RESULT(pvarResult);
	}

	return S_OK;
}

HRESULT JSAPI2::SkinAPI::name(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(config_skin);
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::SkinAPI::font(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BSTR;
		V_BSTR(pvarResult) = SysAllocString(GetFontNameW());
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::SkinAPI::fontsize(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, unsigned int FAR *puArgErr)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_I4;
		V_I4(pvarResult) = GetFontSize();
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

	return E_FAIL;		
}

#undef CHECK_ID
#define CHECK_ID(str, id) 		case id: return str(wFlags, pdispparams, pvarResult, puArgErr);
HRESULT JSAPI2::SkinAPI::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
		DISP_TABLE
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP JSAPI2::SkinAPI::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::SkinAPI::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::SkinAPI::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}
