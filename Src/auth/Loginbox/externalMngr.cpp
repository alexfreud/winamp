#include "./externalMngr.h"
#include "./common.h"

#include "../../winamp/jsapi.h"

ExternalManager::ExternalManager()
	: ref(1), lastDispId(0)
{
	InitializeCriticalSection(&lock);
}

ExternalManager::~ExternalManager()
{
	EnterCriticalSection(&lock);
	
	size_t index= list.size();
	while(index--)
	{
		if (NULL != list[index].object)
			list[index].object->Release();
	}

	LeaveCriticalSection(&lock);

	DeleteCriticalSection(&lock);
}

HRESULT ExternalManager::CreateInstance(ExternalManager **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new ExternalManager();
	if (NULL == instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

STDMETHODIMP_(ULONG) ExternalManager::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) ExternalManager::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP ExternalManager::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (NULL == ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IDispatchEx))
		*ppvObject = static_cast<IDispatchEx*>(this);
	else if (IsEqualIID(riid, IID_IDispatch))
		*ppvObject = static_cast<IDispatch*>(this);
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = static_cast<IUnknown*>(this);
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *ppvObject)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

STDMETHODIMP ExternalManager::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	EnterCriticalSection(&lock);
	UINT unknowns = 0;
	size_t count = list.size();
	for (UINT i = 0; i != cNames ; i++)
	{
		rgdispid[i] = DISPID_UNKNOWN;
		for (size_t j =0; j < count; j++)
		{
			if (CSTR_EQUAL == CompareString(lcid, 0, rgszNames[i], -1, list[j].name, -1)) 
			{
				if (NULL != list[j].object)
					rgdispid[i] = list[j].id; 
				else
					unknowns++;
				break;
			}
			if (DISPID_UNKNOWN == rgdispid[i]) 
				unknowns++;
		}
	}
	
	LeaveCriticalSection(&lock);

	return (0 != unknowns) ? DISP_E_UNKNOWNNAME : S_OK;
}

STDMETHODIMP ExternalManager::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP ExternalManager::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP ExternalManager::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	return InvokeEx(dispid, lcid, wFlags, pdispparams, pvarResult, pexecinfo, 0);
}

STDMETHODIMP ExternalManager::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
	if (NULL == pid) return E_POINTER;
	*pid = DISPID_UNKNOWN;

	HRESULT hr = DISP_E_UNKNOWNNAME;

	if (NULL != bstrName && L'\0' != *bstrName)
	{	
		UINT compareFlags = 0;
		if (0 != (fdexNameCaseInsensitive & grfdex))
			compareFlags |= NORM_IGNORECASE;

		EnterCriticalSection(&lock);
		
		size_t index = list.size();
		while(index--)
		{
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, compareFlags, bstrName, -1, list[index].name, -1)) 
			{
				if (NULL != list[index].object)
				{
					*pid = list[index].id; 
					hr = S_OK;
				}
				break;
			}
		}
		
		LeaveCriticalSection(&lock);
	}

	return hr;
}

STDMETHODIMP ExternalManager::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
{
	if (DISPATCH_METHOD == wFlags || DISPATCH_PROPERTYGET == wFlags || DISPATCH_CONSTRUCT == wFlags)
		JSAPI_INIT_RESULT(pvarRes, VT_DISPATCH);

	HRESULT hr(DISP_E_MEMBERNOTFOUND);

	EnterCriticalSection(&lock);
	
	size_t index = list.size();
	while(index--)
	{
		if (id == list[index].id) 
		{
			if (NULL != list[index].object)
			{
				if (NULL != pvarRes && 
					(DISPATCH_METHOD == wFlags || DISPATCH_PROPERTYGET == wFlags || DISPATCH_CONSTRUCT == wFlags))
				{
					list[index].object->AddRef();
					pvarRes->pdispVal = list[index].object;
				}
				hr = S_OK;
			}
			break;
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

STDMETHODIMP ExternalManager::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
	HRESULT hr = S_FALSE;

	if (NULL != bstrName && L'\0' != *bstrName)
	{	
		UINT compareFlags = 0;
		if (0 != (fdexNameCaseInsensitive & grfdex))
			compareFlags |= NORM_IGNORECASE;

		EnterCriticalSection(&lock);
		
		size_t index = list.size();
		while(index--)
		{
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, compareFlags, bstrName, -1, list[index].name, -1)) 
			{
				if (NULL != list[index].object)
				{
					list[index].object->Release();
					list[index].object = NULL;
					hr = S_OK;
				}
				break;
			}
		}
		
		LeaveCriticalSection(&lock);
	}

	return hr;
}

