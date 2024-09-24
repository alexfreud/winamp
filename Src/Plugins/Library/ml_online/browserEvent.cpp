#include "main.h"
#include "./browserEvent.h"
#include "./serviceHost.h"
#include "./serviceHelper.h"

#include <ifc_omservice.h>
#include <ifc_omserviceeventmngr.h>
#include <ifc_omservicecommand.h>

#include <browserView.h>
#include <browserPopup.h>

BrowserEvent::BrowserEvent()
	: ref(1)
{
}

BrowserEvent::~BrowserEvent()
{
}

HRESULT BrowserEvent::CreateInstance(BrowserEvent **instance)
{
	if (NULL == instance) return E_POINTER;
	*instance = new BrowserEvent();
	if (NULL == *instance) return E_OUTOFMEMORY;
	return S_OK;
}

size_t BrowserEvent::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t BrowserEvent::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int BrowserEvent::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) return E_POINTER;

	if (IsEqualIID(interface_guid, IFC_OmBrowserEvent))
		*object = static_cast<ifc_ombrowserevent*>(this);
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

void BrowserEvent::WindowCreate(HWND hwnd, const GUID *windowType)
{
	if (NULL != windowType)
	{
		if (IsEqualGUID(*windowType, WTID_BrowserView) || 
			IsEqualGUID(*windowType, WTID_BrowserPopup))
		{
			ifc_omservice *service;
			if (FALSE != BrowserControl_GetService(hwnd, &service))
			{
				UINT flags;
				if (SUCCEEDED(service->GetFlags(&flags))  && 
					0 == ((SVCF_SPECIAL | SVCF_VALIDATED | SVCF_VERSIONCHECK) & flags))
				{
					ServiceHelper_BeginVersionCheck(service);
				}
				service->Release();
			}
		}
	}
}

void BrowserEvent::WindowClose(HWND hwnd, const GUID *windowType)
{
}

#define CBCLASS BrowserEvent
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
VCB(API_WINDOWCREATE, WindowCreate)
VCB(API_WINDOWCLOSE, WindowClose)
END_DISPATCH;
#undef CBCLASS
