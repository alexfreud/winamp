#include "main.h"
#include "OnlineMediaCOM.h"
#include "RatingsCOM.h"
RatingsCOM ratingsCOM;
enum
{
	DISP_RATINGS = 0,
	
};

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT OnlineMediaCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("Ratings", DISP_RATINGS)

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT OnlineMediaCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT OnlineMediaCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}


HRESULT OnlineMediaCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISP_RATINGS:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_DISPATCH;
			V_DISPATCH(pvarResult) = &ratingsCOM;
			return S_OK;
	
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP OnlineMediaCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG OnlineMediaCOM::AddRef(void)
{
	return 0;
}


ULONG OnlineMediaCOM::Release(void)
{
	return 0;
}