STDMETHODIMP ExternalManager::DeleteMemberByDispID(DISPID id)
{
	HRESULT hr = S_FALSE;
	
	EnterCriticalSection(&lock);
	
	size_t index = list.size();
	while(index--)
	{
		if (id == list[index].id) 
		{
			if (NULL != list[index].object)
			{
				list[index].object->Release();
				list[index].object = NULL;
				hr = S_OK;
			}
			break;
		}
	}
	
	LeaveCriticalSection(&lock);
	return hr;
}

STDMETHODIMP ExternalManager::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
	HRESULT hr(DISP_E_UNKNOWNNAME);

	EnterCriticalSection(&lock);
	
	size_t index = list.size();
	while(index--)
	{
		if (id == list[index].id) 
		{
			if (NULL != list[index].object)
			{
				if (NULL != pgrfdex)
				{
					*pgrfdex = 0;
					if (0 != (grfdexPropCanAll & grfdexFetch))
						*pgrfdex |= (fdexPropCanGet | fdexPropCanCall);
					if (0 != (grfdexPropCannotAll & grfdexFetch))
						*pgrfdex |= (fdexPropCannotPut | fdexPropCanPutRef | fdexPropCannotConstruct | fdexPropCannotSourceEvents);
					if (0 != (grfdexPropExtraAll & grfdexFetch))
						*pgrfdex |= (fdexPropNoSideEffects | fdexPropNoSideEffects);
				}
				hr = S_OK;
			}
			break;
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

STDMETHODIMP ExternalManager::GetMemberName(DISPID id, BSTR *pbstrName)
{
	HRESULT hr(DISP_E_UNKNOWNNAME);

	EnterCriticalSection(&lock);
	
	size_t index = list.size();
	while(index--)
	{
		if (id == list[index].id) 
		{
			if (NULL != list[index].object)
			{
				if (NULL != pbstrName)
				{
					*pbstrName = SysAllocString(list[index].name);
					hr = S_OK;
				}
			}
			break;
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

STDMETHODIMP ExternalManager::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
	HRESULT hr(S_FALSE);
	if (NULL == pid) return S_FALSE;
	*pid = DISPID_UNKNOWN;

	EnterCriticalSection(&lock);
	
	size_t count = list.size();
	if (DISPID_STARTENUM == id)
	{
		if (count > 0) 
		{
			*pid = list[0].id;
			hr = S_OK;
		}
	}
	else
	{
		for(size_t i = 0; i < count; i++)
		{
			if (id == list[i].id) 
			{
				i++;
				if (i < count)
				{
					*pid = list[i].id;
					hr = S_OK;
				}
				break;
			}
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}

STDMETHODIMP ExternalManager::GetNameSpaceParent(IUnknown **ppunk)
{
	return E_NOTIMPL;
}

HRESULT ExternalManager::AddDispatch(LPCWSTR pszName, IDispatch *pDispatch, DISPID *pid)
{
	if (NULL != pid)
		*pid = DISPID_UNKNOWN;

	if (NULL == pszName || L'\0' == pszName || NULL == pDispatch)
		return E_INVALIDARG;

	HRESULT hr;
	
	EnterCriticalSection(&lock);
	
	size_t index = list.size();
	while(index--)
	{
		if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, pszName, -1, list[index].name, -1))
		{
			if (NULL != list[index].object)
			{
				hr = E_FAIL;
			}
			else
			{
				list[index].object = pDispatch;
				pDispatch->AddRef();
				*pid = list[index].id;
				hr = S_OK;
			}
			break;
		}
	}

	if ((size_t)-1 == index)
	{
		DispatchRecord r;
		r.name = LoginBox_CopyString(pszName);
		if (NULL == r.name)
			hr = E_OUTOFMEMORY;
		else
		{
			r.id = ++lastDispId;
			r.object = pDispatch;
			pDispatch->AddRef();
			list.push_back(r);
			*pid = r.id;
			hr = S_OK;
		}
	}

	LeaveCriticalSection(&lock);
	return hr;
}
