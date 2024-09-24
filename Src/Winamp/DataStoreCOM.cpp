#include "DataStoreCOM.h"

enum
{
	DISP_DATASTORE_STORE,
	DISP_DATASTORE_RETRIEVE,
	DISP_DATASTORE_GENERATE_GUID,
};

#define CHECK_ID(str, id)\
	if (CSTR_EQUAL == CompareStringW(lcid, NORM_IGNORECASE, rgszNames[i], -1, L##str, -1))\
		{ rgdispid[i] = id; continue; }

HRESULT DataStoreCOM::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("Store", DISP_DATASTORE_STORE);
		CHECK_ID("Retrieve", DISP_DATASTORE_RETRIEVE);
		CHECK_ID("GenerateGUID", DISP_DATASTORE_GENERATE_GUID);

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT DataStoreCOM::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT DataStoreCOM::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

HRESULT DataStoreCOM::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	switch (dispid)
	{
	case DISP_DATASTORE_GENERATE_GUID:
		{
			GUID guid;
			UuidCreate(&guid);
			wchar_t *guid_string;
			UuidToStringW(&guid, (unsigned short **)&guid_string);
			BSTR tag = SysAllocString(guid_string);
			RpcStringFreeW((unsigned short **)&guid_string);
			VariantInit(pvarResult);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = tag;
			return S_OK;
		}
	case DISP_DATASTORE_STORE:
		if (pdispparams->cArgs == 2)
		{
			GUID guid;
			UuidFromStringW((unsigned short *)pdispparams->rgvarg[1].bstrVal, &guid);

			SAFEARRAY *bufferArray=pdispparams->rgvarg[0].parray;
			SAFEARRAY *store;
			HRESULT hr = SafeArrayCopy(bufferArray, &store);
			 dataStore[guid]=store;
			 return hr;
  
		}
		else
			return DISP_E_BADPARAMCOUNT;
	case DISP_DATASTORE_RETRIEVE:
		if (pdispparams->cArgs == 1)
		{
			GUID guid;
			UuidFromStringW((unsigned short *)pdispparams->rgvarg[0].bstrVal, &guid);

			SAFEARRAY *store = dataStore[guid];
			if (store)
			{
				SAFEARRAY *bufferArray;
				SafeArrayCopy(store, &bufferArray);
				VariantInit(pvarResult);
				V_VT(pvarResult) = VT_ARRAY|VT_UI1;
				V_ARRAY(pvarResult) = bufferArray;
				return S_OK;
			}
			else
				return E_FAIL;
			
		}
		else
			return DISP_E_BADPARAMCOUNT;
	}
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP DataStoreCOM::QueryInterface(REFIID riid, PVOID *ppvObject)
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


ULONG DataStoreCOM::AddRef(void)
{
	return 0;
}


ULONG DataStoreCOM::Release(void)
{
	return 0;
}
