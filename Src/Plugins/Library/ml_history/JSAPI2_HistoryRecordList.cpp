#include "JSAPI2_HistoryRecordList.h"
#include "JSAPI2_HistoryRecord.h"
#include "ml_history.h"
#include "../Winamp/JSAPI.h"
#include <strsafe.h>

JSAPI2::HistoryRecordList::HistoryRecordList(historyRecordList *_record)
{
	recordList = _record;
	refCount = 1;
}

JSAPI2::HistoryRecordList::~HistoryRecordList()
{
	freeRecentRecordList(recordList);
}

enum
{
	OBJ_ARRAY_DISP_LENGTH,
	OBJ_ARRAY_NUM_DISP,
};

HRESULT JSAPI2::HistoryRecordList::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
{
	bool unknowns = false;
	for (unsigned int i = 0;i != cNames;i++)
	{
		if (GetDispID(rgszNames[i], fdexNameCaseSensitive, &rgdispid[i]) == DISPID_UNKNOWN)
			unknowns=true;
	}
	if (unknowns)
		return DISP_E_UNKNOWNNAME;
	else
		return S_OK;
}

HRESULT JSAPI2::HistoryRecordList::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	return InvokeEx(dispid, lcid, wFlags, pdispparams, pvarResult, pexecinfo, 0);
}

HRESULT JSAPI2::HistoryRecordList::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecordList::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP JSAPI2::HistoryRecordList::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::HistoryRecordList::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}


ULONG JSAPI2::HistoryRecordList::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}

HRESULT JSAPI2::HistoryRecordList::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
	if (!_wcsicmp(bstrName, L"length"))
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

HRESULT JSAPI2::HistoryRecordList::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
{
	JSAPI_VERIFY_PARAMCOUNT(pdp, 0);

	switch(id)
	{
	case OBJ_ARRAY_DISP_LENGTH:
		{
			JSAPI_INIT_RESULT(pvarRes, VT_I4);
			JSAPI_SET_RESULT(pvarRes, lVal, recordList->Size);
			return S_OK;
		}
	default:
		{
			if (id < OBJ_ARRAY_NUM_DISP)
				return DISP_E_MEMBERNOTFOUND;

			int index = id - OBJ_ARRAY_NUM_DISP;
			if (index>=recordList->Size)
				return DISP_E_MEMBERNOTFOUND;

			JSAPI_INIT_RESULT(pvarRes, VT_DISPATCH);
			JSAPI_SET_RESULT(pvarRes, pdispVal, new JSAPI2::HistoryRecord(this, &recordList->Items[index]));
			return S_OK;
		}
	}

}

HRESULT JSAPI2::HistoryRecordList::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecordList::DeleteMemberByDispID(DISPID id)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecordList::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecordList::GetMemberName(DISPID id, BSTR *pbstrName)
{
	if (id >= OBJ_ARRAY_NUM_DISP)
	{
		wchar_t temp[64];
		StringCbPrintfW(temp, sizeof(temp), L"%d", id-OBJ_ARRAY_NUM_DISP);
		*pbstrName = SysAllocString(temp);
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecordList::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
	if (grfdex == fdexEnumDefault)
	{
		if (id == DISPID_UNKNOWN)
		{
			if (recordList->Size == 0)
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
			int index = (id - OBJ_ARRAY_NUM_DISP) + 1;
			if (index >= recordList->Size)
			{
				return S_FALSE;
			}
			else
			{
				*pid = OBJ_ARRAY_NUM_DISP + index;
				return S_OK;
			}

		}
	}

	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecordList::GetNameSpaceParent(IUnknown **ppunk)
{
	return E_NOTIMPL;
}