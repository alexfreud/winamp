/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author: Ben Allison benski@nullsoft.com
 ** Created:
 **/
#include "main.h"
#include "BrowserCOM.h"
#include "Browser.h"
#include "../nu/AutoChar.h"

//extern Browser *browser;
extern UpdateBrowser *updateBrowser;
enum
{
    DISP_BROWSER_SETSIZE = 777,
    DISP_BROWSER_SETTITLE,
		DISP_BROWSER_SETUPDATESIZE,
		DISP_BROWSER_HIDDEN,
};

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT BrowserCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		//CHECK_ID("SetSize", DISP_BROWSER_SETSIZE)
		CHECK_ID("SetUpdateSize", DISP_BROWSER_SETUPDATESIZE)
		//CHECK_ID("SetTitle", DISP_BROWSER_SETTITLE)
		//CHECK_ID("hidden", DISP_BROWSER_HIDDEN)

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT BrowserCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT BrowserCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}


HRESULT BrowserCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{

		case DISP_BROWSER_SETUPDATESIZE:
				if (pdispparams->cArgs == 2 && updateBrowser)
		{
			updateBrowser->Resized(pdispparams->rgvarg[1].lVal, pdispparams->rgvarg[0].lVal);
			return S_OK;
		}
		break;


	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP BrowserCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG BrowserCOM::AddRef(void)
{
	return 0;
}

ULONG BrowserCOM::Release(void)
{
	return 0;
}
