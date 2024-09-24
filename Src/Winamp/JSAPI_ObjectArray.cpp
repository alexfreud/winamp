#include "JSAPI_ObjectArray.h"
#include "JSAPI.h"
#include <strsafe.h>

JSAPI::ObjectArray::ObjectArray()
{
	refCount = 1;
}

JSAPI::ObjectArray::~ObjectArray()
{
	for ( IDispatch *l_object : objects )
		l_object->Release();
}

enum
{
	OBJ_ARRAY_DISP_LENGTH,
	OBJ_ARRAY_NUM_DISP,
};

HRESULT JSAPI::ObjectArray::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		rgdispid[i] = DISPID_UNKNOWN;
		if (!wcscmp(rgszNames[i], L"length"))
			rgdispid[i]=OBJ_ARRAY_DISP_LENGTH;
		else
		{
			if (rgszNames[i][0] >= L'0' && rgszNames[i][0] <= L'9')
				rgdispid[i]=_wtoi(rgszNames[i]) + OBJ_ARRAY_NUM_DISP;
			else
				unknowns=true;
		}

	}

	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI::ObjectArray::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);

	switch(dispid)
	{
	case OBJ_ARRAY_DISP_LENGTH:
		{
			JSAPI_INIT_RESULT(pvarResult, VT_I4);
			JSAPI_SET_RESULT(pvarResult, lVal, (LONG)objects.size());
			return S_OK;
		}
	default:
		{
			if (dispid < OBJ_ARRAY_NUM_DISP)
				return DISP_E_MEMBERNOTFOUND;

			size_t index = (size_t)dispid - OBJ_ARRAY_NUM_DISP;
			if (index>=objects.size())
				return DISP_E_MEMBERNOTFOUND;

			objects[index]->AddRef();
			JSAPI_INIT_RESULT(pvarResult, VT_DISPATCH);
			JSAPI_SET_RESULT(pvarResult, pdispVal, objects[index]);
			return S_OK;
		}
	}
}

HRESULT JSAPI::ObjectArray::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::ObjectArray::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP JSAPI::ObjectArray::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = (IDispatch *)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = this;
	else if (IsEqualIID(riid, IID_IDispatchEx))
		*ppvObject = (IDispatchEx *)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG JSAPI::ObjectArray::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI::ObjectArray::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}

void JSAPI::ObjectArray::AddObject(IDispatch *obj)
{
	obj->AddRef();
	objects.push_back(obj);
}


HRESULT JSAPI::ObjectArray::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
	if (!wcscmp(bstrName, L"length"))
		*pid=OBJ_ARRAY_DISP_LENGTH;
	else
	{
		if (bstrName[0] >= L'0' && bstrName[0] <= L'9')
			*pid=_wtoi(bstrName) + OBJ_ARRAY_NUM_DISP;
		else
			return DISP_E_MEMBERNOTFOUND;

	}
	return S_OK;
}

HRESULT JSAPI::ObjectArray::InvokeEx( 
																		 /* [in] */ DISPID id,
																		 /* [in] */ LCID lcid,
																		 /* [in] */ WORD wFlags,
																		 /* [in] */ DISPPARAMS *pdp,
																		 /* [out] */ VARIANT *pvarRes,
																		 /* [out] */ EXCEPINFO *pei,
																		 /* [unique][in] */ IServiceProvider *pspCaller)
{
	JSAPI_VERIFY_PARAMCOUNT(pdp, 0);

	switch(id)
	{
	case OBJ_ARRAY_DISP_LENGTH:
		{
			JSAPI_INIT_RESULT(pvarRes, VT_I4);
			JSAPI_SET_RESULT(pvarRes, lVal, (LONG)objects.size());
			return S_OK;
		}
	default:
		{
			if (id < OBJ_ARRAY_NUM_DISP)
				return DISP_E_MEMBERNOTFOUND;

			size_t index = (size_t)id - OBJ_ARRAY_NUM_DISP;
			if (index>=objects.size())
				return DISP_E_MEMBERNOTFOUND;

			objects[index]->AddRef();
			JSAPI_INIT_RESULT(pvarRes, VT_DISPATCH);
			JSAPI_SET_RESULT(pvarRes, pdispVal, objects[index]);
			return S_OK;
		}
	}

}

HRESULT JSAPI::ObjectArray::DeleteMemberByName( 
	/* [in] */ BSTR bstrName,
	/* [in] */ DWORD grfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::ObjectArray::DeleteMemberByDispID( 
	/* [in] */ DISPID id)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::ObjectArray::GetMemberProperties( 
	/* [in] */ DISPID id,
	/* [in] */ DWORD grfdexFetch,
	/* [out] */ DWORD *pgrfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::ObjectArray::GetMemberName( 
	/* [in] */ DISPID id,
	/* [out] */ BSTR *pbstrName)
{
	if (id >= OBJ_ARRAY_NUM_DISP)
	{
		wchar_t temp[64] = {0};
		StringCbPrintfW(temp, sizeof(temp), L"%d", id-OBJ_ARRAY_NUM_DISP);
		*pbstrName = SysAllocString(temp);
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT JSAPI::ObjectArray::GetNextDispID( 
	/* [in] */ DWORD grfdex,
	/* [in] */ DISPID id,
	/* [out] */ DISPID *pid)
{
	if (grfdex == fdexEnumDefault)
	{
		if (id == DISPID_UNKNOWN)
		{
			if (objects.empty())
				return S_FALSE;
			else
			{
				*pid = OBJ_ARRAY_NUM_DISP;
				return S_OK;
			}
		}
		else if (id < OBJ_ARRAY_NUM_DISP)
		{
			return S_FALSE;
		}
		else
		{
			size_t index = (id - OBJ_ARRAY_NUM_DISP) + 1;
			if (index >= objects.size())
			{
				return S_FALSE;
			}
			else
			{
				*pid = OBJ_ARRAY_NUM_DISP + (LONG)index;
				return S_OK;
			}

		}
	}

	return E_NOTIMPL;
}

HRESULT JSAPI::ObjectArray::GetNameSpaceParent( 
	/* [out] */ IUnknown **ppunk)
{
	return E_NOTIMPL;
}