#include "JSAPI2_HistoryRecord.h"
#include "../Winamp/JSAPI.h"

#define DISP_TABLE \
	CHECK_ID(filename)\
	CHECK_ID(title)\
	CHECK_ID(length)\
	CHECK_ID(playcount)\
	CHECK_ID(lastplay)\
	CHECK_ID(offset)\
	

#define CHECK_ID(str) JSAPI_DISP_ENUMIFY(str),
enum { 
	DISP_TABLE 
	DISP_TABLE_NUM_ENTRIES,
};

JSAPI2::HistoryRecord::HistoryRecord(IUnknown *_parent, const historyRecord *_record)
{
	parent = _parent;
	parent->AddRef();
	record = _record;
	refCount = 1;
}

JSAPI2::HistoryRecord::~HistoryRecord()
{
	if (parent)
		parent->Release();
}

HRESULT JSAPI2::HistoryRecord::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgdispid)
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

HRESULT JSAPI2::HistoryRecord::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult, EXCEPINFO FAR * pexecinfo, unsigned int FAR *puArgErr)
{
	return InvokeEx(dispid, lcid, wFlags, pdispparams, pvarResult, pexecinfo, 0);
}

HRESULT JSAPI2::HistoryRecord::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecord::GetTypeInfoCount(unsigned int FAR * pctinfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP JSAPI2::HistoryRecord::QueryInterface(REFIID riid, PVOID *ppvObject)
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

ULONG JSAPI2::HistoryRecord::AddRef(void)
{
	return InterlockedIncrement(&refCount);
}

ULONG JSAPI2::HistoryRecord::Release(void)
{
	LONG lRef = InterlockedDecrement(&refCount);
	if (lRef == 0) delete this;
	return lRef;
}

#undef CHECK_ID
#define CHECK_ID(str) 		if (wcscmp(bstrName, L## #str) == 0)	{		*pid = JSAPI_DISP_ENUMIFY(str); return S_OK; }
HRESULT JSAPI2::HistoryRecord::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
		DISP_TABLE 

	return DISP_E_MEMBERNOTFOUND;
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): return str(wFlags, pdp, pvarRes);
HRESULT JSAPI2::HistoryRecord::InvokeEx(DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		switch(id)
		{
			DISP_TABLE 
		}
	}

	return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::HistoryRecord::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecord::DeleteMemberByDispID(DISPID id)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecord::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
{
	return E_NOTIMPL;
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): *pbstrName = SysAllocString(L ## #str); return S_OK;
HRESULT JSAPI2::HistoryRecord::GetMemberName(DISPID id, BSTR *pbstrName)
{
	switch(id)
	{
		DISP_TABLE
	}
	
	return E_NOTIMPL;
}

#undef CHECK_ID
#define CHECK_ID(str) 		case JSAPI_DISP_ENUMIFY(str): *pid = JSAPI_DISP_ENUMIFY(str) + 1; break;
HRESULT JSAPI2::HistoryRecord::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
	if (grfdex == fdexEnumDefault)
	{
		switch(id)
		{
		case DISPID_UNKNOWN:
			*pid = 0;
			break;
			DISP_TABLE

		}
		if (*pid == DISP_TABLE_NUM_ENTRIES)
			return S_FALSE;
		else
			return S_OK;
	}

	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecord::GetNameSpaceParent(IUnknown **ppunk)
{
	return E_NOTIMPL;
}

HRESULT JSAPI2::HistoryRecord::filename(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
		JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(record->filename));
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}


HRESULT JSAPI2::HistoryRecord::title(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_BSTR);
		JSAPI_SET_RESULT(pvarResult, bstrVal, SysAllocString(record->title));
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::HistoryRecord::length(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_I4);
		JSAPI_SET_RESULT(pvarResult, lVal, record->length);
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::HistoryRecord::offset(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_I4);
		JSAPI_SET_RESULT(pvarResult, lVal, record->offset);
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::HistoryRecord::playcount(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_I4);
		JSAPI_SET_RESULT(pvarResult, lVal, record->playcnt);
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}

HRESULT JSAPI2::HistoryRecord::lastplay(WORD wFlags, DISPPARAMS FAR *pdispparams, VARIANT FAR *pvarResult)
{
	if (wFlags & DISPATCH_PROPERTYGET)
	{
		JSAPI_INIT_RESULT(pvarResult, VT_DATE);
		if (pvarResult)
		{
			__time64_t convertTime = record->lastplayed;
			SYSTEMTIME sysTime = {0};
			tm *newtime = _localtime64(&convertTime);

			sysTime.wYear = newtime->tm_year + 1900;
			sysTime.wMonth = newtime->tm_mon + 1;
			sysTime.wDayOfWeek = newtime->tm_wday;
			sysTime.wDay = newtime->tm_mday;
			sysTime.wHour = newtime->tm_hour;
			sysTime.wMinute = newtime->tm_min;
			sysTime.wSecond = newtime->tm_sec;

			SystemTimeToVariantTime(&sysTime, &pvarResult->date);
		}		
		return S_OK;
	}
	else
		return DISP_E_MEMBERNOTFOUND;
}
