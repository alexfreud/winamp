#include "main.h"
#include "./resource.h"
#include "./external.h"

ExternalDispatch::ExternalDispatch() 
	: ref(1)
{
}

ExternalDispatch::~ExternalDispatch()
{
}

HRESULT ExternalDispatch::CreateInstance(ExternalDispatch **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new ExternalDispatch();
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

LPCWSTR ExternalDispatch::GetName()
{
	return L"NowPlaying";
}


ULONG ExternalDispatch::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}


ULONG ExternalDispatch::Release(void)
{ 
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP ExternalDispatch::QueryInterface(REFIID riid, void **ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;

	if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = static_cast<IDispatch*>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = static_cast<IUnknown*>(this);
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

HRESULT ExternalDispatch::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		if (_wcsicmp(rgszNames[i], L"hidden") == 0)
			rgdispid[i] = DISPATCH_HIDDEN;
		else
		{
			rgdispid[i] = DISPID_UNKNOWN;
			unknowns = true;
		}
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT ExternalDispatch::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT ExternalDispatch::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT ExternalDispatch::Invoke(DISPID dispId, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch(dispId)
	{
	case DISPATCH_HIDDEN:
			if (NULL != pvarResult)
			{
				HWND hLibrary = Plugin_GetLibrary();
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_BOOL;
				V_BOOL(pvarResult) = (NULL == hLibrary || FALSE == SENDMLIPC(hLibrary, ML_IPC_IS_VISIBLE, 0));
			}
			return S_OK;
	}
	return DISP_E_MEMBERNOTFOUND;
}