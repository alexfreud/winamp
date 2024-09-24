#include "main.h"
#include "./browserWndRecord.h"

OmBrowserWndRecord::OmBrowserWndRecord(HWND hwnd, const GUID *type) 
	: ref(1)
{
	this->hwnd = hwnd;
	this->type = (NULL != type) ? *type : GUID_NULL;
}

OmBrowserWndRecord::~OmBrowserWndRecord()
{

}

HRESULT OmBrowserWndRecord::CreateInstance(HWND hwnd, const GUID *type, OmBrowserWndRecord **instance)
{
	if (NULL == instance) return E_POINTER;
	if (NULL == hwnd)
	{
		*instance = NULL;
		return E_INVALIDARG;
	}
	*instance = new OmBrowserWndRecord(hwnd, type);
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

ULONG OmBrowserWndRecord::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG OmBrowserWndRecord::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HWND OmBrowserWndRecord::GetHwnd()
{
	return hwnd;
}

HRESULT OmBrowserWndRecord::GetType(GUID *windowType)
{
	if (NULL == windowType) return E_POINTER;
	*windowType = type;
	return S_OK;
}

HRESULT OmBrowserWndRecord::IsEqualType(const GUID *windowType)
{
	if (NULL == windowType)
	{
		return (FALSE != IsEqualGUID(GUID_NULL, type)) ? S_OK : S_FALSE;
	}

	return (FALSE != IsEqualGUID(*windowType, type)) ? S_OK : S_FALSE;
}

