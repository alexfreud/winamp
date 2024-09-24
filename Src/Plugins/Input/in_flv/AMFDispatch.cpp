#include "AMFDispatch.h"

AMFDispatch::AMFDispatch(AMFMixedArray *array)
{
	object=array;
	if (object)
		object->AddRef();
	refCount=1;
}

AMFDispatch::~AMFDispatch()
{
	if (object)
		object->Release();
}

STDMETHODIMP AMFDispatch::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG AMFDispatch::AddRef(void)
{
	return InterlockedIncrement((volatile LONG *)&refCount);
}

ULONG AMFDispatch::Release(void)
{
	ULONG count = InterlockedDecrement((volatile LONG *)&refCount);
	if (count == 0)
		delete this;
	return count;
}

enum
{
	DISP_AMF_DEBUGPRINT,
	DISP_AMF_MAX,
};

#define CHECK_ID(str, id) 		if (wcscmp(rgszNames[i], L##str) == 0)	{		rgdispid[i] = id; continue; }
HRESULT AMFDispatch::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		CHECK_ID("DebugPrint", DISP_AMF_DEBUGPRINT)
		if (object)
		{
			//size_t index = object->array.getPosition(rgszNames[i]);
			size_t index = 0;
			for (auto it = object->array.begin(); it != object->array.end(); it++, index++)
			{
				if (wcscmp(it->first.c_str(), rgszNames[i]) == 0)
				{
					break;
				}
			}

			if (index != object->array.size())
			{
				rgdispid[i] = (DISPID)index + DISP_AMF_MAX;
				continue;
			}
		}

		rgdispid[i] = DISPID_UNKNOWN;
		unknowns = true;

	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT AMFDispatch::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT AMFDispatch::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

static void AMFType_To_Variant(AMFType *obj, VARIANT *pvarResult)
{
	VariantInit(pvarResult);
	switch(obj->type)
	{
	case AMFType::TYPE_DOUBLE: // double
		{
			AMFDouble *cast_obj = static_cast<AMFDouble *>(obj);
			V_VT(pvarResult) = VT_R8;
			V_R8(pvarResult) = cast_obj->val;
		}
		break;
	case AMFType::TYPE_BOOL: // bool
		{
			AMFBoolean *cast_obj = static_cast<AMFBoolean *>(obj);
			V_VT(pvarResult) = VT_BOOL;
			V_BOOL(pvarResult) = cast_obj->boolean;
		}
		break;
	case AMFType::TYPE_MOVIE: // movie (basically just a URL)
	case AMFType::TYPE_STRING: // string
		{
			AMFString *cast_obj = static_cast<AMFString *>(obj);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = SysAllocString(cast_obj->str);
		}
		break;
	case AMFType::TYPE_LONG_STRING: // string
		{
			AMFLongString *cast_obj = static_cast<AMFLongString *>(obj);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = SysAllocString(cast_obj->str);
		}
		break;
	case AMFType::TYPE_MIXEDARRAY:
		{
			AMFMixedArray *cast_obj = static_cast<AMFMixedArray *>(obj);
			V_VT(pvarResult) = VT_DISPATCH;
			V_DISPATCH(pvarResult) = new AMFDispatch(cast_obj);
		}
		break;		
	case AMFType::TYPE_DATE:
		{
			AMFTime *cast_obj = static_cast<AMFTime *>(obj);
			V_VT(pvarResult) = VT_DATE;
			V_DATE(pvarResult) = cast_obj->val;
		}
		break;		
	case AMFType::TYPE_ARRAY:
		{
			AMFArray *cast_obj = static_cast<AMFArray *>(obj);
			SAFEARRAYBOUND rgsabound[1];
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = (ULONG)cast_obj->array.size();
			SAFEARRAY *psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
			VARIANT **data;
			SafeArrayAccessData(psa, (void **)&data);
			for (size_t i=0;i!=cast_obj->array.size();i++)
			{
				AMFType_To_Variant(cast_obj->array[i], data[i]);
			}
			SafeArrayUnaccessData(psa);
			V_VT(pvarResult) = VT_ARRAY;
			V_ARRAY(pvarResult) = psa;
		}
		break;
	}
}

HRESULT AMFDispatch::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	if (pvarResult)
		VariantInit(pvarResult);

	switch(dispid)
	{
	case DISP_AMF_DEBUGPRINT:
		{
			wchar_t debugstring[4096]=L"";
			wchar_t *str = debugstring;
			size_t len = 4096;
			object->DebugPrint(1, str, len);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = SysAllocString(debugstring);
		}
		return S_OK;
	}
	size_t index = dispid - DISP_AMF_MAX;
	if (index >= object->array.size())
		return DISP_E_MEMBERNOTFOUND;

	//AMFType *obj = object->array.at(index).second;
	AMFType* obj = 0;
	auto it = object->array.begin();
	while (index--)
	{
		it++;
	}
	if (it != object->array.end())
	{
		obj = it->second;
	}

	if (!obj)
		return S_OK;

	switch(obj->type)
	{
	case AMFType::TYPE_DOUBLE: 
	case AMFType::TYPE_BOOL:
	case AMFType::TYPE_STRING:
	case AMFType::TYPE_MIXEDARRAY:
		case AMFType::TYPE_ARRAY:
		AMFType_To_Variant(obj, pvarResult);
		return S_OK;

	case AMFType::TYPE_OBJECT: // object
		// TODO
		return DISP_E_TYPEMISMATCH;
	case AMFType::TYPE_NULL: // null
		return S_OK;
	case AMFType::TYPE_REFERENCE: // reference
		return DISP_E_TYPEMISMATCH;
	case AMFType::TYPE_TERMINATOR:
		// TODO?
		return DISP_E_TYPEMISMATCH;
	case AMFType::TYPE_DATE: // date
		return DISP_E_TYPEMISMATCH;
	case AMFType::TYPE_LONG_STRING: // long string
		return DISP_E_TYPEMISMATCH;
	case AMFType::TYPE_XML: // XML
		return DISP_E_TYPEMISMATCH;
	default:
		return DISP_E_TYPEMISMATCH;

	}
	return S_OK;
}
