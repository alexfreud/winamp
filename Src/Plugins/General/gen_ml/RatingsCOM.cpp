#include "main.h"
#include "RatingsCOM.h"
#include "config.h"
extern C_Config *g_config;
enum
{
    DISP_GETRATING = 0,
    DISP_ISEVERYONEALLOWED,
    DISP_ISTEENALLOWED,
    DISP_ISADULTALLOWED,
    DISP_ISXRATEDALLOWED,
    DISP_ISNOTRATEDALLOWED,

};

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT RatingsCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("GetRating", DISP_GETRATING);
		CHECK_ID("IsEveryoneAllowed", DISP_ISEVERYONEALLOWED);
		CHECK_ID("IsTeenAllowed", DISP_ISTEENALLOWED);
		CHECK_ID("IsAdultAllowed", DISP_ISADULTALLOWED);
		CHECK_ID("IsXRatedAllowed", DISP_ISXRATEDALLOWED);
		CHECK_ID("IsNotRatedAllowed", DISP_ISNOTRATEDALLOWED);

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT RatingsCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT RatingsCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}


HRESULT RatingsCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	int rating = g_config->ReadInt(L"tvrating", 7);
	switch (dispid)
	{
		case DISP_GETRATING:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_I4;
			V_I4(pvarResult) = rating;
			return S_OK;

		case DISP_ISEVERYONEALLOWED:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = (rating & 1) || (rating & 2);
			return S_OK;
		case DISP_ISTEENALLOWED:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = rating & 4;
			return S_OK;
		case DISP_ISADULTALLOWED:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = rating & 8;
			return S_OK;
		case DISP_ISXRATEDALLOWED:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = (rating & 16) || (rating & 32);
			return S_OK;
		case DISP_ISNOTRATEDALLOWED:
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = rating & 64 ;
			return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP RatingsCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG RatingsCOM::AddRef(void)
{
	return 0;
}


ULONG RatingsCOM::Release(void)
{
	return 0;
}
