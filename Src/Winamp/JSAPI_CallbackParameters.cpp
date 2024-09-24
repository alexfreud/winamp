#include "JSAPI_CallbackParameters.h"
#include "JSAPI.h"

JSAPI::CallbackParameters::CallbackParameters()
{
	refCount = 1;
}

JSAPI::CallbackParameters::~CallbackParameters()
{
	//for (size_t p=0;p!=params.size();p++)
	//{
	//	ParameterList::value_type &property = params.at(p);
	//	// some types need to be specifically destroyed or released
	//	switch(property.second.vt)
	//	{
	//	case VT_DISPATCH: // add a reference if it's an IDispatch
	//		property.second.pdispVal->Release();
	//		break;
	//	case VT_BSTR: // re-allocate
	//		SysFreeString(property.second.bstrVal);
	//		break;
	//	}
	//}
	for (auto &param : params)
	{
		// some types need to be specifically destroyed or released
		switch(param.second.vt)
		{
		case VT_DISPATCH: // add a reference if it's an IDispatch
			param.second.pdispVal->Release();
			break;
		case VT_BSTR: // re-allocate
			SysFreeString(param.second.bstrVal);
			break;
		}
	}
}

HRESULT JSAPI::CallbackParameters::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		rgdispid[i] = DISPID_UNKNOWN;
		const wchar_t *propertyName = rgszNames[i];

		bool found=false;
		//for (size_t p=0;p!=params.size();p++)
		size_t p = 0;
		for(auto it = params.begin(); it != params.end(); it++, p++)
		{
			//ParameterList::value_type &property = params.at(p);
			if (!wcscmp(it->first.c_str(), propertyName))
			{
				found=true;
				rgdispid[i] = (DISPID)p;
				break;
			}
		}
		if (!found)
			unknowns=true;
	}

	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI::CallbackParameters::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	JSAPI_VERIFY_PARAMCOUNT(pdispparams, 0);
	size_t index = (size_t)dispid;
	if (index>=params.size())
		return DISP_E_MEMBERNOTFOUND;
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		if (pvarResult)
		{
			//ParameterList::value_type &property = params.at(index);
			auto it = params.begin();
			while (index--)
			{
				it++;
			}

			*pvarResult = it->second;
			// do any type-specific allocations that are necessary
			switch(pvarResult->vt)
			{
			case VT_DISPATCH: // add a reference if it's an IDispatch
				pvarResult->pdispVal->AddRef();
				break;
			case VT_BSTR: // re-allocate
				pvarResult->bstrVal = SysAllocString(pvarResult->bstrVal);
				break;
			}
		}
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}


HRESULT JSAPI::CallbackParameters::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::CallbackParameters::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP JSAPI::CallbackParameters::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI::CallbackParameters::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI::CallbackParameters::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}

HRESULT JSAPI::CallbackParameters::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
	//for (size_t p=0;p!=params.size();p++)
	//{
	//	ParameterList::value_type &property = params.at(p);
	//	if (!wcscmp(property.first.c_str(), bstrName))
	//	{
	//		*pid= (DISPID)p;
	//		return S_OK;
	//	}
	//}
	size_t p = 0;
	for (auto it = params.begin(); it != params.end(); it++, p++)
	{
		if (!wcscmp(it->first.c_str(), bstrName))
		{
			*pid = (DISPID)p;
			return S_OK;
		}
	}

	return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI::CallbackParameters::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
{
	JSAPI_VERIFY_PARAMCOUNT(pdp, 0);
	size_t index = (size_t)id;
	if (index>=params.size())
		return DISP_E_MEMBERNOTFOUND;
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		if (pvarRes)
		{
			//ParameterList::value_type &property = params.at(index);
			auto it = params.begin();
			while (index--)
			{
				it++;
			}

			*pvarRes = it->second;
			// do any type-specific allocations that are necessary
			switch(pvarRes->vt)
			{
			case VT_DISPATCH: // add a reference if it's an IDispatch
				pvarRes->pdispVal->AddRef();
				break;
			case VT_BSTR: // re-allocate
				pvarRes->bstrVal = SysAllocString(pvarRes->bstrVal);
				break;
			}
		}
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;

}

