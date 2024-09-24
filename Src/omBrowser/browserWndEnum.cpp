#include "main.h"
#include "./browserWndEnum.h"
#include "./browserWndRecord.h"
#include "./browserView.h"
#include "./browserPopup.h"
#include "./ifc_omservice.h"


OmBrowserWndEnumerator::OmBrowserWndEnumerator(const GUID *windowTypeFilter, const UINT *serviceIdFilter, OmBrowserWndRecord * const *windowList, size_t windowListSize)
	: ref(1), index(0), list(NULL), size(0), filterId(FALSE), filterType(FALSE)
{
	if (NULL != serviceIdFilter)
	{
		serviceId = *serviceIdFilter;
		filterId = TRUE;
	}
	else 
		serviceId = 0;

	if (NULL != windowTypeFilter)
	{
		windowType = *windowTypeFilter;
		filterType = TRUE;
	}
	else
		windowType = GUID_NULL;

	if (NULL != windowList && 0 != windowListSize)
	{
		list = (OmBrowserWndRecord**)calloc(windowListSize, sizeof(OmBrowserWndRecord*));
		if (NULL != list)
		{
			for (size_t i = 0; i < windowListSize; i++)
			{
				list[i] = windowList[i];
				list[i]->AddRef();
			}
			size = windowListSize;
		}
	}

}

OmBrowserWndEnumerator::~OmBrowserWndEnumerator()
{
	if (NULL != list)
	{
		for (size_t i = 0; i < size; i++)
		{
			list[i]->Release();
		}
		free(list);
	}
}

HRESULT OmBrowserWndEnumerator::CreateInstance(const GUID *windowTypeFilter, const UINT *serviceIdFilter, OmBrowserWndRecord * const* windowList, size_t windowListSize, OmBrowserWndEnumerator **instance)
{
	if (NULL == instance) return E_POINTER;
	
	*instance = new OmBrowserWndEnumerator(windowTypeFilter, serviceIdFilter, windowList, windowListSize);
	if (NULL == *instance) return E_OUTOFMEMORY;
	
	return S_OK;
}

size_t OmBrowserWndEnumerator::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t OmBrowserWndEnumerator::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int OmBrowserWndEnumerator::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_OmBrowserWindowEnumerator))
		*object = static_cast<ifc_ombrowserwndenum*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT OmBrowserWndEnumerator::Next(ULONG listSize, HWND *elementList, ULONG *elementCount)
{
	if (NULL == elementList || 0 == listSize) return E_INVALIDARG;
	if (index >= size)
	{
		if (NULL != elementCount) *elementCount = 0;
		return S_FALSE;
	}

	ULONG count = 0;

	for (;index < size && count < listSize; index++)
	{
		OmBrowserWndRecord *r = list[index];
		
		if (FALSE != filterType)
		{
			if (S_OK != r->IsEqualType(&windowType))
				continue;
		}

		if (FALSE != filterId)
		{
			BOOL passOk = FALSE;
			if (S_OK == r->IsEqualType(&WTID_BrowserView) || 
				S_OK == r->IsEqualType(&WTID_BrowserPopup))
			{
				ifc_omservice *service;
				if (FALSE != SendMessage(r->GetHwnd(), NBCM_GETSERVICE, 0, (LPARAM)&service) && NULL != service)
				{
					if (serviceId == service->GetId())
						passOk = TRUE;
					
					service->Release();
				}
			}

			if (FALSE == passOk)
				continue;
		}

		elementList[count] = r->GetHwnd();
		count++;

	}

	if (NULL != elementCount) *elementCount = count;

	return (count == listSize) ? S_OK : S_FALSE;
}

HRESULT OmBrowserWndEnumerator::Reset(void)
{
	index = 0;
	return S_OK;
}

HRESULT OmBrowserWndEnumerator::Skip(ULONG elementCount)
{
	index += elementCount;
	if (index >= size)
	{
		index = (size - 1);
		return S_FALSE;
	}
	return S_OK;
}

#define CBCLASS OmBrowserWndEnumerator
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_NEXT, Next)
CB(API_RESET, Reset)
CB(API_SKIP, Skip)
END_DISPATCH;
#undef CBCLASS