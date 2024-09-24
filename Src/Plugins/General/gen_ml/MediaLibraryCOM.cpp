#include "main.h"
#include "MediaLibraryCOM.h"
#include "OnlineMediaCOM.h"


extern "C" extern winampGeneralPurposePlugin plugin;

extern prefsDlgRecW myPrefsItem;

OnlineMediaCOM onlineMediaCOM;
enum
{
	DISP_ONLINEMEDIA = 312,
  DISP_PREFERENCES,
	DISP_HIDDEN,

};

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT MediaLibraryCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("OnlineMedia", DISP_ONLINEMEDIA)
		CHECK_ID("ShowPreferences", DISP_PREFERENCES);
		CHECK_ID("hidden", DISP_HIDDEN);

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT MediaLibraryCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT MediaLibraryCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}


HRESULT MediaLibraryCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISP_HIDDEN:
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = !IsVisible();
		return S_OK;
	case DISP_ONLINEMEDIA:
		VariantInit(pvarResult);
		V_VT(pvarResult) = VT_DISPATCH;
		V_DISPATCH(pvarResult) = &onlineMediaCOM;
		return S_OK;
	case DISP_PREFERENCES:
		SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&myPrefsItem, IPC_OPENPREFSTOPAGE);
		return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP MediaLibraryCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG MediaLibraryCOM::AddRef(void)
{
	return 0;
}


ULONG MediaLibraryCOM::Release(void)
{
	return 0;
}
