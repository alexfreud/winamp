#include "precomp__gen_ff.h"
#include "minibrowserCOM.h"

MinibrowserCOM::MinibrowserCOM(BrowserWnd* brw)
{
	this->brw = brw;
}

HRESULT __stdcall MinibrowserCOM::GetTypeInfoCount(unsigned int FAR*  pctinfo)
{
	return E_NOTIMPL;
}

HRESULT __stdcall MinibrowserCOM::GetTypeInfo(unsigned int  iTInfo, LCID  lcid, ITypeInfo FAR* FAR*  ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT __stdcall MinibrowserCOM::GetIDsOfNames( REFIID  riid, OLECHAR FAR* FAR*  rgszNames, unsigned int  cNames, LCID   lcid, DISPID FAR*  rgDispId)
{
	int notFound = false;
	for (unsigned int i = 0; i != cNames; i++)
	{
		if (!_wcsicmp(rgszNames[i], L"messageToMaki"))
		{
			rgDispId[i] = MINIBROWSERCOM_MAKI_MESSAGETOMAKI;
			continue;
		}
		notFound = true;
	}

	if (!notFound)
		return S_OK;

	return DISP_E_UNKNOWNNAME;
}

HRESULT __stdcall MinibrowserCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case MINIBROWSERCOM_MAKI_MESSAGETOMAKI:
	/* var ret = window.external.messageToMaki(str1, str2, int1, int2, int3) */
	/* keep in mind that the args are passed in reverse order! */

		/*if (wFlags != DISPATCH_METHOD)
			return DISP_E_MEMBERNOTFOUND;*/
		if (pdispparams->cArgs != 5)
			return DISP_E_BADPARAMCOUNT;

		const wchar_t * ret = this->brw->messageToMaki(pdispparams->rgvarg[4].bstrVal, pdispparams->rgvarg[3].bstrVal, pdispparams->rgvarg[2].iVal, pdispparams->rgvarg[1].iVal, pdispparams->rgvarg[0].iVal);

		// (mpdeimos) we need to check this here since in JS one can omit the return value. In this case we would get a NPE here.
		if (pvarResult != NULL)
		{
			BSTR returnValue = SysAllocString(ret);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = returnValue;
		}
		return S_OK;
	}

	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP MinibrowserCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG MinibrowserCOM::AddRef(void)
{
	return 0;
}

ULONG MinibrowserCOM::Release(void)
{
	return 0;
}