HRESULT JSAPI::CallbackParameters::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::CallbackParameters::DeleteMemberByDispID(DISPID id)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::CallbackParameters::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI::CallbackParameters::GetMemberName(DISPID id, BSTR *pbstrName)
{
	if (id >= 0 && (size_t)id < params.size())
	{
		auto it = params.begin();
		while (id--)
		{
			it++;
		}
		*pbstrName = SysAllocString(it->first.c_str());
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT JSAPI::CallbackParameters::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
	if (grfdex == fdexEnumDefault)
	{
		if (id == DISPID_UNKNOWN)
		{
			if (params.size() == 0)
				return S_FALSE;
			else
			{
				*pid = 0;
				return S_OK;
			}
		}
		else
		{
			size_t index = id+1;
			if (index >= params.size())
			{
				return S_FALSE;
			}
			else
			{
				*pid = (DISPID)index;
				return S_OK;
			}

		}
	}

	return E_NOTIMPL;
}

HRESULT JSAPI::CallbackParameters::GetNameSpaceParent(IUnknown **ppunk)
{
	return E_NOTIMPL;
}

void JSAPI::CallbackParameters::AddProperty(const wchar_t *name, const VARIANT &property)
{
	params[name]=property;	
}

void JSAPI::CallbackParameters::AddString(const wchar_t *name, const wchar_t *value)
{
	VARIANT bstrVar;
	V_VT(&bstrVar) = VT_BSTR;
	V_BSTR(&bstrVar) = SysAllocString(value);
	AddProperty(name, bstrVar);	
}

void JSAPI::CallbackParameters::AddDispatch(const wchar_t *name, IDispatch *disp)
{
	VARIANT dispVar;
	V_VT(&dispVar) = VT_DISPATCH;
	V_DISPATCH(&dispVar) = disp;
	disp->AddRef();
	AddProperty(name, dispVar);
}

void JSAPI::CallbackParameters::AddLong(const wchar_t *name, LONG value)
{
	VARIANT i4Var;
	V_VT(&i4Var) = VT_I4;
	V_I4(&i4Var) = value;
	AddProperty(name, i4Var);
}

void JSAPI::CallbackParameters::AddBoolean(const wchar_t *name, bool value)
{
	VARIANT boolVar;
	V_VT(&boolVar) = VT_BOOL;
	V_BOOL(&boolVar) = value?VARIANT_TRUE:VARIANT_FALSE;
	AddProperty(name, boolVar);
}

size_t JSAPI::CallbackParameters::AddPropertyIndirect(const JSAPI::CallbackParameters::PropertyTemplate *entries, size_t count)
{
	if (NULL == entries) return 0;

	size_t inserted =  0;
	VARIANT val;

	for (size_t  i = 0; i < count; i++)
	{
		const PropertyTemplate *ppt = &entries[i];
		if (NULL == ppt->name) 
			continue;

		switch(ppt->type)
		{
			case typeBool:
				V_VT(&val) = VT_BOOL;
				V_BOOL(&val) = (FALSE != ((BOOL)ppt->value)) ? VARIANT_TRUE : VARIANT_FALSE;
				break;
			case typeString:
				V_VT(&val) = VT_BSTR;
				V_BSTR(&val) = SysAllocString((LPCWSTR)ppt->value);
				break;
			case typeLong:
				V_VT(&val) = VT_I4;
				V_I4(&val) = (ULONG)ppt->value;
				break;
			case typeDispatch:
				V_VT(&val) = VT_DISPATCH;
				V_DISPATCH(&val) = (IDispatch*)ppt->value;
				if (NULL != val.pdispVal)
					val.pdispVal->AddRef();
				break;
			default:
				continue;
				break;
		}
		AddProperty(ppt->name, val);
		inserted++;
	}

	return inserted;
}

/* ---------------------------------------------------------------------------- */
HRESULT JSAPI::InvokeEvent(JSAPI::CallbackParameters *parameters, IDispatch *invokee)
{
	unsigned int ret;
	DISPPARAMS params;
	VARIANTARG arguments[1];

	VariantInit(&arguments[0]);
	V_VT(&arguments[0]) = VT_DISPATCH;
	V_DISPATCH(&arguments[0]) = parameters;
	parameters->AddRef();

	params.cArgs = 1;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;
	params.rgvarg = arguments;
	
	HRESULT hr = invokee->Invoke(0, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, 0, 0, &ret);
	
	VariantClear(&arguments[0]);

	return hr;
}

